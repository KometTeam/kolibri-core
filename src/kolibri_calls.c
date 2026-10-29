/*
 * Звонки: vcp, события ws2, sans-IO сигнализация. См. include/kolibri_calls.h.
 */

#include <string.h>

#include "kolibri_calls.h"
#include "kolibri_proxy.h" /* kb_base64_decode */

/* ------------------------------------------------------------------ */
/* Мелкий писатель в буфер (общий для URL и команд)                    */
/* ------------------------------------------------------------------ */

typedef struct {
    char *buf;
    size_t cap;
    size_t at;
    int overflow;
} writer;

static void w_init(writer *w, char *buf, size_t cap)
{
    w->buf = buf;
    w->cap = cap;
    w->at = 0;
    w->overflow = 0;
}

static void w_char(writer *w, char c)
{
    if (w->at + 1 > w->cap) {
        w->overflow = 1;
        return;
    }
    w->buf[w->at++] = c;
}

static void w_raw(writer *w, const char *s)
{
    while (*s != '\0') {
        w_char(w, *s++);
    }
}

static void w_i64(writer *w, int64_t v)
{
    char tmp[24];
    int n = 0;
    uint64_t u;
    int neg = 0;
    int i;

    if (v < 0) {
        neg = 1;
        /* Модуль без большого литерала: годится и для INT64_MIN. */
        u = (uint64_t)(-(v + 1)) + 1u;
    } else {
        u = (uint64_t)v;
    }
    do {
        tmp[n++] = (char)('0' + (int)(u % 10u));
        u /= 10u;
    } while (u != 0);
    if (neg) {
        w_char(w, '-');
    }
    for (i = n - 1; i >= 0; i--) {
        w_char(w, tmp[i]);
    }
}

/* Экранировать строку как содержимое JSON-строки (без внешних кавычек). */
static void w_json_str(writer *w, const char *s)
{
    const unsigned char *p = (const unsigned char *)s;
    static const char hex[] = "0123456789abcdef";
    for (; *p != '\0'; p++) {
        unsigned char c = *p;
        switch (c) {
        case '"':  w_raw(w, "\\\""); break;
        case '\\': w_raw(w, "\\\\"); break;
        case '\b': w_raw(w, "\\b"); break;
        case '\f': w_raw(w, "\\f"); break;
        case '\n': w_raw(w, "\\n"); break;
        case '\r': w_raw(w, "\\r"); break;
        case '\t': w_raw(w, "\\t"); break;
        default:
            if (c < 0x20) {
                w_raw(w, "\\u00");
                w_char(w, hex[(c >> 4) & 0xF]);
                w_char(w, hex[c & 0xF]);
            } else {
                w_char(w, (char)c);
            }
        }
    }
}

/* Процент-кодирование для query ws2: незарезервированные — как есть. */
static void w_query(writer *w, const char *s)
{
    const unsigned char *p = (const unsigned char *)s;
    static const char hex[] = "0123456789ABCDEF";
    for (; *p != '\0'; p++) {
        unsigned char c = *p;
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' ||
            c == '~') {
            w_char(w, (char)c);
        } else {
            w_char(w, '%');
            w_char(w, hex[(c >> 4) & 0xF]);
            w_char(w, hex[c & 0xF]);
        }
    }
}

static kb_status w_finish(writer *w, size_t *out_len)
{
    if (w->overflow || w->at + 1 > w->cap) {
        return KB_ERR_TOO_LARGE;
    }
    w->buf[w->at] = '\0';
    if (out_len != NULL) {
        *out_len = w->at;
    }
    return KB_OK;
}

/* ------------------------------------------------------------------ */
/* vcp                                                                 */
/* ------------------------------------------------------------------ */

