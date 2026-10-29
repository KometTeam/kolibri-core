/*
 * TLS-бэкенд на mbedTLS.
 *
 * Тот же интерфейс kb_tls_vtable, что и у OpenSSL-адаптера, но именно этот
 * нужен на старых платформах: системного TLS 1.2 там нет, а OpenSSL со своими
 * мегабайтами в приложение той эпохи не влезает.
 *
 * Собирается при KB_WITH_MBEDTLS. mbedTLS стоит собирать урезанным — только
 * клиент TLS 1.2 и нужные шифронаборы; конфигурация лежит в
 * tools/mbedtls_config.h.
 */

#ifndef KOLIBRI_TLS_MBEDTLS_H
#define KOLIBRI_TLS_MBEDTLS_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri_transport.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Сколько байт занимает состояние: выделяет вызывающая сторона. */
size_t kb_tls_mbedtls_state_size(void);

/*
 * Настроить TLS поверх подключённого сокета.
 *
 * server_name — имя для SNI и проверки сертификата, пустое не принимается.
 * ca_pem      — доверенные корни в PEM, прямо в памяти: на старых платформах
 *               системного хранилища нет, и корень (например Минцифры) проще
 *               вшить в бинарь. NULL вместе с insecure == 0 — ошибка.
 * ca_pem_len  — длина вместе с завершающим нулём: mbedTLS этого требует.
 * insecure    — не проверять сертификат. Только для отладки.
 */
kb_status kb_tls_mbedtls_init(kb_tls *tls, void *state, size_t state_size,
                              intptr_t fd, const char *server_name,
                              const char *ca_pem, size_t ca_pem_len,
                              int insecure);

const char *kb_tls_mbedtls_version(void);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_TLS_MBEDTLS_H */
