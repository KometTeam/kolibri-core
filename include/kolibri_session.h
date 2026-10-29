/*
 * Машина сессии: handshake, мультиплекс запрос/ответ по seq, keepalive,
 * таймауты, расписание переподключения.
 *
 * Sans-IO целиком. Сессия не открывает сокетов, не спит и не смотрит на часы —
 * всё это делает хост:
 *
 *     kb_session_on_connected(&s, now);          // TCP и TLS готовы
 *     for (;;) {
 *         kb_session_tick(&s, now);              // время идёт
 *
 *         while ((out = kb_session_pull_out(&s, &n)) != NULL) {
 *             send(fd, out, n);
 *             kb_session_consume_out(&s, n);
 *         }
 *
 *         n = recv(fd, buf, sizeof buf);
 *         if (n <= 0) { kb_session_on_disconnected(&s, now); ... }
 *         kb_session_feed(&s, buf, n);
 *
 *         while (kb_session_poll_event(&s, &event) == KB_OK &&
 *                event.type != KB_EVENT_NONE) {
 *             ...
 *         }
 *
 *         wait_until(kb_session_next_deadline(&s));
 *     }
 *
 * Время — миллисекунды от произвольной точки отсчёта; важна только разность,
 * так что монотонные часы подходят и предпочтительны.
 *
 * Память: ни байта не выделяется. Все буферы даёт вызывающая сторона и владеет
 * ими сама; строки в конфигурации тоже принадлежат ему и должны пережить
 * сессию — handshake собирается не в момент настройки, а при каждом
 * подключении.
 */

#ifndef KOLIBRI_SESSION_H
#define KOLIBRI_SESSION_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Опкоды, которые машина сессии использует сама. */
#define KB_OPCODE_PING 1
#define KB_OPCODE_SESSION_INIT 6

typedef enum {
    KB_STATE_DISCONNECTED = 0,
    /* Хост подключается; сессия ждёт kb_session_on_connected. */
    KB_STATE_CONNECTING,
    /* Транспорт есть, handshake отправлен, ответа ещё нет. */
    KB_STATE_CONNECTED,
    /* Handshake принят: можно слать запросы. */
    KB_STATE_ONLINE
} kb_session_state;

/* ─── Настройка ────────────────────────────────────────────────────────── */

/* Поля `userAgent` в payload sessionInit. Все строки — владения вызывающего. */
typedef struct {
    const char *device_type;
    const char *app_version;
    const char *os_version;
    const char *timezone;
    const char *screen;
    const char *push_device_type;
    const char *arch;
    const char *locale;
    int64_t build_number;
    const char *device_name;
    const char *device_locale;
    /* Опциональны: шлются в userAgent только если заданы. Их шлёт веб-клиент
       MAX (с авг 2026); сервер по isPwa решает про web-push. Мобильный клиент
       их не отправляет — оставь header_user_agent = NULL и has_is_pwa = 0. */
    const char *header_user_agent;
    int has_is_pwa;
    int is_pwa;
} kb_user_agent;

typedef struct {
    const char *instance_id;
    const char *device_id;
    int64_t client_session_id;
    kb_user_agent user_agent;
} kb_handshake_config;

typedef struct {
    kb_handshake_config handshake;
    /* Интервал keepalive в мс; 0 — не пинговать. Первый пинг уходит через
       интервал после подключения, а не сразу — как в эталоне. */
    uint32_t ping_interval_ms;
    /* Флаг `interactive` в пинге. 0 — ghost-режим. */
    int ping_interactive;
    /* Сколько ждать ответа на запрос, мс; 0 — не ограничивать. */
    uint32_t request_timeout_ms;
    /* Планировать ли переподключение после обрыва. */
    int auto_reconnect;
} kb_session_config;

/* Запрос, ожидающий ответа. Таблицу заводит вызывающая сторона. */
typedef struct {
    uint16_t seq;
    uint16_t opcode;
    /* 0 — без таймаута. */
    uint64_t deadline_ms;
    int in_use;
    /* KB_OK, пока ждём; иначе причина, которую отдадим событием. */
    kb_status failed;
} kb_pending;

/*
 * Буферы. Требования к размерам:
 *   rx      — не меньше самого крупного ожидаемого пакета целиком;
 *   tx      — очередь исходящих; хватит нескольких десятков килобайт, если
 *             хост исправно вычитывает kb_session_pull_out;
 *   scratch — сборка payload и распаковка входящих тел; под распакованное
 *             тело нужен запас, иначе придёт KB_ERR_TOO_LARGE;
 *   pending — сколько запросов может быть в полёте одновременно.
 */
