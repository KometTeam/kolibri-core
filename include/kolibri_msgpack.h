/*
 * MessagePack: чтение курсором и запись в буфер вызывающей стороны.
 *
 * Подмножество, которого хватает протоколу. Читатель понимает все типы формата
 * (иначе нельзя корректно пропустить чужое значение), писатель умеет то, что
 * протокол реально отправляет.
 *
 * Правила те же, что у остального ядра: ноль аллокаций, ноль рекурсии.
 * Строки и бинарники при чтении НЕ копируются — kb_mp_value указывает внутрь
 * исходного буфера, и живёт это ровно столько, сколько живёт буфер.
 *
 * Требуется int64_t из <stdint.h>: протокол передаёт 64-битные идентификаторы,
 * так что платформа без 64-битного целого не поддерживается.
 */

#ifndef KOLIBRI_MSGPACK_H
#define KOLIBRI_MSGPACK_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
    KB_MP_NIL = 0,
    KB_MP_BOOL,
    KB_MP_INT,
    KB_MP_UINT,
    KB_MP_FLOAT,
    KB_MP_STR,
    KB_MP_BIN,
    KB_MP_ARRAY,
    KB_MP_MAP,
    KB_MP_EXT
} kb_mp_type;

/*
 * Разобранное значение. Union не используется намеренно: в C89 он должен быть
 * именованным, а обращение к неактивному члену — серая зона, которая ещё и
 * плохо переносится через FFI.
 */
typedef struct {
    kb_mp_type type;
    /* KB_MP_BOOL */
    int boolean;
    /* KB_MP_INT */
    int64_t i;
    /* KB_MP_UINT */
    uint64_t u;
    /* KB_MP_FLOAT */
    double f;
    /* KB_MP_STR, KB_MP_BIN, KB_MP_EXT: указатель внутрь исходного буфера. */
    const uint8_t *data;
    /* Длина data, либо число элементов для KB_MP_ARRAY и пар для KB_MP_MAP. */
    size_t len;
    /* KB_MP_EXT */
    int8_t ext_type;
} kb_mp_value;

/* ─── Чтение ───────────────────────────────────────────────────────────── */

typedef struct {
    const uint8_t *buf;
    size_t cap;
    size_t pos;
} kb_mp_reader;

kb_status kb_mp_reader_init(kb_mp_reader *reader, const uint8_t *data,
                            size_t len);

size_t kb_mp_reader_pos(const kb_mp_reader *reader);

/* Сколько байт осталось непрочитанными. */
size_t kb_mp_reader_left(const kb_mp_reader *reader);

/*
 * Прочитать одно значение. Для массива и map возвращается только заголовок:
 * элементы читаются следующими вызовами, их количество лежит в out->len.
 */
kb_status kb_mp_read(kb_mp_reader *reader, kb_mp_value *out);

/* Пропустить значение целиком, вместе с вложенными. Без рекурсии. */
kb_status kb_mp_skip(kb_mp_reader *reader);

/* Сравнить строковое значение с C-строкой. */
int kb_mp_str_eq(const kb_mp_value *value, const char *text);

/*
 * Достать значение по ключу из map верхнего уровня.
 *
 * `data` должен указывать на начало map. Вложенность обходится повторным
 * вызовом по data/len найденного значения, если оно само map.
 * Возвращает KB_ERR_INVALID, если это не map, и KB_ERR_NOT_FOUND, если ключа
 * нет.
 */
kb_status kb_mp_map_get(const uint8_t *data, size_t len, const char *key,
                        kb_mp_value *out);

int kb_mp_map_has(const uint8_t *data, size_t len, const char *key);

/*
 * Границы значения по ключу внутри буфера — нужно, чтобы передать вложенную
 * map дальше по коду, не копируя её.
 */
kb_status kb_mp_map_get_raw(const uint8_t *data, size_t len, const char *key,
                            const uint8_t **out, size_t *out_len);

/* ─── Запись ───────────────────────────────────────────────────────────── */

/*
 * Ошибка «прилипает»: можно записать весь payload подряд и проверить один раз
 * в конце, а не после каждого вызова.
 */
typedef struct {
    uint8_t *buf;
    size_t cap;
    size_t len;
    kb_status status;
} kb_mp_writer;

kb_status kb_mp_writer_init(kb_mp_writer *writer, uint8_t *buffer,
                            size_t capacity);

/* KB_OK, если всё записалось; иначе первая случившаяся ошибка. */
kb_status kb_mp_writer_status(const kb_mp_writer *writer);

/* Сколько байт записано. Осмысленно только при KB_OK. */
size_t kb_mp_writer_len(const kb_mp_writer *writer);

void kb_mp_write_nil(kb_mp_writer *writer);
void kb_mp_write_bool(kb_mp_writer *writer, int value);

/*
 * Целые кодируются кратчайшим представлением, а неотрицательные — как
 * беззнаковые: ровно так же выбирает эталонная реализация, поэтому байты
 * совпадают с ней один в один.
 */
void kb_mp_write_i64(kb_mp_writer *writer, int64_t value);
void kb_mp_write_u64(kb_mp_writer *writer, uint64_t value);
void kb_mp_write_f64(kb_mp_writer *writer, double value);

void kb_mp_write_str(kb_mp_writer *writer, const char *text, size_t len);
void kb_mp_write_cstr(kb_mp_writer *writer, const char *text);
void kb_mp_write_bin(kb_mp_writer *writer, const uint8_t *data, size_t len);
void kb_mp_write_ext(kb_mp_writer *writer, int8_t ext_type, const uint8_t *data,
                     size_t len);

/* Заголовок: следом пишутся count значений (для map — count пар). */
void kb_mp_write_array(kb_mp_writer *writer, size_t count);
void kb_mp_write_map(kb_mp_writer *writer, size_t count);

/* Вставить готовый кусок msgpack как есть. */
void kb_mp_write_raw(kb_mp_writer *writer, const uint8_t *data, size_t len);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_MSGPACK_H */
