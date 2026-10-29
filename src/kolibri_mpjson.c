/* Мост MessagePack ↔ JSON. См. include/kolibri_mpjson.h. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kolibri_mpjson.h"
#include "kolibri_json.h"
#include "kolibri_msgpack.h"
#include "kolibri_proxy.h" /* kb_base64_encode / kb_base64_decode */

/* ------------------------------------------------------------------ */
/* Писатель JSON-текста в буфер вызывающего                            */
/* ------------------------------------------------------------------ */

typedef struct {
    char *buf;
    size_t cap;
    size_t at;
    int overflow;
} jw;

static void jw_init(jw *w, char *buf, size_t cap)
{
    w->buf = buf;
    w->cap = cap;
    w->at = 0;
    w->overflow = 0;
}

static void jw_char(jw *w, char c)
{
    if (w->at + 1 > w->cap) {
        w->overflow = 1;
        return;
    }
    w->buf[w->at++] = c;
}

static void jw_raw(jw *w, const char *s)
{
    while (*s != '\0') {
        jw_char(w, *s++);
    }
}

static void jw_u64(jw *w, uint64_t v)
{
    char tmp[24];
    int n = 0;
    int i;
    do {
        tmp[n++] = (char)('0' + (int)(v % 10u));
        v /= 10u;
    } while (v != 0);
    for (i = n - 1; i >= 0; i--) {
        jw_char(w, tmp[i]);
    }
}

static void jw_i64(jw *w, int64_t v)
{
    uint64_t u;
    if (v < 0) {
        jw_char(w, '-');
        u = (uint64_t)(-(v + 1)) + 1u; /* модуль без большого литерала */
    } else {
        u = (uint64_t)v;
    }
    jw_u64(w, u);
}

/* Экранировать байты как содержимое JSON-строки. Невалидный UTF-8 заменяется
   на U+FFFD, как lossy-декод в эталоне. */
static void jw_str_escaped(jw *w, const uint8_t *p, size_t len)
{
    static const char hex[] = "0123456789abcdef";
    size_t i = 0;
    while (i < len) {
        unsigned char c = p[i];
        if (c == '"') { jw_raw(w, "\\\""); i++; continue; }
        if (c == '\\') { jw_raw(w, "\\\\"); i++; continue; }
        if (c == '\b') { jw_raw(w, "\\b"); i++; continue; }
        if (c == '\f') { jw_raw(w, "\\f"); i++; continue; }
        if (c == '\n') { jw_raw(w, "\\n"); i++; continue; }
        if (c == '\r') { jw_raw(w, "\\r"); i++; continue; }
        if (c == '\t') { jw_raw(w, "\\t"); i++; continue; }
        if (c < 0x20) {
            jw_raw(w, "\\u00");
            jw_char(w, hex[(c >> 4) & 0xF]);
            jw_char(w, hex[c & 0xF]);
            i++;
            continue;
        }
        if (c < 0x80) {
            jw_char(w, (char)c);
            i++;
            continue;
        }
        /* Проверить многобайтную последовательность UTF-8. */
        {
            int extra;
            unsigned int min;
            unsigned int cp;
            int k;
            if ((c & 0xE0) == 0xC0) { extra = 1; min = 0x80; cp = c & 0x1F; }
            else if ((c & 0xF0) == 0xE0) { extra = 2; min = 0x800; cp = c & 0x0F; }
            else if ((c & 0xF8) == 0xF0) { extra = 3; min = 0x10000; cp = c & 0x07; }
            else { jw_raw(w, "\xEF\xBF\xBD"); i++; continue; }
            if (i + (size_t)extra >= len + 1 && i + (size_t)extra > len) {
                jw_raw(w, "\xEF\xBF\xBD"); i++; continue;
            }
            {
                int ok = 1;
                for (k = 1; k <= extra; k++) {
                    if (i + (size_t)k >= len || (p[i + k] & 0xC0) != 0x80) {
                        ok = 0;
                        break;
                    }
                    cp = (cp << 6) | (unsigned int)(p[i + k] & 0x3F);
                }
                if (!ok || cp < min || cp > 0x10FFFF ||
                    (cp >= 0xD800 && cp <= 0xDFFF)) {
                    jw_raw(w, "\xEF\xBF\xBD");
                    i++;
                    continue;
                }
                /* Валидно — вывести как есть. */
                for (k = 0; k <= extra; k++) {
                    jw_char(w, (char)p[i + k]);
                }
                i += (size_t)extra + 1;
            }
        }
    }
}

