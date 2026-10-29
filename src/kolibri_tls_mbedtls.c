/*
 * TLS-бэкенд на mbedTLS — второй адаптер к тому же kb_tls_vtable.
 *
 * Ради него интерфейс и делался: на iOS 6 и Windows Phone системного TLS 1.2
 * нет, а OpenSSL туда не помещается.
 */

#include "kolibri.h"
#include "kolibri_transport.h"
#include "kolibri_tls_mbedtls.h"

#if defined(KB_WITH_MBEDTLS)

#include <string.h>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/ssl.h>
#include <mbedtls/x509_crt.h>

typedef struct {
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt ca;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context drbg;
    /* Сокетом владеет наш транспорт, модуль net_sockets из mbedTLS не нужен —
       и в урезанной сборке его нет. Читаем и пишем своими колбэками. */
    intptr_t fd;
    int have_ca;
    char detail[192];
} kb_mbed;

size_t kb_tls_mbedtls_state_size(void)
{
    return sizeof(kb_mbed);
}

/* ─── Ввод-вывод: сокет наш, mbedTLS только шифрует ────────────────────── */

#if defined(_WIN32)
#  include <winsock2.h>
#  define KB_SOCK_CAST(fd) ((SOCKET)(fd))
#  define KB_RECV(fd, b, n) recv(KB_SOCK_CAST(fd), (char *)(b), (int)(n), 0)
#  define KB_SEND(fd, b, n) send(KB_SOCK_CAST(fd), (const char *)(b), (int)(n), 0)
#  define KB_WOULDBLOCK() (WSAGetLastError() == WSAEWOULDBLOCK)
#else
#  include <errno.h>
#  include <sys/socket.h>
#  define KB_RECV(fd, b, n) recv((int)(fd), (b), (n), 0)
#  if defined(MSG_NOSIGNAL)
#    define KB_SEND(fd, b, n) send((int)(fd), (b), (n), MSG_NOSIGNAL)
#  else
#    define KB_SEND(fd, b, n) send((int)(fd), (b), (n), 0)
#  endif
#  define KB_WOULDBLOCK() (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
#endif

static int bio_recv(void *ctx, unsigned char *buf, size_t len)
{
    kb_mbed *state = (kb_mbed *)ctx;
    long got = (long)KB_RECV(state->fd, buf, len);

    if (got > 0) {
        return (int)got;
    }
    if (got == 0) {
        /*
         * В BIO-колбэке mbedTLS ноль значит «прочитано ноль байт», а не конец
         * соединения: вернув его, рукопожатие зациклится. Для закрытого пира
         * есть отдельный код.
         */
        return MBEDTLS_ERR_SSL_CONN_EOF;
    }
    /* Коды MBEDTLS_ERR_NET_* живут в выключенном модуле net_sockets,
       поэтому отдаём общий отказ ввода-вывода. */
    return KB_WOULDBLOCK() ? MBEDTLS_ERR_SSL_WANT_READ
                           : MBEDTLS_ERR_SSL_INTERNAL_ERROR;
}

static int bio_send(void *ctx, const unsigned char *buf, size_t len)
{
    kb_mbed *state = (kb_mbed *)ctx;
    long sent = (long)KB_SEND(state->fd, buf, len);

    if (sent >= 0) {
        return (int)sent;
    }
    return KB_WOULDBLOCK() ? MBEDTLS_ERR_SSL_WANT_WRITE
                           : MBEDTLS_ERR_SSL_INTERNAL_ERROR;
}

static void remember(kb_mbed *state, const char *prefix, int code)
{
    size_t at = strlen(prefix);

    if (at >= sizeof state->detail) {
        at = sizeof state->detail - 1;
    }
    memcpy(state->detail, prefix, at);

    if (code != 0 && at + 2 < sizeof state->detail) {
        state->detail[at++] = ':';
        state->detail[at++] = ' ';
        mbedtls_strerror(code, state->detail + at, sizeof state->detail - at);
        return;
    }
    state->detail[at] = '\0';
}

