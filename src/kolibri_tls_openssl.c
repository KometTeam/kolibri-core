/*
 * TLS-бэкенд на OpenSSL: пример реализации kb_tls_vtable.
 *
 * Файл собирается только при KB_WITH_OPENSSL, поэтому в обычной сборке ядро
 * остаётся без единой внешней зависимости.
 */

#include "kolibri.h"
#include "kolibri_transport.h"

#if defined(KB_WITH_OPENSSL)

#include <string.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

typedef struct {
    SSL_CTX *ctx;
    SSL *ssl;
    char detail[192];
} kb_openssl;

size_t kb_tls_openssl_state_size(void)
{
    return sizeof(kb_openssl);
}

static void remember(kb_openssl *state, const char *prefix)
{
    unsigned long code = ERR_get_error();
    size_t at = 0;
    size_t len = strlen(prefix);

    if (len >= sizeof state->detail) {
        len = sizeof state->detail - 1;
    }
    memcpy(state->detail, prefix, len);
    at = len;

    if (code != 0 && at + 2 < sizeof state->detail) {
        state->detail[at++] = ':';
        state->detail[at++] = ' ';
        ERR_error_string_n(code, state->detail + at, sizeof state->detail - at);
    } else {
        state->detail[at] = '\0';
    }

    /*
     * Хвост той же ошибки выгребаем до конца. Иначе он всплывёт в следующем
     * нашем вызове как чужая причина — или, что хуже, у постороннего кода в
     * этом же потоке: очередь у OpenSSL общая.
     */
    while (ERR_get_error() != 0) {
        /* пусто */
    }
}

static int openssl_handshake(void *ctx)
{
    kb_openssl *state = (kb_openssl *)ctx;
    int result;

    /* Чистим очередь: иначе remember() покажет ошибку от прошлого вызова. */
    ERR_clear_error();
    result = SSL_connect(state->ssl);

    if (result == 1) {
        return 0;
    }
    switch (SSL_get_error(state->ssl, result)) {
    case SSL_ERROR_WANT_READ:
        return KB_IO_WANT_READ;
    case SSL_ERROR_WANT_WRITE:
        return KB_IO_WANT_WRITE;
    default:
        break;
    }
    remember(state, "TLS-рукопожатие не удалось");
    return KB_IO_FAILED;
}

static long openssl_read(void *ctx, uint8_t *buf, size_t len)
{
    kb_openssl *state = (kb_openssl *)ctx;
    int n;

    if (len > 0x7FFFFFFF) {
        len = 0x7FFFFFFF;
    }
    ERR_clear_error();
    n = SSL_read(state->ssl, buf, (int)len);
    if (n > 0) {
        return n;
    }
    switch (SSL_get_error(state->ssl, n)) {
    case SSL_ERROR_WANT_READ:
        return KB_IO_WANT_READ;
    case SSL_ERROR_WANT_WRITE:
        return KB_IO_WANT_WRITE;
    case SSL_ERROR_ZERO_RETURN:
        return KB_IO_CLOSED;
    default:
        break;
    }
    remember(state, "ошибка чтения TLS");
    return KB_IO_FAILED;
}

static long openssl_write(void *ctx, const uint8_t *buf, size_t len)
{
    kb_openssl *state = (kb_openssl *)ctx;
    int n;

    if (len > 0x7FFFFFFF) {
        len = 0x7FFFFFFF;
    }
    ERR_clear_error();
    n = SSL_write(state->ssl, buf, (int)len);
    if (n > 0) {
        return n;
    }
    switch (SSL_get_error(state->ssl, n)) {
    case SSL_ERROR_WANT_READ:
        return KB_IO_WANT_READ;
    case SSL_ERROR_WANT_WRITE:
        return KB_IO_WANT_WRITE;
    default:
        break;
    }
    remember(state, "ошибка записи TLS");
    return KB_IO_FAILED;
}

static void openssl_close(void *ctx)
{
    kb_openssl *state = (kb_openssl *)ctx;

    if (state->ssl != NULL) {
        SSL_shutdown(state->ssl);
        SSL_free(state->ssl);
        state->ssl = NULL;
    }
    if (state->ctx != NULL) {
        SSL_CTX_free(state->ctx);
        state->ctx = NULL;
    }
}

static const char *openssl_detail(void *ctx)
{
    kb_openssl *state = (kb_openssl *)ctx;
    return state->detail[0] != '\0' ? state->detail : "ok";
}

static const kb_tls_vtable openssl_vtable = {
    openssl_handshake, openssl_read, openssl_write, openssl_close,
    openssl_detail
};

kb_status kb_tls_openssl_init(kb_tls *tls, void *state_memory,
                              size_t state_size, intptr_t fd,
                              const char *server_name, const char *ca_file,
                              int insecure)
{
    kb_openssl *state = (kb_openssl *)state_memory;

    if (tls == NULL || state_memory == NULL ||
        state_size < sizeof(kb_openssl) || server_name == NULL ||
        server_name[0] == '\0') {
        /* Пустое имя молча отключало бы проверку имени в сертификате, а
           функция рапортовала бы об успехе. Лучше честный отказ. */
        return KB_ERR_INVALID;
    }
    memset(state, 0, sizeof *state);
    /* Очередь могла остаться от прошлого init или от чужой библиотеки. */
    ERR_clear_error();

    state->ctx = SSL_CTX_new(TLS_client_method());
    if (state->ctx == NULL) {
        remember(state, "SSL_CTX_new");
        return KB_ERR_STATE;
    }
    SSL_CTX_set_min_proto_version(state->ctx, TLS1_2_VERSION);

    if (insecure) {
        SSL_CTX_set_verify(state->ctx, SSL_VERIFY_NONE, NULL);
    } else {
        SSL_CTX_set_verify(state->ctx, SSL_VERIFY_PEER, NULL);
        if (ca_file != NULL) {
            if (SSL_CTX_load_verify_locations(state->ctx, ca_file, NULL) != 1) {
                remember(state, "не загрузились доверенные корни");
                SSL_CTX_free(state->ctx);
                state->ctx = NULL;
                return KB_ERR_STATE;
            }
        } else if (SSL_CTX_set_default_verify_paths(state->ctx) != 1) {
            remember(state, "нет системного хранилища корней");
            SSL_CTX_free(state->ctx);
            state->ctx = NULL;
            return KB_ERR_STATE;
        }
    }

    state->ssl = SSL_new(state->ctx);
    if (state->ssl == NULL) {
        remember(state, "SSL_new");
        SSL_CTX_free(state->ctx);
        state->ctx = NULL;
        return KB_ERR_STATE;
    }

    if (SSL_set_fd(state->ssl, (int)fd) != 1) {
        remember(state, "SSL_set_fd");
        openssl_close(state);
        return KB_ERR_STATE;
    }
    /* SNI: без него сервер за общим адресом отдаст не тот сертификат. */
    if (SSL_set_tlsext_host_name(state->ssl, server_name) != 1) {
        remember(state, "SNI не установился");
        openssl_close(state);
        return KB_ERR_STATE;
    }
    if (!insecure && SSL_set1_host(state->ssl, server_name) != 1) {
        remember(state, "проверка имени не настроилась");
        openssl_close(state);
        return KB_ERR_STATE;
    }

    tls->vt = &openssl_vtable;
    tls->ctx = state;
    return KB_OK;
}

#endif /* KB_WITH_OPENSSL */
