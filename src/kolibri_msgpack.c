/*
 * MessagePack: чтение курсором и запись в буфер вызывающей стороны.
 *
 * Читатель разбирает все типы формата — иначе нельзя корректно пропустить
 * чужое значение при поиске по map. Ни аллокаций, ни рекурсии: вложенность
 * обходится счётчиком, так что глубоко вложенный вход от сервера не свалит
 * стек.
 */

#include <string.h>

#include "kolibri.h"

/* ─── Чтение ───────────────────────────────────────────────────────────── */

kb_status kb_mp_reader_init(kb_mp_reader *reader, const uint8_t *data,
                            size_t len)
{
    if (reader == NULL || (data == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    reader->buf = data;
    reader->cap = len;
    reader->pos = 0;
    return KB_OK;
}

size_t kb_mp_reader_pos(const kb_mp_reader *reader)
{
    return reader == NULL ? 0 : reader->pos;
}

size_t kb_mp_reader_left(const kb_mp_reader *reader)
{
    return reader == NULL ? 0 : reader->cap - reader->pos;
}

static kb_status take(kb_mp_reader *reader, size_t count, const uint8_t **at)
{
    if (count > reader->cap - reader->pos) {
        return KB_ERR_TRUNCATED;
    }
    *at = reader->buf + reader->pos;
    reader->pos += count;
    return KB_OK;
}

static uint64_t be_value(const uint8_t *p, size_t width)
{
    uint64_t v = 0;
    size_t i;
    for (i = 0; i < width; i++) {
        v = (v << 8) | (uint64_t)p[i];
    }
    return v;
}

static kb_status read_be(kb_mp_reader *reader, size_t width, uint64_t *out)
{
    const uint8_t *at;
    kb_status st = take(reader, width, &at);
    if (st != KB_OK) {
        return st;
    }
    *out = be_value(at, width);
    return KB_OK;
}

/* Длина строки, бинарника или ext: данные должны реально лежать в буфере. */
static kb_status read_blob(kb_mp_reader *reader, size_t width, kb_mp_type type,
                           kb_mp_value *out)
{
    uint64_t len;
    const uint8_t *at;
    kb_status st = read_be(reader, width, &len);
    if (st != KB_OK) {
        return st;
    }
    if (len > (uint64_t)(reader->cap - reader->pos)) {
        return KB_ERR_TRUNCATED;
    }
    st = take(reader, (size_t)len, &at);
    if (st != KB_OK) {
        return st;
    }
    out->type = type;
    out->data = at;
    out->len = (size_t)len;
    return KB_OK;
}

/*
 * Число элементов массива или пар map. Каждому элементу нужен хотя бы один
 * байт, поэтому заявка крупнее остатка буфера — заведомо ложь; проверка заодно
 * не даёт счётчику в kb_mp_skip переполниться на 32-битной платформе.
 */
static kb_status read_count(kb_mp_reader *reader, size_t width, kb_mp_type type,
                            kb_mp_value *out)
{
    uint64_t count;
    uint64_t needed;
    kb_status st = read_be(reader, width, &count);
    if (st != KB_OK) {
        return st;
    }
    needed = type == KB_MP_MAP ? count * 2 : count;
    if (count > 0xFFFFFFFFu || needed > (uint64_t)(reader->cap - reader->pos)) {
        return KB_ERR_TRUNCATED;
    }
    out->type = type;
    out->len = (size_t)count;
    return KB_OK;
}

static kb_status read_ext(kb_mp_reader *reader, size_t width, kb_mp_value *out)
{
    const uint8_t *at;
    kb_status st = read_blob(reader, width, KB_MP_EXT, out);
    if (st != KB_OK) {
        return st;
    }
    /* Байт типа стоит после длины, но до данных: перечитываем аккуратно. */
    if (out->len == 0) {
        return KB_ERR_TRUNCATED;
    }
    at = out->data;
    out->ext_type = (int8_t)at[0];
    out->data = at + 1;
    out->len -= 1;
    return KB_OK;
}

static kb_status read_fixext(kb_mp_reader *reader, size_t payload,
                             kb_mp_value *out)
{
    const uint8_t *at;
    kb_status st = take(reader, payload + 1, &at);
    if (st != KB_OK) {
        return st;
    }
    out->type = KB_MP_EXT;
    out->ext_type = (int8_t)at[0];
    out->data = at + 1;
    out->len = payload;
    return KB_OK;
}

kb_status kb_mp_read(kb_mp_reader *reader, kb_mp_value *out)
{
    const uint8_t *at;
    uint8_t tag;
    uint64_t raw;
    kb_status st;

    if (reader == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (reader->buf == NULL && reader->cap != 0) {
        return KB_ERR_INVALID;
    }

    memset(out, 0, sizeof *out);

    st = take(reader, 1, &at);
    if (st != KB_OK) {
        return st;
    }
    tag = at[0];

    if (tag <= 0x7F) {
        out->type = KB_MP_UINT;
        out->u = tag;
        return KB_OK;
    }
    if (tag >= 0xE0) {
        out->type = KB_MP_INT;
        out->i = (int64_t)(int8_t)tag;
        return KB_OK;
    }
    if (tag >= 0x80 && tag <= 0x8F) {
        uint64_t count = (uint64_t)(tag & 0x0F);
        if (count * 2 > (uint64_t)(reader->cap - reader->pos)) {
            return KB_ERR_TRUNCATED;
        }
        out->type = KB_MP_MAP;
        out->len = (size_t)count;
        return KB_OK;
    }
    if (tag >= 0x90 && tag <= 0x9F) {
        uint64_t count = (uint64_t)(tag & 0x0F);
        if (count > (uint64_t)(reader->cap - reader->pos)) {
            return KB_ERR_TRUNCATED;
        }
        out->type = KB_MP_ARRAY;
        out->len = (size_t)count;
        return KB_OK;
    }
    if (tag >= 0xA0 && tag <= 0xBF) {
        size_t len = (size_t)(tag & 0x1F);
        st = take(reader, len, &at);
        if (st != KB_OK) {
            return st;
        }
        out->type = KB_MP_STR;
        out->data = at;
        out->len = len;
        return KB_OK;
    }

    switch (tag) {
    case 0xC0:
        out->type = KB_MP_NIL;
        return KB_OK;
    case 0xC1:
        /* Значение, которого в формате не существует. */
        return KB_ERR_INVALID;
    case 0xC2:
    case 0xC3:
        out->type = KB_MP_BOOL;
        out->boolean = tag == 0xC3;
        return KB_OK;
    case 0xC4:
        return read_blob(reader, 1, KB_MP_BIN, out);
    case 0xC5:
        return read_blob(reader, 2, KB_MP_BIN, out);
    case 0xC6:
        return read_blob(reader, 4, KB_MP_BIN, out);
    case 0xC7:
        return read_ext(reader, 1, out);
    case 0xC8:
        return read_ext(reader, 2, out);
    case 0xC9:
        return read_ext(reader, 4, out);
    case 0xCA: {
        uint32_t bits;
        float value;
        st = read_be(reader, 4, &raw);
        if (st != KB_OK) {
            return st;
        }
        bits = (uint32_t)raw;
        memcpy(&value, &bits, sizeof value);
        out->type = KB_MP_FLOAT;
        out->f = (double)value;
        return KB_OK;
    }
    case 0xCB: {
        double value;
        st = read_be(reader, 8, &raw);
        if (st != KB_OK) {
            return st;
        }
        memcpy(&value, &raw, sizeof value);
        out->type = KB_MP_FLOAT;
        out->f = value;
        return KB_OK;
    }
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF: {
        size_t width = (size_t)1 << (tag - 0xCC);
        st = read_be(reader, width, &raw);
        if (st != KB_OK) {
            return st;
        }
        out->type = KB_MP_UINT;
        out->u = raw;
        return KB_OK;
    }
    case 0xD0:
    case 0xD1:
    case 0xD2:
    case 0xD3: {
        size_t width = (size_t)1 << (tag - 0xD0);
        st = read_be(reader, width, &raw);
        if (st != KB_OK) {
            return st;
        }
        out->type = KB_MP_INT;
        switch (width) {
        case 1:
            out->i = (int64_t)(int8_t)(uint8_t)raw;
            break;
        case 2:
            out->i = (int64_t)(int16_t)(uint16_t)raw;
            break;
        case 4:
            out->i = (int64_t)(int32_t)(uint32_t)raw;
            break;
        default:
            /* Дополнительный код: на всех целевых платформах так и есть. */
            out->i = (int64_t)raw;
            break;
        }
        return KB_OK;
    }
    case 0xD4:
        return read_fixext(reader, 1, out);
    case 0xD5:
        return read_fixext(reader, 2, out);
    case 0xD6:
        return read_fixext(reader, 4, out);
    case 0xD7:
        return read_fixext(reader, 8, out);
    case 0xD8:
        return read_fixext(reader, 16, out);
    case 0xD9:
        return read_blob(reader, 1, KB_MP_STR, out);
    case 0xDA:
        return read_blob(reader, 2, KB_MP_STR, out);
    case 0xDB:
        return read_blob(reader, 4, KB_MP_STR, out);
    case 0xDC:
        return read_count(reader, 2, KB_MP_ARRAY, out);
    case 0xDD:
        return read_count(reader, 4, KB_MP_ARRAY, out);
    case 0xDE:
        return read_count(reader, 2, KB_MP_MAP, out);
    case 0xDF:
        return read_count(reader, 4, KB_MP_MAP, out);
    default:
        break;
    }
    return KB_ERR_INVALID;
}

kb_status kb_mp_skip(kb_mp_reader *reader)
{
    size_t pending = 1;

    if (reader == NULL) {
        return KB_ERR_INVALID;
    }

    while (pending > 0) {
        kb_mp_value value;
        kb_status st = kb_mp_read(reader, &value);
        if (st != KB_OK) {
            return st;
        }
        pending--;
        /* read_count уже проверил, что заявленное количество влезает в остаток
           буфера, так что счётчик не переполнится. */
        if (value.type == KB_MP_ARRAY) {
            pending += value.len;
        } else if (value.type == KB_MP_MAP) {
            pending += value.len * 2;
        }
    }
    return KB_OK;
}

int kb_mp_str_eq(const kb_mp_value *value, const char *text)
{
    size_t len;

    if (value == NULL || text == NULL || value->type != KB_MP_STR) {
        return 0;
    }
    len = strlen(text);
    if (len != value->len) {
        return 0;
    }
    return len == 0 || memcmp(value->data, text, len) == 0;
}

/* Обход map верхнего уровня; found_at получает позицию значения. */
static kb_status map_scan(const uint8_t *data, size_t len, const char *key,
                          kb_mp_reader *reader, size_t *value_start)
{
    kb_mp_value head;
    size_t index;
    kb_status st;

    if (key == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_mp_reader_init(reader, data, len);
    if (st != KB_OK) {
        return st;
    }
    st = kb_mp_read(reader, &head);
    if (st != KB_OK) {
        return st;
    }
    if (head.type != KB_MP_MAP) {
        return KB_ERR_INVALID;
    }

    for (index = 0; index < head.len; index++) {
        kb_mp_value found;
        size_t before = reader->pos;

        st = kb_mp_read(reader, &found);
        if (st != KB_OK) {
            return st;
        }
        if (found.type == KB_MP_STR && kb_mp_str_eq(&found, key)) {
            *value_start = reader->pos;
            return KB_OK;
        }

        /* Ключ мог оказаться составным — откатываемся и пропускаем целиком. */
        reader->pos = before;
        st = kb_mp_skip(reader);
        if (st != KB_OK) {
            return st;
        }
        st = kb_mp_skip(reader);
        if (st != KB_OK) {
            return st;
        }
    }
    return KB_ERR_NOT_FOUND;
}

kb_status kb_mp_map_get(const uint8_t *data, size_t len, const char *key,
                        kb_mp_value *out)
{
    kb_mp_reader reader;
    size_t value_start = 0;
    kb_status st;

    if (out == NULL) {
        return KB_ERR_INVALID;
    }
    st = map_scan(data, len, key, &reader, &value_start);
    if (st != KB_OK) {
        return st;
    }
    return kb_mp_read(&reader, out);
}

kb_status kb_mp_map_get_raw(const uint8_t *data, size_t len, const char *key,
                            const uint8_t **out, size_t *out_len)
{
    kb_mp_reader reader;
    size_t value_start = 0;
    kb_status st;

    if (out == NULL || out_len == NULL) {
        return KB_ERR_INVALID;
    }
    st = map_scan(data, len, key, &reader, &value_start);
    if (st != KB_OK) {
        return st;
    }
    st = kb_mp_skip(&reader);
    if (st != KB_OK) {
        return st;
    }
    *out = data + value_start;
    *out_len = reader.pos - value_start;
    return KB_OK;
}

int kb_mp_map_has(const uint8_t *data, size_t len, const char *key)
{
    kb_mp_reader reader;
    size_t value_start = 0;
    return map_scan(data, len, key, &reader, &value_start) == KB_OK;
}

/* ─── Запись ───────────────────────────────────────────────────────────── */

kb_status kb_mp_writer_init(kb_mp_writer *writer, uint8_t *buffer,
                            size_t capacity)
{
    if (writer == NULL || (buffer == NULL && capacity != 0)) {
        return KB_ERR_INVALID;
    }
    writer->buf = buffer;
    writer->cap = capacity;
    writer->len = 0;
    writer->status = KB_OK;
    return KB_OK;
}

kb_status kb_mp_writer_status(const kb_mp_writer *writer)
{
    return writer == NULL ? KB_ERR_INVALID : writer->status;
}

size_t kb_mp_writer_len(const kb_mp_writer *writer)
{
    return writer == NULL ? 0 : writer->len;
}

static void put(kb_mp_writer *writer, const uint8_t *data, size_t count)
{
    if (writer == NULL || writer->status != KB_OK) {
        return;
    }
    if (count > writer->cap - writer->len) {
        writer->status = KB_ERR_TOO_LARGE;
        return;
    }
    if (count > 0) {
        memcpy(writer->buf + writer->len, data, count);
        writer->len += count;
    }
}

static void put_byte(kb_mp_writer *writer, uint8_t value)
{
    put(writer, &value, 1);
}

static void put_be(kb_mp_writer *writer, uint64_t value, size_t width)
{
    uint8_t tmp[8];
    size_t i;
    for (i = 0; i < width; i++) {
        tmp[i] = (uint8_t)(value >> (8 * (width - 1 - i)));
    }
    put(writer, tmp, width);
}

static void put_tagged(kb_mp_writer *writer, uint8_t tag, uint64_t value,
                       size_t width)
{
    put_byte(writer, tag);
    put_be(writer, value, width);
}

void kb_mp_write_nil(kb_mp_writer *writer)
{
    put_byte(writer, 0xC0);
}

void kb_mp_write_bool(kb_mp_writer *writer, int value)
{
    put_byte(writer, value ? 0xC3 : 0xC2);
}

void kb_mp_write_u64(kb_mp_writer *writer, uint64_t value)
{
    if (value < 0x80u) {
        put_byte(writer, (uint8_t)value);
    } else if (value <= 0xFFu) {
        put_tagged(writer, 0xCC, value, 1);
    } else if (value <= 0xFFFFu) {
        put_tagged(writer, 0xCD, value, 2);
    } else if (value <= 0xFFFFFFFFu) {
        put_tagged(writer, 0xCE, value, 4);
    } else {
        put_tagged(writer, 0xCF, value, 8);
    }
}

void kb_mp_write_i64(kb_mp_writer *writer, int64_t value)
{
    /* Неотрицательные уходят беззнаковыми — так же выбирает эталон. */
    if (value >= 0) {
        kb_mp_write_u64(writer, (uint64_t)value);
        return;
    }
    if (value >= -32) {
        put_byte(writer, (uint8_t)(int8_t)value);
    } else if (value >= -128) {
        put_tagged(writer, 0xD0, (uint64_t)(uint8_t)(int8_t)value, 1);
    } else if (value >= -32768) {
        put_tagged(writer, 0xD1, (uint64_t)(uint16_t)(int16_t)value, 2);
    } else if (value >= -2147483647 - 1) {
        put_tagged(writer, 0xD2, (uint64_t)(uint32_t)(int32_t)value, 4);
    } else {
        put_tagged(writer, 0xD3, (uint64_t)value, 8);
    }
}

void kb_mp_write_f64(kb_mp_writer *writer, double value)
{
    uint64_t bits;
    memcpy(&bits, &value, sizeof bits);
    put_tagged(writer, 0xCB, bits, 8);
}

void kb_mp_write_str(kb_mp_writer *writer, const char *text, size_t len)
{
    if (writer == NULL) {
        return;
    }
    if (text == NULL && len != 0) {
        writer->status = KB_ERR_INVALID;
        return;
    }
    if (len < 32) {
        put_byte(writer, (uint8_t)(0xA0 | len));
    } else if (len <= 0xFFu) {
        put_tagged(writer, 0xD9, (uint64_t)len, 1);
    } else if (len <= 0xFFFFu) {
        put_tagged(writer, 0xDA, (uint64_t)len, 2);
    } else if ((uint64_t)len <= 0xFFFFFFFFu) {
        put_tagged(writer, 0xDB, (uint64_t)len, 4);
    } else {
        writer->status = KB_ERR_INVALID;
        return;
    }
    put(writer, (const uint8_t *)text, len);
}

void kb_mp_write_cstr(kb_mp_writer *writer, const char *text)
{
    kb_mp_write_str(writer, text, text == NULL ? 0 : strlen(text));
}

void kb_mp_write_bin(kb_mp_writer *writer, const uint8_t *data, size_t len)
{
    if (writer == NULL) {
        return;
    }
    if (data == NULL && len != 0) {
        writer->status = KB_ERR_INVALID;
        return;
    }
    if (len <= 0xFFu) {
        put_tagged(writer, 0xC4, (uint64_t)len, 1);
    } else if (len <= 0xFFFFu) {
        put_tagged(writer, 0xC5, (uint64_t)len, 2);
    } else if ((uint64_t)len <= 0xFFFFFFFFu) {
        put_tagged(writer, 0xC6, (uint64_t)len, 4);
    } else {
        writer->status = KB_ERR_INVALID;
        return;
    }
    put(writer, data, len);
}

void kb_mp_write_ext(kb_mp_writer *writer, int8_t ext_type, const uint8_t *data,
                     size_t len)
{
    if (writer == NULL) {
        return;
    }
    if (data == NULL && len != 0) {
        writer->status = KB_ERR_INVALID;
        return;
    }
    if (len == 1) {
        put_byte(writer, 0xD4);
    } else if (len == 2) {
        put_byte(writer, 0xD5);
    } else if (len == 4) {
        put_byte(writer, 0xD6);
    } else if (len == 8) {
        put_byte(writer, 0xD7);
    } else if (len == 16) {
        put_byte(writer, 0xD8);
    } else if (len <= 0xFFu) {
        put_tagged(writer, 0xC7, (uint64_t)len, 1);
    } else if (len <= 0xFFFFu) {
        put_tagged(writer, 0xC8, (uint64_t)len, 2);
    } else if ((uint64_t)len <= 0xFFFFFFFFu) {
        put_tagged(writer, 0xC9, (uint64_t)len, 4);
    } else {
        writer->status = KB_ERR_INVALID;
        return;
    }
    put_byte(writer, (uint8_t)ext_type);
    put(writer, data, len);
}

void kb_mp_write_array(kb_mp_writer *writer, size_t count)
{
    if (count < 16) {
        put_byte(writer, (uint8_t)(0x90 | count));
    } else if (count <= 0xFFFFu) {
        put_tagged(writer, 0xDC, (uint64_t)count, 2);
    } else {
        put_tagged(writer, 0xDD, (uint64_t)count, 4);
    }
}

void kb_mp_write_map(kb_mp_writer *writer, size_t count)
{
    if (count < 16) {
        put_byte(writer, (uint8_t)(0x80 | count));
    } else if (count <= 0xFFFFu) {
        put_tagged(writer, 0xDE, (uint64_t)count, 2);
    } else {
        put_tagged(writer, 0xDF, (uint64_t)count, 4);
    }
}

void kb_mp_write_raw(kb_mp_writer *writer, const uint8_t *data, size_t len)
{
    if (writer == NULL) {
        return;
    }
    if (data == NULL && len != 0) {
        writer->status = KB_ERR_INVALID;
        return;
    }
    put(writer, data, len);
}