static void jw_base64(jw *w, const uint8_t *data, size_t len)
{
    /* Кодируем частями, чтобы не держать большой временный буфер. */
    static const char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t i = 0;
    while (i < len) {
        size_t chunk = len - i < 3 ? len - i : 3;
        uint8_t b0 = data[i];
        uint8_t b1 = chunk > 1 ? data[i + 1] : 0;
        uint8_t b2 = chunk > 2 ? data[i + 2] : 0;
        uint32_t n = ((uint32_t)b0 << 16) | ((uint32_t)b1 << 8) | (uint32_t)b2;
        jw_char(w, alphabet[(n >> 18) & 63]);
        jw_char(w, alphabet[(n >> 12) & 63]);
        jw_char(w, chunk > 1 ? alphabet[(n >> 6) & 63] : '=');
        jw_char(w, chunk > 2 ? alphabet[n & 63] : '=');
        i += chunk;
    }
}

static void jw_double(jw *w, double v)
{
    char buf[32];
    size_t k;
    snprintf(buf, sizeof buf, "%.17g", v);
    /* inf/nan — в JSON нет, эталон отдаёт null. */
    for (k = 0; buf[k] != '\0'; k++) {
        char c = buf[k];
        if (c == 'n' || c == 'N' || c == 'i' || c == 'I') {
            jw_raw(w, "null");
            return;
        }
    }
    jw_raw(w, buf);
}

/* ------------------------------------------------------------------ */
/* msgpack → JSON                                                      */
/* ------------------------------------------------------------------ */

static kb_status emit_value(kb_mp_reader *r, jw *w, int tagged);

/* Ключ map как JSON-строка: строку берём как есть, число/булев/nil стягиваем. */
static kb_status emit_key(kb_mp_reader *r, jw *w)
{
    kb_mp_value v;
    kb_status st = kb_mp_read(r, &v);
    if (st != KB_OK) {
        return st;
    }
    switch (v.type) {
    case KB_MP_STR:
        jw_char(w, '"');
        jw_str_escaped(w, v.data, v.len);
        jw_char(w, '"');
        return KB_OK;
    case KB_MP_INT:
        jw_char(w, '"');
        jw_i64(w, v.i);
        jw_char(w, '"');
        return KB_OK;
    case KB_MP_UINT:
        jw_char(w, '"');
        jw_u64(w, v.u);
        jw_char(w, '"');
        return KB_OK;
    case KB_MP_BOOL:
        jw_raw(w, v.boolean ? "\"true\"" : "\"false\"");
        return KB_OK;
    case KB_MP_NIL:
        jw_raw(w, "\"null\"");
        return KB_OK;
    case KB_MP_BIN:
        jw_char(w, '"');
        jw_base64(w, v.data, v.len);
        jw_char(w, '"');
        return KB_OK;
    default:
        /* Массив/карта в роли ключа — экзотика; отдаём пустую строку. */
        jw_raw(w, "\"\"");
        return KB_OK;
    }
}

