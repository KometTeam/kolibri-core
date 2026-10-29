/*
 * Машина сессии, sans-IO.
 *
 * Перенесено с эталонной реализации (kolibri-net, src/session/manager.rs и
 * src/transport/{client,dispatcher}.rs). Сохранено ровно то, что наблюдаемо
 * снаружи: последовательность handshake, правило различения ответа и пуша,
 * нумерация seq, расписание пингов и лестница backoff.
 *
 * Всё, что в эталоне делают tokio и потоки, здесь делает хост: подключается,
 * читает, пишет и спит. Сессия только считает, что и когда должно произойти.
 */

#include <string.h>

#include "kolibri.h"

/* ─── Payload'ы, которые собирает сама сессия ──────────────────────────── */

kb_status kb_build_handshake_payload(const kb_handshake_config *config,
                                     uint8_t *out, size_t out_cap,
                                     size_t *out_len)
{
    kb_mp_writer w;
    const kb_user_agent *ua;
    size_t ua_fields;

    if (config == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    ua = &config->user_agent;

    kb_mp_writer_init(&w, out, out_cap);
    kb_mp_write_map(&w, 4);

    kb_mp_write_cstr(&w, "mt_instanceid");
    kb_mp_write_cstr(&w, config->instance_id);

    kb_mp_write_cstr(&w, "userAgent");
    ua_fields = 11;
    if (ua->header_user_agent != NULL) {
        ua_fields++;
    }
    if (ua->has_is_pwa) {
        ua_fields++;
    }
    kb_mp_write_map(&w, ua_fields);
    kb_mp_write_cstr(&w, "deviceType");
    kb_mp_write_cstr(&w, ua->device_type);
    kb_mp_write_cstr(&w, "appVersion");
    kb_mp_write_cstr(&w, ua->app_version);
    kb_mp_write_cstr(&w, "osVersion");
    kb_mp_write_cstr(&w, ua->os_version);
    kb_mp_write_cstr(&w, "timezone");
    kb_mp_write_cstr(&w, ua->timezone);
    kb_mp_write_cstr(&w, "screen");
    kb_mp_write_cstr(&w, ua->screen);
    kb_mp_write_cstr(&w, "pushDeviceType");
    kb_mp_write_cstr(&w, ua->push_device_type);
    kb_mp_write_cstr(&w, "arch");
    kb_mp_write_cstr(&w, ua->arch);
    kb_mp_write_cstr(&w, "locale");
    kb_mp_write_cstr(&w, ua->locale);
    kb_mp_write_cstr(&w, "buildNumber");
    kb_mp_write_i64(&w, ua->build_number);
    kb_mp_write_cstr(&w, "deviceName");
    kb_mp_write_cstr(&w, ua->device_name);
    kb_mp_write_cstr(&w, "deviceLocale");
    kb_mp_write_cstr(&w, ua->device_locale);
    /* Порядок как в эталоне: headerUserAgent, затем isPwa. */
    if (ua->header_user_agent != NULL) {
        kb_mp_write_cstr(&w, "headerUserAgent");
        kb_mp_write_cstr(&w, ua->header_user_agent);
    }
    if (ua->has_is_pwa) {
        kb_mp_write_cstr(&w, "isPwa");
        kb_mp_write_bool(&w, ua->is_pwa);
    }

    kb_mp_write_cstr(&w, "clientSessionId");
    kb_mp_write_i64(&w, config->client_session_id);

    kb_mp_write_cstr(&w, "deviceId");
    kb_mp_write_cstr(&w, config->device_id);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_build_ping_payload(int interactive, uint8_t *out, size_t out_cap,
                                size_t *out_len)
{
    kb_mp_writer w;

    if (out == NULL) {
        return KB_ERR_INVALID;
    }
    kb_mp_writer_init(&w, out, out_cap);
    kb_mp_write_map(&w, 1);
    kb_mp_write_cstr(&w, "interactive");
    kb_mp_write_bool(&w, interactive);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

uint32_t kb_session_backoff_ms(uint32_t attempt)
{
    /* (2 * 2^min(attempt,3)) секунд, зажатое в 2..15 — как в эталоне. */
    uint32_t shift = attempt > 3 ? 3 : attempt;
    uint32_t seconds = 2u << shift;
    if (seconds > 15u) {
        seconds = 15u;
    }
    if (seconds < 2u) {
        seconds = 2u;
    }
    return seconds * 1000u;
}

/* ─── Внутреннее ───────────────────────────────────────────────────────── */

static void push_state(kb_session *s, kb_session_state state)
{
    if (s->state == state) {
        return;
    }
    s->state = state;

    if (s->state_count < KB_STATE_QUEUE) {
        s->state_queue[(s->state_head + s->state_count) % KB_STATE_QUEUE] = state;
        s->state_count++;
    } else {
        /* Кольцо забито: жертвуем самым старым, свежее важнее. */
        s->state_head = (s->state_head + 1) % KB_STATE_QUEUE;
        s->state_queue[(s->state_head + s->state_count - 1) % KB_STATE_QUEUE] =
            state;
    }
}

/*
 * Кодирует пакет прямо в хвост очереди исходящих. Перед этим очередь
 * уплотняется, поэтому указатель, выданный kb_session_pull_out, после любой
 * постановки в очередь считается недействительным.
 */
static kb_status tx_encode(kb_session *s, uint8_t cmd, uint16_t opcode,
                           uint16_t seq, const uint8_t *payload, size_t len)
{
    size_t written = 0;
    kb_status st;

    if (s->tx_start > 0) {
        size_t queued = s->tx_end - s->tx_start;
        if (queued > 0) {
            memmove(s->bufs.tx, s->bufs.tx + s->tx_start, queued);
        }
        s->tx_start = 0;
        s->tx_end = queued;
    }

    st = kb_encode(cmd, opcode, seq, payload, len, s->bufs.tx + s->tx_end,
                   s->bufs.tx_capacity - s->tx_end, &written);
    if (st != KB_OK) {
        return st == KB_ERR_TOO_LARGE ? KB_ERR_OVERFLOW : st;
    }
    s->tx_end += written;
    if (s->tap != NULL) {
        s->tap(s->tap_user, KB_TAP_OUT, cmd, opcode, seq, payload, len);
    }
    return KB_OK;
}

static uint16_t next_seq(kb_session *s)
{
    /* Предынкремент с переносом: первый запрос получает 1. */
    s->seq = (uint16_t)(s->seq + 1u);
    return s->seq;
}

static kb_pending *pending_find(kb_session *s, uint16_t seq)
{
    size_t i;
    for (i = 0; i < s->bufs.pending_capacity; i++) {
        kb_pending *p = &s->bufs.pending[i];
        if (p->in_use && p->seq == seq) {
            return p;
        }
    }
    return NULL;
}

static kb_pending *pending_alloc(kb_session *s)
{
    size_t i;
    for (i = 0; i < s->bufs.pending_capacity; i++) {
        kb_pending *p = &s->bufs.pending[i];
        if (!p->in_use) {
            return p;
        }
    }
    return NULL;
}

static void schedule_ping(kb_session *s)
{
    if (s->config.ping_interval_ms == 0) {
        s->next_ping_ms = 0;
        return;
    }
    /* Первый keepalive — через интервал после подключения, а не сразу. */
    s->next_ping_ms = s->now_ms + (uint64_t)s->config.ping_interval_ms;
}

static kb_status queue_handshake(kb_session *s)
{
    size_t payload_len = 0;
    kb_pending *slot;
    kb_status st;
    uint16_t seq;

    st = kb_build_handshake_payload(&s->config.handshake, s->bufs.scratch,
                                    s->bufs.scratch_capacity, &payload_len);
    if (st != KB_OK) {
        return st;
    }

    slot = pending_alloc(s);
    if (slot == NULL) {
        return KB_ERR_OVERFLOW;
    }

    seq = next_seq(s);
    st = tx_encode(s, KB_CMD_REQUEST, KB_OPCODE_SESSION_INIT, seq,
                   s->bufs.scratch, payload_len);
    if (st != KB_OK) {
        return st;
    }

    slot->in_use = 1;
    slot->failed = KB_OK;
    slot->seq = seq;
    slot->opcode = KB_OPCODE_SESSION_INIT;
    slot->deadline_ms = s->config.request_timeout_ms == 0
                            ? 0
                            : s->now_ms + (uint64_t)s->config.request_timeout_ms;
    s->handshake_seq = seq;
    return KB_OK;
}

size_t kb_sizeof(int what)
{
    switch (what) {
    case KB_SIZEOF_SESSION:
        return sizeof(kb_session);
    case KB_SIZEOF_CONFIG:
        return sizeof(kb_session_config);
    case KB_SIZEOF_BUFFERS:
        return sizeof(kb_session_buffers);
    case KB_SIZEOF_EVENT:
        return sizeof(kb_event);
    case KB_SIZEOF_PENDING:
        return sizeof(kb_pending);
    case KB_SIZEOF_USER_AGENT:
        return sizeof(kb_user_agent);
    case KB_SIZEOF_HANDSHAKE:
        return sizeof(kb_handshake_config);
    default:
        break;
    }
    return 0;
}

/* ─── Жизненный цикл ───────────────────────────────────────────────────── */

kb_status kb_session_init(kb_session *session, const kb_session_config *config,
                          const kb_session_buffers *buffers)
{
    kb_status st;

    if (session == NULL || config == NULL || buffers == NULL) {
        return KB_ERR_INVALID;
    }
    if (buffers->rx == NULL || buffers->tx == NULL || buffers->scratch == NULL ||
        buffers->pending == NULL) {
        return KB_ERR_INVALID;
    }
    if (buffers->rx_capacity < KB_HEADER_SIZE || buffers->tx_capacity < 64 ||
        buffers->scratch_capacity < 256 || buffers->pending_capacity == 0) {
        return KB_ERR_INVALID;
    }

    memset(session, 0, sizeof *session);
    session->config = *config;
    session->bufs = *buffers;
    session->state = KB_STATE_DISCONNECTED;
    session->fault = KB_OK;

    memset(buffers->pending, 0,
           buffers->pending_capacity * sizeof buffers->pending[0]);

    st = kb_rx_init(&session->rx, buffers->rx, buffers->rx_capacity);
    if (st != KB_OK) {
        return st;
    }
    return KB_OK;
}

kb_status kb_session_on_connecting(kb_session *session, uint64_t now_ms)
{
    if (session == NULL) {
        return KB_ERR_INVALID;
    }
    session->now_ms = now_ms;
    session->reconnect_at_ms = 0;
    push_state(session, KB_STATE_CONNECTING);
    return KB_OK;
}

kb_status kb_session_on_connected(kb_session *session, uint64_t now_ms)
{
    kb_status st;

    if (session == NULL) {
        return KB_ERR_INVALID;
    }
    session->now_ms = now_ms;
    session->reconnect_at_ms = 0;

    /* Новое соединение — новый поток: старые остатки к нему не относятся. */
    kb_rx_reset(&session->rx);
    session->tx_start = 0;
    session->tx_end = 0;
    session->fault = KB_OK;

    push_state(session, KB_STATE_CONNECTED);

    st = queue_handshake(session);
    if (st != KB_OK) {
        push_state(session, KB_STATE_DISCONNECTED);
        return st;
    }
    return KB_OK;
}

kb_status kb_session_on_disconnected(kb_session *session, uint64_t now_ms)
{
    size_t i;

    if (session == NULL) {
        return KB_ERR_INVALID;
    }
    session->now_ms = now_ms;

    /* Запросы в полёте ответа уже не дождутся. */
    for (i = 0; i < session->bufs.pending_capacity; i++) {
        kb_pending *p = &session->bufs.pending[i];
        if (p->in_use && p->failed == KB_OK) {
            p->failed = KB_ERR_CLOSED;
            p->deadline_ms = 0;
        }
    }

    kb_rx_reset(&session->rx);
    session->tx_start = 0;
    session->tx_end = 0;
    session->next_ping_ms = 0;
    session->handshake_seq = 0;

    push_state(session, KB_STATE_DISCONNECTED);

    if (session->config.auto_reconnect) {
        session->reconnect_at_ms =
            now_ms + (uint64_t)kb_session_backoff_ms(session->attempt);
        session->attempt++;
    } else {
        session->reconnect_at_ms = 0;
    }
    return KB_OK;
}

/* ─── Поток данных ─────────────────────────────────────────────────────── */

kb_status kb_session_feed(kb_session *session, const uint8_t *data, size_t len)
{
    kb_status st;

    if (session == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_rx_push(&session->rx, data, len);
    if (st != KB_OK) {
        /* Поток не бьётся на пакеты — соединение надо рвать. */
        session->fault = st;
    }
    return st;
}

kb_status kb_session_tick(kb_session *session, uint64_t now_ms)
{
    if (session == NULL) {
        return KB_ERR_INVALID;
    }
    session->now_ms = now_ms;

    if (session->state == KB_STATE_ONLINE && session->next_ping_ms != 0 &&
        now_ms >= session->next_ping_ms) {
        size_t payload_len = 0;
        kb_status st = kb_build_ping_payload(session->config.ping_interactive,
                                             session->bufs.scratch,
                                             session->bufs.scratch_capacity,
                                             &payload_len);
        if (st != KB_OK) {
            return st;
        }
        /* Пинг уходит без ожидания ответа: seq выделяется, слот — нет. */
        st = tx_encode(session, KB_CMD_REQUEST, KB_OPCODE_PING,
                       next_seq(session), session->bufs.scratch, payload_len);
        if (st != KB_OK) {
            return st;
        }
        session->next_ping_ms =
            now_ms + (uint64_t)session->config.ping_interval_ms;
    }
    return KB_OK;
}

const uint8_t *kb_session_pull_out(kb_session *session, size_t *len)
{
    size_t queued;

    if (session == NULL) {
        return NULL;
    }
    queued = session->tx_end - session->tx_start;
    if (queued == 0) {
        return NULL;
    }
    if (len != NULL) {
        *len = queued;
    }
    return session->bufs.tx + session->tx_start;
}

kb_status kb_session_consume_out(kb_session *session, size_t len)
{
    size_t queued;

    if (session == NULL) {
        return KB_ERR_INVALID;
    }
    queued = session->tx_end - session->tx_start;
    if (len > queued) {
        return KB_ERR_INVALID;
    }
    session->tx_start += len;
    if (session->tx_start == session->tx_end) {
        session->tx_start = 0;
        session->tx_end = 0;
    }
    return KB_OK;
}

/* ─── События ──────────────────────────────────────────────────────────── */

static void fill_packet_event(kb_event *out, const kb_header *header,
                              const uint8_t *payload, size_t payload_len)
{
    out->seq = header->seq;
    out->opcode = header->opcode;
    out->cmd = header->cmd;
    out->payload = payload;
    out->payload_len = payload_len;
}

static void read_handshake_info(kb_event *out, const uint8_t *payload,
                                size_t len)
{
    kb_mp_value value;
    if (kb_mp_map_get(payload, len, "callsSeed", &value) != KB_OK) {
        return;
    }
    if (value.type == KB_MP_INT) {
        out->has_calls_seed = 1;
        out->calls_seed = value.i;
    } else if (value.type == KB_MP_UINT) {
        out->has_calls_seed = 1;
        out->calls_seed = (int64_t)value.u;
    }
}

kb_status kb_session_poll_event(kb_session *session, kb_event *out)
{
    size_t i;

    if (session == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);
    out->type = KB_EVENT_NONE;

    /* 1. Поломка потока: важнее всего, дальше разбирать нечего. */
    if (session->fault != KB_OK) {
        out->type = KB_EVENT_ERROR;
        out->status = session->fault;
        session->fault = KB_OK;
        return KB_OK;
    }

    /* 2. Смены состояния — в том порядке, в каком случились. */
    if (session->state_count > 0) {
        out->type = KB_EVENT_STATE;
        out->state = session->state_queue[session->state_head];
        session->state_head = (session->state_head + 1) % KB_STATE_QUEUE;
        session->state_count--;
        return KB_OK;
    }

    /* 3. Запросы, оставшиеся без ответа. */
    for (i = 0; i < session->bufs.pending_capacity; i++) {
        kb_pending *p = &session->bufs.pending[i];
        if (!p->in_use) {
            continue;
        }
        if (p->failed == KB_OK && p->deadline_ms != 0 &&
            session->now_ms >= p->deadline_ms) {
            p->failed = KB_ERR_TIMEOUT;
        }
        if (p->failed != KB_OK) {
            out->type = KB_EVENT_REQUEST_FAILED;
            out->seq = p->seq;
            out->opcode = p->opcode;
            out->status = p->failed;
            p->in_use = 0;
            p->failed = KB_OK;
            return KB_OK;
        }
    }

    /* 4. Следующий целый пакет из потока. */
    for (;;) {
        const uint8_t *raw;
        size_t raw_len = 0;
        kb_header header;
        size_t payload_len = 0;
        kb_status st;
        kb_pending *waiter;

        raw = kb_rx_next(&session->rx, &raw_len);
        if (raw == NULL) {
            return KB_OK;
        }

        st = kb_decode(raw, raw_len, &header, session->bufs.scratch,
                       session->bufs.scratch_capacity, &payload_len);
        if (st != KB_OK) {
            /* Эталон битый пакет молча пропускает — но тесноту буфера
               замалчивать нельзя, иначе тело просто исчезнет. */
            if (st == KB_ERR_TOO_LARGE || st == KB_ERR_LIMIT) {
                out->type = KB_EVENT_ERROR;
                out->status = st;
                return KB_OK;
            }
            continue;
        }

        if (session->tap != NULL) {
            session->tap(session->tap_user, KB_TAP_IN, header.cmd,
                         header.opcode, header.seq, session->bufs.scratch,
                         payload_len);
        }

        /* Ответ на sessionInit разбирается особо. */
        if (session->state == KB_STATE_CONNECTED &&
            header.seq == session->handshake_seq && header.cmd != KB_CMD_PUSH) {
            waiter = pending_find(session, header.seq);
            if (waiter != NULL) {
                waiter->in_use = 0;
            }
            if (header.cmd != KB_CMD_OK) {
                out->type = KB_EVENT_ERROR;
                out->status = KB_ERR_STATE;
                fill_packet_event(out, &header, session->bufs.scratch,
                                  payload_len);
                return KB_OK;
            }
            out->type = KB_EVENT_HANDSHAKE;
            fill_packet_event(out, &header, session->bufs.scratch, payload_len);
            read_handshake_info(out, session->bufs.scratch, payload_len);

            session->attempt = 0;
            push_state(session, KB_STATE_ONLINE);
            schedule_ping(session);
            return KB_OK;
        }

        /* Ответ определяется командой, а не совпадением seq — как в эталоне. */
        if (header.cmd == KB_CMD_OK || header.cmd == KB_CMD_NOT_FOUND ||
            header.cmd == KB_CMD_ERROR) {
            waiter = pending_find(session, header.seq);
            if (waiter == NULL) {
                /* Ответ на запрос, которого мы не ждём: пинг или уже истёкший. */
                continue;
            }
            waiter->in_use = 0;
            out->type = KB_EVENT_RESPONSE;
            fill_packet_event(out, &header, session->bufs.scratch, payload_len);
            return KB_OK;
        }

        out->type = KB_EVENT_PUSH;
        fill_packet_event(out, &header, session->bufs.scratch, payload_len);
        return KB_OK;
    }
}

/* ─── Запросы ──────────────────────────────────────────────────────────── */

static kb_status submit(kb_session *session, uint16_t opcode,
                        const uint8_t *payload, size_t len, int expect_reply,
                        uint16_t *out_seq)
{
    kb_pending *slot = NULL;
    uint16_t seq;
    kb_status st;

    if (session == NULL) {
        return KB_ERR_INVALID;
    }
    if (payload == NULL && len != 0) {
        return KB_ERR_INVALID;
    }
    if (session->state != KB_STATE_ONLINE) {
        return KB_ERR_STATE;
    }

    if (expect_reply) {
        slot = pending_alloc(session);
        if (slot == NULL) {
            return KB_ERR_OVERFLOW;
        }
    }

    seq = next_seq(session);
    st = tx_encode(session, KB_CMD_REQUEST, opcode, seq, payload, len);
    if (st != KB_OK) {
        return st;
    }

    if (slot != NULL) {
        slot->in_use = 1;
        slot->failed = KB_OK;
        slot->seq = seq;
        slot->opcode = opcode;
        slot->deadline_ms =
            session->config.request_timeout_ms == 0
                ? 0
                : session->now_ms + (uint64_t)session->config.request_timeout_ms;
    }
    if (out_seq != NULL) {
        *out_seq = seq;
    }
    return KB_OK;
}

kb_status kb_session_request(kb_session *session, uint16_t opcode,
                             const uint8_t *payload, size_t len,
                             uint16_t *out_seq)
{
    return submit(session, opcode, payload, len, 1, out_seq);
}

kb_status kb_session_send(kb_session *session, uint16_t opcode,
                          const uint8_t *payload, size_t len, uint16_t *out_seq)
{
    return submit(session, opcode, payload, len, 0, out_seq);
}

/* ─── Справки ──────────────────────────────────────────────────────────── */

uint64_t kb_session_next_deadline(const kb_session *session)
{
    uint64_t best = 0;
    size_t i;

    if (session == NULL) {
        return 0;
    }

    if (session->state == KB_STATE_ONLINE && session->next_ping_ms != 0) {
        best = session->next_ping_ms;
    }
    if (session->state == KB_STATE_DISCONNECTED &&
        session->reconnect_at_ms != 0 &&
        (best == 0 || session->reconnect_at_ms < best)) {
        best = session->reconnect_at_ms;
    }
    for (i = 0; i < session->bufs.pending_capacity; i++) {
        const kb_pending *p = &session->bufs.pending[i];
        if (p->in_use && p->failed == KB_OK && p->deadline_ms != 0 &&
            (best == 0 || p->deadline_ms < best)) {
            best = p->deadline_ms;
        }
    }
    return best;
}

kb_session_state kb_session_get_state(const kb_session *session)
{
    return session == NULL ? KB_STATE_DISCONNECTED : session->state;
}

size_t kb_session_pending_count(const kb_session *session)
{
    size_t count = 0;
    size_t i;

    if (session == NULL) {
        return 0;
    }
    for (i = 0; i < session->bufs.pending_capacity; i++) {
        if (session->bufs.pending[i].in_use) {
            count++;
        }
    }
    return count;
}

void kb_session_set_ping_interactive(kb_session *session, int interactive)
{
    if (session != NULL) {
        session->config.ping_interactive = interactive;
    }
}

const char *kb_tap_direction_str(kb_tap_direction dir)
{
    return dir == KB_TAP_OUT ? "out" : "in";
}

void kb_session_set_tap(kb_session *session, kb_tap_fn tap, void *user)
{
    if (session == NULL) {
        return;
    }
    session->tap = tap;
    session->tap_user = user;
}

/* HTTP User-Agent для заголовка загрузки медиа. Ручная сборка с контролем
   границ — без snprintf, чтобы строго держать C89. */
static int ua_append(char *out, size_t cap, size_t *at, const char *s)
{
    if (s == NULL) {
        return 1;
    }
    while (*s != '\0') {
        if (*at + 1 >= cap) {
            return 0;
        }
        out[(*at)++] = *s++;
    }
    return 1;
}

kb_status kb_user_agent_http(const kb_user_agent *ua, char *out, size_t cap,
                             size_t *out_len)
{
    size_t at = 0;
    int ok = 1;

    if (ua == NULL || out == NULL || cap == 0) {
        return KB_ERR_INVALID;
    }
    ok = ok && ua_append(out, cap, &at, "OKMessages/");
    ok = ok && ua_append(out, cap, &at, ua->app_version);
    ok = ok && ua_append(out, cap, &at, " (");
    ok = ok && ua_append(out, cap, &at, ua->os_version);
    ok = ok && ua_append(out, cap, &at, "; ");
    ok = ok && ua_append(out, cap, &at, ua->device_name);
    ok = ok && ua_append(out, cap, &at, "; ");
    ok = ok && ua_append(out, cap, &at, ua->screen);
    ok = ok && ua_append(out, cap, &at, ")");
    if (!ok) {
        out[cap - 1] = '\0';
        return KB_ERR_TOO_LARGE;
    }
    out[at] = '\0';
    if (out_len != NULL) {
        *out_len = at;
    }
    return KB_OK;
}
