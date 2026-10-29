/*
 * Мелкий JSON-ридер для звонков: и vcp, и вся сигнализация ws2 — это JSON.
 *
 * Как и msgpack-ридер, ничего не копирует и не аллоцирует: kb_json_value —
 * срез, указывающий внутрь исходного буфера. Дерева нет; во вложенность
 * ходишь повторным kb_json_object_get по возвращённому срезу. Ровно та же
 * идея, что kb_mp_map_get.
 */

#ifndef KOLIBRI_JSON_H
#define KOLIBRI_JSON_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
    KB_JSON_NULL = 0,
    KB_JSON_BOOL,
    KB_JSON_NUMBER,
    KB_JSON_STRING,
    KB_JSON_ARRAY,
    KB_JSON_OBJECT
} kb_json_type;

/*
 * Один разобранный узел. start/len покрывают весь токен целиком: строку — со
 * скобками-кавычками, объект — от '{' до '}'. Для строки содержимое (без
 * кавычек, с ещё не раскрытыми escape) достаёт kb_json_str_*.
 */
typedef struct {
    kb_json_type type;
    const char *start;
    size_t len;
} kb_json_value;

/*
 * Разобрать ровно одно значение верхнего уровня и проверить его целиком.
 * Хвост из пробелов допускается, любой другой мусор после значения — ошибка.
 */
kb_status kb_json_parse(const char *buf, size_t len, kb_json_value *out);

/* Найти поле объекта. KB_ERR_NOT_FOUND, если ключа нет. */
kb_status kb_json_object_get(const kb_json_value *object, const char *key,
                             kb_json_value *out);

/*
 * Итерация массива. Курсор — kb_json_value; инициализируй нулём (type/len 0).
 * Возвращает 1 и заполняет out очередным элементом, 0 — когда элементы кончились.
 *
 *   kb_json_value it, elem;
 *   kb_json_array_iter_init(&it, &array);
 *   while (kb_json_array_next(&it, &elem)) { ... }
 */
void kb_json_array_iter_init(kb_json_value *iter, const kb_json_value *array);
int kb_json_array_next(kb_json_value *iter, kb_json_value *out);

/*
 * Итерация полей объекта. Курсор — kb_json_value, инициализируется
 * kb_json_object_iter_init. Возвращает 1 и заполняет key (строковый узел) и
 * val очередной парой, 0 — когда поля кончились.
 */
void kb_json_object_iter_init(kb_json_value *iter, const kb_json_value *object);
int kb_json_object_next(kb_json_value *iter, kb_json_value *key,
                        kb_json_value *val);

/* Геттеры. Возвращают KB_ERR_INVALID, если узел не того типа. */
kb_status kb_json_as_bool(const kb_json_value *value, int *out);
kb_status kb_json_as_i64(const kb_json_value *value, int64_t *out);

/*
 * Сравнить строковый узел с C-строкой, раскрывая escape в узле. Для не-строки
 * или несовпадения — 0.
 */
int kb_json_str_eq(const kb_json_value *value, const char *text);

/*
 * Скопировать строковый узел, раскрыв escape, и завершить '\0'. При нехватке
 * буфера — KB_ERR_TOO_LARGE. out_len (без учёта '\0') можно передать NULL.
 */
kb_status kb_json_str_copy(const kb_json_value *value, char *out, size_t cap,
                           size_t *out_len);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_JSON_H */