/* Значение уже прочитано в v; вывести его как JSON. */
static kb_status emit_read(kb_mp_reader *r, kb_mp_value *v, jw *w, int tagged)
{
    size_t i;
    kb_status st;

    switch (v->type) {
    case KB_MP_NIL:
        jw_raw(w, "null");
        return KB_OK;
    case KB_MP_BOOL:
        jw_raw(w, v->boolean ? "true" : "false");
        return KB_OK;
    case KB_MP_INT:
        jw_i64(w, v->i);
        return KB_OK;
    case KB_MP_UINT:
        jw_u64(w, v->u);
        return KB_OK;
    case KB_MP_FLOAT:
        jw_double(w, v->f);
        return KB_OK;
    case KB_MP_STR:
        jw_char(w, '"');
        jw_str_escaped(w, v->data, v->len);
        jw_char(w, '"');
        return KB_OK;
    case KB_MP_BIN:
        if (tagged) {
            jw_raw(w, "{\"$bin\":\"");
            jw_base64(w, v->data, v->len);
            jw_raw(w, "\"}");
        } else {
            jw_char(w, '"');
            jw_base64(w, v->data, v->len);
            jw_char(w, '"');
        }
        return KB_OK;
    case KB_MP_EXT:
        jw_raw(w, "{\"$ext\":");
        jw_i64(w, (int64_t)v->ext_type);
        jw_raw(w, ",\"data\":\"");
        jw_base64(w, v->data, v->len);
        jw_raw(w, "\"}");
        return KB_OK;
    case KB_MP_ARRAY:
        jw_char(w, '[');
        for (i = 0; i < v->len; i++) {
            if (i != 0) {
                jw_char(w, ',');
            }
            st = emit_value(r, w, tagged);
            if (st != KB_OK) {
                return st;
            }
        }
        jw_char(w, ']');
        return KB_OK;
    case KB_MP_MAP:
        jw_char(w, '{');
        for (i = 0; i < v->len; i++) {
            if (i != 0) {
                jw_char(w, ',');
            }
            st = emit_key(r, w);
            if (st != KB_OK) {
                return st;
            }
            jw_char(w, ':');
            st = emit_value(r, w, tagged);
            if (st != KB_OK) {
                return st;
            }
        }
        jw_char(w, '}');
        return KB_OK;
    default:
        return KB_ERR_INVALID;
    }
}

static kb_status emit_value(kb_mp_reader *r, jw *w, int tagged)
{
    kb_mp_value v;
    kb_status st = kb_mp_read(r, &v);
    if (st != KB_OK) {
        return st;
    }
    return emit_read(r, &v, w, tagged);
}

