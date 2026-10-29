/* Блокирующий клиент. См. include/kolibri_client.h. */

#include <stdlib.h>
#include <string.h>

#include "kolibri_client.h"

#define RX_CAP   (512u * 1024u)
#define TX_CAP   (128u * 1024u)
#define SCR_CAP  (512u * 1024u)
#define IO_CAP   (64u * 1024u)
#define PENDING  32u

static void note(kb_client *c, const char *msg)
{
    size_t n = strlen(msg);
    if (n >= sizeof c->detail) {
        n = sizeof c->detail - 1;
    }
    memcpy(c->detail, msg, n);
    c->detail[n] = '\0';
}

/* Слить в сокет всё, что накопила сессия. */
static int flush_out(kb_client *c)
{
    for (;;) {
        size_t len = 0;
        const uint8_t *chunk = kb_session_pull_out(&c->session, &len);
        long w;
        if (chunk == NULL) {
            return 0;
        }
        w = kb_transport_write(&c->transport, chunk, len);
        if (w == KB_IO_WANT_WRITE || w == KB_IO_WANT_READ) {
            if (kb_transport_wait(&c->transport, w == KB_IO_WANT_WRITE, 5000) != 1) {
                return -1;
            }
            continue;
        }
        if (w < 0) {
            return -1;
        }
        kb_session_consume_out(&c->session, (size_t)w);
    }
}

/* Прокрутить один шаг I/O: tick, flush, подождать, прочитать, скормить. */
static int pump_once(kb_client *c, uint32_t wait_ms)
{
    long got;
    kb_session_tick(&c->session, kb_now_ms());
    if (flush_out(c) != 0) {
        return -1;
    }
    if (kb_transport_wait(&c->transport, 0, wait_ms) != 1) {
        return 0;
    }
    for (;;) {
        got = kb_transport_read(&c->transport, c->io, c->io_cap);
        if (got != KB_IO_WANT_WRITE) {
            break;
        }
        if (kb_transport_wait(&c->transport, 1, 5000) != 1) {
            return -1;
        }
    }
    if (got == KB_IO_CLOSED) {
        return -1;
    }
    if (got > 0) {
        if (kb_session_feed(&c->session, c->io, (size_t)got) != KB_OK) {
            return -1;
        }
    } else if (got != KB_IO_WANT_READ) {
        return -1;
    }
    return 0;
}

kb_status kb_client_open(kb_client *c, const kb_client_config *cfg)
{
    kb_transport_config tc;
    uint64_t deadline;

    if (c == NULL || cfg == NULL || cfg->host == NULL) {
        return KB_ERR_INVALID;
    }
    memset(c, 0, sizeof *c);
    c->io_timeout_ms = cfg->io_timeout_ms ? cfg->io_timeout_ms : 30000u;

    c->rx = (uint8_t *)malloc(RX_CAP);
    c->tx = (uint8_t *)malloc(TX_CAP);
    c->scratch = (uint8_t *)malloc(SCR_CAP);
    c->io = (uint8_t *)malloc(IO_CAP);
    c->pending = (kb_pending *)calloc(PENDING, sizeof(kb_pending));
    if (!c->rx || !c->tx || !c->scratch || !c->io || !c->pending) {
        kb_client_close(c);
        return KB_ERR_TOO_LARGE;
    }
    c->io_cap = IO_CAP;
    c->bufs.rx = c->rx;             c->bufs.rx_capacity = RX_CAP;
    c->bufs.tx = c->tx;             c->bufs.tx_capacity = TX_CAP;
    c->bufs.scratch = c->scratch;   c->bufs.scratch_capacity = SCR_CAP;
    c->bufs.pending = c->pending;   c->bufs.pending_capacity = PENDING;

    if (kb_session_init(&c->session, &cfg->session, &c->bufs) != KB_OK) {
        note(c, "kb_session_init"); kb_client_close(c); return KB_ERR_INVALID;
    }

    memset(&tc, 0, sizeof tc);
    tc.host = cfg->host;
    tc.port = cfg->port;
    tc.connect_timeout_ms = cfg->connect_timeout_ms ? cfg->connect_timeout_ms : 15000u;
    tc.proxy = cfg->proxy;
    if (kb_transport_connect(&c->transport, &tc) != KB_OK) {
        note(c, kb_transport_detail(&c->transport));
        kb_client_close(c);
        return KB_ERR_STATE;
    }
    if (cfg->setup_tls != NULL) {
        int want_write = 0;
        if (cfg->setup_tls(&c->transport, cfg->tls_user) != 0) {
            note(c, kb_transport_detail(&c->transport));
            kb_client_close(c); return KB_ERR_STATE;
        }
        for (;;) {
            kb_status st = kb_transport_handshake(&c->transport, &want_write);
            if (st == KB_OK) {
                break;
            }
            if (st != KB_IO_WANT_READ && st != KB_IO_WANT_WRITE) {
                note(c, kb_transport_detail(&c->transport));
                kb_client_close(c); return KB_ERR_STATE;
            }
            if (kb_transport_wait(&c->transport, st == KB_IO_WANT_WRITE, 15000) != 1) {
                note(c, "TLS handshake timeout"); kb_client_close(c); return KB_ERR_TIMEOUT;
            }
        }
    }

    /* handshake протокола (sessionInit) */
    kb_session_on_connected(&c->session, kb_now_ms());
    deadline = kb_now_ms() + 20000;
    while (kb_now_ms() < deadline) {
        kb_event ev;
        if (pump_once(c, 200) != 0) {
            note(c, "соединение оборвалось до handshake");
            kb_client_close(c); return KB_ERR_CLOSED;
        }
        while (kb_session_poll_event(&c->session, &ev) == KB_OK &&
               ev.type != KB_EVENT_NONE) {
            if (ev.type == KB_EVENT_HANDSHAKE) {
                c->has_calls_seed = ev.has_calls_seed;
                c->calls_seed = ev.calls_seed;
                c->online = 1;
                return KB_OK;
            }
            if (ev.type == KB_EVENT_ERROR) {
                note(c, "сервер отверг handshake");
                kb_client_close(c); return KB_ERR_STATE;
            }
        }
    }
    note(c, "handshake не пришёл");
    kb_client_close(c);
    return KB_ERR_TIMEOUT;
}