kb_status kb_vcp_decode(const char *vcp, size_t len, uint8_t *scratch,
                        size_t scratch_cap, kb_vcp *out)
{
    size_t sep = 0;
    size_t raw_len = 0;
    size_t i;
    size_t b64_len;
    uint8_t *compressed;
    size_t comp_cap;
    size_t comp_len = 0;
    size_t json_len = 0;
    kb_status st;

    if (vcp == NULL || scratch == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    /* rawLen до ':'. */
    while (sep < len && vcp[sep] != ':') {
        sep++;
    }
    if (sep == 0 || sep >= len) {
        return KB_ERR_INVALID;
    }
    for (i = 0; i < sep; i++) {
        if (vcp[i] < '0' || vcp[i] > '9') {
            return KB_ERR_INVALID;
        }
        raw_len = raw_len * 10u + (size_t)(vcp[i] - '0');
        if (raw_len > (size_t)1 << 24) { /* защита от абсурда */
            return KB_ERR_INVALID;
        }
    }
    if (raw_len == 0 || raw_len > scratch_cap) {
        return KB_ERR_TRUNCATED;
    }

    /* JSON распакуем в scratch[0..raw_len]; сжатые байты положим следом. */
    b64_len = len - sep - 1;
    compressed = scratch + raw_len;
    comp_cap = scratch_cap - raw_len;
    st = kb_base64_decode(vcp + sep + 1, b64_len, compressed, comp_cap,
                          &comp_len);
    if (st != KB_OK) {
        return st == KB_ERR_TOO_LARGE ? KB_ERR_TRUNCATED : st;
    }
    st = kb_lz4_decompress(compressed, comp_len, scratch, raw_len, &json_len);
    if (st != KB_OK) {
        return st;
    }
    if (json_len > raw_len) {
        json_len = raw_len;
    }
    return kb_json_parse((const char *)scratch, json_len, &out->root);
}

static kb_status vcp_str(const kb_vcp *vcp, const char *key, char *out,
                         size_t cap)
{
    kb_json_value v;
    kb_status st;
    if (vcp == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_json_object_get(&vcp->root, key, &v);
    if (st != KB_OK) {
        return st;
    }
    return kb_json_str_copy(&v, out, cap, NULL);
}

kb_status kb_vcp_token(const kb_vcp *vcp, char *out, size_t cap)
{
    return vcp_str(vcp, "tkn", out, cap);
}
kb_status kb_vcp_ws_endpoint(const kb_vcp *vcp, char *out, size_t cap)
{
    return vcp_str(vcp, "wse", out, cap);
}
kb_status kb_vcp_stun(const kb_vcp *vcp, char *out, size_t cap)
{
    return vcp_str(vcp, "stne", out, cap);
}
kb_status kb_vcp_turn_user(const kb_vcp *vcp, char *out, size_t cap)
{
    return vcp_str(vcp, "trnu", out, cap);
}
kb_status kb_vcp_turn_password(const kb_vcp *vcp, char *out, size_t cap)
{
    return vcp_str(vcp, "trnp", out, cap);
}

int kb_vcp_is_video(const kb_vcp *vcp)
{
    kb_json_value v;
    int b;
    if (vcp == NULL) {
        return 0;
    }
    if (kb_json_object_get(&vcp->root, "iv", &v) != KB_OK) {
        return 0;
    }
    return kb_json_as_bool(&v, &b) == KB_OK && b;
}

kb_status kb_vcp_expires_at(const kb_vcp *vcp, int64_t *out)
{
    kb_json_value v;
    kb_status st;
    if (vcp == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_json_object_get(&vcp->root, "et", &v);
    if (st != KB_OK) {
        return st;
    }
    return kb_json_as_i64(&v, out);
}

int kb_vcp_is_expired(const kb_vcp *vcp, int64_t now_secs)
{
    int64_t exp;
    if (kb_vcp_expires_at(vcp, &exp) != KB_OK) {
        return 0;
    }
    return now_secs >= exp - 5;
}

int64_t kb_vcp_user_id(const kb_vcp *vcp)
{
    char user[128];
    const char *last;
    const char *p;
    int64_t acc = 0;
    int neg = 0;
    if (kb_vcp_turn_user(vcp, user, sizeof user) != KB_OK) {
        return 0;
    }
    last = user;
    for (p = user; *p != '\0'; p++) {
        if (*p == ':') {
            last = p + 1;
        }
    }
    p = last;
    if (*p == '-') {
        neg = 1;
        p++;
    }
    if (*p == '\0') {
        return 0;
    }
    for (; *p != '\0'; p++) {
        if (*p < '0' || *p > '9') {
            return 0;
        }
        acc = acc * 10 + (*p - '0');
    }
    return neg ? -acc : acc;
}

void kb_vcp_turn_begin(const kb_vcp *vcp, kb_vcp_turn_iter *iter)
{
    kb_json_value v;
    if (iter == NULL) {
        return;
    }
    iter->cur = NULL;
    iter->end = NULL;
    if (vcp == NULL) {
        return;
    }
    if (kb_json_object_get(&vcp->root, "trne", &v) != KB_OK ||
        v.type != KB_JSON_STRING) {
        return;
    }
    /* Содержимое строки без кавычек. Escape в списке серверов не ждём. */
    iter->cur = v.start + 1;
    iter->end = v.start + v.len - 1;
}

kb_status kb_vcp_turn_next(kb_vcp_turn_iter *iter, char *out, size_t cap)
{
    const char *start;
    const char *p;
    size_t n;
    if (iter == NULL || out == NULL || iter->cur == NULL) {
        return KB_ERR_NOT_FOUND;
    }
    /* Пропустить пробелы и запятые. */
    while (iter->cur < iter->end &&
           (*iter->cur == ',' || *iter->cur == ' ')) {
        iter->cur++;
    }
    if (iter->cur >= iter->end) {
        return KB_ERR_NOT_FOUND;
    }
    start = iter->cur;
    while (iter->cur < iter->end && *iter->cur != ',') {
        iter->cur++;
    }
    /* Обрезать хвостовые пробелы. */
    p = iter->cur;
    while (p > start && p[-1] == ' ') {
        p--;
    }
    n = (size_t)(p - start);
    if (n + 1 > cap) {
        return KB_ERR_TOO_LARGE;
    }
    memcpy(out, start, n);
    out[n] = '\0';
    return KB_OK;
}

void kb_vcp_ip_begin(const kb_vcp *vcp, kb_vcp_ip_iter *iter)
{
    kb_json_value arr;
    if (iter == NULL) {
        return;
    }
    iter->ok = 0;
    if (vcp == NULL) {
        return;
    }
    if (kb_json_object_get(&vcp->root, "wsip", &arr) != KB_OK ||
        arr.type != KB_JSON_ARRAY) {
        return;
    }
    kb_json_array_iter_init(&iter->it, &arr);
    iter->ok = 1;
}

kb_status kb_vcp_ip_next(kb_vcp_ip_iter *iter, char *out, size_t cap)
{
    kb_json_value elem;
    if (iter == NULL || out == NULL || !iter->ok) {
        return KB_ERR_NOT_FOUND;
    }
    if (!kb_json_array_next(&iter->it, &elem)) {
        return KB_ERR_NOT_FOUND;
    }
    if (elem.type != KB_JSON_STRING) {
        return KB_ERR_INVALID;
    }
    return kb_json_str_copy(&elem, out, cap, NULL);
}

/* ------------------------------------------------------------------ */
/* Параметры клиента и построение URL                                  */
/* ------------------------------------------------------------------ */

void kb_ws2_client_default(kb_ws2_client *client)
{
    if (client == NULL) {
        return;
    }
    client->capabilities = "3c03f";
    client->device = "Kolibri";
    client->platform = "ANDROID";
    client->client_type = "ONE_ME";
    client->app_version = "sdk-0.1.16.4";
    client->os_version = "36";
}

/* path?k1=v1&k2=v2, значения — процент-кодированием. */
kb_status kb_vcp_ws2_url(const kb_vcp *vcp, const char *conversation_id,
                         const kb_ws2_client *client, char *out, size_t cap)
{
    char endpoint[1024];
    kb_ws2_client def;
    writer w;
    const char *q;
    size_t plen;
    char uid[24];
    char token[1024];
    kb_status st;

    if (vcp == NULL || conversation_id == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (client == NULL) {
        kb_ws2_client_default(&def);
        client = &def;
    }
    st = kb_vcp_ws_endpoint(vcp, endpoint, sizeof endpoint);
    if (st != KB_OK) {
        return st;
    }
    st = kb_vcp_token(vcp, token, sizeof token);
    if (st != KB_OK) {
        return st;
    }
    {
        writer uw;
        w_init(&uw, uid, sizeof uid);
        w_i64(&uw, kb_vcp_user_id(vcp));
        w_finish(&uw, NULL);
    }

    /* Путь без старого query. */
    q = strchr(endpoint, '?');
    plen = q ? (size_t)(q - endpoint) : strlen(endpoint);

    w_init(&w, out, cap);
    {
        size_t i;
        for (i = 0; i < plen; i++) {
            w_char(&w, endpoint[i]);
        }
    }
    w_char(&w, '?');
    w_raw(&w, "userId=");        w_query(&w, uid);
    w_raw(&w, "&entityType=USER");
    w_raw(&w, "&conversationId="); w_query(&w, conversation_id);
    w_raw(&w, "&token=");        w_query(&w, token);
    w_raw(&w, "&version=5");
    w_raw(&w, "&capabilities="); w_query(&w, client->capabilities);
    w_raw(&w, "&device=");       w_query(&w, client->device);
    w_raw(&w, "&platform=");     w_query(&w, client->platform);
    w_raw(&w, "&clientType=");   w_query(&w, client->client_type);
    w_raw(&w, "&appVersion=");   w_query(&w, client->app_version);
    w_raw(&w, "&osVersion=");    w_query(&w, client->os_version);
    return w_finish(&w, NULL);
}

/* Слить extra в существующий query, extra побеждает при совпадении ключа.
   Реализуем через набор ключей: если ключ есть в extra — старое значение
   пропускаем, затем допишем extra. */
kb_status kb_ws2_url_from_endpoint(const char *endpoint,
                                   const kb_ws2_client *client, char *out,
                                   size_t cap)
{
    static const char *keys[] = {"platform",   "version",    "capabilities",
                                 "clientType", "appVersion", "device",
                                 "tgt"};
    const char *vals[7];
    kb_ws2_client def;
    writer w;
    const char *q;
    const char *path;
    size_t plen;
    int first = 1;
    size_t k;

    if (endpoint == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (client == NULL) {
        kb_ws2_client_default(&def);
        client = &def;
    }
    vals[0] = client->platform;
    vals[1] = "5";
    vals[2] = client->capabilities;
    vals[3] = client->client_type;
    vals[4] = client->app_version;
    vals[5] = client->device;
    vals[6] = "start";

    q = strchr(endpoint, '?');
    path = endpoint;
    plen = q ? (size_t)(q - endpoint) : strlen(endpoint);

    w_init(&w, out, cap);
    {
        size_t i;
        for (i = 0; i < plen; i++) {
            w_char(&w, path[i]);
        }
    }
    w_char(&w, '?');

    /* Существующие пары, кроме тех, чьи ключи есть в extra. */
    if (q != NULL) {
        const char *p = q + 1;
        while (*p != '\0') {
            const char *amp = strchr(p, '&');
            const char *eq = strchr(p, '=');
            size_t pair_len = amp ? (size_t)(amp - p) : strlen(p);
            size_t key_len = (eq && (size_t)(eq - p) < pair_len)
                                 ? (size_t)(eq - p)
                                 : pair_len;
            int shadowed = 0;
            for (k = 0; k < 7; k++) {
                if (strlen(keys[k]) == key_len &&
                    strncmp(keys[k], p, key_len) == 0) {
                    shadowed = 1;
                    break;
                }
            }
            if (!shadowed && pair_len > 0) {
                if (!first) {
                    w_char(&w, '&');
                }
                first = 0;
                {
                    size_t i;
                    for (i = 0; i < pair_len; i++) {
                        w_char(&w, p[i]);
                    }
                }
            }
            if (amp == NULL) {
                break;
            }
            p = amp + 1;
        }
    }

    /* extra. */
    for (k = 0; k < 7; k++) {
        if (!first) {
            w_char(&w, '&');
        }
        first = 0;
        w_raw(&w, keys[k]);
        w_char(&w, '=');
        w_query(&w, vals[k]);
    }
    return w_finish(&w, NULL);
}

/* ------------------------------------------------------------------ */
/* События ws2                                                         */
/* ------------------------------------------------------------------ */

kb_status kb_call_event_parse(const char *json, size_t len, kb_call_event *out)
{
    kb_json_value ty;
    kb_json_value notif;

    if (json == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (kb_json_parse(json, len, &out->root) != KB_OK) {
        return KB_ERR_INVALID;
    }
    if (out->root.type != KB_JSON_OBJECT) {
        return KB_ERR_INVALID;
    }
    if (kb_json_object_get(&out->root, "type", &ty) == KB_OK &&
        kb_json_str_eq(&ty, "error")) {
        out->type = KB_CALL_ERROR;
        return KB_OK;
    }
    if (kb_json_object_get(&out->root, "notification", &notif) != KB_OK ||
        notif.type != KB_JSON_STRING) {
        out->type = KB_CALL_OTHER;
        return KB_OK;
    }
    if (kb_json_str_eq(&notif, "connection")) {
        out->type = KB_CALL_CONNECTION;
    } else if (kb_json_str_eq(&notif, "transmitted-data")) {
        out->type = KB_CALL_TRANSMITTED;
    } else if (kb_json_str_eq(&notif, "hungup")) {
        out->type = KB_CALL_HUNGUP;
    } else if (kb_json_str_eq(&notif, "closed-conversation")) {
        out->type = KB_CALL_CLOSED;
    } else if (kb_json_str_eq(&notif, "topology-changed")) {
        out->type = KB_CALL_TOPOLOGY_CHANGED;
    } else {
        out->type = KB_CALL_OTHER;
    }
    return KB_OK;
}

static kb_status conversation_topology(const kb_call_event *ev,
                                       kb_json_value *out)
{
    kb_json_value conv;
    kb_status st = kb_json_object_get(&ev->root, "conversation", &conv);
    if (st != KB_OK) {
        return st;
    }
    return kb_json_object_get(&conv, "topology", out);
}

kb_status kb_call_topology(const kb_call_event *ev, char *out, size_t cap)
{
    kb_json_value topo;
    kb_status st;
    if (ev == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    st = conversation_topology(ev, &topo);
    if (st != KB_OK) {
        return st;
    }
    return kb_json_str_copy(&topo, out, cap, NULL);
}

int kb_call_is_sfu(const kb_call_event *ev)
{
    kb_json_value topo;
    if (ev == NULL || conversation_topology(ev, &topo) != KB_OK) {
        return 0;
    }
    return kb_json_str_eq(&topo, "SERVER");
}

void kb_call_participants_begin(const kb_call_event *ev,
                                kb_call_participant_iter *iter)
{
    kb_json_value conv;
    kb_json_value parts;
    if (iter == NULL) {
        return;
    }
    iter->ok = 0;
    if (ev == NULL) {
        return;
    }
    if (kb_json_object_get(&ev->root, "conversation", &conv) != KB_OK) {
        return;
    }
    if (kb_json_object_get(&conv, "participants", &parts) != KB_OK ||
        parts.type != KB_JSON_ARRAY) {
        return;
    }
    kb_json_array_iter_init(&iter->it, &parts);
    iter->ok = 1;
}

int kb_call_participants_next(kb_call_participant_iter *iter, int64_t *out)
{
    kb_json_value elem;
    kb_json_value id;
    if (iter == NULL || out == NULL || !iter->ok) {
        return 0;
    }
    while (kb_json_array_next(&iter->it, &elem)) {
        if (elem.type == KB_JSON_OBJECT &&
            kb_json_object_get(&elem, "id", &id) == KB_OK &&
            kb_json_as_i64(&id, out) == KB_OK) {
            return 1;
        }
    }
    return 0;
}

kb_status kb_call_peer_of(const kb_call_event *ev, int64_t my_user_id,
                          int64_t *out)
{
    kb_call_participant_iter it;
    int64_t id;
    if (ev == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    kb_call_participants_begin(ev, &it);
    while (kb_call_participants_next(&it, &id)) {
        if (id != my_user_id) {
            *out = id;
            return KB_OK;
        }
    }
    return KB_ERR_NOT_FOUND;
}

static kb_status data_field(const kb_call_event *ev, kb_json_value *data)
{
    if (ev == NULL || ev->type != KB_CALL_TRANSMITTED) {
        return KB_ERR_INVALID;
    }
    return kb_json_object_get(&ev->root, "data", data);
}

kb_status kb_call_transmitted_type(const kb_call_event *ev,
                                   kb_transmitted_type *out)
{
    kb_json_value data, sub;
    kb_status st;
    if (out == NULL) {
        return KB_ERR_INVALID;
    }
    st = data_field(ev, &data);
    if (st != KB_OK) {
        return st;
    }
    if (kb_json_object_get(&data, "sdp", &sub) == KB_OK) {
        *out = KB_TD_SDP;
        return KB_OK;
    }
    if (kb_json_object_get(&data, "candidate", &sub) == KB_OK) {
        *out = KB_TD_CANDIDATE;
        return KB_OK;
    }
    return KB_ERR_NOT_FOUND;
}

static kb_status nested_str(const kb_call_event *ev, const char *outer,
                            const char *inner, char *out, size_t cap)
{
    kb_json_value data, obj, v;
    kb_status st;
    st = data_field(ev, &data);
    if (st != KB_OK) {
        return st;
    }
    st = kb_json_object_get(&data, outer, &obj);
    if (st != KB_OK) {
        return st;
    }
    st = kb_json_object_get(&obj, inner, &v);
    if (st != KB_OK) {
        return st;
    }
    return kb_json_str_copy(&v, out, cap, NULL);
}

kb_status kb_call_sdp_type(const kb_call_event *ev, char *out, size_t cap)
{
    return nested_str(ev, "sdp", "type", out, cap);
}
kb_status kb_call_sdp(const kb_call_event *ev, char *out, size_t cap)
{
    return nested_str(ev, "sdp", "sdp", out, cap);
}
kb_status kb_call_candidate(const kb_call_event *ev, char *out, size_t cap)
{
    return nested_str(ev, "candidate", "candidate", out, cap);
}
kb_status kb_call_candidate_mid(const kb_call_event *ev, char *out, size_t cap)
{
    return nested_str(ev, "candidate", "sdpMid", out, cap);
}

kb_status kb_call_candidate_mline(const kb_call_event *ev, int64_t *out)
{
    kb_json_value data, cand, v;
    kb_status st;
    if (out == NULL) {
        return KB_ERR_INVALID;
    }
    st = data_field(ev, &data);
    if (st != KB_OK) {
        return st;
    }
    st = kb_json_object_get(&data, "candidate", &cand);
    if (st != KB_OK) {
        return st;
    }
    st = kb_json_object_get(&cand, "sdpMLineIndex", &v);
    if (st != KB_OK) {
        return st;
    }
    return kb_json_as_i64(&v, out);
}

kb_status kb_call_error_text(const kb_call_event *ev, char *out, size_t cap)
{
    kb_json_value v;
    kb_status st;
    if (ev == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_json_object_get(&ev->root, "error", &v);
    if (st != KB_OK) {
        return st;
    }
    if (v.type == KB_JSON_STRING) {
        return kb_json_str_copy(&v, out, cap, NULL);
    }
    /* Не строка — отдать как есть (сериализованный JSON). */
    if (v.len + 1 > cap) {
        return KB_ERR_TOO_LARGE;
    }
    memcpy(out, v.start, v.len);
    out[v.len] = '\0';
    return KB_OK;
}

/* ------------------------------------------------------------------ */
/* Сигнализация ws2                                                    */
/* ------------------------------------------------------------------ */

void kb_ws2_init(kb_ws2 *ws2)
{
    if (ws2 != NULL) {
        ws2->seq = 0;
    }
}

kb_ws2_frame kb_ws2_classify(const char *text, size_t len, int64_t *seq)
{
    kb_json_value root, ty, seqv;
    int is_response;
    int is_error;

    if (seq != NULL) {
        *seq = -1;
    }
    if (text == NULL) {
        return KB_WS2_IGNORE;
    }
    if (len == 4 && memcmp(text, "ping", 4) == 0) {
        return KB_WS2_PING;
    }
    if (kb_json_parse(text, len, &root) != KB_OK ||
        root.type != KB_JSON_OBJECT) {
        return KB_WS2_IGNORE;
    }
    is_response = 0;
    is_error = 0;
    if (kb_json_object_get(&root, "type", &ty) == KB_OK) {
        is_response = kb_json_str_eq(&ty, "response");
        is_error = kb_json_str_eq(&ty, "error");
    }
    if (is_response || is_error) {
        if (seq != NULL &&
            kb_json_object_get(&root, "sequence", &seqv) == KB_OK) {
            kb_json_as_i64(&seqv, seq);
        }
        return is_error ? KB_WS2_ERROR : KB_WS2_RESPONSE;
    }
    if (kb_json_object_get(&root, "notification", &ty) == KB_OK) {
        return KB_WS2_NOTIFICATION;
    }
    return KB_WS2_IGNORE;
}

static kb_status command_finish(kb_ws2 *ws2, writer *w, size_t *out_len,
                                int64_t *seq)
{
    int64_t s;
    kb_status st;
    if (ws2 == NULL) {
        return KB_ERR_INVALID;
    }
    s = ws2->seq + 1;
    w_raw(w, ",\"sequence\":");
    w_i64(w, s);
    w_char(w, '}');
    st = w_finish(w, out_len);
    if (st != KB_OK) {
        return st; /* seq не потрачен — буфер мал */
    }
    ws2->seq = s;
    if (seq != NULL) {
        *seq = s;
    }
    return KB_OK;
}

kb_status kb_ws2_transmit_sdp(kb_ws2 *ws2, int64_t participant_id,
                              const char *sdp_type, const char *sdp, char *out,
                              size_t cap, size_t *out_len, int64_t *seq)
{
    writer w;
    if (sdp_type == NULL || sdp == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    w_init(&w, out, cap);
    w_raw(&w, "{\"command\":\"transmit-data\",\"participantId\":");
    w_i64(&w, participant_id);
    w_raw(&w, ",\"participantType\":\"USER\",\"deviceIdx\":0,\"data\":{\"sdp\":{"
              "\"type\":\"");
    w_json_str(&w, sdp_type);
    w_raw(&w, "\",\"sdp\":\"");
    w_json_str(&w, sdp);
    w_raw(&w, "\"}},\"capabilities\":\"1\"");
    return command_finish(ws2, &w, out_len, seq);
}

kb_status kb_ws2_transmit_candidate(kb_ws2 *ws2, int64_t participant_id,
                                    const char *candidate, const char *sdp_mid,
                                    int64_t sdp_mline_index, char *out,
                                    size_t cap, size_t *out_len, int64_t *seq)
{
    writer w;
    if (candidate == NULL || sdp_mid == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    w_init(&w, out, cap);
    w_raw(&w, "{\"command\":\"transmit-data\",\"participantId\":");
    w_i64(&w, participant_id);
    w_raw(&w, ",\"participantType\":\"USER\",\"deviceIdx\":0,\"data\":"
              "{\"candidate\":{\"candidate\":\"");
    w_json_str(&w, candidate);
    w_raw(&w, "\",\"sdpMid\":\"");
    w_json_str(&w, sdp_mid);
    w_raw(&w, "\",\"sdpMLineIndex\":");
    w_i64(&w, sdp_mline_index);
    w_raw(&w, "}}");
    return command_finish(ws2, &w, out_len, seq);
}

kb_status kb_ws2_accept_call(kb_ws2 *ws2, char *out, size_t cap,
                             size_t *out_len, int64_t *seq)
{
    writer w;
    if (out == NULL) {
        return KB_ERR_INVALID;
    }
    w_init(&w, out, cap);
    w_raw(&w, "{\"command\":\"accept-call\"");
    return command_finish(ws2, &w, out_len, seq);
}

kb_status kb_ws2_hangup(kb_ws2 *ws2, const char *reason, char *out, size_t cap,
                        size_t *out_len, int64_t *seq)
{
    writer w;
    if (reason == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    w_init(&w, out, cap);
    w_raw(&w, "{\"command\":\"hangup\",\"reason\":\"");
    w_json_str(&w, reason);
    w_raw(&w, "\"");
    return command_finish(ws2, &w, out_len, seq);
}

kb_status kb_ws2_change_media(kb_ws2 *ws2, int audio, int video, int screen,
                              char *out, size_t cap, size_t *out_len,
                              int64_t *seq)
{
    writer w;
    if (out == NULL) {
        return KB_ERR_INVALID;
    }
    w_init(&w, out, cap);
    w_raw(&w, "{\"command\":\"change-media-settings\",\"mediaSettings\":{"
              "\"isAudioEnabled\":");
    w_raw(&w, audio ? "true" : "false");
    w_raw(&w, ",\"isVideoEnabled\":");
    w_raw(&w, video ? "true" : "false");
    w_raw(&w, ",\"isScreenSharingEnabled\":");
    w_raw(&w, screen ? "true" : "false");
    w_raw(&w, ",\"isAnimojiEnabled\":false}");
    return command_finish(ws2, &w, out_len, seq);
}
