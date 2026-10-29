/*
 * Машина сессии: последовательность handshake, мультиплекс, пинги, таймауты,
 * расписание переподключения.
 *
 * Сокета тут нет: «сервер» — это функция, которая собирает ответный пакет, а
 * «сеть» — вызов kb_session_feed. Ровно так же будет выглядеть настоящий цикл,
 * только байты придут из recv().
 */

#include <string.h>

#include "kb_test.h"
#include "kolibri.h"

#define BUF 65536u

static unsigned char rx_buf[BUF];
static unsigned char tx_buf[BUF];
static unsigned char scratch[BUF];
static kb_pending pending_slots[16];
static unsigned char wire[BUF];
static unsigned char body[BUF];
static unsigned char tmp[BUF];

static kb_session_config make_config(void)
{
    kb_session_config config;

    memset(&config, 0, sizeof config);
    config.handshake.instance_id = "kolibri-core-0001";
    config.handshake.device_id = "kolibri-core-0001";
    config.handshake.client_session_id = 1755730000000LL;
    config.handshake.user_agent.device_type = "ANDROID";
    config.handshake.user_agent.app_version = "26.25.0";
    config.handshake.user_agent.os_version = "Android 14";
    config.handshake.user_agent.timezone = "Europe/Moscow";
    config.handshake.user_agent.screen = "480dpi 480dpi 1344x2992";
    config.handshake.user_agent.push_device_type = "GCM";
    config.handshake.user_agent.arch = "arm64-v8a";
    config.handshake.user_agent.locale = "ru";
    config.handshake.user_agent.build_number = 6790;
    config.handshake.user_agent.device_name = "Google Pixel 8 Pro";
    config.handshake.user_agent.device_locale = "ru";
    config.ping_interval_ms = 30000;
    config.ping_interactive = 1;
    config.request_timeout_ms = 5000;
    config.auto_reconnect = 1;
    return config;
}

static kb_session_buffers make_buffers(void)
{
    kb_session_buffers bufs;

    bufs.rx = rx_buf;
    bufs.rx_capacity = sizeof rx_buf;
    bufs.tx = tx_buf;
    bufs.tx_capacity = sizeof tx_buf;
    bufs.scratch = scratch;
    bufs.scratch_capacity = sizeof scratch;
    bufs.pending = pending_slots;
    bufs.pending_capacity = sizeof pending_slots / sizeof pending_slots[0];
    return bufs;
}

/* Собрать ответ «сервера». */
static size_t server_packet(unsigned char *out, size_t cap, unsigned char cmd,
                            unsigned short opcode, unsigned short seq,
                            const unsigned char *payload, size_t payload_len)
{
    size_t written = 0;
    if (kb_encode(cmd, opcode, seq, payload, payload_len, out, cap, &written) !=
        KB_OK) {
        return 0;
    }
    return written;
}

/* Забрать всё, что сессия хочет отправить, в буфер. */
static size_t drain(kb_session *s, unsigned char *out, size_t cap)
{
    size_t total = 0;
    for (;;) {
        size_t len = 0;
        const unsigned char *chunk = kb_session_pull_out(s, &len);
        if (chunk == NULL) {
            return total;
        }
        if (len > cap - total) {
            return total;
        }
        memcpy(out + total, chunk, len);
        total += len;
        kb_session_consume_out(s, len);
    }
}

/* Пропустить события смены состояния, вернуть первое содержательное. */
static kb_event_type next_meaningful(kb_session *s, kb_event *event)
{
    for (;;) {
        if (kb_session_poll_event(s, event) != KB_OK) {
            return KB_EVENT_NONE;
        }
        if (event->type != KB_EVENT_STATE) {
            return event->type;
        }
    }
}

