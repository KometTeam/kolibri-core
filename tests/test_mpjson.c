/* Мост msgpack ↔ JSON: обе стороны и round-trip. */

#include <string.h>

#include "kb_test.h"
#include "kolibri.h"
#include "kolibri_mpjson.h"
#include "kolibri_msgpack.h"

static unsigned char mp[4096];
static unsigned char mp2[4096];
static unsigned char scratch[4096];
static char json[4096];

void kb_tests_mpjson(void)
{
    KB_TEST("msgpack → JSON: скаляры и вложенность");
    {
        kb_mp_writer w;
        size_t len = 0;
        kb_mp_writer_init(&w, mp, sizeof mp);
        kb_mp_write_map(&w, 3);
        kb_mp_write_cstr(&w, "id");
        kb_mp_write_u64(&w, 42);
        kb_mp_write_cstr(&w, "ok");
        kb_mp_write_bool(&w, 1);
        kb_mp_write_cstr(&w, "tags");
        kb_mp_write_array(&w, 2);
        kb_mp_write_cstr(&w, "a");
        kb_mp_write_cstr(&w, "b");
        CHECK_EQ(kb_mp_writer_status(&w), KB_OK);

        CHECK_EQ(kb_mp_to_json(mp, kb_mp_writer_len(&w), 0, json, sizeof json,
                               &len),
                 KB_OK);
        CHECK_EQ(strcmp(json,
                        "{\"id\":42,\"ok\":true,\"tags\":[\"a\",\"b\"]}"),
                 0);
    }

    KB_TEST("msgpack → JSON: бинарник и целочисленный ключ");
    {
        kb_mp_writer w;
        static const unsigned char deadbeef[] = {0xDE, 0xAD, 0xBE, 0xEF};
        static const unsigned char dead[] = {0xDE, 0xAD};
        kb_mp_writer_init(&w, mp, sizeof mp);
        kb_mp_write_map(&w, 1);
        kb_mp_write_i64(&w, 7);
        kb_mp_write_bin(&w, dead, 2);
        CHECK_EQ(kb_mp_to_json(mp, kb_mp_writer_len(&w), 0, json, sizeof json,
                               NULL),
                 KB_OK);
        /* Ключ-число стянут в строку, бинарник — плоский base64. */
        CHECK_EQ(strcmp(json, "{\"7\":\"3q0=\"}"), 0);

        /* Плоский против tagged для 4-байтного бинарника. */
        kb_mp_writer_init(&w, mp, sizeof mp);
        kb_mp_write_bin(&w, deadbeef, 4);
        CHECK_EQ(kb_mp_to_json(mp, kb_mp_writer_len(&w), 0, json, sizeof json,
                               NULL),
                 KB_OK);
        CHECK_EQ(strcmp(json, "\"3q2+7w==\""), 0);
        CHECK_EQ(kb_mp_to_json(mp, kb_mp_writer_len(&w), 1, json, sizeof json,
                               NULL),
                 KB_OK);
        CHECK_EQ(strcmp(json, "{\"$bin\":\"3q2+7w==\"}"), 0);
    }

    KB_TEST("JSON → msgpack: карта и восстановление $bin");
    {
        static const char *src =
            "{\"id\":42,\"ok\":true,\"name\":\"x\",\"fp\":{\"$bin\":\"3q0=\"},"
            "\"list\":[1,2,3]}";
        size_t len = 0;
        kb_mp_value v;

        CHECK_EQ(kb_json_to_mp(src, strlen(src), scratch, sizeof scratch, mp,
                               sizeof mp, &len),
                 KB_OK);
        CHECK_EQ(kb_mp_map_get(mp, len, "id", &v), KB_OK);
        CHECK(v.type == KB_MP_UINT || v.type == KB_MP_INT);
        CHECK_EQ(kb_mp_map_get(mp, len, "ok", &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_BOOL);
        CHECK_EQ(v.boolean, 1);
        CHECK_EQ(kb_mp_map_get(mp, len, "fp", &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_BIN);
        CHECK_EQ(v.len, (size_t)2);
        CHECK_EQ(v.data[0], 0xDE);
        CHECK_EQ(v.data[1], 0xAD);
        CHECK_EQ(kb_mp_map_get(mp, len, "list", &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_ARRAY);
        CHECK_EQ(v.len, (size_t)3);
    }

    KB_TEST("JSON → msgpack: $bin снимает escaped slash");
    {
        static const char *src = "{\"fp\":{\"$bin\":\"A\\/8=\"}}";
        size_t len = 0;
        kb_mp_value v;

        CHECK_EQ(kb_json_to_mp(src, strlen(src), scratch, sizeof scratch, mp,
                               sizeof mp, &len),
                 KB_OK);
        CHECK_EQ(kb_mp_map_get(mp, len, "fp", &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_BIN);
        CHECK_EQ(v.len, (size_t)2);
        CHECK_EQ(v.data[0], 0x03);
        CHECK_EQ(v.data[1], 0xFF);
    }

    KB_TEST("JSON → msgpack: $ext восстанавливается");
    {
        static const char *src = "{\"$ext\":5,\"data\":\"3q0=\"}";
        size_t len = 0;
        kb_mp_reader r;
        kb_mp_value v;
        CHECK_EQ(kb_json_to_mp(src, strlen(src), scratch, sizeof scratch, mp,
                               sizeof mp, &len),
                 KB_OK);
        kb_mp_reader_init(&r, mp, len);
        CHECK_EQ(kb_mp_read(&r, &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_EXT);
        CHECK_EQ(v.ext_type, 5);
        CHECK_EQ(v.len, (size_t)2);
        CHECK_EQ(v.data[0], 0xDE);
    }

    KB_TEST("JSON → msgpack: ключи $int:<n>");
    {
        static const char *src =
            "{\"$int:12345\":true,\"$int:-7\":false,"
            "\"$int:18446744073709551615\":1,\"$int:oops\":true,"
            "\"plain\":true}";
        size_t len = 0;
        kb_mp_reader r;
        kb_mp_value v;
        int seen_pos = 0, seen_neg = 0, seen_umax = 0, seen_oops = 0,
            seen_plain = 0;
        size_t pairs;

        CHECK_EQ(kb_json_to_mp(src, strlen(src), scratch, sizeof scratch, mp,
                               sizeof mp, &len),
                 KB_OK);
        kb_mp_reader_init(&r, mp, len);
        CHECK_EQ(kb_mp_read(&r, &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_MAP);
        pairs = v.len;
        CHECK_EQ(pairs, (size_t)5);
        while (pairs-- > 0) {
            kb_mp_value key, val;
            CHECK_EQ(kb_mp_read(&r, &key), KB_OK);
            CHECK_EQ(kb_mp_read(&r, &val), KB_OK);
            if ((key.type == KB_MP_INT && key.i == 12345) ||
                (key.type == KB_MP_UINT && key.u == 12345u)) seen_pos = 1;
            if (key.type == KB_MP_INT && key.i == -7) seen_neg = 1;
            if (key.type == KB_MP_UINT) seen_umax = 1;
            if (key.type == KB_MP_STR && kb_mp_str_eq(&key, "$int:oops"))
                seen_oops = 1;
            if (key.type == KB_MP_STR && kb_mp_str_eq(&key, "plain"))
                seen_plain = 1;
        }
        CHECK(seen_pos && seen_neg && seen_umax && seen_oops && seen_plain);
    }

    KB_TEST("round-trip msgpack → tagged JSON → msgpack");
    {
        kb_mp_writer w;
        static const unsigned char fp[] = {0xDE, 0xAD, 0xBE, 0xEF};
        size_t orig_len, back_len = 0, jlen = 0;

        kb_mp_writer_init(&w, mp, sizeof mp);
        kb_mp_write_map(&w, 4);
        kb_mp_write_cstr(&w, "fp");
        kb_mp_write_bin(&w, fp, 4);
        kb_mp_write_cstr(&w, "name");
        kb_mp_write_cstr(&w, "x");
        kb_mp_write_cstr(&w, "n");
        kb_mp_write_i64(&w, -100);
        kb_mp_write_cstr(&w, "kids");
        kb_mp_write_array(&w, 2);
        kb_mp_write_bool(&w, 1);
        kb_mp_write_nil(&w);
        CHECK_EQ(kb_mp_writer_status(&w), KB_OK);
        orig_len = kb_mp_writer_len(&w);

        CHECK_EQ(kb_mp_to_json(mp, orig_len, 1, json, sizeof json, &jlen),
                 KB_OK);
        CHECK_EQ(kb_json_to_mp(json, jlen, scratch, sizeof scratch, mp2,
                               sizeof mp2, &back_len),
                 KB_OK);
        /* Байт в байт: порядок сохранён, кодировки стабильны. */
        CHECK_MEM(mp2, back_len, mp, orig_len);
    }

    KB_TEST("строки с escape и юникодом");
    {
        kb_mp_writer w;
        size_t len = 0, back = 0;
        static const char *s = "a\"b\\c\nd\tе";  /* кавычка, слэш, \n, \t, кир. */
        kb_mp_writer_init(&w, mp, sizeof mp);
        kb_mp_write_cstr(&w, s);
        CHECK_EQ(kb_mp_to_json(mp, kb_mp_writer_len(&w), 0, json, sizeof json,
                               &len),
                 KB_OK);
        /* Обратно — та же строка. */
        CHECK_EQ(kb_json_to_mp(json, len, scratch, sizeof scratch, mp2,
                               sizeof mp2, &back),
                 KB_OK);
        {
            kb_mp_reader r;
            kb_mp_value v;
            kb_mp_reader_init(&r, mp2, back);
            CHECK_EQ(kb_mp_read(&r, &v), KB_OK);
            CHECK_EQ(v.type, KB_MP_STR);
            CHECK_MEM(v.data, v.len, s, strlen(s));
        }
    }

    KB_TEST("мал буфер и битый вход");
    {
        static const char *src = "{\"a\":1}";
        size_t len = 0;
        kb_mp_writer w;
        kb_mp_writer_init(&w, mp, sizeof mp);
        kb_mp_write_cstr(&w, "hello world this is long");
        CHECK_EQ(kb_mp_to_json(mp, kb_mp_writer_len(&w), 0, json, 4, NULL),
                 KB_ERR_TOO_LARGE);
        CHECK_EQ(kb_json_to_mp(src, strlen(src), scratch, sizeof scratch, mp, 2,
                               &len),
                 KB_ERR_TOO_LARGE);
        CHECK_EQ(kb_json_to_mp("{bad", 4, scratch, sizeof scratch, mp,
                               sizeof mp, &len),
                 KB_ERR_INVALID);
    }
}
