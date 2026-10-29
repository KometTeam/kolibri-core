/*
 * Рукопожатие с прокси, sans-IO.
 *
 * Перенесено с эталонной реализации (kolibri-net, src/transport/proxy.rs).
 * Отличие одно и намеренное: там рукопожатие блокирующее, здесь — машина
 * состояний. Байты на проводе те же.
 */

#include <string.h>

#include "kolibri.h"

/* Шаги HTTP CONNECT. */
#define HTTP_SEND_REQUEST 0
#define HTTP_READ_RESPONSE 1

/* Шаги SOCKS5. */
#define S5_SEND_GREETING 0
#define S5_READ_METHOD 1
#define S5_SEND_AUTH 2
#define S5_READ_AUTH 3
#define S5_SEND_CONNECT 4
#define S5_READ_REPLY 5
#define S5_READ_ADDR_LEN 6
#define S5_READ_ADDR 7

/* ─── Base64 ───────────────────────────────────────────────────────────── */

size_t kb_base64_encode(const uint8_t *data, size_t len, char *out,
                        size_t out_cap)
{
    static const char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t need = ((len + 2) / 3) * 4;
    size_t i = 0;
    size_t o = 0;

    if (out == NULL || out_cap < need + 1) {
        return 0;
    }
    while (i < len) {
        uint32_t n;
        size_t chunk = len - i < 3 ? len - i : 3;
        uint8_t b0 = data[i];
        uint8_t b1 = chunk > 1 ? data[i + 1] : 0;
        uint8_t b2 = chunk > 2 ? data[i + 2] : 0;

        n = ((uint32_t)b0 << 16) | ((uint32_t)b1 << 8) | (uint32_t)b2;
        out[o++] = alphabet[(n >> 18) & 63];
        out[o++] = alphabet[(n >> 12) & 63];
        out[o++] = chunk > 1 ? alphabet[(n >> 6) & 63] : '=';
        out[o++] = chunk > 2 ? alphabet[n & 63] : '=';
        i += chunk;
    }
    out[o] = '\0';
    return o;
}

/*
 * Base64-декод. Пробелы и переводы строк пропускаются, '=' завершает.
 * Возвращает KB_ERR_INVALID на посторонний символ или кривое выравнивание,
 * KB_ERR_TOO_LARGE — если результат не влезает в out_cap. out_len — число
 * записанных байт.
 */
kb_status kb_base64_decode(const char *text, size_t len, uint8_t *out,
                           size_t out_cap, size_t *out_len)
{
    uint32_t acc = 0;
    int bits = 0;
    size_t o = 0;
    size_t i;
    int padded = 0;

    if (text == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    for (i = 0; i < len; i++) {
        unsigned char c = (unsigned char)text[i];
        int val;
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            continue;
        }
        if (c == '=') {
            padded = 1;
            continue;
        }
        if (padded) {
            return KB_ERR_INVALID; /* данные после padding */
        }
        if (c >= 'A' && c <= 'Z') {
            val = c - 'A';
        } else if (c >= 'a' && c <= 'z') {
            val = c - 'a' + 26;
        } else if (c >= '0' && c <= '9') {
            val = c - '0' + 52;
        } else if (c == '+') {
            val = 62;
        } else if (c == '/') {
            val = 63;
        } else {
            return KB_ERR_INVALID;
        }
        acc = (acc << 6) | (uint32_t)val;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            if (o >= out_cap) {
                return KB_ERR_TOO_LARGE;
            }
            out[o++] = (uint8_t)((acc >> bits) & 0xFF);
        }
    }
    /* Остаток бит должен быть строго меньше байта — иначе вход обрезан. */
    if (bits >= 8) {
        return KB_ERR_INVALID;
    }
    if (out_len != NULL) {
        *out_len = o;
    }
    return KB_OK;
}

/* ─── Разбор URL ───────────────────────────────────────────────────────── */

static const char *find_last(const char *text, char needle)
{
    const char *found = NULL;
    while (*text != '\0') {
        if (*text == needle) {
            found = text;
        }
        text++;
    }
    return found;
}