typedef struct {
    uint8_t *rx;
    size_t rx_capacity;
    uint8_t *tx;
    size_t tx_capacity;
    uint8_t *scratch;
    size_t scratch_capacity;
    kb_pending *pending;
    size_t pending_capacity;
} kb_session_buffers;

/* ─── События ──────────────────────────────────────────────────────────── */

typedef enum {
    KB_EVENT_NONE = 0,
    /* Состояние сессии изменилось; новое лежит в event.state. */
    KB_EVENT_STATE,
    /* Пришёл разобранный ответ на sessionInit. */
    KB_EVENT_HANDSHAKE,
    /* Ответ на наш запрос: seq, opcode, cmd, payload. */
    KB_EVENT_RESPONSE,
    /* Серверный пуш (cmd == 0). */
    KB_EVENT_PUSH,
    /* Запрос остался без ответа: status — KB_ERR_TIMEOUT или KB_ERR_CLOSED. */
    KB_EVENT_REQUEST_FAILED,
    /* Поток испорчен: соединение надо закрыть. Причина в status. */
    KB_EVENT_ERROR
} kb_event_type;

typedef struct {
    kb_event_type type;
    /* KB_EVENT_STATE */
    kb_session_state state;
    /* RESPONSE, PUSH, HANDSHAKE, REQUEST_FAILED */
    uint16_t seq;
    uint16_t opcode;
    uint8_t cmd;
    /*
     * Тело, уже распакованное. Указывает внутрь scratch и живёт до следующего
     * вызова kb_session_poll_event или kb_session_feed — копируй, если нужно
     * дольше.
     */
    const uint8_t *payload;
    size_t payload_len;
    /* KB_EVENT_HANDSHAKE: `callsSeed` из ответа, если сервер его прислал. */
    int has_calls_seed;
    int64_t calls_seed;
    /* KB_EVENT_ERROR, KB_EVENT_REQUEST_FAILED */
    kb_status status;
} kb_event;

/* ─── Сессия ───────────────────────────────────────────────────────────── */

#define KB_STATE_QUEUE 8

/*
 * Состояние целиком на виду, потому что выделять его негде. Поля читать можно,
 * менять — только через функции ниже.
 */
/*
 * Тап на трафик: срабатывает по разу на пакет в каждую сторону, ДО диспатча, с
 * уже распакованным msgpack. Держи колбэк дешёвым. Портирован с
 * transport/wiretap.rs.
 *
 *   dir     — сторона: OUT (клиент→сервер: запрос/ping) или IN (сервер→клиент);
 *   cmd     — командный байт (OUT всегда REQUEST; IN — OK/NOT_FOUND/ERROR либо
 *             PUSH == 0);
 *   opcode  — опкод пакета;
 *   seq     — номер пакета;
 *   payload — распакованный msgpack (живёт только на время вызова).
 */
typedef enum {
    KB_TAP_OUT = 0,   /* клиент → сервер */
    KB_TAP_IN = 1     /* сервер → клиент */
} kb_tap_direction;

typedef void (*kb_tap_fn)(void *user, kb_tap_direction dir, uint8_t cmd,
                          uint16_t opcode, uint16_t seq, const uint8_t *payload,
                          size_t payload_len);

/* "out" или "in". */
const char *kb_tap_direction_str(kb_tap_direction dir);

typedef struct {
    kb_session_config config;
    kb_session_buffers bufs;
    kb_rx rx;

    kb_session_state state;
    uint64_t now_ms;

    /* Предынкремент с переносом: первый запрос получает seq 1, как в эталоне. */
    uint16_t seq;
    uint16_t handshake_seq;

    uint64_t next_ping_ms;
    uint64_t reconnect_at_ms;
    uint32_t attempt;

    /* Очередь исходящих байт: линейный буфер с уплотнением. */
    size_t tx_start;
    size_t tx_end;

    /* Кольцо смен состояния, чтобы промежуточные не терялись. */
    kb_session_state state_queue[KB_STATE_QUEUE];
    size_t state_head;
    size_t state_count;

    /* Тап на трафик (может быть NULL). */
    kb_tap_fn tap;
    void *tap_user;

    /* Ошибка потока, которую ещё не отдали событием. */
    kb_status fault;
} kb_session;

/*
 * Размеры публичных структур в рантайме.
 *
 * Нужны обёрткам на других языках: под kb_session можно выделить непрозрачный
 * блок нужного размера и вообще не повторять его разметку, а разметку мелких
 * структур — сверить с этим числом и упасть сразу, а не портить память молча.
 */
