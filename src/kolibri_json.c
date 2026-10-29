/*
 * JSON-ридер: разбор без аллокаций поверх буфера вызывающего. См.
 * include/kolibri_json.h.
 */

#include "kolibri_json.h"

/* ------------------------------------------------------------------ */
/* Сканер: пройти один элемент и вернуть его конец                      */
/* ------------------------------------------------------------------ */

static const char *skip_ws(const char *p, const char *end)
{
    while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) {
        p++;
    }
    return p;
}

/* Тело строки от символа после открывающей кавычки. Вернуть указатель на
   закрывающую кавычку, либо NULL при обрыве/битом escape. */
static const char *scan_string(const char *p, const char *end)
{
    while (p < end) {
        unsigned char c = (unsigned char)*p;
        if (c == '"') {
            return p;
        }
        if (c == '\\') {
            p++;
            if (p >= end) {
                return NULL;
            }
            if (*p == 'u') {
                int i;
                if (end - p < 5) {
                    return NULL;
                }
                for (i = 1; i <= 4; i++) {
                    char h = p[i];
                    int ok = (h >= '0' && h <= '9') || (h >= 'a' && h <= 'f') ||
                             (h >= 'A' && h <= 'F');
                    if (!ok) {
                        return NULL;
                    }
                }
                p += 5;
                continue;
            }
            /* Прочие escape — один символ. Разрешаем стандартный набор. */
            switch (*p) {
            case '"': case '\\': case '/': case 'b': case 'f':
            case 'n': case 'r': case 't':
                p++;
                break;
            default:
                return NULL;
            }
            continue;
        }
        if (c < 0x20) {
            return NULL; /* сырой управляющий символ в строке недопустим */
        }
        p++;
    }
    return NULL;
}

static const char *scan_value(const char *p, const char *end);

static const char *scan_number(const char *p, const char *end)
{
    const char *s = p;
    if (p < end && *p == '-') {
        p++;
    }
    if (p >= end) {
        return NULL;
    }
    if (*p == '0') {
        p++;
    } else if (*p >= '1' && *p <= '9') {
        while (p < end && *p >= '0' && *p <= '9') {
            p++;
        }
    } else {
        return NULL;
    }
    if (p < end && *p == '.') {
        p++;
        if (p >= end || *p < '0' || *p > '9') {
            return NULL;
        }
        while (p < end && *p >= '0' && *p <= '9') {
            p++;
        }
    }
    if (p < end && (*p == 'e' || *p == 'E')) {
        p++;
        if (p < end && (*p == '+' || *p == '-')) {
            p++;
        }
        if (p >= end || *p < '0' || *p > '9') {
            return NULL;
        }
        while (p < end && *p >= '0' && *p <= '9') {
            p++;
        }
    }
    return p == s ? NULL : p;
}

static const char *scan_literal(const char *p, const char *end,
                                const char *lit)
{
    size_t n = 0;
    while (lit[n] != '\0') {
        if (p + n >= end || p[n] != lit[n]) {
            return NULL;
        }
        n++;
    }
    return p + n;
}

/* Общий сканер: p указывает на первый непробельный символ значения. */
static const char *scan_value(const char *p, const char *end)
{
    if (p >= end) {
        return NULL;
    }
    switch (*p) {
    case '"':
        p = scan_string(p + 1, end);
        return p ? p + 1 : NULL;
    case '{':
    case '[': {
        char open = *p;
        char close = (open == '{') ? '}' : ']';
        int expect_key = (open == '{');
        int first = 1;
        p = skip_ws(p + 1, end);
        if (p < end && *p == close) {
            return p + 1;
        }
        for (;;) {
            if (!first) {
                p = skip_ws(p, end);
                if (p >= end || *p != ',') {
                    return NULL;
                }
                p = skip_ws(p + 1, end);
            }
            first = 0;
            if (open == '{') {
                if (p >= end || *p != '"') {
                    return NULL;
                }
                p = scan_string(p + 1, end);
                if (p == NULL) {
                    return NULL;
                }
                p = skip_ws(p + 1, end);
                if (p >= end || *p != ':') {
                    return NULL;
                }
                p = skip_ws(p + 1, end);
            }
            p = scan_value(p, end);
            if (p == NULL) {
                return NULL;
            }
            p = skip_ws(p, end);
            if (p < end && *p == close) {
                return p + 1;
            }
            (void)expect_key;
        }
    }
    case 't':
        return scan_literal(p, end, "true");
    case 'f':
        return scan_literal(p, end, "false");
    case 'n':
        return scan_literal(p, end, "null");
    default:
        return scan_number(p, end);
    }
}

