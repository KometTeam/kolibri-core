/*
 * Мост MessagePack ↔ JSON.
 *
 * Для логов и удобной отправки произвольных пейлоадов через JSON вместо ручной
 * сборки msgpack. Работает над байтами, без дерева значений: одна проходка в
 * каждую сторону, всё в буфер вызывающего.
 *
 * msgpack → JSON лоссовый (для чтения): Binary/Ext становятся base64, нестроковые
 * ключи map стягиваются в строки — обратно в тот же msgpack плоский вид не
 * свернётся. Обратное направление восстанавливает то, что хост пометил явно:
 * {"$bin":…}, {"$ext":…,"data":…} и ключ "$int:<n>".
 */

#ifndef KOLIBRI_MPJSON_H
#define KOLIBRI_MPJSON_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * MessagePack → JSON-текст (с '\0'). tagged=0: Binary → плоская base64-строка
 * (читаемо, но не восстановимо); tagged=1: Binary → {"$bin":"<b64>"}, что
 * kb_json_to_mp разворачивает обратно. out_len — длина без '\0'.
 */
kb_status kb_mp_to_json(const uint8_t *mp, size_t mp_len, int tagged, char *out,
                        size_t cap, size_t *out_len);

/*
 * JSON-текст → MessagePack. Восстанавливает {"$bin":…} → Binary,
 * {"$ext":tag,"data":…} → Ext, ключ "$int:<n>" → целочисленный ключ map.
 * scratch — временный буфер под раскодированные строки и base64 (переиспользуется
 * на каждый лист, так что хватает размера самой длинной строки/бинарника).
 */
kb_status kb_json_to_mp(const char *json, size_t json_len, uint8_t *scratch,
                        size_t scratch_cap, uint8_t *out, size_t cap,
                        size_t *out_len);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_MPJSON_H */