kb_status kb_proxy_parse(const char *url, kb_proxy_config *out, char *storage,
                         size_t storage_len)
{
    const char *scheme_end;
    const char *rest;
    const char *at;
    const char *authority;
    const char *colon;
    size_t used = 0;
    long port = 0;
    const char *p;

    if (url == NULL || out == NULL || storage == NULL ||
        storage_len < KB_PROXY_URL_MAX) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);
    memset(storage, 0, storage_len);

    scheme_end = strstr(url, "://");
    if (scheme_end == NULL) {
        return KB_ERR_INVALID;
    }
    if (strncmp(url, "http", (size_t)(scheme_end - url)) == 0 &&
        (size_t)(scheme_end - url) == 4) {
        out->kind = KB_PROXY_HTTP;
    } else if ((size_t)(scheme_end - url) == 6 &&
               strncmp(url, "socks5", 6) == 0) {
        out->kind = KB_PROXY_SOCKS5;
    } else if ((size_t)(scheme_end - url) == 7 &&
               strncmp(url, "socks5h", 7) == 0) {
        out->kind = KB_PROXY_SOCKS5;
    } else {
        return KB_ERR_INVALID;
    }

    rest = scheme_end + 3;
    at = find_last(rest, '@');
    authority = at != NULL ? at + 1 : rest;

    /* Порт отделяется последним двоеточием — как в эталоне; IPv6-литералы в
       квадратных скобках он тоже не поддерживает. */
    colon = find_last(authority, ':');
    if (colon == NULL || colon[1] == '\0') {
        return KB_ERR_INVALID;
    }
    for (p = colon + 1; *p != '\0'; p++) {
        if (*p < '0' || *p > '9') {
            return KB_ERR_INVALID;
        }
        port = port * 10 + (*p - '0');
        if (port > 65535) {
            return KB_ERR_INVALID;
        }
    }
    out->port = (uint16_t)port;

    /* host */
    {
        size_t host_len = (size_t)(colon - authority);
        if (host_len == 0 || used + host_len + 1 > storage_len) {
            return KB_ERR_INVALID;
        }
        memcpy(storage + used, authority, host_len);
        out->host = storage + used;
        used += host_len + 1;
    }

    if (at != NULL) {
        size_t auth_len = (size_t)(at - rest);
        const char *sep = NULL;
        size_t i;

        for (i = 0; i < auth_len; i++) {
            if (rest[i] == ':') {
                sep = rest + i;
                break;
            }
        }
        {
            size_t user_len = sep != NULL ? (size_t)(sep - rest) : auth_len;
            if (used + user_len + 1 > storage_len) {
                return KB_ERR_INVALID;
            }
            memcpy(storage + used, rest, user_len);
            out->username = storage + used;
            used += user_len + 1;
        }
        if (sep != NULL) {
            size_t pass_len = auth_len - (size_t)(sep + 1 - rest);
            if (used + pass_len + 1 > storage_len) {
                return KB_ERR_INVALID;
            }
            memcpy(storage + used, sep + 1, pass_len);
            out->password = storage + used;
            used += pass_len + 1;
        }
    }
    return KB_OK;
}

/* ─── Машина рукопожатия ───────────────────────────────────────────────── */

static void fail(kb_proxy *p, kb_status status, const char *detail)
{
    p->step = KB_PROXY_STEP_FAILED;
    p->error = status;
    if (detail != NULL) {
        size_t len = strlen(detail);
        if (len >= sizeof p->detail) {
            len = sizeof p->detail - 1;
        }
        memcpy(p->detail, detail, len);
        p->detail[len] = '\0';
    }
}

static void set_out(kb_proxy *p, const uint8_t *data, size_t len)
{
    if (len > sizeof p->out) {
        fail(p, KB_ERR_TOO_LARGE, "proxy handshake message too long");
        return;
    }
    memcpy(p->out, data, len);
    p->out_len = len;
    p->out_sent = 0;
    p->step = KB_PROXY_STEP_WRITE;
}

static void expect(kb_proxy *p, size_t bytes)
{
    p->in_len = 0;
    p->need = bytes;
    p->step = KB_PROXY_STEP_READ;
}

