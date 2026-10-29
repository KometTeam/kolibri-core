/*
 * Полный клиент на C: транспорт, прокси, TLS и машина сессии вместе.
 *
 *   client <host> <port> [--proxy URL] [--tls] [--insecure] [--ca FILE]
 *                       [--handshake-only]
 *
 * Делает handshake, отправляет один запрос, ждёт ответ, слушает пуши и
 * завершается. С --handshake-only останавливается сразу после handshake и
 * печатает, что ответил сервер, — этого хватает, чтобы проверить живой сервис,
 * ничего ему не отправляя. Он же служит интеграционным тестом: tests/integration/run.py
 * поднимает сервер на localhost и сверяет вывод.
 *
 * Печатает по строке на событие, чтобы вывод было удобно проверять:
 *   state=<...>  handshake callsSeed=<...>  response seq=.. opcode=.. len=..
 *   push opcode=.. len=..  failed seq=.. status=..  error status=..
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kolibri.h"
#include "kolibri_transport.h"

#if defined(KB_WITH_OPENSSL)
#include "kolibri_tls_openssl.h"
#endif
#if defined(KB_WITH_MBEDTLS)
#include "kolibri_tls_mbedtls.h"
#include <stdio.h>
#endif

#define RX_CAP (1024u * 1024u)
#define TX_CAP (256u * 1024u)
#define SCRATCH_CAP (2048u * 1024u)
#define MAX_PENDING 32

static unsigned char rx_buf[RX_CAP];
static unsigned char tx_buf[TX_CAP];
static unsigned char scratch_buf[SCRATCH_CAP];
static kb_pending pending_slots[MAX_PENDING];
static unsigned char io_buf[65536];

/* Коротко показать, что лежит в ответе на sessionInit. */
static void dump_handshake(const unsigned char *payload, size_t len)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    kb_mp_value value;
    size_t i;

    if (kb_mp_reader_init(&reader, payload, len) != KB_OK) {
        return;
    }
    if (kb_mp_read(&reader, &head) != KB_OK || head.type != KB_MP_MAP) {
        printf("  тело не map (%lu Б)\n", (unsigned long)len);
        return;
    }

    for (i = 0; i < head.len; i++) {
        if (kb_mp_read(&reader, &key) != KB_OK || key.type != KB_MP_STR) {
            return;
        }
        if (kb_mp_read(&reader, &value) != KB_OK) {
            return;
        }
        printf("  %.*s = ", (int)key.len, (const char *)key.data);
        switch (value.type) {
        case KB_MP_INT:
            printf("%ld\n", (long)value.i);
            break;
        case KB_MP_UINT:
            printf("%lu\n", (unsigned long)value.u);
            break;
        case KB_MP_STR:
            printf("\"%.*s\"\n", (int)value.len, (const char *)value.data);
            break;
        case KB_MP_BOOL:
            printf("%s\n", value.boolean ? "true" : "false");
            break;
        case KB_MP_ARRAY:
            printf("[%lu элементов]\n", (unsigned long)value.len);
            break;
        case KB_MP_MAP:
            printf("{%lu полей}\n", (unsigned long)value.len);
            break;
        case KB_MP_NIL:
            printf("nil\n");
            break;
        default:
            printf("<%d>\n", (int)value.type);
            break;
        }
        /* Вложенное значение прочитано только заголовком — пропускаем тело. */
        if (value.type == KB_MP_ARRAY || value.type == KB_MP_MAP) {
            size_t pending = value.type == KB_MP_MAP ? value.len * 2 : value.len;
            while (pending > 0) {
                if (kb_mp_skip(&reader) != KB_OK) {
                    return;
                }
                pending--;
            }
        }
    }
}

static const char *state_name(kb_session_state state)
{
    switch (state) {
    case KB_STATE_DISCONNECTED: return "disconnected";
    case KB_STATE_CONNECTING:   return "connecting";
    case KB_STATE_CONNECTED:    return "connected";
    case KB_STATE_ONLINE:       return "online";
    default: break;
    }
    return "?";
}

