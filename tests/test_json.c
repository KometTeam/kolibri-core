/* JSON-ридер: срезы, поиск полей, итерация, escape и битый вход. */

#include <string.h>

#include "kb_test.h"
#include "kolibri.h"
#include "kolibri_json.h"

static kb_json_value P(const char *s)
{
    kb_json_value v;
    v.type = KB_JSON_NULL;
    v.start = NULL;
    v.len = 0;
    kb_json_parse(s, strlen(s), &v);
    return v;
}

void kb_tests_json(void)
{
    KB_TEST("разбор скаляров");
    {
        kb_json_value v;
        CHECK_EQ(kb_json_parse("true", 4, &v), KB_OK);
        CHECK_EQ(v.type, KB_JSON_BOOL);
        CHECK_EQ(kb_json_parse("  false ", 8, &v), KB_OK);
        CHECK_EQ(v.type, KB_JSON_BOOL);
        CHECK_EQ(kb_json_parse("null", 4, &v), KB_OK);
        CHECK_EQ(v.type, KB_JSON_NULL);
        CHECK_EQ(kb_json_parse("-12.5e3", 7, &v), KB_OK);
        CHECK_EQ(v.type, KB_JSON_NUMBER);
        CHECK_EQ(kb_json_parse("\"hi\"", 4, &v), KB_OK);
        CHECK_EQ(v.type, KB_JSON_STRING);
    }

    KB_TEST("мусор после значения — ошибка");
    {
        kb_json_value v;
        CHECK_EQ(kb_json_parse("1 2", 3, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("{}x", 3, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("", 0, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("   ", 3, &v), KB_ERR_INVALID);
    }

    KB_TEST("поиск поля объекта");
    {
        kb_json_value o = P("{\"a\":1,\"b\":\"x\",\"c\":true}");
        kb_json_value f;
        int b;
        int64_t n;
        CHECK_EQ(o.type, KB_JSON_OBJECT);
        CHECK_EQ(kb_json_object_get(&o, "a", &f), KB_OK);
        CHECK_EQ(kb_json_as_i64(&f, &n), KB_OK);
        CHECK_EQ((int)n, 1);
        CHECK_EQ(kb_json_object_get(&o, "b", &f), KB_OK);
        CHECK(kb_json_str_eq(&f, "x"));
        CHECK_EQ(kb_json_object_get(&o, "c", &f), KB_OK);
        CHECK_EQ(kb_json_as_bool(&f, &b), KB_OK);
        CHECK_EQ(b, 1);
        CHECK_EQ(kb_json_object_get(&o, "z", &f), KB_ERR_NOT_FOUND);
    }

    KB_TEST("вложенность через повторный get");
    {
        kb_json_value root = P("{\"conversation\":{\"topology\":\"DIRECT\","
                               "\"participants\":[{\"id\":42},{\"id\":99}]}}");
        kb_json_value conv, topo, parts, elem, id;
        kb_json_value it;
        int64_t ids[4];
        int count = 0;

        CHECK_EQ(kb_json_object_get(&root, "conversation", &conv), KB_OK);
        CHECK_EQ(kb_json_object_get(&conv, "topology", &topo), KB_OK);
        CHECK(kb_json_str_eq(&topo, "DIRECT"));
        CHECK_EQ(kb_json_object_get(&conv, "participants", &parts), KB_OK);
        CHECK_EQ(parts.type, KB_JSON_ARRAY);

        kb_json_array_iter_init(&it, &parts);
        while (kb_json_array_next(&it, &elem)) {
            CHECK_EQ(kb_json_object_get(&elem, "id", &id), KB_OK);
            CHECK_EQ(kb_json_as_i64(&id, &ids[count]), KB_OK);
            count++;
        }
        CHECK_EQ(count, 2);
        CHECK_EQ((int)ids[0], 42);
        CHECK_EQ((int)ids[1], 99);
    }

    KB_TEST("массив строк и пустой массив");
    {
        kb_json_value arr = P("[\"a\",\"bb\",\"ccc\"]");
        kb_json_value it, elem;
        int n = 0;
        char buf[8];
        kb_json_array_iter_init(&it, &arr);
        while (kb_json_array_next(&it, &elem)) {
            CHECK_EQ(kb_json_str_copy(&elem, buf, sizeof buf, NULL), KB_OK);
            n++;
        }
        CHECK_EQ(n, 3);

        arr = P("[]");
        kb_json_array_iter_init(&it, &arr);
        CHECK_EQ(kb_json_array_next(&it, &elem), 0);

        arr = P("[  ]");
        kb_json_array_iter_init(&it, &arr);
        CHECK_EQ(kb_json_array_next(&it, &elem), 0);
    }

    KB_TEST("escape в строках");
    {
        kb_json_value v = P("\"a\\\"b\\\\c\\nd\"");
        char buf[16];
        size_t len = 0;
        CHECK_EQ(v.type, KB_JSON_STRING);
        CHECK_EQ(kb_json_str_copy(&v, buf, sizeof buf, &len), KB_OK);
        CHECK_MEM(buf, len, "a\"b\\c\nd", 7);
        CHECK(kb_json_str_eq(&v, "a\"b\\c\nd"));

        /* \u00XX → байт. */
        v = P("\"A\\u0042C\"");
        CHECK_EQ(kb_json_str_copy(&v, buf, sizeof buf, &len), KB_OK);
        CHECK_MEM(buf, len, "ABC", 3);

        /* \u2013 → трёхбайтный UTF-8 (–). */
        v = P("\"\\u2013\"");
        CHECK_EQ(kb_json_str_copy(&v, buf, sizeof buf, &len), KB_OK);
        CHECK_EQ((int)len, 3);
        CHECK_EQ((unsigned char)buf[0], 0xE2);
    }

    KB_TEST("ключ с escape находится");
    {
        kb_json_value o = P("{\"a\\u0062c\":7}");
        kb_json_value f;
        int64_t n;
        CHECK_EQ(kb_json_object_get(&o, "abc", &f), KB_OK);
        CHECK_EQ(kb_json_as_i64(&f, &n), KB_OK);
        CHECK_EQ((int)n, 7);
    }

    KB_TEST("as_i64: минус, границы, переполнение");
    {
        kb_json_value v = P("-9223372036854775808");
        int64_t n;
        CHECK_EQ(kb_json_as_i64(&v, &n), KB_OK);
        CHECK(n == (-0x7fffffffffffffffLL - 1));
        v = P("9223372036854775807");
        CHECK_EQ(kb_json_as_i64(&v, &n), KB_OK);
        CHECK(n == 0x7fffffffffffffffLL);
        v = P("9223372036854775808");
        CHECK_EQ(kb_json_as_i64(&v, &n), KB_ERR_OVERFLOW);
        v = P("1.5");
        CHECK_EQ(kb_json_as_i64(&v, &n), KB_ERR_INVALID);
    }

    KB_TEST("несоответствие типа");
    {
        kb_json_value v = P("\"x\"");
        int64_t n;
        int b;
        CHECK_EQ(kb_json_as_i64(&v, &n), KB_ERR_INVALID);
        CHECK_EQ(kb_json_as_bool(&v, &b), KB_ERR_INVALID);
        v = P("123");
        CHECK_EQ(kb_json_str_eq(&v, "123"), 0);
    }

    KB_TEST("битый JSON отвергается");
    {
        kb_json_value v;
        CHECK_EQ(kb_json_parse("{\"a\":}", 6, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("{\"a\" 1}", 7, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("{\"a\":1,}", 8, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("[1,2", 4, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("\"unterminated", 13, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("\"bad\\x\"", 7, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("\"\\u00zz\"", 8, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("01", 2, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("+1", 2, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("tru", 3, &v), KB_ERR_INVALID);
        CHECK_EQ(kb_json_parse("{", 1, &v), KB_ERR_INVALID);
        /* сырой перевод строки внутри строки */
        CHECK_EQ(kb_json_parse("\"a\nb\"", 5, &v), KB_ERR_INVALID);
    }

    KB_TEST("буфер str_copy мал");
    {
        kb_json_value v = P("\"abcdef\"");
        char buf[4];
        CHECK_EQ(kb_json_str_copy(&v, buf, sizeof buf, NULL), KB_ERR_TOO_LARGE);
    }
}
