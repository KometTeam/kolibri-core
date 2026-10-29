/*
 * Загрузка медиа: разбор URL, сборка HTTP-запроса, разбор ответа.
 *
 * Перенесено с эталонной реализации (kolibri-net, src/media/http.rs и
 * upload.rs). Отличие то же, что у прокси: там обмен блокирующий, здесь —
 * машина состояний, поэтому разбор ответа проверяется тестами без сети.
 */

#include <string.h>

#include "kolibri.h"
#include "kolibri_media.h"

/* ─── Мелкие помощники ─────────────────────────────────────────────────── */

static int append_str(char *dst, size_t *at, size_t cap, const char *text)
{
    while (*text != '\0') {
        if (*at + 1 >= cap) {
            return 0;
        }
        dst[(*at)++] = *text++;
    }
    dst[*at] = '\0';
    return 1;
}

static int append_uint(char *dst, size_t *at, size_t cap, uint64_t value)
{
    char tmp[24];
    size_t n = 0;

    if (value == 0) {
        tmp[n++] = '0';
    }
    while (value > 0 && n < sizeof tmp) {
        tmp[n++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (n > 0) {
        if (*at + 1 >= cap) {
            return 0;
        }
        dst[(*at)++] = tmp[--n];
    }
    dst[*at] = '\0';
    return 1;
}

static int ascii_lower(int c)
{
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

static const uint8_t *find_bytes(const uint8_t *hay, size_t hay_len,
                                 const char *needle, size_t needle_len)
{
    size_t i;

    if (needle_len == 0 || hay_len < needle_len) {
        return NULL;
    }
    for (i = 0; i + needle_len <= hay_len; i++) {
        if (memcmp(hay + i, needle, needle_len) == 0) {
            return hay + i;
        }
    }
    return NULL;
}

/* ─── URL ──────────────────────────────────────────────────────────────── */

kb_status kb_url_parse(const char *url, kb_url *out)
{
    const char *scheme_end;
    const char *rest;
    const char *slash;
    const char *colon;
    const char *authority_end;
    size_t host_len;
    size_t path_len;

    if (url == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    scheme_end = strstr(url, "://");
    if (scheme_end == NULL) {
        return KB_ERR_INVALID;
    }
    if ((size_t)(scheme_end - url) == 5 && strncmp(url, "https", 5) == 0) {
        out->https = 1;
        out->port = 443;
    } else if ((size_t)(scheme_end - url) == 4 && strncmp(url, "http", 4) == 0) {
        out->https = 0;
        out->port = 80;
    } else {
        return KB_ERR_INVALID;
    }

    rest = scheme_end + 3;
    slash = strchr(rest, '/');
    authority_end = slash != NULL ? slash : rest + strlen(rest);

    /* Порт отделяется последним двоеточием и только если это цифры: иначе
       двоеточие принадлежит чему-то ещё. */
    colon = NULL;
    {
        const char *p;
        for (p = rest; p < authority_end; p++) {
            if (*p == ':') {
                colon = p;
            }
        }
    }
    if (colon != NULL) {
        const char *p;
        long port = 0;
        int digits = 0;

        for (p = colon + 1; p < authority_end; p++) {
            if (*p < '0' || *p > '9') {
                digits = 0;
                break;
            }
            port = port * 10 + (*p - '0');
            digits++;
            if (port > 65535) {
                digits = 0;
                break;
            }
        }
        if (digits > 0) {
            out->port = (uint16_t)port;
            authority_end = colon;
        }
    }

    host_len = (size_t)(authority_end - rest);
    if (host_len == 0 || host_len >= sizeof out->host) {
        return KB_ERR_INVALID;
    }
    memcpy(out->host, rest, host_len);
    out->host[host_len] = '\0';

    if (slash == NULL) {
        out->path[0] = '/';
        out->path[1] = '\0';
        return KB_OK;
    }
    path_len = strlen(slash);
    if (path_len >= sizeof out->path) {
        return KB_ERR_TOO_LARGE;
    }
    memcpy(out->path, slash, path_len + 1);
    return KB_OK;
}

size_t kb_percent_encode(const char *input, char *out, size_t out_cap)
{
    static const char hex[] = "0123456789ABCDEF";
    size_t at = 0;
    const unsigned char *p;

    if (input == NULL || out == NULL || out_cap == 0) {
        return 0;
    }
    for (p = (const unsigned char *)input; *p != '\0'; p++) {
        unsigned char b = *p;
        int plain = (b >= 'A' && b <= 'Z') || (b >= 'a' && b <= 'z') ||
                    (b >= '0' && b <= '9') || b == '-' || b == '_' ||
                    b == '.' || b == '!' || b == '~' || b == '*' ||
                    b == '\'' || b == '(' || b == ')';
        if (plain) {
            if (at + 1 >= out_cap) {
                return 0;
            }
            out[at++] = (char)b;
        } else {
            if (at + 3 >= out_cap) {
                return 0;
            }
            out[at++] = '%';
            out[at++] = hex[b >> 4];
            out[at++] = hex[b & 0x0F];
        }
    }
    out[at] = '\0';
    return at;
}

const char *kb_content_type_for_filename(const char *filename)
{
    const char *dot;
    char ext[8];
    size_t i;

    if (filename == NULL) {
        return "image/jpeg";
    }
    dot = strrchr(filename, '.');
    if (dot == NULL || dot[1] == '\0') {
        return "image/jpeg";
    }
    dot++;
    for (i = 0; i + 1 < sizeof ext && dot[i] != '\0'; i++) {
        ext[i] = (char)ascii_lower((unsigned char)dot[i]);
    }
    ext[i] = '\0';

    if (strcmp(ext, "png") == 0) {
        return "image/png";
    }
    if (strcmp(ext, "gif") == 0) {
        return "image/gif";
    }
    if (strcmp(ext, "webp") == 0) {
        return "image/webp";
    }
    if (strcmp(ext, "heic") == 0 || strcmp(ext, "heif") == 0) {
        return "image/heic";
    }
    if (strcmp(ext, "bmp") == 0) {
        return "image/bmp";
    }
    return "image/jpeg";
}

/* ─── Запрос ───────────────────────────────────────────────────────────── */

kb_status kb_http_build_head(const char *method, const char *path,
                             const kb_http_header *headers, size_t count,
                             uint8_t *out, size_t cap, size_t *out_len)
{
    char *dst = (char *)out;
    size_t at = 0;
    size_t i;

    if (method == NULL || path == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (headers == NULL && count != 0) {
        return KB_ERR_INVALID;
    }

    if (!append_str(dst, &at, cap, method) ||
        !append_str(dst, &at, cap, " ") ||
        !append_str(dst, &at, cap, path) ||
        !append_str(dst, &at, cap, " HTTP/1.1\r\n")) {
        return KB_ERR_TOO_LARGE;
    }
    for (i = 0; i < count; i++) {
        if (headers[i].name == NULL || headers[i].value == NULL) {
            return KB_ERR_INVALID;
        }
        if (!append_str(dst, &at, cap, headers[i].name) ||
            !append_str(dst, &at, cap, ": ") ||
            !append_str(dst, &at, cap, headers[i].value) ||
            !append_str(dst, &at, cap, "\r\n")) {
            return KB_ERR_TOO_LARGE;
        }
    }
    if (!append_str(dst, &at, cap, "\r\n")) {
        return KB_ERR_TOO_LARGE;
    }
    if (out_len != NULL) {
        *out_len = at;
    }
    return KB_OK;
}

/* ─── Ответ ────────────────────────────────────────────────────────────── */

kb_status kb_http_response_init(kb_http_response *r, uint8_t *buffer,
                                size_t capacity)
{
    if (r == NULL || buffer == NULL || capacity < 64) {
        return KB_ERR_INVALID;
    }
    memset(r, 0, sizeof *r);
    r->buf = buffer;
    r->cap = capacity;
    r->content_length = -1;
    return KB_OK;
}

/* Разбор строки статуса и интересных заголовков. */
static void parse_header(kb_http_response *r)
{
    const char *text = (const char *)r->buf;
    size_t end = r->header_end;
    size_t i = 0;
    size_t line_start;

    /* Код ответа — второй токен первой строки. */
    while (i < end && text[i] != ' ' && text[i] != '\r') {
        i++;
    }
    while (i < end && text[i] == ' ') {
        i++;
    }
    {
        unsigned status = 0;
        int digits = 0;
        while (i < end && text[i] >= '0' && text[i] <= '9') {
            status = status * 10 + (unsigned)(text[i] - '0');
            i++;
            digits++;
            if (status > 65535) {
                digits = 0;
                break;
            }
        }
        r->status = digits > 0 ? (uint16_t)status : 0;
    }

    /* Дальше построчно ищем Content-Length и Transfer-Encoding. */
    while (i < end && text[i] != '\n') {
        i++;
    }
    line_start = i + 1;

    while (line_start < end) {
        size_t line_end = line_start;
        size_t colon;

        while (line_end < end && text[line_end] != '\r' &&
               text[line_end] != '\n') {
            line_end++;
        }
        if (line_end == line_start) {
            break;
        }
        colon = line_start;
        while (colon < line_end && text[colon] != ':') {
            colon++;
        }
        if (colon < line_end) {
            size_t name_len = colon - line_start;
            size_t value_at = colon + 1;

            while (value_at < line_end && (text[value_at] == ' ' ||
                                           text[value_at] == '\t')) {
                value_at++;
            }
            if (name_len == 14 &&
                ascii_lower((unsigned char)text[line_start]) == 'c') {
                char name[15];
                size_t k;
                for (k = 0; k < 14; k++) {
                    name[k] = (char)ascii_lower((unsigned char)text[line_start + k]);
                }
                name[14] = '\0';
                if (strcmp(name, "content-length") == 0) {
                    long value = 0;
                    size_t p = value_at;
                    int digits = 0;
                    while (p < line_end && text[p] >= '0' && text[p] <= '9') {
                        value = value * 10 + (text[p] - '0');
                        p++;
                        digits++;
                    }
                    if (digits > 0) {
                        r->content_length = value;
                    }
                }
            }
            if (name_len == 17) {
                char name[18];
                size_t k;
                for (k = 0; k < 17; k++) {
                    name[k] = (char)ascii_lower((unsigned char)text[line_start + k]);
                }
                name[17] = '\0';
                if (strcmp(name, "transfer-encoding") == 0) {
                    size_t p;
                    for (p = value_at; p + 7 <= line_end; p++) {
                        char word[8];
                        size_t k2;
                        for (k2 = 0; k2 < 7; k2++) {
                            word[k2] = (char)ascii_lower((unsigned char)text[p + k2]);
                        }
                        word[7] = '\0';
                        if (strcmp(word, "chunked") == 0) {
                            r->chunked = 1;
                            break;
                        }
                    }
                }
            }
        }
        line_start = line_end;
        while (line_start < end && (text[line_start] == '\r' ||
                                    text[line_start] == '\n')) {
            line_start++;
        }
    }
    r->header_parsed = 1;
    r->body_at = r->header_end;
}

/* Раскодировать chunked прямо в буфере: результат всегда короче исходного. */
static void decode_chunked(kb_http_response *r)
{
    uint8_t *body = r->buf + r->header_end;
    size_t len = r->len - r->header_end;
    size_t read_at = 0;
    size_t write_at = 0;

    while (read_at < len) {
        size_t size = 0;
        int digits = 0;
        const uint8_t *eol;

        eol = find_bytes(body + read_at, len - read_at, "\r\n", 2);
        if (eol == NULL) {
            break;
        }
        while (read_at < (size_t)(eol - body)) {
            int c = ascii_lower(body[read_at]);
            int v;
            if (c >= '0' && c <= '9') {
                v = c - '0';
            } else if (c >= 'a' && c <= 'f') {
                v = c - 'a' + 10;
            } else {
                break; /* расширение после ';' или мусор */
            }
            size = size * 16 + (size_t)v;
            read_at++;
            digits++;
        }
        read_at = (size_t)(eol - body) + 2;
        if (digits == 0 || size == 0) {
            break;
        }
        if (read_at + size > len) {
            break;
        }
        memmove(body + write_at, body + read_at, size);
        write_at += size;
        read_at += size;
        if (read_at + 2 <= len && body[read_at] == '\r' &&
            body[read_at + 1] == '\n') {
            read_at += 2;
        }
    }
    r->body_len = write_at;
}

/* Готов ли ответ целиком; at_close — соединение закрылось. */
static void try_complete(kb_http_response *r, int at_close)
{
    size_t body_available;

    if (!r->header_parsed) {
        const uint8_t *end = find_bytes(r->buf, r->len, "\r\n\r\n", 4);
        if (end == NULL) {
            return;
        }
        r->header_end = (size_t)(end - r->buf) + 4;
        parse_header(r);
    }

    body_available = r->len - r->header_end;

    if (r->chunked) {
        if (!at_close &&
            find_bytes(r->buf + r->header_end, body_available, "0\r\n\r\n", 5) ==
                NULL) {
            return;
        }
        decode_chunked(r);
        r->done = 1;
        return;
    }
    if (r->content_length >= 0) {
        if (!at_close && body_available < (size_t)r->content_length) {
            return;
        }
        r->body_len = body_available < (size_t)r->content_length
                          ? body_available
                          : (size_t)r->content_length;
        r->done = 1;
        return;
    }
    /* Ни длины, ни chunked: тело кончается вместе с соединением. */
    if (at_close) {
        r->body_len = body_available;
        r->done = 1;
    }
}

kb_status kb_http_response_feed(kb_http_response *r, const uint8_t *data,
                                size_t len)
{
    if (r == NULL || r->buf == NULL || (data == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    if (r->done) {
        return KB_OK;
    }
    if (len > r->cap - r->len) {
        return KB_ERR_TOO_LARGE;
    }
    if (len > 0) {
        memcpy(r->buf + r->len, data, len);
        r->len += len;
    }
    try_complete(r, 0);
    return KB_OK;
}

kb_status kb_http_response_finish(kb_http_response *r)
{
    if (r == NULL || r->buf == NULL) {
        return KB_ERR_INVALID;
    }
    if (r->done) {
        return KB_OK;
    }
    try_complete(r, 1);
    return r->done ? KB_OK : KB_ERR_TRUNCATED;
}

int kb_http_response_done(const kb_http_response *r)
{
    return r != NULL && r->done;
}

uint16_t kb_http_response_status(const kb_http_response *r)
{
    return r == NULL ? 0 : r->status;
}

const uint8_t *kb_http_response_body(const kb_http_response *r, size_t *len)
{
    if (r == NULL || !r->done) {
        if (len != NULL) {
            *len = 0;
        }
        return NULL;
    }
    if (len != NULL) {
        *len = r->body_len;
    }
    return r->buf + r->body_at;
}

/* ─── Заголовки трёх режимов ───────────────────────────────────────────── */

/*
 * Маленький распределитель поверх storage: значения заголовков должны пережить
 * вызов, а выделять память ядру нельзя.
 */
typedef struct {
    char *buf;
    size_t cap;
    size_t at;
    int failed;
} kb_arena;

static void arena_init(kb_arena *a, char *buf, size_t cap)
{
    a->buf = buf;
    a->cap = cap;
    a->at = 0;
    a->failed = 0;
}

/* Начать новую строку в арене; возвращает её начало. */
static char *arena_begin(kb_arena *a)
{
    if (a->failed || a->at >= a->cap) {
        a->failed = 1;
        return NULL;
    }
    return a->buf + a->at;
}

static void arena_end(kb_arena *a, size_t written)
{
    a->at += written + 1; /* вместе с нулевым байтом */
    if (a->at > a->cap) {
        a->failed = 1;
    }
}

static const char *arena_str(kb_arena *a, const char *text)
{
    char *start = arena_begin(a);
    size_t at = 0;

    if (start == NULL) {
        return "";
    }
    if (!append_str(start, &at, a->cap - a->at, text)) {
        a->failed = 1;
        return "";
    }
    arena_end(a, at);
    return start;
}

static const char *arena_uint(kb_arena *a, uint64_t value)
{
    char *start = arena_begin(a);
    size_t at = 0;

    if (start == NULL) {
        return "";
    }
    if (!append_uint(start, &at, a->cap - a->at, value)) {
        a->failed = 1;
        return "";
    }
    arena_end(a, at);
    return start;
}

/* Склейка нескольких кусков в одну строку арены. */
static const char *arena_join(kb_arena *a, const char *parts[], size_t count)
{
    char *start = arena_begin(a);
    size_t at = 0;
    size_t i;

    if (start == NULL) {
        return "";
    }
    for (i = 0; i < count; i++) {
        if (!append_str(start, &at, a->cap - a->at, parts[i])) {
            a->failed = 1;
            return "";
        }
    }
    arena_end(a, at);
    return start;
}

static const char *arena_percent(kb_arena *a, const char *text)
{
    char *start = arena_begin(a);
    size_t written;

    if (start == NULL) {
        return "";
    }
    written = kb_percent_encode(text, start, a->cap - a->at);
    if (written == 0 && text[0] != '\0') {
        a->failed = 1;
        return "";
    }
    arena_end(a, written);
    return start;
}

kb_status kb_media_content_range(uint64_t from, uint64_t to, uint64_t total,
                                 char *out, size_t cap)
{
    size_t at = 0;

    if (out == NULL) {
        return KB_ERR_INVALID;
    }
    if (!append_str(out, &at, cap, "bytes ") ||
        !append_uint(out, &at, cap, from) ||
        !append_str(out, &at, cap, "-") ||
        !append_uint(out, &at, cap, to) ||
        !append_str(out, &at, cap, "/") ||
        !append_uint(out, &at, cap, total)) {
        return KB_ERR_TOO_LARGE;
    }
    return KB_OK;
}

kb_status kb_media_file_headers(const kb_url *url, const char *filename,
                                size_t body_len, const char *user_agent,
                                kb_http_header *headers, size_t *out_count,
                                char *storage, size_t storage_cap)
{
    kb_arena arena;
    size_t n = 0;
    const char *parts[2];
    char range[80];

    if (url == NULL || filename == NULL || headers == NULL ||
        storage == NULL || storage_cap < KB_MEDIA_STORAGE_MIN) {
        return KB_ERR_INVALID;
    }
    arena_init(&arena, storage, storage_cap);

    headers[n].name = "Host";
    headers[n++].value = arena_str(&arena, url->host);

    headers[n].name = "Content-Type";
    headers[n++].value = "application/x-binary; charset=x-user-defined";

    parts[0] = "attachment; filename=";
    parts[1] = filename;
    headers[n].name = "Content-Disposition";
    headers[n++].value = arena_join(&arena, parts, 2);

    headers[n].name = "Connection";
    headers[n++].value = "keep-alive";

    headers[n].name = "User-Agent";
    headers[n++].value = arena_percent(&arena, user_agent ? user_agent : "");

    /* Диапазон покрывает всё тело; для пустого тела верхняя граница 0. */
    if (kb_media_content_range(0, body_len > 0 ? body_len - 1 : 0,
                               (uint64_t)body_len, range, sizeof range) != KB_OK) {
        return KB_ERR_TOO_LARGE;
    }
    headers[n].name = "Content-Range";
    headers[n++].value = arena_str(&arena, range);

    headers[n].name = "Content-Length";
    headers[n++].value = arena_uint(&arena, (uint64_t)body_len);

    if (arena.failed) {
        return KB_ERR_TOO_LARGE;
    }
    if (out_count != NULL) {
        *out_count = n;
    }
    return KB_OK;
}

kb_status kb_media_photo_headers(const kb_url *url, const char *filename,
                                 size_t file_len, const char *user_agent,
                                 uint64_t boundary_seed, kb_http_header *headers,
                                 size_t *out_count, char *storage,
                                 size_t storage_cap, char *prologue,
                                 size_t prologue_cap, size_t *prologue_len,
                                 char *epilogue, size_t epilogue_cap,
                                 size_t *epilogue_len)
{
    kb_arena arena;
    size_t n = 0;
    size_t at;
    char boundary[64];
    const char *parts[3];

    if (url == NULL || filename == NULL || headers == NULL || storage == NULL ||
        prologue == NULL || epilogue == NULL ||
        storage_cap < KB_MEDIA_STORAGE_MIN) {
        return KB_ERR_INVALID;
    }
    arena_init(&arena, storage, storage_cap);

    at = 0;
    if (!append_str(boundary, &at, sizeof boundary, "----KolibriBoundary") ||
        !append_uint(boundary, &at, sizeof boundary,
                     boundary_seed & 0x7FFFFFFFu)) {
        return KB_ERR_TOO_LARGE;
    }

    /* Обрамление: до файла и после него. */
    at = 0;
    if (!append_str(prologue, &at, prologue_cap, "--") ||
        !append_str(prologue, &at, prologue_cap, boundary) ||
        !append_str(prologue, &at, prologue_cap,
                    "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"") ||
        !append_str(prologue, &at, prologue_cap, filename) ||
        !append_str(prologue, &at, prologue_cap, "\"\r\nContent-Type: ") ||
        !append_str(prologue, &at, prologue_cap,
                    kb_content_type_for_filename(filename)) ||
        !append_str(prologue, &at, prologue_cap, "\r\n\r\n")) {
        return KB_ERR_TOO_LARGE;
    }
    if (prologue_len != NULL) {
        *prologue_len = at;
    }
    {
        size_t prologue_size = at;

        at = 0;
        if (!append_str(epilogue, &at, epilogue_cap, "\r\n--") ||
            !append_str(epilogue, &at, epilogue_cap, boundary) ||
            !append_str(epilogue, &at, epilogue_cap, "--\r\n")) {
            return KB_ERR_TOO_LARGE;
        }
        if (epilogue_len != NULL) {
            *epilogue_len = at;
        }

        headers[n].name = "Host";
        headers[n++].value = arena_str(&arena, url->host);

        parts[0] = "multipart/form-data; boundary=";
        parts[1] = boundary;
        headers[n].name = "Content-Type";
        headers[n++].value = arena_join(&arena, parts, 2);

        /* Длина — сумма обрамления и самого файла. Порядок заголовков здесь
           повторяет официальный клиент: он тоже часть отпечатка. */
        headers[n].name = "Content-Length";
        headers[n++].value = arena_uint(
            &arena, (uint64_t)(prologue_size + file_len + at));

        headers[n].name = "Connection";
        headers[n++].value = "keep-alive";

        headers[n].name = "User-Agent";
        headers[n++].value = arena_percent(&arena, user_agent ? user_agent : "");
    }

    if (arena.failed) {
        return KB_ERR_TOO_LARGE;
    }
    if (out_count != NULL) {
        *out_count = n;
    }
    return KB_OK;
}

kb_status kb_media_chunk_headers(const kb_url *url, const char *filename,
                                 size_t body_len, const char *content_range,
                                 kb_http_header *headers, size_t *out_count,
                                 char *storage, size_t storage_cap)
{
    kb_arena arena;
    size_t n = 0;
    const char *parts[3];

    if (url == NULL || filename == NULL || headers == NULL ||
        storage == NULL || storage_cap < KB_MEDIA_STORAGE_MIN) {
        return KB_ERR_INVALID;
    }
    arena_init(&arena, storage, storage_cap);

    headers[n].name = "Host";
    headers[n++].value = arena_str(&arena, url->host);

    headers[n].name = "Content-Type";
    headers[n++].value = "application/x-binary; charset=x-user-defined";

    /* У кусков имя в кавычках и с заглавной N — так шлёт эталон. */
    parts[0] = "attachment; fileName=\"";
    parts[1] = filename;
    parts[2] = "\"";
    headers[n].name = "Content-Disposition";
    headers[n++].value = arena_join(&arena, parts, 3);

    headers[n].name = "Content-Length";
    headers[n++].value = arena_uint(&arena, (uint64_t)body_len);

    headers[n].name = "X-Uploading-Mode";
    headers[n++].value = "parallel";

    headers[n].name = "Connection";
    headers[n++].value = "close";

    if (content_range != NULL) {
        headers[n].name = "Content-Range";
        headers[n++].value = arena_str(&arena, content_range);
    }

    if (arena.failed) {
        return KB_ERR_TOO_LARGE;
    }
    if (out_count != NULL) {
        *out_count = n;
    }
    return KB_OK;
}

/* ------------------------------------------------------------------ */
/* Раскладка файла на куски и докачка                                  */
/* ------------------------------------------------------------------ */

uint64_t kb_media_parse_resume(const uint8_t *body, size_t len, uint64_t total)
{
    /* Не UINT64_MAX: его литерал имеет тип long long, а это уже не C89. */
    const uint64_t limit = ~(uint64_t)0;
    uint64_t value = 0;
    size_t i = 0;
    size_t end = len;
    int digits = 0;

    if (body == NULL) {
        return 0;
    }
    /* Эталон делает trim перед разбором: пробелы по краям — не ошибка. */
    while (i < end && (body[i] == ' ' || body[i] == '\t' || body[i] == '\r' ||
                       body[i] == '\n')) {
        i++;
    }
    while (end > i && (body[end - 1] == ' ' || body[end - 1] == '\t' ||
                       body[end - 1] == '\r' || body[end - 1] == '\n')) {
        end--;
    }
    for (; i < end; i++) {
        if (body[i] < '0' || body[i] > '9') {
            return 0; /* не число — начинаем сначала */
        }
        /* Переполнение означает заведомо больше файла: тоже сначала. */
        if (value > (limit - (uint64_t)(body[i] - '0')) / 10u) {
            return 0;
        }
        value = value * 10u + (uint64_t)(body[i] - '0');
        digits++;
    }
    if (digits == 0 || value > total) {
        return 0;
    }
    return value;
}

kb_status kb_media_plan_init(kb_media_plan *plan, uint64_t total,
                             uint64_t chunk_size, uint64_t start_offset)
{
    if (plan == NULL || chunk_size == 0) {
        return KB_ERR_INVALID;
    }
    if (start_offset > total) {
        start_offset = 0;
    }
    plan->total = total;
    plan->chunk_size = chunk_size;
    plan->offset = start_offset;
    plan->next = start_offset;
    plan->sent = start_offset;
    return KB_OK;
}

int kb_media_plan_next(kb_media_plan *plan, uint64_t *from, uint64_t *to)
{
    uint64_t start;
    uint64_t end;

    if (plan == NULL || plan->next >= plan->total) {
        return 0;
    }
    start = plan->next;
    end = start + plan->chunk_size;
    if (end > plan->total || end < start) {
        end = plan->total;
    }
    plan->next = end;
    if (from != NULL) {
        *from = start;
    }
    if (to != NULL) {
        *to = end;
    }
    return 1;
}

uint64_t kb_media_plan_ack(kb_media_plan *plan, uint64_t from, uint64_t to)
{
    if (plan == NULL || to <= from || to > plan->total) {
        return plan == NULL ? 0 : plan->sent;
    }
    plan->sent += to - from;
    if (plan->sent > plan->total) {
        plan->sent = plan->total;
    }
    return plan->sent;
}

int kb_media_plan_done(const kb_media_plan *plan)
{
    return plan != NULL && plan->sent >= plan->total;
}

int kb_media_status_ok(int status)
{
    return status == 200 || status == 201;
}