static void fill_config(kb_session_config *config, const char *device_id)
{
    memset(config, 0, sizeof *config);
    config->handshake.instance_id = device_id;
    config->handshake.device_id = device_id;
    config->handshake.client_session_id = (int64_t)kb_now_ms();
    config->handshake.user_agent.device_type = "ANDROID";
    config->handshake.user_agent.app_version = "26.25.0";
    config->handshake.user_agent.os_version = "Android 14";
    config->handshake.user_agent.timezone = "Europe/Moscow";
    config->handshake.user_agent.screen = "480dpi 480dpi 1344x2992";
    config->handshake.user_agent.push_device_type = "GCM";
    config->handshake.user_agent.arch = "arm64-v8a";
    config->handshake.user_agent.locale = "ru";
    config->handshake.user_agent.build_number = 6790;
    config->handshake.user_agent.device_name = "Google Pixel 8 Pro";
    config->handshake.user_agent.device_locale = "ru";
    config->ping_interval_ms = 30000;
    config->ping_interactive = 1;
    config->request_timeout_ms = 5000;
    config->auto_reconnect = 0;
}

static void fill_buffers(kb_session_buffers *bufs)
{
    bufs->rx = rx_buf;
    bufs->rx_capacity = sizeof rx_buf;
    bufs->tx = tx_buf;
    bufs->tx_capacity = sizeof tx_buf;
    bufs->scratch = scratch_buf;
    bufs->scratch_capacity = sizeof scratch_buf;
    bufs->pending = pending_slots;
    bufs->pending_capacity = MAX_PENDING;
}

/* Отдать сокету всё, что накопила сессия. */
static int flush_out(kb_session *session, kb_transport *transport)
{
    for (;;) {
        size_t len = 0;
        const unsigned char *chunk = kb_session_pull_out(session, &len);
        long written;

        if (chunk == NULL) {
            return 0;
        }
        written = kb_transport_write(transport, chunk, len);
        /* Под TLS запись умеет запросить чтение — например при пересогласовании.
           Ждём ту сторону, которую попросили, а не ту, которую ожидали. */
        if (written == KB_IO_WANT_WRITE || written == KB_IO_WANT_READ) {
            if (kb_transport_wait(transport, written == KB_IO_WANT_WRITE, 5000)
                != 1) {
                return -1;
            }
            continue;
        }
        if (written < 0) {
            return -1;
        }
        kb_session_consume_out(session, (size_t)written);
    }
}