/* Довести сессию до ONLINE, вернуть seq handshake. */
static unsigned short bring_online(kb_session *s, long calls_seed)
{
    kb_header header;
    kb_mp_writer w;
    kb_event event;
    size_t queued;
    size_t payload_len = 0;
    size_t reply_len;

    kb_session_on_connecting(s, 1000);
    kb_session_on_connected(s, 1000);

    queued = drain(s, wire, sizeof wire);
    if (queued == 0) {
        return 0;
    }
    if (kb_decode(wire, queued, &header, body, sizeof body, &payload_len) != KB_OK) {
        return 0;
    }

    kb_mp_writer_init(&w, tmp, sizeof tmp);
    kb_mp_write_map(&w, 2);
    kb_mp_write_cstr(&w, "callsSeed");
    kb_mp_write_i64(&w, calls_seed);
    kb_mp_write_cstr(&w, "device_name");
    kb_mp_write_cstr(&w, "Google Pixel 8 Pro");
    if (kb_mp_writer_status(&w) != KB_OK) {
        return 0;
    }

    reply_len = server_packet(wire, sizeof wire, KB_CMD_OK,
                              KB_OPCODE_SESSION_INIT, header.seq, tmp,
                              kb_mp_writer_len(&w));
    kb_session_feed(s, wire, reply_len);

    if (next_meaningful(s, &event) != KB_EVENT_HANDSHAKE) {
        return 0;
    }
    return header.seq;
}

