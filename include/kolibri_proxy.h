/*
 * Рукопожатие с прокси: HTTP CONNECT и SOCKS5.
 *
 * Как и всё остальное — sans-IO: машина говорит, что отправить и сколько
 * прочитать, а сокетом занимается хост. Побочная выгода в том, что оба
 * протокола проверяются тестами целиком, без единого сетевого вызова.
 *
 * Оба варианта отдают в конце обычный TCP-туннель до цели, поэтому TLS и всё
 * выше него не замечают разницы.
 *
 * Буферы рукопожатия лежат внутри структуры: они маленькие и фиксированные, а
 * лишний параметр в API дороже пары килобайт. Саму kb_proxy размещай где
 * удобно — она нигде не выделяется динамически.
 */

#ifndef KOLIBRI_PROXY_H
#define KOLIBRI_PROXY_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Запроса хватает с запасом: CONNECT с Basic-авторизацией укладывается. */
#define KB_PROXY_OUT_MAX 1024
/* Столько же, сколько допускает эталон для заголовков ответа CONNECT. */
#define KB_PROXY_IN_MAX 8192
/* Строки конфигурации при разборе URL складываются сюда. */
#define KB_PROXY_URL_MAX 512

typedef enum {
    KB_PROXY_NONE = 0,
    KB_PROXY_HTTP,
    KB_PROXY_SOCKS5
} kb_proxy_kind;

/*
 * Строки принадлежат вызывающему и должны пережить рукопожатие.
 * username == NULL означает «без авторизации».
 */
typedef struct {
    kb_proxy_kind kind;
    const char *host;
    uint16_t port;
    const char *username;
    const char *password;
} kb_proxy_config;

/*
 * Разобрать `scheme://[user:pass@]host:port`.
 *
 * Схемы: http, socks5, socks5h — обе socks-разновидности передают цель именем,
 * резолвит её прокси.
 *
 * Строки копируются в `storage` (нужно KB_PROXY_URL_MAX байт), и поля конфига
 * указывают внутрь него: буфер должен пережить конфиг.
 */
kb_status kb_proxy_parse(const char *url, kb_proxy_config *out, char *storage,
                         size_t storage_len);

typedef enum {
    KB_PROXY_STEP_WRITE = 0, /* есть что отправить */
    KB_PROXY_STEP_READ,      /* ждём ответа прокси */
    KB_PROXY_STEP_DONE,      /* туннель открыт */
    KB_PROXY_STEP_FAILED     /* прокси отказал; причина в kb_proxy_error */
} kb_proxy_step;

typedef struct {
    /* Копия конфигурации: сами строки по-прежнему принадлежат вызывающему. */
    kb_proxy_config config;
    char target_host[256];
    uint16_t target_port;

    kb_proxy_kind kind;
    int state;
    int has_auth;
    kb_proxy_step step;
    kb_status error;
    /* Короткое пояснение к отказу: код ответа прокси и тому подобное. */
    char detail[64];

    uint8_t out[KB_PROXY_OUT_MAX];
    size_t out_len;
    size_t out_sent;

    uint8_t in[KB_PROXY_IN_MAX];
    size_t in_len;
    /* Сколько байт ждём на текущем шаге; 0 — до разделителя заголовков. */
    size_t need;
} kb_proxy;

/*
 * Подготовить рукопожатие до (target_host, target_port).
 * Сразу после init машина находится в состоянии WRITE.
 */
kb_status kb_proxy_init(kb_proxy *proxy, const kb_proxy_config *config,
                        const char *target_host, uint16_t target_port);

/* Что нужно отправить прокси; NULL — сейчас писать нечего. */
const uint8_t *kb_proxy_pull_out(kb_proxy *proxy, size_t *len);

/* Подтвердить, сколько байт ушло в сокет. */
kb_status kb_proxy_consume_out(kb_proxy *proxy, size_t len);

/*
 * Скормить байты, пришедшие от прокси.
 *
 * `consumed` получает, сколько из них съело рукопожатие. Остаток — уже данные
 * туннеля: после KB_PROXY_STEP_DONE его надо передать дальше, в сессию.
 */
kb_status kb_proxy_feed(kb_proxy *proxy, const uint8_t *data, size_t len,
                        size_t *consumed);

kb_proxy_step kb_proxy_get_step(const kb_proxy *proxy);

/* KB_OK, пока всё идёт хорошо. */
kb_status kb_proxy_error(const kb_proxy *proxy);

/* Пояснение к отказу; всегда непустая строка. */
const char *kb_proxy_detail(const kb_proxy *proxy);

/* Base64 без переносов строк. Возвращает длину без нулевого байта. */
size_t kb_base64_encode(const uint8_t *data, size_t len, char *out,
                        size_t out_cap);

/* Base64-декод, пробелы и '=' терпит. См. .c про коды ошибок. */
kb_status kb_base64_decode(const char *text, size_t len, uint8_t *out,
                           size_t out_cap, size_t *out_len);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_PROXY_H */
