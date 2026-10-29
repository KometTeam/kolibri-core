/*
 * Вшитый корень Минцифры (api2.oneme.ru). См. src/kolibri_mincifry_ca.c.
 */
#ifndef KOLIBRI_CA_H
#define KOLIBRI_CA_H

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * PEM корня Минцифры. *len (можно NULL) — размер ВКЛЮЧАЯ завершающий 0
 * (kb_tls_mbedtls_init требует именно такую длину для PEM).
 * Передавай как ca_pem в TLS-init для проверки цепочки *.oneme.ru за Минцифрой.
 */
const char *kb_mincifry_ca_pem(size_t *len);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_CA_H */