void kb_tests_session(void)
{
    kb_session s;
    kb_session_config config = make_config();
    kb_session_buffers bufs = make_buffers();
    kb_event event;

    KB_TEST("лестница backoff совпадает с эталоном");
    {
        CHECK_EQ(kb_session_backoff_ms(0), 2000);
        CHECK_EQ(kb_session_backoff_ms(1), 4000);
        CHECK_EQ(kb_session_backoff_ms(2), 8000);
        CHECK_EQ(kb_session_backoff_ms(3), 15000);
        CHECK_EQ(kb_session_backoff_ms(4), 15000);
        CHECK_EQ(kb_session_backoff_ms(99), 15000);
    }

    KB_TEST("handshake уходит сразу после подключения");
    {
        kb_header header;
        size_t queued;
        size_t payload_len = 0;
        size_t expected_len = 0;

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK_EQ(kb_session_get_state(&s), KB_STATE_DISCONNECTED);
        CHECK(kb_session_pull_out(&s, &queued) == NULL);

        CHECK_EQ(kb_session_on_connecting(&s, 1000), KB_OK);
        CHECK_EQ(kb_session_get_state(&s), KB_STATE_CONNECTING);
        CHECK(kb_session_pull_out(&s, &queued) == NULL);

        CHECK_EQ(kb_session_on_connected(&s, 1000), KB_OK);
        CHECK_EQ(kb_session_get_state(&s), KB_STATE_CONNECTED);

        queued = drain(&s, wire, sizeof wire);
        CHECK(queued > KB_HEADER_SIZE);
        CHECK_EQ(kb_decode(wire, queued, &header, body, sizeof body,
                           &payload_len),
                 KB_OK);
        CHECK_EQ(header.opcode, KB_OPCODE_SESSION_INIT);
        CHECK_EQ(header.cmd, KB_CMD_REQUEST);
        /* Первый запрос получает seq 1 — как в эталоне. */
        CHECK_EQ(header.seq, 1);

        /* Тело — ровно то, что отдаёт публичный сборщик. */
        CHECK_EQ(kb_build_handshake_payload(&config.handshake, tmp, sizeof tmp,
                                            &expected_len),
                 KB_OK);
        CHECK_MEM(body, payload_len, tmp, expected_len);

        CHECK_EQ(kb_session_pending_count(&s), 1);
    }

    KB_TEST("HTTP User-Agent собирается как в эталоне");
    {
        kb_user_agent ua;
        char buf[128];
        size_t len = 0;
        memset(&ua, 0, sizeof ua);
        ua.app_version = "26.20.2";
        ua.os_version = "Android 14";
        ua.device_name = "Google Pixel 7";
        ua.screen = "420dpi 420dpi 1080x2340";
        CHECK_EQ(kb_user_agent_http(&ua, buf, sizeof buf, &len), KB_OK);
        CHECK_EQ(strcmp(buf, "OKMessages/26.20.2 (Android 14; "
                             "Google Pixel 7; 420dpi 420dpi 1080x2340)"), 0);
        CHECK_EQ(len, strlen(buf));
        /* тесный буфер — отказ, но строка завершена. */
        CHECK_EQ(kb_user_agent_http(&ua, buf, 10, NULL), KB_ERR_TOO_LARGE);
        CHECK_EQ(buf[9], '\0');
        CHECK_EQ(kb_user_agent_http(NULL, buf, sizeof buf, NULL), KB_ERR_INVALID);
    }

    KB_TEST("опциональные web-поля userAgent: isPwa/headerUserAgent");
    {
        kb_session_config cfg = make_config();
        unsigned char doc[BUF];
        size_t len = 0;
        const uint8_t *ua = NULL;
        size_t ua_len = 0;

        /* По умолчанию (мобильный) — полей нет. */
        CHECK_EQ(kb_build_handshake_payload(&cfg.handshake, doc, sizeof doc, &len),
                 KB_OK);
        CHECK_EQ(kb_mp_map_get_raw(doc, len, "userAgent", &ua, &ua_len), KB_OK);
        CHECK_EQ(kb_mp_map_has(ua, ua_len, "headerUserAgent"), 0);
        CHECK_EQ(kb_mp_map_has(ua, ua_len, "isPwa"), 0);

        /* Заданы — оба присутствуют, headerUserAgent раньше isPwa. */
        cfg.handshake.user_agent.header_user_agent = "Mozilla/5.0";
        cfg.handshake.user_agent.has_is_pwa = 1;
        cfg.handshake.user_agent.is_pwa = 1;
        CHECK_EQ(kb_build_handshake_payload(&cfg.handshake, doc, sizeof doc, &len),
                 KB_OK);
        CHECK_EQ(kb_mp_map_get_raw(doc, len, "userAgent", &ua, &ua_len), KB_OK);
        {
            kb_mp_value v;
            CHECK_EQ(kb_mp_map_get(ua, ua_len, "headerUserAgent", &v), KB_OK);
            CHECK(kb_mp_str_eq(&v, "Mozilla/5.0"));
            CHECK_EQ(kb_mp_map_get(ua, ua_len, "isPwa", &v), KB_OK);
            CHECK_EQ(v.type, KB_MP_BOOL);
            CHECK_EQ(v.boolean, 1);
        }
        /* Порядок: headerUserAgent встречается раньше isPwa в байтах. */
        {
            const unsigned char *h = NULL;
            const unsigned char *pw = NULL;
            size_t i;
            for (i = 0; i < ua_len; i++) {
                if (i + 15 <= ua_len &&
                    memcmp(ua + i, "headerUserAgent", 15) == 0) h = ua + i;
                if (i + 5 <= ua_len && memcmp(ua + i, "isPwa", 5) == 0) pw = ua + i;
            }
            CHECK(h != NULL && pw != NULL && h < pw);
        }
    }

    KB_TEST("смены состояния приходят по порядку");
    {
        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        kb_session_on_connecting(&s, 1000);
        kb_session_on_connected(&s, 1000);

        CHECK_EQ(kb_session_poll_event(&s, &event), KB_OK);
        CHECK_EQ(event.type, KB_EVENT_STATE);
        CHECK_EQ(event.state, KB_STATE_CONNECTING);

        CHECK_EQ(kb_session_poll_event(&s, &event), KB_OK);
        CHECK_EQ(event.type, KB_EVENT_STATE);
        CHECK_EQ(event.state, KB_STATE_CONNECTED);

        CHECK_EQ(kb_session_poll_event(&s, &event), KB_OK);
        CHECK_EQ(event.type, KB_EVENT_NONE);
    }

    KB_TEST("ответ на handshake переводит в онлайн");
    {
        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK(bring_online(&s, 987654321) == 1);
        CHECK_EQ(kb_session_get_state(&s), KB_STATE_ONLINE);
        CHECK_EQ(kb_session_pending_count(&s), 0);

        /* Следом придёт смена состояния на ONLINE. */
        CHECK_EQ(kb_session_poll_event(&s, &event), KB_OK);
        CHECK_EQ(event.type, KB_EVENT_STATE);
        CHECK_EQ(event.state, KB_STATE_ONLINE);
    }

    KB_TEST("callsSeed достаётся из ответа");
    {
        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        kb_session_on_connecting(&s, 1000);
        kb_session_on_connected(&s, 1000);
        {
            kb_header header;
            kb_mp_writer w;
            size_t queued = drain(&s, wire, sizeof wire);
            size_t payload_len = 0;
            size_t reply_len;

            kb_decode(wire, queued, &header, body, sizeof body, &payload_len);
            kb_mp_writer_init(&w, tmp, sizeof tmp);
            kb_mp_write_map(&w, 1);
            kb_mp_write_cstr(&w, "callsSeed");
            kb_mp_write_i64(&w, -42);
            reply_len = server_packet(wire, sizeof wire, KB_CMD_OK,
                                      KB_OPCODE_SESSION_INIT, header.seq, tmp,
                                      kb_mp_writer_len(&w));
            kb_session_feed(&s, wire, reply_len);

            CHECK_EQ(next_meaningful(&s, &event), KB_EVENT_HANDSHAKE);
            CHECK_EQ(event.has_calls_seed, 1);
            CHECK_EQ(event.calls_seed, -42);
        }
    }

    KB_TEST("сервер отверг handshake");
    {
        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        kb_session_on_connecting(&s, 1000);
        kb_session_on_connected(&s, 1000);
        {
            kb_header header;
            size_t queued = drain(&s, wire, sizeof wire);
            size_t payload_len = 0;
            size_t reply_len;

            kb_decode(wire, queued, &header, body, sizeof body, &payload_len);
            reply_len = server_packet(wire, sizeof wire, KB_CMD_ERROR,
                                      KB_OPCODE_SESSION_INIT, header.seq, NULL, 0);
            kb_session_feed(&s, wire, reply_len);

            CHECK_EQ(next_meaningful(&s, &event), KB_EVENT_ERROR);
            CHECK(kb_session_get_state(&s) != KB_STATE_ONLINE);
        }
    }

    KB_TEST("запрос и ответ по seq");
    {
        unsigned short seq = 0;
        kb_header header;
        size_t queued;
        size_t payload_len = 0;
        size_t reply_len;
        static const unsigned char request_body[] = {0x81, 0xA6, 'c', 'h', 'a',
                                                     't', 'I', 'd', 0x2A};

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK(bring_online(&s, 1) == 1);

        CHECK_EQ(kb_session_request(&s, 64, request_body, sizeof request_body,
                                    &seq),
                 KB_OK);
        /* seq 1 ушёл на handshake, значит запросу достаётся 2. */
        CHECK_EQ(seq, 2);
        CHECK_EQ(kb_session_pending_count(&s), 1);

        queued = drain(&s, wire, sizeof wire);
        CHECK_EQ(kb_decode(wire, queued, &header, body, sizeof body,
                           &payload_len),
                 KB_OK);
        CHECK_EQ(header.opcode, 64);
        CHECK_EQ(header.seq, seq);
        CHECK_MEM(body, payload_len, request_body, sizeof request_body);

        reply_len = server_packet(wire, sizeof wire, KB_CMD_OK, 64, seq,
                                  request_body, sizeof request_body);
        kb_session_feed(&s, wire, reply_len);

        CHECK_EQ(next_meaningful(&s, &event), KB_EVENT_RESPONSE);
        CHECK_EQ(event.seq, seq);
        CHECK_EQ(event.opcode, 64);
        CHECK_EQ(event.cmd, KB_CMD_OK);
        CHECK_MEM(event.payload, event.payload_len, request_body,
                  sizeof request_body);
        CHECK_EQ(kb_session_pending_count(&s), 0);
    }

    KB_TEST("пуш отличается от ответа по команде");
    {
        size_t reply_len;
        static const unsigned char push_body[] = {0x81, 0xA4, 't', 'y', 'p',
                                                  'e', 0xA3, 'M', 'S', 'G'};

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK(bring_online(&s, 1) == 1);

        /* seq у пуша свой, к нашим запросам отношения не имеет. */
        reply_len = server_packet(wire, sizeof wire, KB_CMD_PUSH, 128, 777,
                                  push_body, sizeof push_body);
        kb_session_feed(&s, wire, reply_len);

        CHECK_EQ(next_meaningful(&s, &event), KB_EVENT_PUSH);
        CHECK_EQ(event.opcode, 128);
        CHECK_EQ(event.seq, 777);
        CHECK_MEM(event.payload, event.payload_len, push_body, sizeof push_body);
    }

    KB_TEST("ответ на незнакомый seq игнорируется");
    {
        size_t reply_len;

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK(bring_online(&s, 1) == 1);

        reply_len = server_packet(wire, sizeof wire, KB_CMD_OK, 64, 4242, NULL, 0);
        kb_session_feed(&s, wire, reply_len);

        CHECK_EQ(kb_session_poll_event(&s, &event), KB_OK);
        CHECK_EQ(event.type, KB_EVENT_STATE); /* ONLINE */
        CHECK_EQ(kb_session_poll_event(&s, &event), KB_OK);
        CHECK_EQ(event.type, KB_EVENT_NONE);
    }

    KB_TEST("keepalive: первый пинг через интервал, а не сразу");
    {
        kb_header header;
        size_t queued;
        size_t payload_len = 0;
        size_t expected_len = 0;

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK(bring_online(&s, 1) == 1);
        CHECK_EQ(drain(&s, wire, sizeof wire), 0);

        CHECK_EQ(kb_session_tick(&s, 1000 + 29999), KB_OK);
        CHECK_EQ(drain(&s, wire, sizeof wire), 0);

        CHECK_EQ(kb_session_tick(&s, 1000 + 30000), KB_OK);
        queued = drain(&s, wire, sizeof wire);
        CHECK(queued > 0);
        CHECK_EQ(kb_decode(wire, queued, &header, body, sizeof body,
                           &payload_len),
                 KB_OK);
        CHECK_EQ(header.opcode, KB_OPCODE_PING);
        CHECK_EQ(kb_build_ping_payload(1, tmp, sizeof tmp, &expected_len), KB_OK);
        CHECK_MEM(body, payload_len, tmp, expected_len);

        /* Пинг ответа не ждёт. */
        CHECK_EQ(kb_session_pending_count(&s), 0);

        /* Следующий — ещё через интервал. */
        CHECK_EQ(kb_session_tick(&s, 1000 + 59999), KB_OK);
        CHECK_EQ(drain(&s, wire, sizeof wire), 0);
        CHECK_EQ(kb_session_tick(&s, 1000 + 60000), KB_OK);
        CHECK(drain(&s, wire, sizeof wire) > 0);
    }

    KB_TEST("ghost-режим меняет флаг в пинге");
    {
        kb_header header;
        size_t queued;
        size_t payload_len = 0;
        size_t expected_len = 0;

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK(bring_online(&s, 1) == 1);
        kb_session_set_ping_interactive(&s, 0);
        kb_session_tick(&s, 1000 + 30000);

        queued = drain(&s, wire, sizeof wire);
        kb_decode(wire, queued, &header, body, sizeof body, &payload_len);
        CHECK_EQ(kb_build_ping_payload(0, tmp, sizeof tmp, &expected_len), KB_OK);
        CHECK_MEM(body, payload_len, tmp, expected_len);
    }

    KB_TEST("запрос без ответа истекает");
    {
        unsigned short seq = 0;

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK(bring_online(&s, 1) == 1);
        CHECK_EQ(kb_session_request(&s, 64, NULL, 0, &seq), KB_OK);
        drain(&s, wire, sizeof wire);

        /* Сначала вычерпываем смену состояния на ONLINE. */
        CHECK_EQ(kb_session_poll_event(&s, &event), KB_OK);
        CHECK_EQ(event.type, KB_EVENT_STATE);

        CHECK_EQ(kb_session_tick(&s, 1000 + 4999), KB_OK);
        CHECK_EQ(kb_session_poll_event(&s, &event), KB_OK);
        CHECK_EQ(event.type, KB_EVENT_NONE);

        CHECK_EQ(kb_session_tick(&s, 1000 + 5000), KB_OK);
        CHECK_EQ(next_meaningful(&s, &event), KB_EVENT_REQUEST_FAILED);
        CHECK_EQ(event.seq, seq);
        CHECK_EQ(event.opcode, 64);
        CHECK_EQ(event.status, KB_ERR_TIMEOUT);
        CHECK_EQ(kb_session_pending_count(&s), 0);
    }

    KB_TEST("обрыв роняет запросы в полёте и назначает переподключение");
    {
        unsigned short first = 0;
        unsigned short second = 0;

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK(bring_online(&s, 1) == 1);
        CHECK_EQ(kb_session_request(&s, 64, NULL, 0, &first), KB_OK);
        CHECK_EQ(kb_session_request(&s, 65, NULL, 0, &second), KB_OK);
        CHECK_EQ(kb_session_pending_count(&s), 2);

        CHECK_EQ(kb_session_on_disconnected(&s, 9000), KB_OK);
        CHECK_EQ(kb_session_get_state(&s), KB_STATE_DISCONNECTED);

        CHECK_EQ(next_meaningful(&s, &event), KB_EVENT_REQUEST_FAILED);
        CHECK_EQ(event.status, KB_ERR_CLOSED);
        CHECK_EQ(next_meaningful(&s, &event), KB_EVENT_REQUEST_FAILED);
        CHECK_EQ(event.status, KB_ERR_CLOSED);
        CHECK_EQ(kb_session_pending_count(&s), 0);

        /* Первая попытка — через две секунды. */
        CHECK_EQ(kb_session_next_deadline(&s), 9000 + 2000);

        /* Вторая — через четыре, третья — через восемь. */
        kb_session_on_connecting(&s, 11000);
        kb_session_on_disconnected(&s, 12000);
        CHECK_EQ(kb_session_next_deadline(&s), 12000 + 4000);
        kb_session_on_connecting(&s, 16000);
        kb_session_on_disconnected(&s, 17000);
        CHECK_EQ(kb_session_next_deadline(&s), 17000 + 8000);
    }

    KB_TEST("удачный handshake сбрасывает счётчик попыток");
    {
        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        kb_session_on_connecting(&s, 1000);
        kb_session_on_disconnected(&s, 2000);
        kb_session_on_connecting(&s, 5000);
        kb_session_on_disconnected(&s, 6000);
        CHECK_EQ(kb_session_next_deadline(&s), 6000 + 4000);

        CHECK(bring_online(&s, 1) != 0);
        kb_session_on_disconnected(&s, 20000);
        CHECK_EQ(kb_session_next_deadline(&s), 20000 + 2000);
    }

    KB_TEST("запросы вне онлайна отвергаются");
    {
        unsigned short seq = 0;

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK_EQ(kb_session_request(&s, 64, NULL, 0, &seq), KB_ERR_STATE);
        kb_session_on_connecting(&s, 1000);
        CHECK_EQ(kb_session_request(&s, 64, NULL, 0, &seq), KB_ERR_STATE);
        kb_session_on_connected(&s, 1000);
        /* Handshake ещё не подтверждён — рано. */
        CHECK_EQ(kb_session_request(&s, 64, NULL, 0, &seq), KB_ERR_STATE);
        CHECK_EQ(kb_session_send(&s, 64, NULL, 0, &seq), KB_ERR_STATE);
    }

    KB_TEST("таблица ожидающих может кончиться");
    {
        size_t i;
        unsigned short seq = 0;
        size_t capacity = sizeof pending_slots / sizeof pending_slots[0];

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK(bring_online(&s, 1) == 1);
        for (i = 0; i < capacity; i++) {
            CHECK_EQ(kb_session_request(&s, 64, NULL, 0, &seq), KB_OK);
        }
        CHECK_EQ(kb_session_request(&s, 64, NULL, 0, &seq), KB_ERR_OVERFLOW);
        CHECK_EQ(kb_session_pending_count(&s), capacity);

        /* Отправка без ожидания места не занимает. */
        CHECK_EQ(kb_session_send(&s, 64, NULL, 0, &seq), KB_OK);
    }

    KB_TEST("поток по одному байту");
    {
        unsigned short seq = 0;
        size_t reply_len;
        size_t i;

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK(bring_online(&s, 1) == 1);
        CHECK_EQ(kb_session_request(&s, 64, NULL, 0, &seq), KB_OK);
        drain(&s, tmp, sizeof tmp);

        reply_len = server_packet(wire, sizeof wire, KB_CMD_OK, 64, seq,
                                  (const unsigned char *)"kolibri", 7);
        for (i = 0; i < reply_len; i++) {
            CHECK_EQ(kb_session_feed(&s, wire + i, 1), KB_OK);
        }
        CHECK_EQ(next_meaningful(&s, &event), KB_EVENT_RESPONSE);
        CHECK_MEM(event.payload, event.payload_len, "kolibri", 7);
    }

    KB_TEST("несколько пакетов одним куском");
    {
        unsigned short a = 0;
        unsigned short b = 0;
        size_t total = 0;

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK(bring_online(&s, 1) == 1);
        CHECK_EQ(kb_session_request(&s, 64, NULL, 0, &a), KB_OK);
        CHECK_EQ(kb_session_request(&s, 65, NULL, 0, &b), KB_OK);
        drain(&s, tmp, sizeof tmp);

        total = server_packet(wire, sizeof wire, KB_CMD_OK, 64, a,
                              (const unsigned char *)"aa", 2);
        total += server_packet(wire + total, sizeof wire - total, KB_CMD_PUSH,
                               128, 5, (const unsigned char *)"push", 4);
        total += server_packet(wire + total, sizeof wire - total, KB_CMD_OK, 65,
                               b, (const unsigned char *)"bb", 2);
        CHECK_EQ(kb_session_feed(&s, wire, total), KB_OK);

        CHECK_EQ(next_meaningful(&s, &event), KB_EVENT_RESPONSE);
        CHECK_EQ(event.seq, a);
        CHECK_EQ(kb_session_poll_event(&s, &event), KB_OK);
        CHECK_EQ(event.type, KB_EVENT_PUSH);
        CHECK_EQ(kb_session_poll_event(&s, &event), KB_OK);
        CHECK_EQ(event.type, KB_EVENT_RESPONSE);
        CHECK_EQ(event.seq, b);
        CHECK_EQ(kb_session_pending_count(&s), 0);
    }

    KB_TEST("ближайший дедлайн выбирается верно");
    {
        unsigned short seq = 0;

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK_EQ(kb_session_next_deadline(&s), 0);

        CHECK(bring_online(&s, 1) == 1);
        /* Пинг через 30 секунд после подключения на отметке 1000. */
        CHECK_EQ(kb_session_next_deadline(&s), 31000);

        /* Таймаут запроса ближе пинга. */
        kb_session_tick(&s, 2000);
        CHECK_EQ(kb_session_request(&s, 64, NULL, 0, &seq), KB_OK);
        CHECK_EQ(kb_session_next_deadline(&s), 7000);
    }

    KB_TEST("испорченный поток даёт ошибку, а не тишину");
    {
        unsigned char tiny_rx[32];
        kb_session_buffers small = make_buffers();

        small.rx = tiny_rx;
        small.rx_capacity = sizeof tiny_rx;

        CHECK_EQ(kb_session_init(&s, &config, &small), KB_OK);
        kb_session_on_connecting(&s, 1000);
        kb_session_on_connected(&s, 1000);
        drain(&s, wire, sizeof wire);

        memset(wire, 0xEE, 64);
        CHECK_EQ(kb_session_feed(&s, wire, 64), KB_ERR_OVERFLOW);
        CHECK_EQ(next_meaningful(&s, &event), KB_EVENT_ERROR);
        CHECK_EQ(event.status, KB_ERR_OVERFLOW);
    }

    KB_TEST("аргументы");
    {
        CHECK_EQ(kb_session_init(NULL, &config, &bufs), KB_ERR_INVALID);
        CHECK_EQ(kb_session_init(&s, NULL, &bufs), KB_ERR_INVALID);
        CHECK_EQ(kb_session_init(&s, &config, NULL), KB_ERR_INVALID);
        {
            kb_session_buffers broken = make_buffers();
            broken.pending_capacity = 0;
            CHECK_EQ(kb_session_init(&s, &config, &broken), KB_ERR_INVALID);
            broken = make_buffers();
            broken.rx = NULL;
            CHECK_EQ(kb_session_init(&s, &config, &broken), KB_ERR_INVALID);
        }
        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        CHECK(kb_session_pull_out(NULL, NULL) == NULL);
        CHECK_EQ(kb_session_consume_out(&s, 10), KB_ERR_INVALID);
        CHECK_EQ(kb_session_poll_event(&s, NULL), KB_ERR_INVALID);
        CHECK_EQ(kb_session_next_deadline(NULL), 0);
        CHECK_EQ(kb_session_get_state(NULL), KB_STATE_DISCONNECTED);
    }
}
