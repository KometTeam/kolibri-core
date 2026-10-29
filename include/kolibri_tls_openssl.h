/*
 * TLS-бэкенд на OpenSSL.
 *
 * Собирается отдельно и только при KB_WITH_OPENSSL — ядро от OpenSSL не
 * зависит и знать про него не хочет. Это образец того, как выглядит любой
 * бэкенд: заполнить четыре функции в kb_tls_vtable.
 *
 * Для mbedTLS, wolfSSL, Secure Transport или SChannel файл пишется по тому же
 * шаблону и примерно того же размера. Именно mbedTLS нужен на старых iOS и
 * Windows Phone, где системного TLS с TLS 1.2 нет.
 *
 * OpenSSL здесь не подключается заголовком: состояние выделяет вызывающая
 * сторона блоком нужного размера, как и в остальном ядре.
 */

#ifndef KOLIBRI_TLS_OPENSSL_H
#define KOLIBRI_TLS_OPENSSL_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri_transport.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Сколько байт занимает состояние. Память под него выравнивай хотя бы под
   указатель: проще всего malloc или статический массив. */
size_t kb_tls_openssl_state_size(void);

/*
 * Настроить TLS поверх уже подключённого сокета.
 *
 * server_name — имя для SNI и проверки сертификата.
 * ca_file     — PEM с доверенными корнями; NULL означает системное хранилище.
 *               Для api2.oneme.ru сюда кладут корень Минцифры.
 * insecure    — не проверять сертификат. Только для отладки.
 *
 * После успеха гоняй kb_transport_handshake, пока не вернёт KB_OK.
 */
kb_status kb_tls_openssl_init(kb_tls *tls, void *state, size_t state_size,
                              intptr_t fd, const char *server_name,
                              const char *ca_file, int insecure);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_TLS_OPENSSL_H */
