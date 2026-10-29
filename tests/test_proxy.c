/*
 * Рукопожатие с прокси. Сети тут нет: «прокси» — это байты, которые тест
 * скармливает машине, а её ответы сверяются с тем, что кладёт на провод
 * эталонная реализация.
 */

#include <string.h>

#include "kb_test.h"
#include "kolibri.h"

static char storage[KB_PROXY_URL_MAX];

/* Забрать всё, что машина хочет отправить. */
static size_t drain_proxy(kb_proxy *p, unsigned char *out, size_t cap)
{
    size_t total = 0;
    for (;;) {
        size_t len = 0;
        const unsigned char *chunk = kb_proxy_pull_out(p, &len);
        if (chunk == NULL || len > cap - total) {
            return total;
        }
        memcpy(out + total, chunk, len);
        total += len;
        kb_proxy_consume_out(p, len);
    }
}

void kb_tests_proxy(void)
{
    kb_proxy_config config;
    kb_proxy p;
    unsigned char out[2048];
    size_t len;
    size_t consumed = 0;

    KB_TEST("base64 совпадает с известными векторами");
    {
        char text[64];
        CHECK_EQ(kb_base64_encode((const unsigned char *)"", 0, text, sizeof text),
                 0);
        CHECK_EQ(strcmp(text, ""), 0);
        kb_base64_encode((const unsigned char *)"f", 1, text, sizeof text);
        CHECK_EQ(strcmp(text, "Zg=="), 0);
        kb_base64_encode((const unsigned char *)"fo", 2, text, sizeof text);
        CHECK_EQ(strcmp(text, "Zm8="), 0);
        kb_base64_encode((const unsigned char *)"foo", 3, text, sizeof text);
        CHECK_EQ(strcmp(text, "Zm9v"), 0);
        kb_base64_encode((const unsigned char *)"user:pass", 9, text, sizeof text);
        CHECK_EQ(strcmp(text, "dXNlcjpwYXNz"), 0);
        /* Тесный буфер — отказ, а не порча памяти. */
        CHECK_EQ(kb_base64_encode((const unsigned char *)"foo", 3, text, 4), 0);
    }

    KB_TEST("разбор URL прокси");
    {
        CHECK_EQ(kb_proxy_parse("http://bob:secret@10.0.0.1:8080", &config,
                                storage, sizeof storage),
                 KB_OK);
        CHECK_EQ(config.kind, KB_PROXY_HTTP);
        CHECK_EQ(strcmp(config.host, "10.0.0.1"), 0);
        CHECK_EQ(config.port, 8080);
        CHECK_EQ(strcmp(config.username, "bob"), 0);
        CHECK_EQ(strcmp(config.password, "secret"), 0);

        CHECK_EQ(kb_proxy_parse("socks5://127.0.0.1:1080", &config, storage,
                                sizeof storage),
                 KB_OK);
        CHECK_EQ(config.kind, KB_PROXY_SOCKS5);
        CHECK_EQ(config.port, 1080);
        CHECK(config.username == NULL);

        CHECK_EQ(kb_proxy_parse("socks5h://host:1080", &config, storage,
                                sizeof storage),
                 KB_OK);
        CHECK_EQ(config.kind, KB_PROXY_SOCKS5);

        /* Логин без пароля — как в эталоне. */
        CHECK_EQ(kb_proxy_parse("http://onlyuser@h:3128", &config, storage,
                                sizeof storage),
                 KB_OK);
        CHECK_EQ(strcmp(config.username, "onlyuser"), 0);
        CHECK(config.password == NULL);

        CHECK_EQ(kb_proxy_parse("ftp://x:1", &config, storage, sizeof storage),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_proxy_parse("http://host", &config, storage, sizeof storage),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_proxy_parse("http://host:99999", &config, storage,
                                sizeof storage),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_proxy_parse("http://host:abc", &config, storage,
                                sizeof storage),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_proxy_parse("no-scheme", &config, storage, sizeof storage),
                 KB_ERR_INVALID);
    }

    KB_TEST("HTTP CONNECT без авторизации");
    {
        static const char expected[] =
            "CONNECT api.oneme.ru:443 HTTP/1.1\r\n"
            "Host: api.oneme.ru:443\r\n"
            "Proxy-Connection: keep-alive\r\n\r\n";
        static const char reply[] = "HTTP/1.1 200 Connection established\r\n\r\n";

        kb_proxy_parse("http://127.0.0.1:3128", &config, storage, sizeof storage);
        CHECK_EQ(kb_proxy_init(&p, &config, "api.oneme.ru", 443), KB_OK);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_WRITE);

        len = drain_proxy(&p, out, sizeof out);
        CHECK_MEM(out, len, expected, sizeof expected - 1);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_READ);

        CHECK_EQ(kb_proxy_feed(&p, (const unsigned char *)reply,
                               sizeof reply - 1, &consumed),
                 KB_OK);
        CHECK_EQ(consumed, sizeof reply - 1);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_DONE);
        CHECK_EQ(kb_proxy_error(&p), KB_OK);
    }

    KB_TEST("HTTP CONNECT с авторизацией");
    {
        char token[64];
        kb_proxy_parse("http://bob:secret@127.0.0.1:3128", &config, storage,
                       sizeof storage);
        CHECK_EQ(kb_proxy_init(&p, &config, "example.org", 443), KB_OK);
        len = drain_proxy(&p, out, sizeof out);
        out[len] = '\0';

        kb_base64_encode((const unsigned char *)"bob:secret", 10, token,
                         sizeof token);
        CHECK(strstr((const char *)out, "Proxy-Authorization: Basic ") != NULL);
        CHECK(strstr((const char *)out, token) != NULL);
        CHECK(strstr((const char *)out, "CONNECT example.org:443 HTTP/1.1") != NULL);
    }

    KB_TEST("HTTP: прокси отказал");
    {
        static const char reply[] =
            "HTTP/1.1 407 Proxy Authentication Required\r\n"
            "Proxy-Authenticate: Basic\r\n\r\n";

        kb_proxy_parse("http://127.0.0.1:3128", &config, storage, sizeof storage);
        kb_proxy_init(&p, &config, "api.oneme.ru", 443);
        drain_proxy(&p, out, sizeof out);

        CHECK(kb_proxy_feed(&p, (const unsigned char *)reply, sizeof reply - 1,
                            &consumed) != KB_OK);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_FAILED);
        CHECK(strstr(kb_proxy_detail(&p), "407") != NULL);
    }

    KB_TEST("HTTP: ответ приходит по байту");
    {
        static const char reply[] = "HTTP/1.1 200 OK\r\nX-Via: proxy\r\n\r\n";
        size_t i;

        kb_proxy_parse("http://127.0.0.1:3128", &config, storage, sizeof storage);
        kb_proxy_init(&p, &config, "api.oneme.ru", 443);
        drain_proxy(&p, out, sizeof out);

        for (i = 0; i + 1 < sizeof reply - 1; i++) {
            CHECK_EQ(kb_proxy_feed(&p, (const unsigned char *)reply + i, 1,
                                   &consumed),
                     KB_OK);
            CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_READ);
        }
        CHECK_EQ(kb_proxy_feed(&p, (const unsigned char *)reply + i, 1,
                               &consumed),
                 KB_OK);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_DONE);
    }

    KB_TEST("хвост после рукопожатия достаётся туннелю");
    {
        static const char reply[] = "HTTP/1.1 200 OK\r\n\r\nПРИВЕТ";
        size_t header_len = strlen("HTTP/1.1 200 OK\r\n\r\n");

        kb_proxy_parse("http://127.0.0.1:3128", &config, storage, sizeof storage);
        kb_proxy_init(&p, &config, "api.oneme.ru", 443);
        drain_proxy(&p, out, sizeof out);

        CHECK_EQ(kb_proxy_feed(&p, (const unsigned char *)reply,
                               sizeof reply - 1, &consumed),
                 KB_OK);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_DONE);
        /* Съедены только заголовки; остальное — данные соединения. */
        CHECK_EQ(consumed, header_len);
        CHECK_MEM(reply + consumed, (sizeof reply - 1) - consumed, "ПРИВЕТ",
                  strlen("ПРИВЕТ"));
    }

    KB_TEST("разбор статуса HTTP: регрессии аудита");
    {
        struct {
            const char *reply;
            int ok;
        } cases[] = {
            {"HTTP/1.1 200 Connection established\r\n\r\n", 1},
            /* Эталон режет строку по любым пробельным символам. */
            {"HTTP/1.1  200 OK\r\n\r\n", 1},
            {"HTTP/1.1\t200 OK\r\n\r\n", 1},
            {"HTTP/1.0 200\r\n\r\n", 1},
            /* Накопитель переполнялся по модулю: 4294967496 mod 2^32 == 200,
               и отказ прокси читался как открытый туннель. */
            {"HTTP/1.1 4294967496 Nope\r\n\r\n", 0},
            {"HTTP/1.1 65536 Nope\r\n\r\n", 0},
            {"HTTP/1.1 4294967496\r\n\r\n", 0},
            /* Токен должен быть числом целиком, как parse::<u16>() у эталона. */
            {"HTTP/1.1 200x OK\r\n\r\n", 0},
            {"HTTP/1.1 x200 OK\r\n\r\n", 0},
            {"HTTP/1.1 407 Proxy Authentication Required\r\n\r\n", 0},
            {"HTTP/1.1\r\n\r\n", 0},
            {"\r\n\r\n", 0},
        };
        size_t c;

        kb_proxy_parse("http://127.0.0.1:3128", &config, storage, sizeof storage);
        for (c = 0; c < sizeof cases / sizeof cases[0]; c++) {
            kb_proxy_init(&p, &config, "api.oneme.ru", 443);
            drain_proxy(&p, out, sizeof out);
            kb_proxy_feed(&p, (const unsigned char *)cases[c].reply,
                          strlen(cases[c].reply), &consumed);
            if (cases[c].ok) {
                CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_DONE);
            } else {
                CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_FAILED);
            }
        }
    }

    KB_TEST("SOCKS5: ответы склеены в один сегмент");
    {
        /* Регрессия аудита: прокси вправе прислать выбор метода и ответ на
           CONNECT одним куском. Машина обязана съесть первый, дать отправить
           запрос и доесть второй, а не потерять хвост. */
        static const unsigned char glued[] = {
            0x05, 0x00,                                     /* метод */
            0x05, 0x00, 0x00, 0x01, 127, 0, 0, 1, 0x01, 0xBB /* ответ CONNECT */
        };
        size_t at = 0;

        kb_proxy_parse("socks5://127.0.0.1:1080", &config, storage,
                       sizeof storage);
        kb_proxy_init(&p, &config, "host", 443);
        drain_proxy(&p, out, sizeof out);

        /* Первый feed разбирает только выбор метода и останавливается. */
        CHECK_EQ(kb_proxy_feed(&p, glued, sizeof glued, &consumed), KB_OK);
        CHECK_EQ(consumed, 2);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_WRITE);
        at += consumed;

        drain_proxy(&p, out, sizeof out);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_READ);

        /* Остаток того же сегмента доедается следующим шагом. */
        CHECK_EQ(kb_proxy_feed(&p, glued + at, sizeof glued - at, &consumed),
                 KB_OK);
        CHECK_EQ(consumed, sizeof glued - at);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_DONE);
    }

    KB_TEST("SOCKS5 без авторизации");
    {
        static const unsigned char greeting[] = {0x05, 0x01, 0x00};
        static const unsigned char method_ok[] = {0x05, 0x00};
        static const unsigned char reply_ipv4[] = {0x05, 0x00, 0x00, 0x01,
                                                   127,  0,    0,    1,
                                                   0x01, 0xBB};
        unsigned char expected_connect[32];
        size_t host_len = strlen("api.oneme.ru");

        kb_proxy_parse("socks5://127.0.0.1:1080", &config, storage,
                       sizeof storage);
        CHECK_EQ(kb_proxy_init(&p, &config, "api.oneme.ru", 443), KB_OK);

        len = drain_proxy(&p, out, sizeof out);
        CHECK_MEM(out, len, greeting, sizeof greeting);

        CHECK_EQ(kb_proxy_feed(&p, method_ok, sizeof method_ok, &consumed), KB_OK);
        CHECK_EQ(consumed, 2);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_WRITE);

        expected_connect[0] = 0x05;
        expected_connect[1] = 0x01;
        expected_connect[2] = 0x00;
        expected_connect[3] = 0x03;
        expected_connect[4] = (unsigned char)host_len;
        memcpy(expected_connect + 5, "api.oneme.ru", host_len);
        expected_connect[5 + host_len] = 0x01;
        expected_connect[6 + host_len] = 0xBB;

        len = drain_proxy(&p, out, sizeof out);
        CHECK_MEM(out, len, expected_connect, 7 + host_len);

        CHECK_EQ(kb_proxy_feed(&p, reply_ipv4, sizeof reply_ipv4, &consumed),
                 KB_OK);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_DONE);
        CHECK_EQ(consumed, sizeof reply_ipv4);
    }

    KB_TEST("SOCKS5 с логином и паролем");
    {
        static const unsigned char greeting[] = {0x05, 0x02, 0x00, 0x02};
        static const unsigned char method_auth[] = {0x05, 0x02};
        static const unsigned char auth_ok[] = {0x01, 0x00};
        static const unsigned char reply_ipv4[] = {0x05, 0x00, 0x00, 0x01,
                                                   1, 2, 3, 4, 0x01, 0xBB};
        static const unsigned char expected_auth[] = {0x01, 0x03, 'b', 'o', 'b',
                                                      0x06, 's', 'e', 'c', 'r',
                                                      'e',  't'};

        kb_proxy_parse("socks5://bob:secret@127.0.0.1:1080", &config, storage,
                       sizeof storage);
        CHECK_EQ(kb_proxy_init(&p, &config, "host", 443), KB_OK);

        len = drain_proxy(&p, out, sizeof out);
        CHECK_MEM(out, len, greeting, sizeof greeting);

        kb_proxy_feed(&p, method_auth, sizeof method_auth, &consumed);
        len = drain_proxy(&p, out, sizeof out);
        CHECK_MEM(out, len, expected_auth, sizeof expected_auth);

        kb_proxy_feed(&p, auth_ok, sizeof auth_ok, &consumed);
        len = drain_proxy(&p, out, sizeof out);
        CHECK(len > 0); /* запрос CONNECT */

        CHECK_EQ(kb_proxy_feed(&p, reply_ipv4, sizeof reply_ipv4, &consumed),
                 KB_OK);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_DONE);
    }

    KB_TEST("SOCKS5: адрес в ответе бывает разной длины");
    {
        static const unsigned char method_ok[] = {0x05, 0x00};
        static const unsigned char reply_domain[] = {0x05, 0x00, 0x00, 0x03,
                                                     0x03, 'a',  'b',  'c',
                                                     0x01, 0xBB};
        static const unsigned char reply_ipv6[] = {
            0x05, 0x00, 0x00, 0x04, 0, 0, 0, 0, 0, 0, 0, 0,
            0,    0,    0,    0,    0, 0, 0, 0, 0x01, 0xBB};

        kb_proxy_parse("socks5://127.0.0.1:1080", &config, storage,
                       sizeof storage);

        kb_proxy_init(&p, &config, "host", 443);
        drain_proxy(&p, out, sizeof out);
        kb_proxy_feed(&p, method_ok, sizeof method_ok, &consumed);
        drain_proxy(&p, out, sizeof out);
        CHECK_EQ(kb_proxy_feed(&p, reply_domain, sizeof reply_domain, &consumed),
                 KB_OK);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_DONE);
        CHECK_EQ(consumed, sizeof reply_domain);

        kb_proxy_init(&p, &config, "host", 443);
        drain_proxy(&p, out, sizeof out);
        kb_proxy_feed(&p, method_ok, sizeof method_ok, &consumed);
        drain_proxy(&p, out, sizeof out);
        CHECK_EQ(kb_proxy_feed(&p, reply_ipv6, sizeof reply_ipv6, &consumed),
                 KB_OK);
        CHECK_EQ(kb_proxy_get_step(&p), KB_PROXY_STEP_DONE);
    }

    KB_TEST("SOCKS5: отказы");
    {
        static const unsigned char method_ok[] = {0x05, 0x00};
        static const unsigned char not_socks5[] = {0x04, 0x00};
        static const unsigned char no_methods[] = {0x05, 0xFF};
        static const unsigned char weird_method[] = {0x05, 0x07};
        static const unsigned char refused[] = {0x05, 0x05, 0x00, 0x01};
        static const unsigned char bad_atyp[] = {0x05, 0x00, 0x00, 0x09};
        static const unsigned char auth_bad[] = {0x01, 0x01};
        static const unsigned char method_auth[] = {0x05, 0x02};

        kb_proxy_parse("socks5://127.0.0.1:1080", &config, storage,
                       sizeof storage);

        kb_proxy_init(&p, &config, "host", 443);
        drain_proxy(&p, out, sizeof out);
        CHECK(kb_proxy_feed(&p, not_socks5, 2, &consumed) != KB_OK);
        CHECK(strstr(kb_proxy_detail(&p), "SOCKS5") != NULL);

        kb_proxy_init(&p, &config, "host", 443);
        drain_proxy(&p, out, sizeof out);
        CHECK(kb_proxy_feed(&p, no_methods, 2, &consumed) != KB_OK);

        kb_proxy_init(&p, &config, "host", 443);
        drain_proxy(&p, out, sizeof out);
        CHECK(kb_proxy_feed(&p, weird_method, 2, &consumed) != KB_OK);
        CHECK(strstr(kb_proxy_detail(&p), "7") != NULL);

        kb_proxy_init(&p, &config, "host", 443);
        drain_proxy(&p, out, sizeof out);
        kb_proxy_feed(&p, method_ok, 2, &consumed);
        drain_proxy(&p, out, sizeof out);
        CHECK(kb_proxy_feed(&p, refused, 4, &consumed) != KB_OK);
        CHECK(strstr(kb_proxy_detail(&p), "5") != NULL);

        kb_proxy_init(&p, &config, "host", 443);
        drain_proxy(&p, out, sizeof out);
        kb_proxy_feed(&p, method_ok, 2, &consumed);
        drain_proxy(&p, out, sizeof out);
        CHECK(kb_proxy_feed(&p, bad_atyp, 4, &consumed) != KB_OK);

        kb_proxy_parse("socks5://bob:secret@127.0.0.1:1080", &config, storage,
                       sizeof storage);
        kb_proxy_init(&p, &config, "host", 443);
        drain_proxy(&p, out, sizeof out);
        kb_proxy_feed(&p, method_auth, 2, &consumed);
        drain_proxy(&p, out, sizeof out);
        CHECK(kb_proxy_feed(&p, auth_bad, 2, &consumed) != KB_OK);
        CHECK(strstr(kb_proxy_detail(&p), "auth") != NULL);
    }

    KB_TEST("аргументы");
    {
        kb_proxy_parse("http://127.0.0.1:3128", &config, storage, sizeof storage);
        CHECK_EQ(kb_proxy_init(NULL, &config, "host", 443), KB_ERR_INVALID);
        CHECK_EQ(kb_proxy_init(&p, NULL, "host", 443), KB_ERR_INVALID);
        CHECK_EQ(kb_proxy_init(&p, &config, NULL, 443), KB_ERR_INVALID);
        CHECK_EQ(kb_proxy_init(&p, &config, "", 443), KB_ERR_INVALID);
        {
            kb_proxy_config none;
            memset(&none, 0, sizeof none);
            none.kind = KB_PROXY_NONE;
            CHECK_EQ(kb_proxy_init(&p, &none, "host", 443), KB_ERR_INVALID);
        }
        CHECK(kb_proxy_pull_out(NULL, &len) == NULL);
        CHECK_EQ(kb_proxy_error(NULL), KB_ERR_INVALID);
        CHECK_EQ(strcmp(kb_proxy_detail(NULL), "ok"), 0);
    }
}
