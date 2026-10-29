/* Тап на трафик: срабатывает по разу на пакет в каждую сторону, до диспатча. */

#include <string.h>

#include "kb_test.h"
#include "kolibri.h"
#include "kolibri_session.h"
#include "kolibri_msgpack.h"

#define TBUF 65536u

static unsigned char t_rx[TBUF];
static unsigned char t_tx[TBUF];
static unsigned char t_scratch[TBUF];
static kb_pending t_pending[16];
static unsigned char t_wire[TBUF];

typedef struct {
    kb_tap_direction dir;
    uint8_t cmd;
    uint16_t opcode;
    uint16_t seq;
    unsigned char payload[512];
    size_t len;
} tap_rec;

static tap_rec taps[32];
static int tap_count;

static void record_tap(void *user, kb_tap_direction dir, uint8_t cmd,
                       uint16_t opcode, uint16_t seq, const uint8_t *payload,
                       size_t payload_len)
{
    tap_rec *r;
    *(int *)user += 1; /* заодно проверяем, что user доходит */
    if (tap_count >= 32) {
        return;
    }
    r = &taps[tap_count++];
    r->dir = dir;
    r->cmd = cmd;
    r->opcode = opcode;
    r->seq = seq;
    r->len = payload_len < sizeof r->payload ? payload_len : sizeof r->payload;
    if (r->len > 0) {
        memcpy(r->payload, payload, r->len);
    }
}

static kb_session_config tap_config(void)
{
    kb_session_config c;
    memset(&c, 0, sizeof c);
    c.handshake.instance_id = "kolibri-core-0001";
    c.handshake.device_id = "kolibri-core-0001";
    c.handshake.client_session_id = 1755730000000LL;
    c.handshake.user_agent.device_type = "ANDROID";
    c.handshake.user_agent.app_version = "26.25.0";
    c.handshake.user_agent.os_version = "Android 14";
    c.handshake.user_agent.timezone = "Europe/Moscow";
    c.handshake.user_agent.screen = "480dpi 480dpi 1344x2992";
    c.handshake.user_agent.push_device_type = "GCM";
    c.handshake.user_agent.arch = "arm64-v8a";
    c.handshake.user_agent.locale = "ru";
    c.handshake.user_agent.build_number = 6790;
    c.handshake.user_agent.device_name = "Google Pixel 8 Pro";
    c.handshake.user_agent.device_locale = "ru";
    c.ping_interval_ms = 30000;
    c.ping_interactive = 1;
    c.request_timeout_ms = 5000;
    c.auto_reconnect = 1;
    return c;
}

static kb_session_buffers tap_buffers(void)
{
    kb_session_buffers b;
    b.rx = t_rx;
    b.rx_capacity = sizeof t_rx;
    b.tx = t_tx;
    b.tx_capacity = sizeof t_tx;
    b.scratch = t_scratch;
    b.scratch_capacity = sizeof t_scratch;
    b.pending = t_pending;
    b.pending_capacity = sizeof t_pending / sizeof t_pending[0];
    return b;
}

/* Валиден ли payload как msgpack-карта и есть ли в ней ключ. */
static int payload_has_key(const unsigned char *p, size_t len, const char *key)
{
    kb_mp_value v;
    return kb_mp_map_get(p, len, key, &v) == KB_OK;
}

static void drain_tx(kb_session *s)
{
    size_t len = 0;
    while (kb_session_pull_out(s, &len) != NULL) {
        kb_session_consume_out(s, len);
    }
}

