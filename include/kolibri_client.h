/*
 * Блокирующий клиент: транспорт + TLS + сессия + цикл ввода-вывода в одном.
 *
 * Это УДОБСТВО поверх sans-IO ядра, а не часть его: клиент владеет сокетом и
 * сам крутит select-цикл, поэтому (в отличие от ядра) он аллоцирует буферы и
 * блокирует поток. Для event-loop это не нужно — там бери kb_session_<op>()
 * из kolibri_ops.h (кодирует типизированный запрос) и веди I/O сам.
 *
 * Типизированные методы на операцию (kb_client_<op>) генерируются из схемы и
 * лежат в kolibri_ops.h — они прячут опкод, кодирование, I/O и разбор ответа.
 */

#ifndef KOLIBRI_CLIENT_H
#define KOLIBRI_CLIENT_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri.h"
#include "kolibri_session.h"
#include "kolibri_transport.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    const char *host;
    uint16_t port;
    uint32_t connect_timeout_ms; /* 0 -> 15000 */
    uint32_t io_timeout_ms;      /* ожидание ответа на вызов; 0 -> 30000 */
    const kb_proxy_config *proxy;
    /*
     * Настроить TLS поверх уже подключённого сокета. NULL -> без шифрования.
     * Внутри обычно kb_tls_*_init(fd) + kb_transport_set_tls. Так клиент не
     * привязан к конкретному TLS-бэкенду (openssl/mbedtls).
     */
    int (*setup_tls)(kb_transport *transport, void *user);
    void *tls_user;
    /* Параметры handshake (deviceId, userAgent, ...). */
    kb_session_config session;
} kb_client_config;

typedef struct {
    kb_transport transport;
    kb_session session;
    kb_session_buffers bufs;
    uint8_t *rx;
    uint8_t *tx;
    uint8_t *scratch;
    kb_pending *pending;
    uint8_t *io;
    size_t io_cap;
    uint32_t io_timeout_ms;
    int has_calls_seed;
    int64_t calls_seed;
    int online;
    char detail[160];
} kb_client;

/*
 * Подключиться, поднять TLS (если задан) и пройти handshake. После успеха
 * доступен callsSeed и можно звать kb_client_<op>. Аллоцирует буферы —
 * освобождает kb_client_close.
 */
kb_status kb_client_open(kb_client *c, const kb_client_config *config);

int kb_client_has_calls_seed(const kb_client *c);
int64_t kb_client_calls_seed(const kb_client *c);

/*
 * Низкоуровневый вызов: послать готовый msgpack-пейлоад под опкодом и дождаться
 * ответа. *resp указывает внутрь внутреннего буфера и живёт до СЛЕДУЮЩЕГО
 * вызова. Обычно это прячут типизированные kb_client_<op> из kolibri_ops.h.
 */
kb_status kb_client_call(kb_client *c, uint16_t opcode, const uint8_t *payload,
                         size_t len, const uint8_t **resp, size_t *resp_len);

const char *kb_client_detail(const kb_client *c);
void kb_client_close(kb_client *c);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_CLIENT_H */