int main(int argc, char **argv)
{
    kb_session session;
    kb_session_config config;
    kb_session_buffers bufs;
    kb_transport transport;
    kb_transport_config tconfig;
    kb_proxy_config proxy;
    char proxy_storage[KB_PROXY_URL_MAX];
    const char *host;
    const char *proxy_url = NULL;
    const char *ca_file = NULL;
    int want_tls = 0;
    int insecure = 0;
    int use_proxy = 0;
    int handshake_only = 0;
    int handshake_seen = 0;
    unsigned short port;
    int i;
    int online = 0;
    int got_response = 0;
    unsigned short request_seq = 0;
    uint64_t deadline;

    if (argc < 3) {
        fprintf(stderr, "использование: client <host> <port> "
                        "[--proxy URL] [--tls] [--insecure] [--ca FILE]\n");
        return 2;
    }
    host = argv[1];
    port = (unsigned short)atoi(argv[2]);

    for (i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--proxy") == 0 && i + 1 < argc) {
            proxy_url = argv[++i];
        } else if (strcmp(argv[i], "--tls") == 0) {
            want_tls = 1;
        } else if (strcmp(argv[i], "--insecure") == 0) {
            insecure = 1;
        } else if (strcmp(argv[i], "--ca") == 0 && i + 1 < argc) {
            ca_file = argv[++i];
        } else if (strcmp(argv[i], "--handshake-only") == 0) {
            handshake_only = 1;
        }
    }

    if (kb_net_startup() != KB_OK) {
        fprintf(stderr, "сеть не поднялась\n");
        return 1;
    }

    fill_config(&config, "kolibri-core-client");
    fill_buffers(&bufs);
    if (kb_session_init(&session, &config, &bufs) != KB_OK) {
        fprintf(stderr, "сессия не создалась\n");
        return 1;
    }

    memset(&tconfig, 0, sizeof tconfig);
    tconfig.host = host;
    tconfig.port = port;
    tconfig.connect_timeout_ms = 10000;

    if (proxy_url != NULL) {
        if (kb_proxy_parse(proxy_url, &proxy, proxy_storage,
                           sizeof proxy_storage) != KB_OK) {
            fprintf(stderr, "не разобрал адрес прокси: %s\n", proxy_url);
            return 1;
        }
        tconfig.proxy = &proxy;
        use_proxy = 1;
    }

    kb_session_on_connecting(&session, kb_now_ms());

    if (kb_transport_connect(&transport, &tconfig) != KB_OK) {
        fprintf(stderr, "не подключился: %s\n", kb_transport_detail(&transport));
        return 1;
    }
    printf("connected%s\n", use_proxy ? " via=proxy" : "");

    if (want_tls) {
#if defined(KB_WITH_MBEDTLS)
        /* Корни читаем в память: на старых платформах системного хранилища
           нет, и PEM обычно просто вшит в бинарь. */
        static double tls_state[2048];
        static char ca_buf[32768];
        size_t ca_len = 0;
        kb_tls tls;
        kb_status st;

        if (ca_file != NULL) {
            FILE *pem = fopen(ca_file, "rb");
            if (pem == NULL) {
                fprintf(stderr, "не открылся файл корней: %s\n", ca_file);
                return 1;
            }
            ca_len = fread(ca_buf, 1, sizeof ca_buf - 1, pem);
            fclose(pem);
            ca_buf[ca_len] = '\0';
            ca_len++; /* mbedTLS требует завершающий ноль в длине */
        }

        if (kb_tls_mbedtls_init(&tls, tls_state, sizeof tls_state, transport.fd,
                                host, ca_len ? ca_buf : NULL, ca_len,
                                insecure) != KB_OK) {
            fprintf(stderr, "TLS не настроился\n");
            return 1;
        }
        if (kb_transport_set_tls(&transport, &tls) != KB_OK) {
            fprintf(stderr, "TLS не подключился: %s\n",
                    kb_transport_detail(&transport));
            return 1;
        }
        for (;;) {
            int want_write = 0;
            st = kb_transport_handshake(&transport, &want_write);
            if (st == KB_OK) {
                break;
            }
            if (st != KB_ERR_TRUNCATED) {
                fprintf(stderr, "TLS: %s\n", kb_transport_detail(&transport));
                return 1;
            }
            if (kb_transport_wait(&transport, want_write, 10000) != 1) {
                fprintf(stderr, "TLS: сокет молчит\n");
                return 1;
            }
        }
        printf("tls=ok\n");
#elif defined(KB_WITH_OPENSSL)
        static double tls_state[512]; /* выравнивание под указатели */
        kb_tls tls;
        kb_status st;

        if (kb_tls_openssl_init(&tls, tls_state, sizeof tls_state, transport.fd,
                                host, ca_file, insecure) != KB_OK) {
            fprintf(stderr, "TLS не настроился\n");
            return 1;
        }
        if (kb_transport_set_tls(&transport, &tls) != KB_OK) {
            fprintf(stderr, "TLS не подключился: %s\n",
                    kb_transport_detail(&transport));
            return 1;
        }

        for (;;) {
            int want_write = 0;
            st = kb_transport_handshake(&transport, &want_write);
            if (st == KB_OK) {
                break;
            }
            if (st != KB_ERR_TRUNCATED) {
                fprintf(stderr, "TLS: %s\n", kb_transport_detail(&transport));
                return 1;
            }
            /* Ждать надо ровно ту сторону, которую попросил стек. */
            if (kb_transport_wait(&transport, want_write, 10000) != 1) {
                fprintf(stderr, "TLS: сокет молчит\n");
                return 1;
            }
        }
        printf("tls=ok\n");
#else
        (void)ca_file;
        (void)insecure;
        fprintf(stderr, "собрано без TLS: пересобери с KB_WITH_OPENSSL\n");
        return 1;
#endif
    }

    kb_session_on_connected(&session, kb_now_ms());
    deadline = kb_now_ms() + 15000;

    while (kb_now_ms() < deadline) {
        kb_event event;
        long got;
        uint64_t next;
        uint32_t wait_ms;

        kb_session_tick(&session, kb_now_ms());
        if (flush_out(&session, &transport) != 0) {
            fprintf(stderr, "запись не удалась\n");
            return 1;
        }

        next = kb_session_next_deadline(&session);
        wait_ms = 200;
        if (next != 0 && next > kb_now_ms()) {
            uint64_t left = next - kb_now_ms();
            wait_ms = left > 200 ? 200 : (uint32_t)left;
        }

        if (kb_transport_wait(&transport, 0, wait_ms) == 1) {
            for (;;) {
                got = kb_transport_read(&transport, io_buf, sizeof io_buf);
                /*
                 * Чтение под TLS умеет запросить запись — например когда
                 * недописан флайт рукопожатия. Дождаться готовности мало:
                 * надо ПОВТОРИТЬ чтение здесь же. Уйти на верх цикла нельзя,
                 * там ждут читаемости, а она в этом случае не наступит —
                 * пир молчит, пока не получит остаток.
                 */
                if (got != KB_IO_WANT_WRITE) {
                    break;
                }
                if (kb_transport_wait(&transport, 1, 5000) != 1) {
                    fprintf(stderr, "сокет не готов к записи\n");
                    return 1;
                }
            }
            if (got == KB_IO_CLOSED) {
                printf("closed\n");
                break;
            }
            if (got > 0) {
                kb_session_feed(&session, io_buf, (size_t)got);
            } else if (got != KB_IO_WANT_READ) {
                fprintf(stderr, "чтение не удалось\n");
                return 1;
            }
        }

        while (kb_session_poll_event(&session, &event) == KB_OK &&
               event.type != KB_EVENT_NONE) {
            switch (event.type) {
            case KB_EVENT_STATE:
                printf("state=%s\n", state_name(event.state));
                if (event.state == KB_STATE_ONLINE) {
                    online = 1;
                }
                break;
            case KB_EVENT_HANDSHAKE:
                if (event.has_calls_seed) {
                    printf("handshake callsSeed=%ld\n", (long)event.calls_seed);
                } else {
                    printf("handshake callsSeed=none\n");
                }
                handshake_seen = 1;
                if (handshake_only) {
                    printf("ответ сервера (%lu Б):\n",
                           (unsigned long)event.payload_len);
                    dump_handshake(event.payload, event.payload_len);
                }
                break;
            case KB_EVENT_RESPONSE:
                printf("response seq=%u opcode=%u len=%lu\n",
                       (unsigned)event.seq, (unsigned)event.opcode,
                       (unsigned long)event.payload_len);
                got_response = 1;
                break;
            case KB_EVENT_PUSH:
                printf("push opcode=%u len=%lu\n", (unsigned)event.opcode,
                       (unsigned long)event.payload_len);
                break;
            case KB_EVENT_REQUEST_FAILED:
                printf("failed seq=%u status=%d\n", (unsigned)event.seq,
                       (int)event.status);
                break;
            case KB_EVENT_ERROR:
                printf("error status=%d\n", (int)event.status);
                kb_transport_close(&transport);
                return 1;
            default:
                break;
            }
        }

        if (handshake_only && handshake_seen && online) {
            break;
        }

        if (!handshake_only && online && request_seq == 0) {
            /* msgpack: {"chatId": 42} */
            static const unsigned char body[] = {0x81, 0xA6, 'c', 'h', 'a', 't',
                                                 'I',  'd',  0x2A};
            if (kb_session_request(&session, 64, body, sizeof body,
                                   &request_seq) != KB_OK) {
                fprintf(stderr, "запрос не ушёл\n");
                return 1;
            }
            printf("request seq=%u\n", (unsigned)request_seq);
        }

        if (got_response) {
            break;
        }
    }

    kb_transport_close(&transport);
    kb_net_cleanup();

    if (handshake_only) {
        if (!handshake_seen) {
            fprintf(stderr, "handshake не прошёл\n");
            return 1;
        }
        printf("done\n");
        return 0;
    }
    if (!got_response) {
        fprintf(stderr, "ответа не дождался\n");
        return 1;
    }
    printf("done\n");
    return 0;
}