/* mbedTLS не различает «нужно читать» и «нужно писать» кодом возврата
   операции, но сообщает это отдельными константами — переводим их в наши. */
static int translate(int code)
{
    if (code == MBEDTLS_ERR_SSL_WANT_READ) {
        return KB_IO_WANT_READ;
    }
    if (code == MBEDTLS_ERR_SSL_WANT_WRITE) {
        return KB_IO_WANT_WRITE;
    }
    return KB_IO_FAILED;
}

static int mbed_handshake(void *ctx)
{
    kb_mbed *state = (kb_mbed *)ctx;
    int code = mbedtls_ssl_handshake(&state->ssl);

    if (code == 0) {
        return 0;
    }
    if (code == MBEDTLS_ERR_SSL_WANT_READ ||
        code == MBEDTLS_ERR_SSL_WANT_WRITE) {
        return translate(code);
    }
    remember(state, "TLS-рукопожатие не удалось", code);
    return KB_IO_FAILED;
}

static long mbed_read(void *ctx, uint8_t *buf, size_t len)
{
    kb_mbed *state = (kb_mbed *)ctx;
    int code = mbedtls_ssl_read(&state->ssl, buf, len);

    if (code > 0) {
        return code;
    }
    if (code == 0 || code == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
        return KB_IO_CLOSED;
    }
    if (code == MBEDTLS_ERR_SSL_WANT_READ ||
        code == MBEDTLS_ERR_SSL_WANT_WRITE) {
        return translate(code);
    }
    remember(state, "ошибка чтения TLS", code);
    return KB_IO_FAILED;
}

static long mbed_write(void *ctx, const uint8_t *buf, size_t len)
{
    kb_mbed *state = (kb_mbed *)ctx;
    int code = mbedtls_ssl_write(&state->ssl, buf, len);

    if (code >= 0) {
        return code;
    }
    if (code == MBEDTLS_ERR_SSL_WANT_READ ||
        code == MBEDTLS_ERR_SSL_WANT_WRITE) {
        return translate(code);
    }
    remember(state, "ошибка записи TLS", code);
    return KB_IO_FAILED;
}

static void mbed_close(void *ctx)
{
    kb_mbed *state = (kb_mbed *)ctx;

    /* На неблокирующем сокете close_notify может не уйти сразу — не ждём. */
    mbedtls_ssl_close_notify(&state->ssl);
    mbedtls_ssl_free(&state->ssl);
    mbedtls_ssl_config_free(&state->conf);
    if (state->have_ca) {
        mbedtls_x509_crt_free(&state->ca);
        state->have_ca = 0;
    }
    mbedtls_ctr_drbg_free(&state->drbg);
    mbedtls_entropy_free(&state->entropy);
}

static const char *mbed_detail(void *ctx)
{
    kb_mbed *state = (kb_mbed *)ctx;
    return state->detail[0] != '\0' ? state->detail : "ok";
}

static const kb_tls_vtable mbed_vtable = {
    mbed_handshake, mbed_read, mbed_write, mbed_close, mbed_detail
};

