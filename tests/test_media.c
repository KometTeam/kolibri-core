/*
 * Загрузка медиа: URL, сборка запроса, разбор ответа, заголовки трёх режимов.
 *
 * Сети нет: «сервер» — это байты, которые тест скармливает машине разбора.
 */

#include <stdio.h>
#include <string.h>

#include "kb_test.h"
#include "kolibri.h"
#include "kolibri_media.h"

static unsigned char scratch[16384];
static char storage[KB_MEDIA_STORAGE_MIN];

/* Найти значение заголовка в собранном запросе. */
static int head_has(const char *head, const char *needle)
{
    return strstr(head, needle) != NULL;
}

void kb_tests_media(void)
{
    kb_url url;
    size_t len = 0;

    KB_TEST("разбор URL");
    {
        CHECK_EQ(kb_url_parse("https://cdn.example.com/upload?x=1", &url), KB_OK);
        CHECK_EQ(url.https, 1);
        CHECK_EQ(url.port, 443);
        CHECK_EQ(strcmp(url.host, "cdn.example.com"), 0);
        CHECK_EQ(strcmp(url.path, "/upload?x=1"), 0);

        CHECK_EQ(kb_url_parse("http://1.2.3.4:8080/a/b", &url), KB_OK);
        CHECK_EQ(url.https, 0);
        CHECK_EQ(url.port, 8080);
        CHECK_EQ(strcmp(url.host, "1.2.3.4"), 0);

        /* Без пути — путь становится "/". */
        CHECK_EQ(kb_url_parse("https://host", &url), KB_OK);
        CHECK_EQ(strcmp(url.path, "/"), 0);
        CHECK_EQ(url.port, 443);

        /* Двоеточие не от порта не должно съесть хост. */
        CHECK_EQ(kb_url_parse("https://host/path:with:colons", &url), KB_OK);
        CHECK_EQ(strcmp(url.host, "host"), 0);
        CHECK_EQ(strcmp(url.path, "/path:with:colons"), 0);

        CHECK_EQ(kb_url_parse("ftp://host/x", &url), KB_ERR_INVALID);
        CHECK_EQ(kb_url_parse("no-scheme", &url), KB_ERR_INVALID);
        CHECK_EQ(kb_url_parse("https://", &url), KB_ERR_INVALID);
        CHECK_EQ(kb_url_parse("https://host:99999/x", &url), KB_OK);
        /* Слишком большой порт — не порт, значит часть хоста. */
        CHECK_EQ(strcmp(url.host, "host:99999"), 0);
    }

    KB_TEST("percent-кодирование как у Dart");
    {
        char out[128];
        CHECK(kb_percent_encode("abc", out, sizeof out) > 0);
        CHECK_EQ(strcmp(out, "abc"), 0);
        kb_percent_encode("-_.!~*'()", out, sizeof out);
        CHECK_EQ(strcmp(out, "-_.!~*'()"), 0);
        kb_percent_encode("a b", out, sizeof out);
        CHECK_EQ(strcmp(out, "a%20b"), 0);
        kb_percent_encode("OKMessages/26.20.2 (Android 14; Pixel)", out,
                          sizeof out);
        CHECK_EQ(strcmp(out,
                        "OKMessages%2F26.20.2%20(Android%2014%3B%20Pixel)"),
                 0);
        /* Тесный буфер — отказ, а не обрезка. */
        CHECK_EQ(kb_percent_encode("a b", out, 3), 0);
    }

    KB_TEST("тип содержимого по расширению");
    {
        CHECK_EQ(strcmp(kb_content_type_for_filename("a.PNG"), "image/png"), 0);
        CHECK_EQ(strcmp(kb_content_type_for_filename("a.heif"), "image/heic"), 0);
        CHECK_EQ(strcmp(kb_content_type_for_filename("a.webp"), "image/webp"), 0);
        CHECK_EQ(strcmp(kb_content_type_for_filename("noext"), "image/jpeg"), 0);
        CHECK_EQ(strcmp(kb_content_type_for_filename(NULL), "image/jpeg"), 0);
    }

    KB_TEST("сборка запроса");
    {
        kb_http_header headers[2];
        headers[0].name = "Host";
        headers[0].value = "cdn";
        headers[1].name = "Content-Length";
        headers[1].value = "42";

        CHECK_EQ(kb_http_build_head("POST", "/up", headers, 2, scratch,
                                    sizeof scratch, &len),
                 KB_OK);
        scratch[len] = '\0';
        CHECK_EQ(strcmp((const char *)scratch,
                        "POST /up HTTP/1.1\r\nHost: cdn\r\n"
                        "Content-Length: 42\r\n\r\n"),
                 0);
        CHECK_EQ(kb_http_build_head("POST", "/up", headers, 2, scratch, 10, &len),
                 KB_ERR_TOO_LARGE);
    }

    KB_TEST("ответ с Content-Length");
    {
        static const char reply[] =
            "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhello";
        kb_http_response r;
        const unsigned char *body;
        size_t body_len = 0;

        CHECK_EQ(kb_http_response_init(&r, scratch, sizeof scratch), KB_OK);
        CHECK_EQ(kb_http_response_feed(&r, (const unsigned char *)reply,
                                       sizeof reply - 1),
                 KB_OK);
        CHECK_EQ(kb_http_response_done(&r), 1);
        CHECK_EQ(kb_http_response_status(&r), 200);
        body = kb_http_response_body(&r, &body_len);
        CHECK_MEM(body, body_len, "hello", 5);
    }

    KB_TEST("ответ приходит по одному байту");
    {
        static const char reply[] =
            "HTTP/1.1 201 Created\r\nContent-Length: 3\r\n\r\nabc";
        kb_http_response r;
        size_t i;

        kb_http_response_init(&r, scratch, sizeof scratch);
        for (i = 0; i + 1 < sizeof reply - 1; i++) {
            kb_http_response_feed(&r, (const unsigned char *)reply + i, 1);
            CHECK_EQ(kb_http_response_done(&r), 0);
        }
        kb_http_response_feed(&r, (const unsigned char *)reply + i, 1);
        CHECK_EQ(kb_http_response_done(&r), 1);
        CHECK_EQ(kb_http_response_status(&r), 201);
    }

    KB_TEST("ответ chunked");
    {
        static const char reply[] =
            "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n"
            "5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n";
        kb_http_response r;
        const unsigned char *body;
        size_t body_len = 0;

        kb_http_response_init(&r, scratch, sizeof scratch);
        CHECK_EQ(kb_http_response_feed(&r, (const unsigned char *)reply,
                                       sizeof reply - 1),
                 KB_OK);
        CHECK_EQ(kb_http_response_done(&r), 1);
        body = kb_http_response_body(&r, &body_len);
        CHECK_MEM(body, body_len, "hello world", 11);
    }

    KB_TEST("chunked с расширением после размера");
    {
        static const char reply[] =
            "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n"
            "3;ext=1\r\nabc\r\n0\r\n\r\n";
        kb_http_response r;
        const unsigned char *body;
        size_t body_len = 0;

        kb_http_response_init(&r, scratch, sizeof scratch);
        kb_http_response_feed(&r, (const unsigned char *)reply, sizeof reply - 1);
        CHECK_EQ(kb_http_response_done(&r), 1);
        body = kb_http_response_body(&r, &body_len);
        CHECK_MEM(body, body_len, "abc", 3);
    }

    KB_TEST("тело кончается закрытием соединения");
    {
        static const char reply[] = "HTTP/1.1 200 OK\r\nServer: x\r\n\r\ntail";
        kb_http_response r;
        const unsigned char *body;
        size_t body_len = 0;

        kb_http_response_init(&r, scratch, sizeof scratch);
        kb_http_response_feed(&r, (const unsigned char *)reply, sizeof reply - 1);
        /* Ни длины, ни chunked: пока соединение живо, ответ не закончен. */
        CHECK_EQ(kb_http_response_done(&r), 0);
        CHECK_EQ(kb_http_response_finish(&r), KB_OK);
        CHECK_EQ(kb_http_response_done(&r), 1);
        body = kb_http_response_body(&r, &body_len);
        CHECK_MEM(body, body_len, "tail", 4);
    }

    KB_TEST("оборванный ответ не выдаётся за целый");
    {
        static const char reply[] = "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n";
        kb_http_response r;

        kb_http_response_init(&r, scratch, sizeof scratch);
        kb_http_response_feed(&r, (const unsigned char *)reply, sizeof reply - 1);
        CHECK_EQ(kb_http_response_done(&r), 0);
        CHECK_EQ(kb_http_response_finish(&r), KB_ERR_TRUNCATED);
        CHECK(kb_http_response_body(&r, NULL) == NULL);
    }

    KB_TEST("ответ не влезает в буфер");
    {
        static const char reply[] =
            "HTTP/1.1 200 OK\r\nContent-Length: 100000\r\n\r\n";
        unsigned char tiny[80];
        kb_http_response r;

        kb_http_response_init(&r, tiny, sizeof tiny);
        CHECK_EQ(kb_http_response_feed(&r, (const unsigned char *)reply,
                                       sizeof reply - 1),
                 KB_OK);
        CHECK_EQ(kb_http_response_feed(&r, (const unsigned char *)reply,
                                       sizeof reply - 1),
                 KB_ERR_TOO_LARGE);
    }

    KB_TEST("заголовки: одиночный файл");
    {
        kb_http_header headers[KB_MEDIA_HEADERS_MAX];
        size_t count = 0;

        kb_url_parse("https://cdn.example.com/up", &url);
        CHECK_EQ(kb_media_file_headers(&url, "doc.pdf", 1000,
                                       "OKMessages/26.20.2 (Android 14)",
                                       headers, &count, storage,
                                       sizeof storage),
                 KB_OK);
        CHECK_EQ(count, 7);
        CHECK_EQ(kb_http_build_head("POST", url.path, headers, count, scratch,
                                    sizeof scratch, &len),
                 KB_OK);
        scratch[len] = '\0';
        CHECK(head_has((const char *)scratch, "Host: cdn.example.com\r\n"));
        CHECK(head_has((const char *)scratch,
                       "Content-Range: bytes 0-999/1000\r\n"));
        CHECK(head_has((const char *)scratch, "Content-Length: 1000\r\n"));
        CHECK(head_has((const char *)scratch,
                       "Content-Disposition: attachment; filename=doc.pdf\r\n"));
        /* User-Agent обязан быть закодирован: пробелы и скобки. */
        CHECK(head_has((const char *)scratch, "OKMessages%2F26.20.2%20"));
        /* И весь запрос целиком — дословно, включая порядок. */
        CHECK_EQ(strcmp((const char *)scratch,
                        "POST /up HTTP/1.1\r\n"
                        "Host: cdn.example.com\r\n"
                        "Content-Type: application/x-binary; "
                        "charset=x-user-defined\r\n"
                        "Content-Disposition: attachment; filename=doc.pdf\r\n"
                        "Connection: keep-alive\r\n"
                        "User-Agent: OKMessages%2F26.20.2%20"
                        "(Android%2014)\r\n"
                        "Content-Range: bytes 0-999/1000\r\n"
                        "Content-Length: 1000\r\n\r\n"),
                 0);
    }

    KB_TEST("заголовки: фото через multipart");
    {
        kb_http_header headers[KB_MEDIA_HEADERS_MAX];
        size_t count = 0;
        char prologue[512];
        char epilogue[128];
        size_t prologue_len = 0;
        size_t epilogue_len = 0;

        kb_url_parse("https://cdn/up", &url);
        CHECK_EQ(kb_media_photo_headers(&url, "pic.png", 500, "UA", 12345,
                                        headers, &count, storage,
                                        sizeof storage, prologue,
                                        sizeof prologue, &prologue_len,
                                        epilogue, sizeof epilogue,
                                        &epilogue_len),
                 KB_OK);
        CHECK(head_has(prologue, "Content-Disposition: form-data; name=\"file\""));
        CHECK(head_has(prologue, "filename=\"pic.png\""));
        CHECK(head_has(prologue, "Content-Type: image/png"));
        CHECK(head_has(epilogue, "--\r\n"));

        kb_http_build_head("POST", url.path, headers, count, scratch,
                           sizeof scratch, &len);
        scratch[len] = '\0';
        CHECK(head_has((const char *)scratch,
                       "Content-Type: multipart/form-data; boundary=----KolibriBoundary12345"));
        /* Порядок заголовков — как у официального клиента, дословно. */
        {
            char expect[192];
            sprintf(expect,
                    "Host: cdn\r\n"
                    "Content-Type: multipart/form-data; "
                    "boundary=----KolibriBoundary12345\r\n"
                    "Content-Length: %lu\r\n"
                    "Connection: keep-alive\r\n"
                    "User-Agent: UA\r\n",
                    (unsigned long)(prologue_len + 500 + epilogue_len));
            CHECK(head_has((const char *)scratch, expect));
        }
        /* Длина = обрамление плюс файл: иначе сервер оборвёт приём. */
        {
            char expect[64];
            size_t at = 0;
            expect[0] = '\0';
            (void)at;
            sprintf(expect, "Content-Length: %lu\r\n",
                    (unsigned long)(prologue_len + 500 + epilogue_len));
            CHECK(head_has((const char *)scratch, expect));
        }
    }

    KB_TEST("заголовки: кусок видео");
    {
        kb_http_header headers[KB_MEDIA_HEADERS_MAX];
        size_t count = 0;
        char range[80];

        kb_url_parse("https://cdn/up", &url);
        CHECK_EQ(kb_media_content_range(1024, 2047, 4096, range, sizeof range),
                 KB_OK);
        CHECK_EQ(strcmp(range, "bytes 1024-2047/4096"), 0);

        CHECK_EQ(kb_media_chunk_headers(&url, "v.mp4", 1024, range, headers,
                                        &count, storage, sizeof storage),
                 KB_OK);
        kb_http_build_head("POST", url.path, headers, count, scratch,
                           sizeof scratch, &len);
        scratch[len] = '\0';
        CHECK(head_has((const char *)scratch, "X-Uploading-Mode: parallel\r\n"));
        CHECK(head_has((const char *)scratch, "Connection: close\r\n"));
        CHECK(head_has((const char *)scratch,
                       "Content-Disposition: attachment; fileName=\"v.mp4\"\r\n"));
        CHECK(head_has((const char *)scratch,
                       "Content-Range: bytes 1024-2047/4096\r\n"));
        CHECK_EQ(strcmp((const char *)scratch,
                        "POST /up HTTP/1.1\r\n"
                        "Host: cdn\r\n"
                        "Content-Type: application/x-binary; "
                        "charset=x-user-defined\r\n"
                        "Content-Disposition: attachment; "
                        "fileName=\"v.mp4\"\r\n"
                        "Content-Length: 1024\r\n"
                        "X-Uploading-Mode: parallel\r\n"
                        "Connection: close\r\n"
                        "Content-Range: bytes 1024-2047/4096\r\n\r\n"),
                 0);

        /* Без диапазона — это GET, узнающий смещение докачки. */
        CHECK_EQ(kb_media_chunk_headers(&url, "v.mp4", 0, NULL, headers, &count,
                                        storage, sizeof storage),
                 KB_OK);
        kb_http_build_head("GET", url.path, headers, count, scratch,
                           sizeof scratch, &len);
        scratch[len] = '\0';
        CHECK(!head_has((const char *)scratch, "Content-Range"));
    }

    KB_TEST("смещение докачки из ответа сервера");
    {
        CHECK_EQ(kb_media_parse_resume((const unsigned char *)"1024", 4, 4096),
                 1024);
        CHECK_EQ(kb_media_parse_resume((const unsigned char *)" 512\r\n", 6,
                                       4096),
                 512);
        /* Больше файла, мусор, пусто — всё значит «сначала». */
        CHECK_EQ(kb_media_parse_resume((const unsigned char *)"9999", 4, 4096), 0);
        CHECK_EQ(kb_media_parse_resume((const unsigned char *)"abc", 3, 4096), 0);
        CHECK_EQ(kb_media_parse_resume((const unsigned char *)"12a", 3, 4096), 0);
        CHECK_EQ(kb_media_parse_resume((const unsigned char *)"", 0, 4096), 0);
        CHECK_EQ(kb_media_parse_resume((const unsigned char *)"   ", 3, 4096), 0);
        /* Число, не влезающее в 64 бита, не должно завернуться в малое. */
        CHECK_EQ(kb_media_parse_resume(
                     (const unsigned char *)"99999999999999999999999", 23,
                     ~(uint64_t)0),
                 0);
        CHECK_EQ(kb_media_parse_resume(NULL, 4, 4096), 0);
    }

    KB_TEST("раскладка файла на куски");
    {
        kb_media_plan plan;
        uint64_t from = 0;
        uint64_t to = 0;
        int chunks = 0;
        uint64_t covered = 0;

        CHECK_EQ(kb_media_plan_init(&plan, 4096, 1024, 0), KB_OK);
        while (kb_media_plan_next(&plan, &from, &to)) {
            CHECK_EQ(from, covered);
            covered = to;
            chunks++;
            kb_media_plan_ack(&plan, from, to);
        }
        CHECK_EQ(chunks, 4);
        CHECK_EQ(covered, 4096);
        CHECK_EQ(kb_media_plan_done(&plan), 1);

        /* Хвост короче куска — отдельный последний диапазон. */
        kb_media_plan_init(&plan, 2500, 1024, 0);
        chunks = 0;
        while (kb_media_plan_next(&plan, &from, &to)) {
            chunks++;
        }
        CHECK_EQ(chunks, 3);
        CHECK_EQ(to, 2500);

        /* Докачка: первые байты сервер уже принял. */
        kb_media_plan_init(&plan, 4096, 1024, 3000);
        CHECK_EQ(kb_media_plan_next(&plan, &from, &to), 1);
        CHECK_EQ(from, 3000);
        CHECK_EQ(to, 4024);
        CHECK_EQ(kb_media_plan_ack(&plan, from, to), 4024);
        CHECK_EQ(kb_media_plan_next(&plan, &from, &to), 1);
        CHECK_EQ(from, 4024);
        CHECK_EQ(to, 4096);
        CHECK_EQ(kb_media_plan_ack(&plan, from, to), 4096);
        CHECK_EQ(kb_media_plan_next(&plan, &from, &to), 0);
        CHECK_EQ(kb_media_plan_done(&plan), 1);

        /* Файл уже целиком у сервера — качать нечего. */
        kb_media_plan_init(&plan, 4096, 1024, 4096);
        CHECK_EQ(kb_media_plan_next(&plan, &from, &to), 0);
        CHECK_EQ(kb_media_plan_done(&plan), 1);

        /* Пустой файл. */
        kb_media_plan_init(&plan, 0, 1024, 0);
        CHECK_EQ(kb_media_plan_next(&plan, &from, &to), 0);
        CHECK_EQ(kb_media_plan_done(&plan), 1);

        /* Смещение больше файла — не доверяем, начинаем с нуля. */
        kb_media_plan_init(&plan, 100, 1024, 999);
        CHECK_EQ(plan.offset, 0);
        CHECK_EQ(kb_media_plan_next(&plan, &from, &to), 1);
        CHECK_EQ(from, 0);
        CHECK_EQ(to, 100);

        /* Кусок больше файла — один диапазон, без переполнения. */
        kb_media_plan_init(&plan, 10, ~(uint64_t)0, 0);
        CHECK_EQ(kb_media_plan_next(&plan, &from, &to), 1);
        CHECK_EQ(from, 0);
        CHECK_EQ(to, 10);

        CHECK_EQ(kb_media_plan_init(&plan, 10, 0, 0), KB_ERR_INVALID);
        CHECK_EQ(kb_media_plan_init(NULL, 10, 10, 0), KB_ERR_INVALID);
        CHECK_EQ(kb_media_plan_next(NULL, &from, &to), 0);
        CHECK_EQ(kb_media_plan_done(NULL), 0);
    }

    KB_TEST("раздача кусков нескольким качальщикам");
    {
        /* Два «потока» тянут из одного плана: диапазоны не должны пересечься
           и обязаны покрыть файл целиком. */
        kb_media_plan plan;
        unsigned char covered[4096];
        uint64_t from = 0;
        uint64_t to = 0;
        int worker = 0;
        size_t i;

        memset(covered, 0, sizeof covered);
        kb_media_plan_init(&plan, sizeof covered, 700, 0);
        while (kb_media_plan_next(&plan, &from, &to)) {
            for (i = (size_t)from; i < (size_t)to; i++) {
                CHECK_EQ(covered[i], 0); /* никто не взял этот байт дважды */
                covered[i] = (unsigned char)(1 + (worker & 1));
            }
            kb_media_plan_ack(&plan, from, to);
            worker++;
        }
        for (i = 0; i < sizeof covered; i++) {
            CHECK(covered[i] != 0);
        }
        CHECK_EQ(kb_media_plan_done(&plan), 1);
    }

    KB_TEST("успешные коды ответа");
    {
        CHECK_EQ(kb_media_status_ok(200), 1);
        CHECK_EQ(kb_media_status_ok(201), 1);
        CHECK_EQ(kb_media_status_ok(204), 0);
        CHECK_EQ(kb_media_status_ok(500), 0);
        CHECK_EQ(kb_media_status_ok(0), 0);
    }

    KB_TEST("аргументы");
    {
        kb_http_header headers[KB_MEDIA_HEADERS_MAX];
        size_t count = 0;
        kb_http_response r;

        CHECK_EQ(kb_url_parse(NULL, &url), KB_ERR_INVALID);
        CHECK_EQ(kb_http_build_head(NULL, "/", NULL, 0, scratch, 10, &len),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_http_response_init(&r, NULL, 100), KB_ERR_INVALID);
        CHECK_EQ(kb_http_response_init(&r, scratch, 10), KB_ERR_INVALID);
        CHECK_EQ(kb_media_file_headers(NULL, "f", 1, "u", headers, &count,
                                       storage, sizeof storage),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_media_file_headers(&url, "f", 1, "u", headers, &count,
                                       storage, 10),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_http_response_done(NULL), 0);
        CHECK_EQ(kb_http_response_status(NULL), 0);
    }
}
