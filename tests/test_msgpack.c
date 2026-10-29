#include <string.h>

#include "kb_test.h"
#include "kolibri.h"

static unsigned char out[80000];
static unsigned char deep[100000];

static size_t write_with(kb_mp_writer *w)
{
    return kb_mp_writer_len(w);
}

/* Записать одно значение и сверить байты. */
#define EXPECT_BYTES(call, ...)                                             \
    do {                                                                    \
        static const unsigned char want[] = {__VA_ARGS__};                  \
        kb_mp_writer mw;                                                    \
        kb_mp_writer_init(&mw, out, sizeof out);                            \
        call;                                                               \
        CHECK_EQ(kb_mp_writer_status(&mw), KB_OK);                          \
        CHECK_MEM(out, write_with(&mw), want, sizeof want);                 \
    } while (0)

void kb_tests_msgpack(void)
{
    kb_mp_reader r;
    kb_mp_value v;
    kb_mp_writer w;
    size_t i;

    KB_TEST("целые кодируются кратчайшим представлением");
    {
        EXPECT_BYTES(kb_mp_write_i64(&mw, 0), 0x00);
        EXPECT_BYTES(kb_mp_write_i64(&mw, 127), 0x7F);
        EXPECT_BYTES(kb_mp_write_i64(&mw, 128), 0xCC, 0x80);
        EXPECT_BYTES(kb_mp_write_i64(&mw, 255), 0xCC, 0xFF);
        EXPECT_BYTES(kb_mp_write_i64(&mw, 256), 0xCD, 0x01, 0x00);
        EXPECT_BYTES(kb_mp_write_i64(&mw, 65535), 0xCD, 0xFF, 0xFF);
        EXPECT_BYTES(kb_mp_write_i64(&mw, 65536), 0xCE, 0x00, 0x01, 0x00, 0x00);
        EXPECT_BYTES(kb_mp_write_i64(&mw, 6790), 0xCD, 0x1A, 0x86);
        EXPECT_BYTES(kb_mp_write_i64(&mw, -1), 0xFF);
        EXPECT_BYTES(kb_mp_write_i64(&mw, -32), 0xE0);
        EXPECT_BYTES(kb_mp_write_i64(&mw, -33), 0xD0, 0xDF);
        EXPECT_BYTES(kb_mp_write_i64(&mw, -128), 0xD0, 0x80);
        EXPECT_BYTES(kb_mp_write_i64(&mw, -129), 0xD1, 0xFF, 0x7F);
        EXPECT_BYTES(kb_mp_write_i64(&mw, -32768), 0xD1, 0x80, 0x00);
        EXPECT_BYTES(kb_mp_write_i64(&mw, -32769), 0xD2, 0xFF, 0xFF, 0x7F, 0xFF);
    }

    KB_TEST("строки, бинарники, заголовки");
    {
        EXPECT_BYTES(kb_mp_write_cstr(&mw, ""), 0xA0);
        EXPECT_BYTES(kb_mp_write_cstr(&mw, "chatId"), 0xA6, 'c', 'h', 'a', 't',
                     'I', 'd');
        EXPECT_BYTES(kb_mp_write_nil(&mw), 0xC0);
        EXPECT_BYTES(kb_mp_write_bool(&mw, 1), 0xC3);
        EXPECT_BYTES(kb_mp_write_bool(&mw, 0), 0xC2);
        EXPECT_BYTES(kb_mp_write_map(&mw, 3), 0x83);
        EXPECT_BYTES(kb_mp_write_array(&mw, 0), 0x90);
        EXPECT_BYTES(kb_mp_write_map(&mw, 16), 0xDE, 0x00, 0x10);
        EXPECT_BYTES(kb_mp_write_array(&mw, 16), 0xDC, 0x00, 0x10);
        EXPECT_BYTES(kb_mp_write_bin(&mw, (const unsigned char *)"\x0b\x32", 2),
                     0xC4, 0x02, 0x0B, 0x32);
    }

    KB_TEST("границы длины строки");
    {
        static char long_str[70000];
        memset(long_str, 'x', sizeof long_str);

        kb_mp_writer_init(&w, out, sizeof out);
        kb_mp_write_str(&w, long_str, 31);
        CHECK_EQ(out[0], 0xBF);

        kb_mp_writer_init(&w, out, sizeof out);
        kb_mp_write_str(&w, long_str, 32);
        CHECK_EQ(out[0], 0xD9);
        CHECK_EQ(out[1], 32);

        kb_mp_writer_init(&w, out, sizeof out);
        kb_mp_write_str(&w, long_str, 256);
        CHECK_EQ(out[0], 0xDA);

        kb_mp_writer_init(&w, out, sizeof out);
        kb_mp_write_str(&w, long_str, 65536);
        CHECK_EQ(out[0], 0xDB);
        CHECK_EQ(kb_mp_writer_status(&w), KB_OK);
    }

    KB_TEST("ошибка записи прилипает");
    {
        kb_mp_writer_init(&w, out, 4);
        kb_mp_write_map(&w, 2);
        kb_mp_write_cstr(&w, "слишком длинный ключ");
        kb_mp_write_i64(&w, 1);
        CHECK_EQ(kb_mp_writer_status(&w), KB_ERR_TOO_LARGE);

        /* После ошибки буфер не трогается дальше своей ёмкости. */
        kb_mp_writer_init(&w, out, 0);
        kb_mp_write_nil(&w);
        CHECK_EQ(kb_mp_writer_status(&w), KB_ERR_TOO_LARGE);
    }

    KB_TEST("чтение туда и обратно");
    {
        kb_mp_writer_init(&w, out, sizeof out);
        kb_mp_write_map(&w, 4);
        kb_mp_write_cstr(&w, "chatId");
        kb_mp_write_i64(&w, -42);
        kb_mp_write_cstr(&w, "text");
        kb_mp_write_cstr(&w, "привет");
        kb_mp_write_cstr(&w, "mode");
        kb_mp_write_bin(&w, (const unsigned char *)"\x01\x02\x03", 3);
        kb_mp_write_cstr(&w, "interactive");
        kb_mp_write_bool(&w, 1);
        CHECK_EQ(kb_mp_writer_status(&w), KB_OK);

        kb_mp_reader_init(&r, out, kb_mp_writer_len(&w));
        CHECK_EQ(kb_mp_read(&r, &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_MAP);
        CHECK_EQ(v.len, 4);

        CHECK_EQ(kb_mp_read(&r, &v), KB_OK);
        CHECK(kb_mp_str_eq(&v, "chatId"));
        CHECK_EQ(kb_mp_read(&r, &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_INT);
        CHECK_EQ(v.i, -42);

        CHECK_EQ(kb_mp_read(&r, &v), KB_OK);
        CHECK(kb_mp_str_eq(&v, "text"));
        CHECK_EQ(kb_mp_read(&r, &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_STR);
        CHECK_MEM(v.data, v.len, "привет", strlen("привет"));

        CHECK_EQ(kb_mp_read(&r, &v), KB_OK);
        CHECK(kb_mp_str_eq(&v, "mode"));
        CHECK_EQ(kb_mp_read(&r, &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_BIN);
        CHECK_MEM(v.data, v.len, "\x01\x02\x03", 3);

        CHECK_EQ(kb_mp_read(&r, &v), KB_OK);
        CHECK(kb_mp_str_eq(&v, "interactive"));
        CHECK_EQ(kb_mp_read(&r, &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_BOOL);
        CHECK_EQ(v.boolean, 1);

        CHECK_EQ(kb_mp_reader_left(&r), 0);
    }

    KB_TEST("поиск по ключу");
    {
        size_t doc_len;
        const unsigned char *nested = NULL;
        size_t nested_len = 0;

        kb_mp_writer_init(&w, out, sizeof out);
        kb_mp_write_map(&w, 3);
        kb_mp_write_cstr(&w, "callsSeed");
        kb_mp_write_i64(&w, 1234567890123LL);
        kb_mp_write_cstr(&w, "userAgent");
        kb_mp_write_map(&w, 2);
        kb_mp_write_cstr(&w, "deviceType");
        kb_mp_write_cstr(&w, "ANDROID");
        kb_mp_write_cstr(&w, "buildNumber");
        kb_mp_write_i64(&w, 6790);
        kb_mp_write_cstr(&w, "device_name");
        kb_mp_write_cstr(&w, "Pixel");
        CHECK_EQ(kb_mp_writer_status(&w), KB_OK);
        doc_len = kb_mp_writer_len(&w);

        CHECK_EQ(kb_mp_map_get(out, doc_len, "callsSeed", &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_UINT);
        CHECK_EQ((long)v.u, 1234567890123LL);

        /* Ключ после вложенной map — значит вложенное пропущено правильно. */
        CHECK_EQ(kb_mp_map_get(out, doc_len, "device_name", &v), KB_OK);
        CHECK(kb_mp_str_eq(&v, "Pixel"));

        CHECK_EQ(kb_mp_map_get(out, doc_len, "нет такого", &v), KB_ERR_NOT_FOUND);
        CHECK_EQ(kb_mp_map_has(out, doc_len, "userAgent"), 1);
        CHECK_EQ(kb_mp_map_has(out, doc_len, "нет такого"), 0);

        CHECK_EQ(kb_mp_map_get_raw(out, doc_len, "userAgent", &nested,
                                   &nested_len),
                 KB_OK);
        CHECK_EQ(kb_mp_map_get(nested, nested_len, "buildNumber", &v), KB_OK);
        CHECK_EQ((long)v.u, 6790);
        CHECK_EQ(kb_mp_map_get(nested, nested_len, "deviceType", &v), KB_OK);
        CHECK(kb_mp_str_eq(&v, "ANDROID"));

        /* Не map — не поиск. */
        kb_mp_writer_init(&w, out, sizeof out);
        kb_mp_write_i64(&w, 5);
        CHECK_EQ(kb_mp_map_get(out, kb_mp_writer_len(&w), "x", &v),
                 KB_ERR_INVALID);
    }

    KB_TEST("глубокая вложенность не съедает стек");
    {
        /* 99999 вложенных массивов по одному элементу и nil на дне. */
        for (i = 0; i < sizeof deep - 1; i++) {
            deep[i] = 0x91;
        }
        deep[sizeof deep - 1] = 0xC0;

        kb_mp_reader_init(&r, deep, sizeof deep);
        CHECK_EQ(kb_mp_skip(&r), KB_OK);
        CHECK_EQ(kb_mp_reader_left(&r), 0);
    }

    KB_TEST("битый вход");
    {
        static const unsigned char never_used[] = {0xC1};
        static const unsigned char short_str[] = {0xA5, 'a', 'b'};
        static const unsigned char lying_map[] = {0xDF, 0xFF, 0xFF, 0xFF, 0xFF};
        static const unsigned char lying_str[] = {0xDB, 0xFF, 0xFF, 0xFF, 0xFF};

        kb_mp_reader_init(&r, never_used, sizeof never_used);
        CHECK_EQ(kb_mp_read(&r, &v), KB_ERR_INVALID);

        kb_mp_reader_init(&r, short_str, sizeof short_str);
        CHECK_EQ(kb_mp_read(&r, &v), KB_ERR_TRUNCATED);

        /* Заявка на 4 миллиарда пар при пяти байтах входа. */
        kb_mp_reader_init(&r, lying_map, sizeof lying_map);
        CHECK_EQ(kb_mp_read(&r, &v), KB_ERR_TRUNCATED);

        kb_mp_reader_init(&r, lying_str, sizeof lying_str);
        CHECK_EQ(kb_mp_read(&r, &v), KB_ERR_TRUNCATED);

        kb_mp_reader_init(&r, never_used, 0);
        CHECK_EQ(kb_mp_read(&r, &v), KB_ERR_TRUNCATED);
    }

    KB_TEST("пропуск значений");
    {
        size_t doc_len;

        kb_mp_writer_init(&w, out, sizeof out);
        kb_mp_write_array(&w, 3);
        kb_mp_write_map(&w, 1);
        kb_mp_write_cstr(&w, "a");
        kb_mp_write_array(&w, 2);
        kb_mp_write_i64(&w, 1);
        kb_mp_write_i64(&w, 2);
        kb_mp_write_cstr(&w, "хвост");
        kb_mp_write_bool(&w, 0);
        CHECK_EQ(kb_mp_writer_status(&w), KB_OK);
        doc_len = kb_mp_writer_len(&w);

        kb_mp_reader_init(&r, out, doc_len);
        CHECK_EQ(kb_mp_skip(&r), KB_OK);
        CHECK_EQ(kb_mp_reader_left(&r), 0);
    }
}
