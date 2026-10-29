/*
 * Транспорт: TCP, прокси и TLS.
 *
 * Это единственная часть kolibri, которая знает про операционную систему.
 * Она НЕ обязательна: хост со своим сокетом (Python, Go, Flutter) просто
 * кормит машину сессии байтами и этот заголовок не подключает. Модуль нужен
 * там, где клиент целиком на C — на старых iOS и Windows Phone.
 *
 * Сборка: `make` кладёт его в библиотеку на POSIX и Windows. TLS подключается
 * бэкендом; без него получается обычный TCP.
 */

#ifndef KOLIBRI_TRANSPORT_H
#define KOLIBRI_TRANSPORT_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Возвраты чтения и записи: неотрицательное — число байт. */
#define KB_IO_WANT_READ (-1)
#define KB_IO_WANT_WRITE (-2)
#define KB_IO_CLOSED (-3)
#define KB_IO_FAILED (-4)

/*
 * TLS подключается таблицей функций. Ядро не знает ни про один конкретный
 * стек: бэкенд сам держит сокет и сам решает, как с ним работать.
 *
 * Готовый бэкенд на OpenSSL лежит в src/kolibri_tls_openssl.c и собирается
 * при KB_WITH_OPENSSL. Свой пишется примерно на сотню строк — для mbedTLS,
 * wolfSSL, Secure Transport или SChannel одинаково.
 */
typedef struct {
    /* 0 — рукопожатие закончено; KB_IO_WANT_* — нужно ещё I/O; иначе отказ. */
    int (*handshake)(void *ctx);
    long (*read)(void *ctx, uint8_t *buf, size_t len);
    long (*write)(void *ctx, const uint8_t *buf, size_t len);
    void (*close)(void *ctx);
    /* Последняя ошибка человеческим языком; может быть NULL. */
    const char *(*detail)(void *ctx);
} kb_tls_vtable;

typedef struct {
    const kb_tls_vtable *vt;
    void *ctx;
} kb_tls;

typedef struct {
    const char *host;
    uint16_t port;
    uint32_t connect_timeout_ms;
    /* NULL — подключаться напрямую. */
    const kb_proxy_config *proxy;
    /* NULL — без шифрования; иначе бэкенд, уже настроенный на этот сокет. */
    const kb_tls *tls;
} kb_transport_config;

typedef struct {
    /* На Windows это SOCKET, приведённый к intptr_t. */
    intptr_t fd;
    kb_tls tls;
    int has_tls;
    int open;
    /*
     * Байты, пришедшие вместе с ответом прокси и относящиеся уже к туннелю.
     * kb_transport_read отдаёт их первыми.
     */
    uint8_t carry[KB_PROXY_IN_MAX];
    size_t carry_len;
    size_t carry_read;
    char detail[128];
} kb_transport;

/*
 * Монотонные миллисекунды — то, что ждёт машина сессии.
 *
 * Стенные часы не годятся принципиально: шаг NTP назад останавливает пинги и
 * таймауты до конца скачка. Поэтому на Apple используется mach_absolute_time
 * (clock_gettime появился только в macOS 10.12 / iOS 10), на Windows —
 * накопитель поверх GetTickCount, на прочих POSIX — CLOCK_MONOTONIC.
 *
 * Не потокобезопасна на Windows: там переполнение 32-битного счётчика
 * отслеживается статикой. Библиотека и так рассчитана на одну сессию в одном
 * потоке, но если зовёшь kb_now_ms из нескольких — заведи свой источник
 * времени и передавай его в kb_session_tick сам.
 */
uint64_t kb_now_ms(void);

/*
 * Разрешить имя, подключиться, при необходимости пройти прокси.
 * Сокет остаётся неблокирующим: читать и писать надо через функции ниже.
 *
 * TLS-рукопожатие тут не делается — вызови kb_transport_handshake, когда
 * сокет готов.
 */
kb_status kb_transport_connect(kb_transport *transport,
                               const kb_transport_config *config);

/*
 * Продвинуть TLS-рукопожатие.
 *
 * KB_OK — готово. KB_ERR_TRUNCATED — нужно ещё I/O: подожди сокета и позови
 * снова. `want_write` при этом получает 1, если ждать надо готовности к
 * ЗАПИСИ, и 0 — если к чтению; ждать не в ту сторону значит подвиснуть до
 * таймаута. Можно передать NULL, если направление неважно.
 *
 * Без TLS сразу KB_OK.
 */
kb_status kb_transport_handshake(kb_transport *transport, int *want_write);

/*
 * Подключить TLS к уже открытому сокету — то же, что передать его в
 * kb_transport_connect, но когда бэкенд настраивается после подключения
 * (ему нужен дескриптор).
 *
 * Возвращает KB_ERR_STATE, если прокси успел дописать байты за своим ответом:
 * под TLS их там быть не может, значит их сочинил прокси, и такое соединение
 * поднимать нельзя.
 */
kb_status kb_transport_set_tls(kb_transport *transport, const kb_tls *tls);

long kb_transport_read(kb_transport *transport, uint8_t *buf, size_t len);
long kb_transport_write(kb_transport *transport, const uint8_t *buf, size_t len);

/*
 * Подождать готовности сокета. timeout_ms == 0 — не ждать вовсе,
 * KB_WAIT_FOREVER — ждать сколько угодно.
 * Возвращает 1, если сокет готов, 0 по таймауту, отрицательное при ошибке.
 */
#define KB_WAIT_FOREVER 0xFFFFFFFFu
int kb_transport_wait(kb_transport *transport, int for_write,
                      uint32_t timeout_ms);

void kb_transport_close(kb_transport *transport);

/* Пояснение к последней неудаче; всегда непустая строка. */
const char *kb_transport_detail(const kb_transport *transport);

/* Инициализация сетевой подсистемы. На POSIX ничего не делает. */
kb_status kb_net_startup(void);
void kb_net_cleanup(void);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_TRANSPORT_H */