int kb_client_has_calls_seed(const kb_client *c)
{
    return c != NULL && c->has_calls_seed;
}
int64_t kb_client_calls_seed(const kb_client *c)
{
    return c != NULL ? c->calls_seed : 0;
}

kb_status kb_client_call(kb_client *c, uint16_t opcode, const uint8_t *payload,
                         size_t len, const uint8_t **resp, size_t *resp_len)
{
    uint16_t seq = 0;
    uint64_t deadline;
    kb_status st;

    if (c == NULL || !c->online) {
        return KB_ERR_STATE;
    }
    st = kb_session_request(&c->session, opcode, payload, len, &seq);
    if (st != KB_OK) {
        note(c, "kb_session_request");
        return st;
    }
    deadline = kb_now_ms() + c->io_timeout_ms;
    while (kb_now_ms() < deadline) {
        kb_event ev;
        if (pump_once(c, 200) != 0) {
            note(c, "соединение оборвалось");
            return KB_ERR_CLOSED;
        }
        while (kb_session_poll_event(&c->session, &ev) == KB_OK &&
               ev.type != KB_EVENT_NONE) {
            if (ev.seq == seq && ev.type == KB_EVENT_RESPONSE) {
                if (resp != NULL) {
                    *resp = ev.payload;
                }
                if (resp_len != NULL) {
                    *resp_len = ev.payload_len;
                }
                return KB_OK;
            }
            if (ev.seq == seq && ev.type == KB_EVENT_REQUEST_FAILED) {
                note(c, "запрос отклонён");
                return ev.status != KB_OK ? ev.status : KB_ERR_STATE;
            }
            if (ev.type == KB_EVENT_ERROR) {
                note(c, "ошибка сессии");
                return ev.status != KB_OK ? ev.status : KB_ERR_STATE;
            }
        }
    }
    note(c, "ответ не пришёл (таймаут)");
    return KB_ERR_TIMEOUT;
}

const char *kb_client_detail(const kb_client *c)
{
    if (c == NULL || c->detail[0] == '\0') {
        return "ok";
    }
    return c->detail;
}

void kb_client_close(kb_client *c)
{
    if (c == NULL) {
        return;
    }
    if (c->online || c->transport.open) {
        kb_transport_close(&c->transport);
    }
    free(c->rx); free(c->tx); free(c->scratch); free(c->io); free(c->pending);
    c->rx = c->tx = c->scratch = c->io = NULL;
    c->pending = NULL;
    c->online = 0;
}
