/*
 * Кадры WebSocket (RFC 6455) для сигнализации ws2.
 *
 * Ровно как машина HTTP-ответа в медиа: хост ведёт TCP и TLS, а разбор и сборку
 * кадров берёт ядро. Клиент обязан маскировать исходящие кадры; серверные
 * приходят без маски (замаскированный кадр от сервера — протокольная ошибка,
 * такой отвергаем).
 */

#ifndef KOLIBRI_WS_H
#define KOLIBRI_WS_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define KB_WS_CONT   0x0
#define KB_WS_TEXT   0x1
#define KB_WS_BINARY 0x2
#define KB_WS_CLOSE  0x8
#define KB_WS_PING   0x9
#define KB_WS_PONG   0xA

/*
 * Собрать замаскированный клиентский кадр. mask_key — 32 бита энтропии от
 * хоста (RFC требует случайный ключ на кадр; ядру часов и random взять
 * неоткуда). out_len — длина готового кадра. KB_ERR_TOO_LARGE — мал буфер.
 */
kb_status kb_ws_build(uint8_t opcode, const uint8_t *payload,
                      size_t payload_len, uint32_t mask_key, uint8_t *out,
                      size_t cap, size_t *out_len);

/* Текстовый кадр (opcode TEXT). */
kb_status kb_ws_build_text(const char *text, size_t len, uint32_t mask_key,
                           uint8_t *out, size_t cap, size_t *out_len);

/* Один разобранный кадр. payload указывает внутрь buf. */
typedef struct {
    uint8_t opcode;
    int fin;
    const uint8_t *payload;
    size_t payload_len;
} kb_ws_frame;

/*
 * Разобрать один кадр из начала buf.
 *   KB_OK            — frame заполнен, *consumed = сколько байт съедено;
 *   KB_ERR_TRUNCATED — кадра пока не хватает, дождись ещё байт;
 *   KB_ERR_INVALID   — кадр битый или замаскирован сервером.
 */
kb_status kb_ws_parse(const uint8_t *buf, size_t len, kb_ws_frame *frame,
                      size_t *consumed);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_WS_H */