#define KB_SIZEOF_SESSION 0
#define KB_SIZEOF_CONFIG 1
#define KB_SIZEOF_BUFFERS 2
#define KB_SIZEOF_EVENT 3
#define KB_SIZEOF_PENDING 4
#define KB_SIZEOF_USER_AGENT 5
#define KB_SIZEOF_HANDSHAKE 6

size_t kb_sizeof(int what);

kb_status kb_session_init(kb_session *session, const kb_session_config *config,
                          const kb_session_buffers *buffers);

/* Хост начал подключаться. Состояние -> CONNECTING. */
kb_status kb_session_on_connecting(kb_session *session, uint64_t now_ms);

/*
 * Транспорт готов: сессия ставит в очередь payload sessionInit и переходит в
 * CONNECTED. Ответ придёт событием KB_EVENT_HANDSHAKE, после него — ONLINE.
 */
kb_status kb_session_on_connected(kb_session *session, uint64_t now_ms);

/*
 * Соединение оборвалось. Каждый запрос в полёте даёт KB_EVENT_REQUEST_FAILED
 * со статусом KB_ERR_CLOSED, очередь исходящих очищается, и, если включено
 * auto_reconnect, назначается время следующей попытки.
 */
kb_status kb_session_on_disconnected(kb_session *session, uint64_t now_ms);

/* Байты из сокета. */
kb_status kb_session_feed(kb_session *session, const uint8_t *data, size_t len);

/* Единственный источник времени: пинги, таймауты, момент переподключения. */
kb_status kb_session_tick(kb_session *session, uint64_t now_ms);

/* Что отправить в сокет; NULL, если нечего. */
const uint8_t *kb_session_pull_out(kb_session *session, size_t *len);

/* Подтвердить, сколько байт из pull_out ушло в сокет. */
kb_status kb_session_consume_out(kb_session *session, size_t len);

/* Следующее событие. KB_EVENT_NONE означает, что пока всё разобрано. */
kb_status kb_session_poll_event(kb_session *session, kb_event *out);

/* Запрос с ожиданием ответа. Возвращённый seq придёт в KB_EVENT_RESPONSE. */
kb_status kb_session_request(kb_session *session, uint16_t opcode,
                             const uint8_t *payload, size_t len,
                             uint16_t *out_seq);

/* Отправка без ожидания ответа: seq выделяется, но в таблицу не заносится. */
kb_status kb_session_send(kb_session *session, uint16_t opcode,
                          const uint8_t *payload, size_t len,
                          uint16_t *out_seq);

/*
 * Когда сессию нужно разбудить: ближайший пинг, таймаут или переподключение.
 * 0 — ждать нечего, можно блокироваться на сокете.
 */
uint64_t kb_session_next_deadline(const kb_session *session);

kb_session_state kb_session_get_state(const kb_session *session);

/* Сколько запросов ждёт ответа. */
size_t kb_session_pending_count(const kb_session *session);

/* Переключить флаг `interactive`; следующий пинг уедет с новым значением. */
void kb_session_set_ping_interactive(kb_session *session, int interactive);

/*
 * Задержка перед попыткой номер `attempt`, мс: 2, 4, 8, 15, 15, ... секунд.
 * Ровно та же лестница, что у эталона.
 */
uint32_t kb_session_backoff_ms(uint32_t attempt);

/* Собрать payload sessionInit отдельно — удобно для тестов и отладки. */
kb_status kb_build_handshake_payload(const kb_handshake_config *config,
                                     uint8_t *out, size_t out_cap,
                                     size_t *out_len);

kb_status kb_build_ping_payload(int interactive, uint8_t *out, size_t out_cap,
                                size_t *out_len);

/* Поставить тап (или снять, передав NULL). user отдаётся в колбэк как есть. */
/*
 * Собрать HTTP User-Agent: "OKMessages/<app_version> (<os_version>; <device_name>;
 * <screen>)" — та же строка, что шлёт официальный клиент в заголовке загрузки
 * медиа (её и передавай в kb_media_*_headers). NULL-поля берутся как пустые.
 * KB_ERR_TOO_LARGE, если не влезло; out всегда завершается '\0' при cap>0.
 * out_len (без '\0') можно NULL.
 */
kb_status kb_user_agent_http(const kb_user_agent *ua, char *out, size_t cap,
                             size_t *out_len);

void kb_session_set_tap(kb_session *session, kb_tap_fn tap, void *user);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_SESSION_H */