/* Число в строку без stdio: на целевых платформах printf может быть дорог. */
static void append_uint(char *dst, size_t *at, size_t cap, unsigned long value)
{
    char tmp[12];
    size_t n = 0;

    if (value == 0) {
        tmp[n++] = '0';
    }
    while (value > 0 && n < sizeof tmp) {
        tmp[n++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (n > 0 && *at + 1 < cap) {
        dst[(*at)++] = tmp[--n];
    }
}

static void append_str(char *dst, size_t *at, size_t cap, const char *text)
{
    while (*text != '\0' && *at + 1 < cap) {
        dst[(*at)++] = *text++;
    }
}

static void build_http_request(kb_proxy *p, const kb_proxy_config *config,
                               const char *host, uint16_t port)
{
    char request[KB_PROXY_OUT_MAX];
    size_t at = 0;

    append_str(request, &at, sizeof request, "CONNECT ");
    append_str(request, &at, sizeof request, host);
    append_str(request, &at, sizeof request, ":");
    append_uint(request, &at, sizeof request, port);
    append_str(request, &at, sizeof request, " HTTP/1.1\r\nHost: ");
    append_str(request, &at, sizeof request, host);
    append_str(request, &at, sizeof request, ":");
    append_uint(request, &at, sizeof request, port);
    append_str(request, &at, sizeof request, "\r\n");

    if (config->username != NULL) {
        char pair[KB_PROXY_URL_MAX];
        /* base64 раздувает вход на треть: буфер под пару максимальной длины. */
        char token[(KB_PROXY_URL_MAX / 3 + 2) * 4];
        size_t pair_at = 0;

        append_str(pair, &pair_at, sizeof pair, config->username);
        append_str(pair, &pair_at, sizeof pair, ":");
        append_str(pair, &pair_at, sizeof pair,
                   config->password != NULL ? config->password : "");
        pair[pair_at] = '\0';

        if (kb_base64_encode((const uint8_t *)pair, pair_at, token,
                             sizeof token) == 0) {
            fail(p, KB_ERR_TOO_LARGE, "proxy credentials too long");
            return;
        }
        append_str(request, &at, sizeof request, "Proxy-Authorization: Basic ");
        append_str(request, &at, sizeof request, token);
        append_str(request, &at, sizeof request, "\r\n");
    }
    append_str(request, &at, sizeof request, "Proxy-Connection: keep-alive\r\n\r\n");

    if (at + 1 >= sizeof request) {
        fail(p, KB_ERR_TOO_LARGE, "proxy CONNECT request too long");
        return;
    }
    set_out(p, (const uint8_t *)request, at);
}

static void build_socks5_connect(kb_proxy *p, const char *host, uint16_t port)
{
    uint8_t request[KB_PROXY_OUT_MAX];
    size_t host_len = strlen(host);

    if (host_len > 255) {
        fail(p, KB_ERR_TOO_LARGE, "SOCKS5 target host too long");
        return;
    }
    request[0] = 0x05;
    request[1] = 0x01; /* CONNECT */
    request[2] = 0x00;
    request[3] = 0x03; /* доменное имя: резолвит прокси */
    request[4] = (uint8_t)host_len;
    memcpy(request + 5, host, host_len);
    request[5 + host_len] = (uint8_t)(port >> 8);
    request[6 + host_len] = (uint8_t)(port & 0xFF);
    set_out(p, request, 7 + host_len);
}

kb_status kb_proxy_init(kb_proxy *proxy, const kb_proxy_config *config,
                        const char *target_host, uint16_t target_port)
{
    size_t host_len;

    if (proxy == NULL || config == NULL || target_host == NULL) {
        return KB_ERR_INVALID;
    }
    if (config->kind != KB_PROXY_HTTP && config->kind != KB_PROXY_SOCKS5) {
        return KB_ERR_INVALID;
    }
    host_len = strlen(target_host);
    if (host_len == 0 || host_len > 255) {
        return KB_ERR_INVALID;
    }

    memset(proxy, 0, sizeof *proxy);
    proxy->config = *config;
    proxy->kind = config->kind;
    proxy->has_auth = config->username != NULL;
    proxy->error = KB_OK;
    strcpy(proxy->detail, "ok");
    memcpy(proxy->target_host, target_host, host_len + 1);
    proxy->target_port = target_port;

    if (config->kind == KB_PROXY_HTTP) {
        proxy->state = HTTP_SEND_REQUEST;
        build_http_request(proxy, config, target_host, target_port);
    } else {
        uint8_t greeting[4];
        proxy->state = S5_SEND_GREETING;
        if (proxy->has_auth) {
            greeting[0] = 0x05;
            greeting[1] = 0x02;
            greeting[2] = 0x00;
            greeting[3] = 0x02;
            set_out(proxy, greeting, 4);
        } else {
            greeting[0] = 0x05;
            greeting[1] = 0x01;
            greeting[2] = 0x00;
            set_out(proxy, greeting, 3);
        }
    }
    if (config->username != NULL) {
        size_t user_len = strlen(config->username);
        size_t pass_len = config->password != NULL ? strlen(config->password) : 0;
        if (user_len > 255 || pass_len > 255) {
            fail(proxy, KB_ERR_TOO_LARGE, "SOCKS5 credentials too long");
            return KB_ERR_TOO_LARGE;
        }
    }
    return proxy->step == KB_PROXY_STEP_FAILED ? proxy->error : KB_OK;
}

const uint8_t *kb_proxy_pull_out(kb_proxy *proxy, size_t *len)
{
    if (proxy == NULL || proxy->step != KB_PROXY_STEP_WRITE) {
        return NULL;
    }
    if (proxy->out_sent >= proxy->out_len) {
        return NULL;
    }
    if (len != NULL) {
        *len = proxy->out_len - proxy->out_sent;
    }
    return proxy->out + proxy->out_sent;
}

const char *kb_proxy_detail(const kb_proxy *proxy)
{
    if (proxy == NULL || proxy->detail[0] == '\0') {
        return "ok";
    }
    return proxy->detail;
}

kb_proxy_step kb_proxy_get_step(const kb_proxy *proxy)
{
    return proxy == NULL ? KB_PROXY_STEP_FAILED : proxy->step;
}

kb_status kb_proxy_error(const kb_proxy *proxy)
{
    return proxy == NULL ? KB_ERR_INVALID : proxy->error;
}

/* ─── Продвижение после отправки ───────────────────────────────────────── */

static void build_socks5_auth(kb_proxy *p)
{
    uint8_t message[KB_PROXY_OUT_MAX];
    const char *user = p->config.username != NULL ? p->config.username : "";
    const char *pass = p->config.password != NULL ? p->config.password : "";
    size_t user_len = strlen(user);
    size_t pass_len = strlen(pass);

    if (user_len > 255 || pass_len > 255) {
        fail(p, KB_ERR_TOO_LARGE, "SOCKS5 credentials too long");
        return;
    }
    message[0] = 0x01; /* версия подпротокола user/pass */
    message[1] = (uint8_t)user_len;
    memcpy(message + 2, user, user_len);
    message[2 + user_len] = (uint8_t)pass_len;
    memcpy(message + 3 + user_len, pass, pass_len);
    set_out(p, message, 3 + user_len + pass_len);
}

/* Отправка текущего сообщения закончилась — решаем, что дальше. */
static void after_write(kb_proxy *p)
{
    if (p->kind == KB_PROXY_HTTP) {
        p->state = HTTP_READ_RESPONSE;
        /* Заголовки читаются до пустой строки, а не фиксированной длиной. */
        expect(p, 0);
        return;
    }

    switch (p->state) {
    case S5_SEND_GREETING:
        p->state = S5_READ_METHOD;
        expect(p, 2);
        break;
    case S5_SEND_AUTH:
        p->state = S5_READ_AUTH;
        expect(p, 2);
        break;
    case S5_SEND_CONNECT:
        p->state = S5_READ_REPLY;
        expect(p, 4);
        break;
    default:
        fail(p, KB_ERR_STATE, "proxy handshake in a bad state");
        break;
    }
}

kb_status kb_proxy_consume_out(kb_proxy *proxy, size_t len)
{
    if (proxy == NULL) {
        return KB_ERR_INVALID;
    }
    if (proxy->step != KB_PROXY_STEP_WRITE) {
        return KB_ERR_STATE;
    }
    if (len > proxy->out_len - proxy->out_sent) {
        return KB_ERR_INVALID;
    }
    proxy->out_sent += len;
    if (proxy->out_sent == proxy->out_len) {
        proxy->out_len = 0;
        proxy->out_sent = 0;
        after_write(proxy);
    }
    return KB_OK;
}

/* ─── Разбор ответов ───────────────────────────────────────────────────── */

static int is_space(char c)
{
    return c == ' ' || c == '\t';
}

/*
 * Код ответа — второй токен первой строки. Эталон берёт его через
 * split_whitespace().nth(1) и разбирает как u16, поэтому здесь ровно то же:
 * разделителей может быть сколько угодно, а токен обязан быть числом целиком
 * и влезать в u16.
 *
 * Без ограничения сверху накопитель переполнялся бы по модулю: строка
 * «HTTP/1.1 4294967496» дала бы 200 и открыла туннель туда, куда прокси не
 * пускал.
 */
static void handle_http_response(kb_proxy *p)
{
    const char *text = (const char *)p->in;
    size_t len = p->in_len;
    size_t line_end = 0;
    size_t i = 0;
    size_t token_start;
    unsigned long status = 0;
    int parsed = 0;

    while (line_end < len && text[line_end] != '\r' && text[line_end] != '\n') {
        line_end++;
    }
    while (i < line_end && !is_space(text[i])) {
        i++;
    }
    while (i < line_end && is_space(text[i])) {
        i++;
    }

    token_start = i;
    while (i < line_end && !is_space(text[i])) {
        i++;
    }
    if (i > token_start) {
        size_t k;
        parsed = 1;
        for (k = token_start; k < i; k++) {
            if (text[k] < '0' || text[k] > '9') {
                parsed = 0;
                break;
            }
            status = status * 10 + (unsigned long)(text[k] - '0');
            if (status > 65535) {
                parsed = 0;
                break;
            }
        }
    }

    if (!parsed || status != 200) {
        char detail[64];
        size_t at = 0;
        append_str(detail, &at, sizeof detail, "proxy CONNECT failed: ");
        append_uint(detail, &at, sizeof detail, parsed ? status : 0);
        detail[at] = '\0';
        fail(p, KB_ERR_STATE, detail);
        return;
    }
    p->step = KB_PROXY_STEP_DONE;
}

static void handle_socks5(kb_proxy *p)
{
    switch (p->state) {
    case S5_READ_METHOD:
        if (p->in[0] != 0x05) {
            fail(p, KB_ERR_STATE, "not a SOCKS5 proxy");
            return;
        }
        if (p->in[1] == 0x00) {
            p->state = S5_SEND_CONNECT;
            build_socks5_connect(p, p->target_host, p->target_port);
        } else if (p->in[1] == 0x02) {
            p->state = S5_SEND_AUTH;
            build_socks5_auth(p);
        } else if (p->in[1] == 0xFF) {
            fail(p, KB_ERR_STATE, "SOCKS5 proxy rejected auth methods");
        } else {
            char detail[64];
            size_t at = 0;
            append_str(detail, &at, sizeof detail, "SOCKS5 unexpected method ");
            append_uint(detail, &at, sizeof detail, p->in[1]);
            detail[at] = '\0';
            fail(p, KB_ERR_STATE, detail);
        }
        return;

    case S5_READ_AUTH:
        if (p->in[1] != 0x00) {
            fail(p, KB_ERR_STATE, "SOCKS5 auth rejected");
            return;
        }
        p->state = S5_SEND_CONNECT;
        build_socks5_connect(p, p->target_host, p->target_port);
        return;

    case S5_READ_REPLY:
        if (p->in[1] != 0x00) {
            char detail[64];
            size_t at = 0;
            append_str(detail, &at, sizeof detail, "SOCKS5 connect failed, reply ");
            append_uint(detail, &at, sizeof detail, p->in[1]);
            detail[at] = '\0';
            fail(p, KB_ERR_STATE, detail);
            return;
        }
        /* Дальше идёт привязанный адрес, длина зависит от типа. */
        if (p->in[3] == 0x01) {
            p->state = S5_READ_ADDR;
            expect(p, 4 + 2);
        } else if (p->in[3] == 0x04) {
            p->state = S5_READ_ADDR;
            expect(p, 16 + 2);
        } else if (p->in[3] == 0x03) {
            p->state = S5_READ_ADDR_LEN;
            expect(p, 1);
        } else {
            char detail[64];
            size_t at = 0;
            append_str(detail, &at, sizeof detail, "SOCKS5 bad address type ");
            append_uint(detail, &at, sizeof detail, p->in[3]);
            detail[at] = '\0';
            fail(p, KB_ERR_STATE, detail);
        }
        return;

    case S5_READ_ADDR_LEN:
        p->state = S5_READ_ADDR;
        expect(p, (size_t)p->in[0] + 2);
        return;

    case S5_READ_ADDR:
        p->step = KB_PROXY_STEP_DONE;
        return;

    default:
        fail(p, KB_ERR_STATE, "proxy handshake in a bad state");
        return;
    }
}

kb_status kb_proxy_feed(kb_proxy *proxy, const uint8_t *data, size_t len,
                        size_t *consumed)
{
    size_t taken = 0;

    if (proxy == NULL || (data == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    if (consumed != NULL) {
        *consumed = 0;
    }

    while (taken < len && proxy->step == KB_PROXY_STEP_READ) {
        if (proxy->in_len >= sizeof proxy->in) {
            fail(proxy, KB_ERR_TOO_LARGE, "proxy response too long");
            break;
        }
        proxy->in[proxy->in_len] = data[taken];
        proxy->in_len++;
        taken++;

        if (proxy->need != 0) {
            if (proxy->in_len < proxy->need) {
                continue;
            }
        } else {
            /* Заголовки HTTP: ждём пустой строки. */
            if (proxy->in_len < 4 ||
                memcmp(proxy->in + proxy->in_len - 4, "\r\n\r\n", 4) != 0) {
                continue;
            }
        }

        if (proxy->kind == KB_PROXY_HTTP) {
            handle_http_response(proxy);
        } else {
            handle_socks5(proxy);
        }
    }

    if (consumed != NULL) {
        *consumed = taken;
    }
    if (proxy->step == KB_PROXY_STEP_FAILED) {
        return proxy->error;
    }
    return KB_OK;
}