kb_status kb_tls_mbedtls_init(kb_tls *tls, void *state_memory,
                              size_t state_size, intptr_t fd,
                              const char *server_name, const char *ca_pem,
                              size_t ca_pem_len, int insecure)
{
    kb_mbed *state = (kb_mbed *)state_memory;
    static const char *seed = "kolibri";
    int code;

    if (tls == NULL || state_memory == NULL || state_size < sizeof(kb_mbed) ||
        server_name == NULL || server_name[0] == '\0') {
        return KB_ERR_INVALID;
    }
    if (!insecure && (ca_pem == NULL || ca_pem_len == 0)) {
        /* Без корней проверять нечем, а молча доверять всему нельзя. */
        return KB_ERR_INVALID;
    }

    memset(state, 0, sizeof *state);
    mbedtls_ssl_init(&state->ssl);
    mbedtls_ssl_config_init(&state->conf);
    mbedtls_x509_crt_init(&state->ca);
    mbedtls_entropy_init(&state->entropy);
    mbedtls_ctr_drbg_init(&state->drbg);
    state->fd = fd;

    code = mbedtls_ctr_drbg_seed(&state->drbg, mbedtls_entropy_func,
                                 &state->entropy, (const unsigned char *)seed,
                                 strlen(seed));
    if (code != 0) {
        remember(state, "нет источника случайности", code);
        goto fail;
    }

    code = mbedtls_ssl_config_defaults(&state->conf, MBEDTLS_SSL_IS_CLIENT,
                                       MBEDTLS_SSL_TRANSPORT_STREAM,
                                       MBEDTLS_SSL_PRESET_DEFAULT);
    if (code != 0) {
        remember(state, "конфигурация TLS", code);
        goto fail;
    }

    if (insecure) {
        mbedtls_ssl_conf_authmode(&state->conf, MBEDTLS_SSL_VERIFY_NONE);
    } else {
        /*
         * Пропускаем всё до первого «-----BEGIN». Файлы корней часто начинаются
         * со строк-комментариев (у корня Минцифры это именно так): OpenSSL их
         * терпит, а mbedTLS требует, чтобы данные начинались с маркера, и иначе
         * отвечает невнятным «ASN1 tag was of an unexpected value».
         *
         * Длина обязана включать завершающий ноль — так требует mbedTLS.
         */
        {
            const char *begin = strstr(ca_pem, "-----BEGIN");
            size_t offset = begin != NULL ? (size_t)(begin - ca_pem) : 0;
            size_t left;

            if (offset >= ca_pem_len) {
                remember(state, "в файле корней нет ни одного сертификата", 0);
                goto fail;
            }
            /* Длину считаем от места разреза, вместе с завершающим нулём:
               вычитание смещения из полной длины его бы съело. */
            left = strlen(ca_pem + offset) + 1;

            code = mbedtls_x509_crt_parse(
                &state->ca, (const unsigned char *)ca_pem + offset, left);
        }
        if (code != 0) {
            remember(state, "не разобрались доверенные корни", code);
            goto fail;
        }
        state->have_ca = 1;
        mbedtls_ssl_conf_ca_chain(&state->conf, &state->ca, NULL);
        mbedtls_ssl_conf_authmode(&state->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    }

    mbedtls_ssl_conf_rng(&state->conf, mbedtls_ctr_drbg_random, &state->drbg);

    code = mbedtls_ssl_setup(&state->ssl, &state->conf);
    if (code != 0) {
        remember(state, "SSL setup", code);
        goto fail;
    }
    /* Одновременно SNI и имя для проверки сертификата. */
    code = mbedtls_ssl_set_hostname(&state->ssl, server_name);
    if (code != 0) {
        remember(state, "имя сервера", code);
        goto fail;
    }
    mbedtls_ssl_set_bio(&state->ssl, state, bio_send, bio_recv, NULL);

    tls->vt = &mbed_vtable;
    tls->ctx = state;
    return KB_OK;

fail:
    mbedtls_ssl_free(&state->ssl);
    mbedtls_ssl_config_free(&state->conf);
    mbedtls_x509_crt_free(&state->ca);
    mbedtls_ctr_drbg_free(&state->drbg);
    mbedtls_entropy_free(&state->entropy);
    return KB_ERR_STATE;
}

const char *kb_tls_mbedtls_version(void)
{
    return MBEDTLS_VERSION_STRING;
}

#else /* без KB_WITH_MBEDTLS */

size_t kb_tls_mbedtls_state_size(void)
{
    return 0;
}

kb_status kb_tls_mbedtls_init(kb_tls *tls, void *state, size_t state_size,
                              intptr_t fd, const char *server_name,
                              const char *ca_pem, size_t ca_pem_len,
                              int insecure)
{
    (void)tls; (void)state; (void)state_size; (void)fd;
    (void)server_name; (void)ca_pem; (void)ca_pem_len; (void)insecure;
    return KB_ERR_UNSUPPORTED;
}

const char *kb_tls_mbedtls_version(void)
{
    return NULL;
}

#endif /* KB_WITH_MBEDTLS */
