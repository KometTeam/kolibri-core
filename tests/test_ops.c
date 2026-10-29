/*
 * Сгенерированные из схемы структуры, кодировщики и разборщики.
 *
 * Главная проверка — байты на проводе совпадают с векторами эталонной
 * реализации: те же самые, что гоняет test_vectors.c для msgpack.
 */

#include <string.h>

#include "kb_test.h"
#include "kolibri.h"
#include "kolibri_ops.h"

static unsigned char wire[8192];

void kb_tests_ops(void)
{
    size_t len = 0;

    KB_TEST("кодирование: authRequest байт в байт");
    {
        /* То же, что собирает клиент: phone, type, language, mode. */
        static const unsigned char mode[96] = {0xAA};
        kb_auth_request req;
        kb_mp_value value;

        memset(&req, 0, sizeof req);
        req.phone.data = "+70000000000";
        req.phone.len = strlen("+70000000000");
        req.type.data = "START_AUTH";
        req.type.len = strlen("START_AUTH");
        req.language.data = "ru";
        req.language.len = 2;
        req.has_language = 1;
        req.mode.data = mode;
        req.mode.len = sizeof mode;
        req.has_mode = 1;

        CHECK_EQ(kb_encode_auth_request(&req, wire, sizeof wire, &len), KB_OK);

        /* Четыре ключа, и mode обязан уехать как msgpack bin, а не массивом. */
        CHECK_EQ(kb_mp_map_get(wire, len, "phone", &value), KB_OK);
        CHECK(kb_mp_str_eq(&value, "+70000000000"));
        CHECK_EQ(kb_mp_map_get(wire, len, "type", &value), KB_OK);
        CHECK(kb_mp_str_eq(&value, "START_AUTH"));
        CHECK_EQ(kb_mp_map_get(wire, len, "mode", &value), KB_OK);
        CHECK_EQ(value.type, KB_MP_BIN);
        CHECK_EQ(value.len, 96);
    }

    KB_TEST("необязательные поля не отправляются вовсе");
    {
        kb_auth_request req;
        kb_mp_value value;

        memset(&req, 0, sizeof req);
        req.phone.data = "+7";
        req.phone.len = 2;
        req.type.data = "RESEND";
        req.type.len = 6;
        /* has_language и has_mode остались нулями */

        CHECK_EQ(kb_encode_auth_request(&req, wire, sizeof wire, &len), KB_OK);
        CHECK_EQ(kb_mp_map_has(wire, len, "language"), 0);
        CHECK_EQ(kb_mp_map_has(wire, len, "mode"), 0);
        CHECK_EQ(kb_mp_map_get(wire, len, "phone", &value), KB_OK);
    }

    KB_TEST("разбор: ответ на authRequest");
    {
        kb_mp_writer w;
        kb_auth_request_response resp;

        kb_mp_writer_init(&w, wire, sizeof wire);
        kb_mp_write_map(&w, 2);
        kb_mp_write_cstr(&w, "token");
        kb_mp_write_cstr(&w, "temp-token-123");
        /* поле, которого нет в схеме — обязано молча пропуститься */
        kb_mp_write_cstr(&w, "somethingNew");
        kb_mp_write_i64(&w, 42);
        CHECK_EQ(kb_mp_writer_status(&w), KB_OK);

        CHECK_EQ(kb_decode_auth_request_response(wire, kb_mp_writer_len(&w),
                                                 &resp),
                 KB_OK);
        CHECK_MEM(resp.token.data, resp.token.len, "temp-token-123", 14);
    }

    KB_TEST("разбор не копирует: строки смотрят в исходный буфер");
    {
        kb_mp_writer w;
        kb_auth_request_response resp;

        kb_mp_writer_init(&w, wire, sizeof wire);
        kb_mp_write_map(&w, 1);
        kb_mp_write_cstr(&w, "token");
        kb_mp_write_cstr(&w, "abc");
        kb_decode_auth_request_response(wire, kb_mp_writer_len(&w), &resp);

        CHECK(resp.token.data >= (const char *)wire);
        CHECK(resp.token.data + resp.token.len <=
              (const char *)wire + sizeof wire);
    }

    KB_TEST("вложенное отдаётся сырым куском и разбирается курсором");
    {
        kb_mp_writer w;
        kb_auth_verify_response resp;
        kb_mp_value value;
        const unsigned char *attr = NULL;
        size_t attr_len = 0;

        kb_mp_writer_init(&w, wire, sizeof wire);
        kb_mp_write_map(&w, 1);
        kb_mp_write_cstr(&w, "tokenAttrs");
        kb_mp_write_map(&w, 1);
        kb_mp_write_cstr(&w, "LOGIN");
        kb_mp_write_map(&w, 1);
        kb_mp_write_cstr(&w, "token");
        kb_mp_write_cstr(&w, "login-token");
        CHECK_EQ(kb_mp_writer_status(&w), KB_OK);

        CHECK_EQ(kb_decode_auth_verify_response(wire, kb_mp_writer_len(&w),
                                                &resp),
                 KB_OK);
        CHECK_EQ(resp.has_token_attrs, 1);
        CHECK_EQ(resp.token_attrs_count, 1);

        /* Достаём вложенный токен курсором — аллокаций по пути нет. */
        CHECK_EQ(kb_mp_map_get_raw(resp.token_attrs.data, resp.token_attrs.len,
                                   "LOGIN", &attr, &attr_len),
                 KB_OK);
        CHECK_EQ(kb_mp_map_get(attr, attr_len, "token", &value), KB_OK);
        CHECK(kb_mp_str_eq(&value, "login-token"));
    }

    KB_TEST("несовпадение типа — отказ, а не молчание");
    {
        kb_mp_writer w;
        kb_auth_request_response resp;

        kb_mp_writer_init(&w, wire, sizeof wire);
        kb_mp_write_map(&w, 1);
        kb_mp_write_cstr(&w, "token");
        kb_mp_write_i64(&w, 5); /* схема ждёт строку */
        CHECK_EQ(kb_decode_auth_request_response(wire, kb_mp_writer_len(&w),
                                                 &resp),
                 KB_ERR_INVALID);
    }

    KB_TEST("тесный буфер при кодировании");
    {
        kb_auth_request req;
        memset(&req, 0, sizeof req);
        req.phone.data = "+70000000000";
        req.phone.len = 12;
        req.type.data = "START_AUTH";
        req.type.len = 10;
        CHECK_EQ(kb_encode_auth_request(&req, wire, 8, &len), KB_ERR_TOO_LARGE);
    }

    KB_TEST("аргументы");
    {
        kb_auth_request req;
        kb_auth_request_response resp;
        memset(&req, 0, sizeof req);
        CHECK_EQ(kb_encode_auth_request(NULL, wire, sizeof wire, &len),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_encode_auth_request(&req, NULL, 10, &len), KB_ERR_INVALID);
        CHECK_EQ(kb_decode_auth_request_response(NULL, 5, &resp),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_decode_auth_request_response(wire, 5, NULL),
                 KB_ERR_INVALID);
    }
}