static kb_json_type type_of(char c)
{
    switch (c) {
    case '"': return KB_JSON_STRING;
    case '{': return KB_JSON_OBJECT;
    case '[': return KB_JSON_ARRAY;
    case 't':
    case 'f': return KB_JSON_BOOL;
    case 'n': return KB_JSON_NULL;
    default:  return KB_JSON_NUMBER;
    }
}

kb_status kb_json_parse(const char *buf, size_t len, kb_json_value *out)
{
    const char *p;
    const char *end;
    const char *stop;

    if (buf == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    end = buf + len;
    p = skip_ws(buf, end);
    if (p >= end) {
        return KB_ERR_INVALID;
    }
    stop = scan_value(p, end);
    if (stop == NULL) {
        return KB_ERR_INVALID;
    }
    out->type = type_of(*p);
    out->start = p;
    out->len = (size_t)(stop - p);
    /* После значения — только пробелы. */
    if (skip_ws(stop, end) != end) {
        return KB_ERR_INVALID;
    }
    return KB_OK;
}

/* ------------------------------------------------------------------ */
/* Доступ к полям и элементам                                          */
/* ------------------------------------------------------------------ */

/* Сравнить строковый узел (start указывает на открывающую кавычку) с C-строкой,
   раскрывая escape. Возврат 1 при равенстве. */
static int str_eq_raw(const char *start, size_t len, const char *text)
{
    const char *p = start + 1;        /* после кавычки */
    const char *end = start + len - 1; /* закрывающая кавычка */
    const unsigned char *t = (const unsigned char *)text;

    while (p < end) {
        unsigned char c = (unsigned char)*p;
        if (c == '\\') {
            char e = p[1];
            unsigned char decoded;
            if (e == 'u') {
                /* Сравниваем только ASCII-диапазон; \u00XX → байт, прочее не
                   встречается в ключах и полях, которые мы читаем. */
                int hi = (p[4]);
                int lo = (p[5]);
                if (p[2] != '0' || p[3] != '0') {
                    return 0;
                }
                hi = (hi <= '9') ? hi - '0' : (hi | 32) - 'a' + 10;
                lo = (lo <= '9') ? lo - '0' : (lo | 32) - 'a' + 10;
                decoded = (unsigned char)((hi << 4) | lo);
                p += 6;
            } else {
                switch (e) {
                case '"': decoded = '"'; break;
                case '\\': decoded = '\\'; break;
                case '/': decoded = '/'; break;
                case 'b': decoded = '\b'; break;
                case 'f': decoded = '\f'; break;
                case 'n': decoded = '\n'; break;
                case 'r': decoded = '\r'; break;
                case 't': decoded = '\t'; break;
                default: return 0;
                }
                p += 2;
            }
            if (*t == '\0' || (unsigned char)*t != decoded) {
                return 0;
            }
            t++;
            continue;
        }
        if (*t == '\0' || (unsigned char)*t != c) {
            return 0;
        }
        t++;
        p++;
    }
    return *t == '\0';
}

kb_status kb_json_object_get(const kb_json_value *object, const char *key,
                             kb_json_value *out)
{
    const char *p;
    const char *end;

    if (object == NULL || key == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (object->type != KB_JSON_OBJECT) {
        return KB_ERR_INVALID;
    }
    end = object->start + object->len - 1; /* '}' */
    p = skip_ws(object->start + 1, end);
    if (p < end && *p == '}') {
        return KB_ERR_NOT_FOUND;
    }
    for (;;) {
        const char *key_start;
        const char *key_stop;
        const char *val_start;
        const char *val_stop;

        if (p >= end || *p != '"') {
            return KB_ERR_INVALID;
        }
        key_start = p;
        key_stop = scan_string(p + 1, end);
        if (key_stop == NULL) {
            return KB_ERR_INVALID;
        }
        key_stop += 1; /* после закрывающей кавычки */
        p = skip_ws(key_stop, end);
        if (p >= end || *p != ':') {
            return KB_ERR_INVALID;
        }
        val_start = skip_ws(p + 1, end);
        val_stop = scan_value(val_start, end);
        if (val_stop == NULL) {
            return KB_ERR_INVALID;
        }
        if (str_eq_raw(key_start, (size_t)(key_stop - key_start), key)) {
            out->type = type_of(*val_start);
            out->start = val_start;
            out->len = (size_t)(val_stop - val_start);
            return KB_OK;
        }
        p = skip_ws(val_stop, end);
        if (p < end && *p == ',') {
            p = skip_ws(p + 1, end);
            continue;
        }
        return KB_ERR_NOT_FOUND;
    }
}

void kb_json_array_iter_init(kb_json_value *iter, const kb_json_value *array)
{
    if (iter == NULL) {
        return;
    }
    if (array == NULL || array->type != KB_JSON_ARRAY) {
        iter->start = NULL;
        iter->len = 0;
        iter->type = KB_JSON_NULL;
        return;
    }
    /* start указывает на позицию следующего элемента, len — байты до конца. */
    iter->start = array->start + 1;               /* после '[' */
    iter->len = array->len - 1;                    /* включает ']' */
    iter->type = KB_JSON_ARRAY;
}

int kb_json_array_next(kb_json_value *iter, kb_json_value *out)
{
    const char *p;
    const char *end;
    const char *stop;

    if (iter == NULL || out == NULL || iter->start == NULL) {
        return 0;
    }
    end = iter->start + iter->len;      /* указывает на ']' + 0 */
    p = skip_ws(iter->start, end);
    if (p >= end || *p == ']') {
        return 0;
    }
    if (*p == ',') {
        p = skip_ws(p + 1, end);
    }
    if (p >= end || *p == ']') {
        return 0;
    }
    stop = scan_value(p, end);
    if (stop == NULL) {
        return 0;
    }
    out->type = type_of(*p);
    out->start = p;
    out->len = (size_t)(stop - p);
    iter->start = stop;
    iter->len = (size_t)(end - stop);
    return 1;
}

void kb_json_object_iter_init(kb_json_value *iter, const kb_json_value *object)
{
    if (iter == NULL) {
        return;
    }
    if (object == NULL || object->type != KB_JSON_OBJECT) {
        iter->start = NULL;
        iter->len = 0;
        iter->type = KB_JSON_NULL;
        return;
    }
    iter->start = object->start + 1; /* после '{' */
    iter->len = object->len - 1;     /* включает '}' */
    iter->type = KB_JSON_OBJECT;
}

int kb_json_object_next(kb_json_value *iter, kb_json_value *key,
                        kb_json_value *val)
{
    const char *p;
    const char *end;
    const char *key_start;
    const char *key_stop;
    const char *val_start;
    const char *val_stop;

    if (iter == NULL || key == NULL || val == NULL || iter->start == NULL) {
        return 0;
    }
    end = iter->start + iter->len; /* на '}' + 0 */
    p = skip_ws(iter->start, end);
    if (p < end && *p == '}') {
        return 0;
    }
    if (p < end && *p == ',') {
        p = skip_ws(p + 1, end);
    }
    if (p >= end || *p == '}' || *p != '"') {
        return 0;
    }
    key_start = p;
    key_stop = scan_string(p + 1, end);
    if (key_stop == NULL) {
        return 0;
    }
    key_stop += 1;
    p = skip_ws(key_stop, end);
    if (p >= end || *p != ':') {
        return 0;
    }
    val_start = skip_ws(p + 1, end);
    val_stop = scan_value(val_start, end);
    if (val_stop == NULL) {
        return 0;
    }
    key->type = KB_JSON_STRING;
    key->start = key_start;
    key->len = (size_t)(key_stop - key_start);
    val->type = type_of(*val_start);
    val->start = val_start;
    val->len = (size_t)(val_stop - val_start);
    iter->start = val_stop;
    iter->len = (size_t)(end - val_stop);
    return 1;
}

kb_status kb_json_as_bool(const kb_json_value *value, int *out)
{
    if (value == NULL || out == NULL || value->type != KB_JSON_BOOL) {
        return KB_ERR_INVALID;
    }
    *out = (value->start[0] == 't');
    return KB_OK;
}

kb_status kb_json_as_i64(const kb_json_value *value, int64_t *out)
{
    const char *p;
    const char *end;
    int neg = 0;
    uint64_t acc = 0;

    if (value == NULL || out == NULL || value->type != KB_JSON_NUMBER) {
        return KB_ERR_INVALID;
    }
    /* Границы без long long-литералов (INT64_MAX разворачивается в LL). */
    {
    const uint64_t i64_min_mag = (uint64_t)1 << 63;       /* |INT64_MIN| */
    const uint64_t i64_max = ((uint64_t)1 << 63) - 1u;    /* INT64_MAX   */
    p = value->start;
    end = value->start + value->len;
    if (p < end && *p == '-') {
        neg = 1;
        p++;
    }
    if (p >= end) {
        return KB_ERR_INVALID;
    }
    for (; p < end; p++) {
        if (*p < '0' || *p > '9') {
            /* Дробное/экспонента — не целое. */
            return KB_ERR_INVALID;
        }
        if (acc > i64_max / 10u + 1u) {
            return KB_ERR_OVERFLOW;
        }
        acc = acc * 10u + (uint64_t)(*p - '0');
    }
    if (neg) {
        if (acc > i64_min_mag) {
            return KB_ERR_OVERFLOW;
        }
        /* -(acc): без переполнения даже при acc == 2^63. */
        *out = (acc == 0) ? 0 : -(int64_t)(acc - 1u) - 1;
    } else {
        if (acc > i64_max) {
            return KB_ERR_OVERFLOW;
        }
        *out = (int64_t)acc;
    }
    return KB_OK;
    }
}

int kb_json_str_eq(const kb_json_value *value, const char *text)
{
    if (value == NULL || text == NULL || value->type != KB_JSON_STRING) {
        return 0;
    }
    return str_eq_raw(value->start, value->len, text);
}

kb_status kb_json_str_copy(const kb_json_value *value, char *out, size_t cap,
                           size_t *out_len)
{
    const char *p;
    const char *end;
    size_t at = 0;

    if (value == NULL || out == NULL || value->type != KB_JSON_STRING) {
        return KB_ERR_INVALID;
    }
    p = value->start + 1;
    end = value->start + value->len - 1;
    while (p < end) {
        unsigned char c = (unsigned char)*p;
        unsigned char decoded;
        if (c == '\\') {
            char e = p[1];
            if (e == 'u') {
                int hi3 = p[2], lo3 = p[3], hi = p[4], lo = p[5];
                unsigned int cp;
                hi3 = (hi3 <= '9') ? hi3 - '0' : (hi3 | 32) - 'a' + 10;
                lo3 = (lo3 <= '9') ? lo3 - '0' : (lo3 | 32) - 'a' + 10;
                hi = (hi <= '9') ? hi - '0' : (hi | 32) - 'a' + 10;
                lo = (lo <= '9') ? lo - '0' : (lo | 32) - 'a' + 10;
                cp = (unsigned int)((hi3 << 12) | (lo3 << 8) | (hi << 4) | lo);
                p += 6;
                /* Минимальный UTF-8 энкодер для BMP; суррогаты не разбираем. */
                if (cp < 0x80) {
                    if (at + 1 > cap) return KB_ERR_TOO_LARGE;
                    out[at++] = (char)cp;
                } else if (cp < 0x800) {
                    if (at + 2 > cap) return KB_ERR_TOO_LARGE;
                    out[at++] = (char)(0xC0 | (cp >> 6));
                    out[at++] = (char)(0x80 | (cp & 0x3F));
                } else {
                    if (at + 3 > cap) return KB_ERR_TOO_LARGE;
                    out[at++] = (char)(0xE0 | (cp >> 12));
                    out[at++] = (char)(0x80 | ((cp >> 6) & 0x3F));
                    out[at++] = (char)(0x80 | (cp & 0x3F));
                }
                continue;
            }
            switch (e) {
            case '"': decoded = '"'; break;
            case '\\': decoded = '\\'; break;
            case '/': decoded = '/'; break;
            case 'b': decoded = '\b'; break;
            case 'f': decoded = '\f'; break;
            case 'n': decoded = '\n'; break;
            case 'r': decoded = '\r'; break;
            case 't': decoded = '\t'; break;
            default: return KB_ERR_INVALID;
            }
            p += 2;
        } else {
            decoded = c;
            p++;
        }
        if (at + 1 > cap) {
            return KB_ERR_TOO_LARGE;
        }
        out[at++] = (char)decoded;
    }
    if (at + 1 > cap) {
        return KB_ERR_TOO_LARGE;
    }
    out[at] = '\0';
    if (out_len != NULL) {
        *out_len = at;
    }
    return KB_OK;
}