kb_status kb_mp_to_json(const uint8_t *mp, size_t mp_len, int tagged, char *out,
                        size_t cap, size_t *out_len)
{
    kb_mp_reader r;
    jw w;
    kb_status st;

    if (mp == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (kb_mp_reader_init(&r, mp, mp_len) != KB_OK) {
        return KB_ERR_INVALID;
    }
    jw_init(&w, out, cap);
    st = emit_value(&r, &w, tagged);
    if (st != KB_OK) {
        return st;
    }
    if (w.overflow || w.at + 1 > w.cap) {
        return KB_ERR_TOO_LARGE;
    }
    w.buf[w.at] = '\0';
    if (out_len != NULL) {
        *out_len = w.at;
    }
    return KB_OK;
}

/* ------------------------------------------------------------------ */
/* JSON → msgpack                                                      */
/* ------------------------------------------------------------------ */

static kb_status build(const kb_json_value *v, kb_mp_writer *mw,
                       uint8_t *scratch, size_t scratch_cap);

/* Записать число из JSON: сперва i64, затем u64, затем f64. */
static void build_number(const kb_json_value *v, kb_mp_writer *mw)
{
    const char *p = v->start;
    const char *end = v->start + v->len;
    int is_float = 0;
    int64_t iv;

    for (; p < end; p++) {
        if (*p == '.' || *p == 'e' || *p == 'E') {
            is_float = 1;
            break;
        }
    }
    if (!is_float && kb_json_as_i64(v, &iv) == KB_OK) {
        kb_mp_write_i64(mw, iv);
        return;
    }
    if (!is_float && v->start[0] != '-') {
        /* Большое положительное — пробуем u64. */
        uint64_t u = 0;
        int ok = 1;
        for (p = v->start; p < end; p++) {
            if (*p < '0' || *p > '9') {
                ok = 0;
                break;
            }
            if (u > (~(uint64_t)0 - (uint64_t)(*p - '0')) / 10u) {
                ok = 0;
                break;
            }
            u = u * 10u + (uint64_t)(*p - '0');
        }
        if (ok) {
            kb_mp_write_u64(mw, u);
            return;
        }
    }
    /* Иначе — double через strtod по завершённой строке. */
    {
        char buf[64];
        size_t n = v->len < sizeof buf - 1 ? v->len : sizeof buf - 1;
        memcpy(buf, v->start, n);
        buf[n] = '\0';
        kb_mp_write_f64(mw, strtod(buf, NULL));
    }
}

/* Проверить {"$bin":"…"} или {"$ext":tag,"data":"…"}; вернуть 1, если это
   помеченный бинарник/ext и он записан. */
static int build_tagged(const kb_json_value *obj, kb_mp_writer *mw,
                        uint8_t *scratch, size_t scratch_cap)
{
    kb_json_value it, key, val;
    int count = 0;
    kb_json_value bin, ext, data;
    int has_bin = 0, has_ext = 0, has_data = 0;

    kb_json_object_iter_init(&it, obj);
    while (kb_json_object_next(&it, &key, &val)) {
        count++;
        if (kb_json_str_eq(&key, "$bin")) { has_bin = 1; bin = val; }
        else if (kb_json_str_eq(&key, "$ext")) { has_ext = 1; ext = val; }
        else if (kb_json_str_eq(&key, "data")) { has_data = 1; data = val; }
    }

    if (count == 1 && has_bin && bin.type == KB_JSON_STRING) {
        size_t text_len = 0;
        size_t blen = 0;
        /* Сначала снять JSON escapes (в частности `\/` от Android JSONObject),
           затем безопасно декодировать base64 в том же scratch-буфере. */
        if (kb_json_str_copy(&bin, (char *)scratch, scratch_cap, &text_len) != KB_OK ||
            kb_base64_decode((const char *)scratch, text_len, scratch, scratch_cap,
                             &blen) != KB_OK) {
            return 0; /* не base64 — трактуем как обычный объект */
        }
        kb_mp_write_bin(mw, scratch, blen);
        return 1;
    }
    if (count == 2 && has_ext && has_data && ext.type == KB_JSON_NUMBER &&
        data.type == KB_JSON_STRING) {
        int64_t tag = 0;
        size_t text_len = 0;
        size_t blen = 0;
        if (kb_json_as_i64(&ext, &tag) != KB_OK) {
            return 0;
        }
        if (kb_json_str_copy(&data, (char *)scratch, scratch_cap, &text_len) != KB_OK ||
            kb_base64_decode((const char *)scratch, text_len, scratch, scratch_cap,
                             &blen) != KB_OK) {
            return 0;
        }
        kb_mp_write_ext(mw, (int8_t)tag, scratch, blen);
        return 1;
    }
    return 0;
}

/* Ключ объекта → msgpack: "$int:<n>" разворачивается в целочисленный ключ. */
static void build_key(const kb_json_value *key, kb_mp_writer *mw,
                      uint8_t *scratch, size_t scratch_cap)
{
    size_t klen = 0;
    if (kb_json_str_copy(key, (char *)scratch, scratch_cap, &klen) != KB_OK) {
        kb_mp_write_str(mw, "", 0);
        return;
    }
    if (klen > 5 && memcmp(scratch, "$int:", 5) == 0) {
        const char *num = (const char *)scratch + 5;
        const char *p = num;
        int neg = (*p == '-');
        int digits = 1;
        if (neg) {
            p++;
        }
        if (*p == '\0') {
            digits = 0;
        }
        for (; *p != '\0'; p++) {
            if (*p < '0' || *p > '9') {
                digits = 0;
                break;
            }
        }
        if (digits) {
            /* Без long long: копим модуль в uint64 с контролем переполнения. */
            const uint64_t i64_max = ((uint64_t)1 << 63) - 1u;
            const uint64_t i64_min_mag = (uint64_t)1 << 63;
            const char *q = neg ? num + 1 : num;
            uint64_t acc = 0;
            int ok = 1;
            for (; *q != '\0'; q++) {
                if (acc > (~(uint64_t)0 - (uint64_t)(*q - '0')) / 10u) {
                    ok = 0;
                    break;
                }
                acc = acc * 10u + (uint64_t)(*q - '0');
            }
            if (ok) {
                if (neg) {
                    if (acc <= i64_min_mag) {
                        kb_mp_write_i64(
                            mw, acc == 0 ? 0 : -(int64_t)(acc - 1u) - 1);
                        return;
                    }
                } else if (acc <= i64_max) {
                    kb_mp_write_i64(mw, (int64_t)acc);
                    return;
                } else {
                    kb_mp_write_u64(mw, acc);
                    return;
                }
            }
        }
    }
    kb_mp_write_str(mw, (const char *)scratch, klen);
}

static kb_status build(const kb_json_value *v, kb_mp_writer *mw,
                       uint8_t *scratch, size_t scratch_cap)
{
    switch (v->type) {
    case KB_JSON_NULL:
        kb_mp_write_nil(mw);
        return KB_OK;
    case KB_JSON_BOOL: {
        int b = 0;
        kb_json_as_bool(v, &b);
        kb_mp_write_bool(mw, b);
        return KB_OK;
    }
    case KB_JSON_NUMBER:
        build_number(v, mw);
        return KB_OK;
    case KB_JSON_STRING: {
        size_t slen = 0;
        kb_status st = kb_json_str_copy(v, (char *)scratch, scratch_cap, &slen);
        if (st != KB_OK) {
            return st;
        }
        kb_mp_write_str(mw, (const char *)scratch, slen);
        return KB_OK;
    }
    case KB_JSON_ARRAY: {
        kb_json_value it, elem;
        size_t count = 0;
        kb_json_array_iter_init(&it, v);
        while (kb_json_array_next(&it, &elem)) {
            count++;
        }
        kb_mp_write_array(mw, count);
        kb_json_array_iter_init(&it, v);
        while (kb_json_array_next(&it, &elem)) {
            kb_status st = build(&elem, mw, scratch, scratch_cap);
            if (st != KB_OK) {
                return st;
            }
        }
        return KB_OK;
    }
    case KB_JSON_OBJECT: {
        kb_json_value it, key, val;
        size_t count = 0;
        if (build_tagged(v, mw, scratch, scratch_cap)) {
            return kb_mp_writer_status(mw);
        }
        kb_json_object_iter_init(&it, v);
        while (kb_json_object_next(&it, &key, &val)) {
            count++;
        }
        kb_mp_write_map(mw, count);
        kb_json_object_iter_init(&it, v);
        while (kb_json_object_next(&it, &key, &val)) {
            kb_status st;
            build_key(&key, mw, scratch, scratch_cap);
            st = build(&val, mw, scratch, scratch_cap);
            if (st != KB_OK) {
                return st;
            }
        }
        return KB_OK;
    }
    default:
        return KB_ERR_INVALID;
    }
}

kb_status kb_json_to_mp(const char *json, size_t json_len, uint8_t *scratch,
                        size_t scratch_cap, uint8_t *out, size_t cap,
                        size_t *out_len)
{
    kb_json_value root;
    kb_mp_writer mw;
    kb_status st;

    if (json == NULL || scratch == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (kb_json_parse(json, json_len, &root) != KB_OK) {
        return KB_ERR_INVALID;
    }
    if (kb_mp_writer_init(&mw, out, cap) != KB_OK) {
        return KB_ERR_INVALID;
    }
    st = build(&root, &mw, scratch, scratch_cap);
    if (st != KB_OK) {
        return st;
    }
    st = kb_mp_writer_status(&mw);
    if (st != KB_OK) {
        return st;
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&mw);
    }
    return KB_OK;
}