void kb_tests_tap(void)
{
    kb_session_config config = tap_config();
    kb_session_buffers bufs = tap_buffers();
    kb_session s;
    kb_event event;
    int user_hits = 0;

    KB_TEST("тап видит обе стороны с распакованным payload");
    {
        kb_mp_writer w;
        size_t reply_len;
        unsigned char req_body[16];
        uint16_t req_seq = 0;

        tap_count = 0;
        user_hits = 0;

        CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK);
        kb_session_set_tap(&s, record_tap, &user_hits);

        /* handshake: sessionInit уходит при on_connected. */
        CHECK_EQ(kb_session_on_connected(&s, 1000), KB_OK);
        drain_tx(&s);

        CHECK_EQ(tap_count, 1);
        CHECK_EQ(taps[0].dir, KB_TAP_OUT);
        CHECK_EQ(taps[0].cmd, KB_CMD_REQUEST);
        CHECK_EQ(taps[0].opcode, KB_OPCODE_SESSION_INIT);
        CHECK_EQ(taps[0].seq, 1);
        /* payload — распакованный msgpack sessionInit. */
        CHECK(payload_has_key(taps[0].payload, taps[0].len, "userAgent"));

        /* Ответ сервера на handshake с callsSeed. */
        kb_mp_writer_init(&w, t_wire + 40000, 2048);
        kb_mp_write_map(&w, 1);
        kb_mp_write_cstr(&w, "callsSeed");
        kb_mp_write_i64(&w, 777777);
        reply_len = 0;
        {
            size_t written = 0;
            kb_encode(KB_CMD_OK, KB_OPCODE_SESSION_INIT, 1, t_wire + 40000,
                      kb_mp_writer_len(&w), t_wire, sizeof t_wire, &written);
            reply_len = written;
        }
        CHECK_EQ(kb_session_feed(&s, t_wire, reply_len), KB_OK);
        CHECK_EQ(kb_session_poll_event(&s, &event), KB_OK);
        /* Первое событие может быть сменой состояния; докрутим до HANDSHAKE. */
        while (event.type == KB_EVENT_STATE) {
            kb_session_poll_event(&s, &event);
        }
        CHECK_EQ(event.type, KB_EVENT_HANDSHAKE);

        /* IN-тап на ответе handshake. */
        CHECK_EQ(tap_count, 2);
        CHECK_EQ(taps[1].dir, KB_TAP_IN);
        CHECK_EQ(taps[1].cmd, KB_CMD_OK);
        CHECK_EQ(taps[1].opcode, KB_OPCODE_SESSION_INIT);
        CHECK_EQ(taps[1].seq, 1);
        CHECK(payload_has_key(taps[1].payload, taps[1].len, "callsSeed"));

        /* Исходящий запрос. */
        memset(req_body, 0xC0, sizeof req_body); /* набор nil — валидный msgpack */
        CHECK_EQ(kb_session_request(&s, 64, req_body, sizeof req_body, &req_seq),
                 KB_OK);
        drain_tx(&s);
        CHECK_EQ(tap_count, 3);
        CHECK_EQ(taps[2].dir, KB_TAP_OUT);
        CHECK_EQ(taps[2].cmd, KB_CMD_REQUEST);
        CHECK_EQ(taps[2].opcode, 64);
        CHECK_EQ(taps[2].seq, req_seq);
        CHECK_MEM(taps[2].payload, taps[2].len, req_body, sizeof req_body);

        /* Входящий пуш (cmd == PUSH == 0). */
        {
            size_t written = 0;
            kb_mp_writer pw;
            kb_mp_writer_init(&pw, t_wire + 40000, 512);
            kb_mp_write_map(&pw, 1);
            kb_mp_write_cstr(&pw, "chatId");
            kb_mp_write_i64(&pw, 42);
            kb_encode(KB_CMD_PUSH, 100, 0, t_wire + 40000,
                      kb_mp_writer_len(&pw), t_wire, sizeof t_wire, &written);
            kb_session_feed(&s, t_wire, written);
        }
        while (kb_session_poll_event(&s, &event) == KB_OK &&
               event.type != KB_EVENT_NONE) {
            if (event.type == KB_EVENT_PUSH) {
                break;
            }
        }
        CHECK_EQ(tap_count, 4);
        CHECK_EQ(taps[3].dir, KB_TAP_IN);
        CHECK_EQ(taps[3].cmd, KB_CMD_PUSH);
        CHECK_EQ(taps[3].opcode, 100);
        CHECK(payload_has_key(taps[3].payload, taps[3].len, "chatId"));

        /* user дошёл в колбэк все четыре раза. */
        CHECK_EQ(user_hits, 4);
    }

    KB_TEST("снятый тап больше не зовётся");
    {
        int before;
        uint16_t seq = 0;
        unsigned char b[4];

        kb_session_set_tap(&s, NULL, NULL);
        before = tap_count;
        memset(b, 0xC0, sizeof b);
        kb_session_request(&s, 65, b, sizeof b, &seq);
        drain_tx(&s);
        CHECK_EQ(tap_count, before); /* ничего не прибавилось */
    }

    KB_TEST("строка направления");
    {
        CHECK_EQ(strcmp(kb_tap_direction_str(KB_TAP_OUT), "out"), 0);
        CHECK_EQ(strcmp(kb_tap_direction_str(KB_TAP_IN), "in"), 0);
    }
}
