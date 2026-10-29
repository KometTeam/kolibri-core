/*
 * Драйвер загрузки медиа поверх sans-IO кирпичей (аналог upload_file из
 * kolibri-net): одиночный POST с Content-Range на весь файл.
 *
 *   upload <url> <файл> [--insecure]
 *
 * Показывает связку: kb_url_parse -> kb_transport_connect -> (TLS) ->
 * kb_media_file_headers -> kb_http_build_head -> запись тела -> kb_http_response.
 * Ядро в сеть не ходит; сокет и цикл — здесь.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kolibri_media.h"
#include "kolibri_transport.h"
#if defined(KB_WITH_MBEDTLS)
#include "kolibri_tls_mbedtls.h"
#include "kolibri_ca.h"
#endif

static int wait_ready(kb_transport *t, int for_write) {
    return kb_transport_wait(t, for_write, 15000) == 1;
}
static int write_all(kb_transport *t, const uint8_t *p, size_t n) {
    size_t off = 0;
    while (off < n) {
        long w = kb_transport_write(t, p + off, n - off);
        if (w == KB_IO_WANT_WRITE || w == KB_IO_WANT_READ) {
            if (!wait_ready(t, w == KB_IO_WANT_WRITE)) return -1;
            continue;
        }
        if (w < 0) return -1;
        off += (size_t)w;
    }
    return 0;
}

/* Загрузить буфер как файл. Возвращает HTTP-код (>0) или -1. */
static int upload_file(const char *url_s, const char *filename,
                       const uint8_t *data, size_t len,
                       const char *user_agent, int insecure) {
    kb_url url;
    kb_transport t;
    kb_transport_config tc;
    kb_http_header headers[KB_MEDIA_HEADERS_MAX];
    size_t count = 0, head_len = 0;
    char storage[2048];
    unsigned char head[2048];
    unsigned char rxbuf[8192];
    kb_http_response resp;
    unsigned char respbuf[65536];

    if (kb_url_parse(url_s, &url) != KB_OK) { fprintf(stderr, "плохой URL\n"); return -1; }
    memset(&tc, 0, sizeof tc);
    tc.host = url.host; tc.port = url.port; tc.connect_timeout_ms = 15000;
    if (kb_transport_connect(&t, &tc) != KB_OK) {
        fprintf(stderr, "connect: %s\n", kb_transport_detail(&t)); return -1;
    }
    if (url.https) {
#if defined(KB_WITH_MBEDTLS)
        static double st[8192]; kb_tls tls; size_t calen = 0;
        const char *ca = insecure ? NULL : kb_mincifry_ca_pem(&calen);
        int ww = 0; kb_status hs;
        if (kb_tls_mbedtls_init(&tls, st, sizeof st, t.fd, url.host, ca, calen, insecure) != KB_OK ||
            kb_transport_set_tls(&t, &tls) != KB_OK) {
            fprintf(stderr, "tls init: %s\n", kb_transport_detail(&t)); kb_transport_close(&t); return -1;
        }
        for (;;) {
            hs = kb_transport_handshake(&t, &ww);
            if (hs == KB_OK) break;
            if (hs != KB_IO_WANT_READ && hs != KB_IO_WANT_WRITE) {
                fprintf(stderr, "tls handshake: %s\n", kb_transport_detail(&t)); kb_transport_close(&t); return -1;
            }
            if (!wait_ready(&t, hs == KB_IO_WANT_WRITE)) { kb_transport_close(&t); return -1; }
        }
#else
        fprintf(stderr, "https требует сборки с KB_WITH_MBEDTLS\n"); kb_transport_close(&t); return -1;
#endif
    }

    /* Заголовки одиночного файла (Content-Range на весь размер) + запрос. */
    if (kb_media_file_headers(&url, filename, len, user_agent, headers, &count,
                              storage, sizeof storage) != KB_OK) {
        fprintf(stderr, "заголовки\n"); kb_transport_close(&t); return -1;
    }
    if (kb_http_build_head("POST", url.path, headers, count, head, sizeof head, &head_len) != KB_OK) {
        fprintf(stderr, "сборка запроса\n"); kb_transport_close(&t); return -1;
    }
    if (write_all(&t, head, head_len) != 0 || write_all(&t, data, len) != 0) {
        fprintf(stderr, "запись\n"); kb_transport_close(&t); return -1;
    }

    /* Разбор ответа машиной kb_http_response. */
    kb_http_response_init(&resp, respbuf, sizeof respbuf);
    while (!kb_http_response_done(&resp)) {
        long n;
        if (!wait_ready(&t, 0)) { kb_http_response_finish(&resp); break; }
        n = kb_transport_read(&t, rxbuf, sizeof rxbuf);
        if (n == KB_IO_WANT_READ || n == KB_IO_WANT_WRITE) continue;
        if (n <= 0) { kb_http_response_finish(&resp); break; }
        if (kb_http_response_feed(&resp, rxbuf, (size_t)n) != KB_OK) break;
    }
    kb_transport_close(&t);
    if (!kb_http_response_done(&resp)) return -1;
    {
        size_t blen = 0; const unsigned char *body = kb_http_response_body(&resp, &blen);
        int code = kb_http_response_status(&resp);
        printf("HTTP %d, тело %zu Б: %.120s\n", code, blen, body ? (const char*)body : "");
        return code;
    }
}

int main(int argc, char **argv) {
    const char *url, *path; int insecure = 0; int i;
    FILE *f; long sz; unsigned char *data; int code;
    if (argc < 3) { fprintf(stderr, "usage: upload <url> <файл> [--insecure]\n"); return 2; }
    url = argv[1]; path = argv[2];
    for (i = 3; i < argc; i++) if (strcmp(argv[i], "--insecure") == 0) insecure = 1;
    f = fopen(path, "rb"); if (!f) { perror("open"); return 1; }
    fseek(f, 0, SEEK_END); sz = ftell(f); fseek(f, 0, SEEK_SET);
    data = (unsigned char *)malloc((size_t)sz); fread(data, 1, (size_t)sz, f); fclose(f);
    code = upload_file(url, "upload.bin", data, (size_t)sz,
                       "OKMessages/26.20.2 (Android 14)", insecure);
    free(data);
    return code > 0 && code < 400 ? 0 : 1;
}
