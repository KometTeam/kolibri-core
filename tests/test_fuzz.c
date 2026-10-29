/*
 * Устойчивость к битому входу.
 *
 * Ядро разбирает недоверенные байты из сети, поэтому мало проверить, что
 * корректный поток разбирается правильно: испорченный не должен ни писать за
 * пределы буфера, ни зацикливаться, ни возвращать успех с мусором.
 *
 * Вокруг выходного буфера кладутся канарейки, так что переполнение видно даже
 * без санитайзеров; под ASan/UBSan (`make asan`) этот же проход ловит и более
 * тонкие нарушения.
 */

#include <string.h>

#include "kb_test.h"
#include "kolibri.h"
#include "kolibri_calls.h"
#include "kolibri_json.h"
#include "kolibri_mpjson.h"
#include "kolibri_msgpack.h"

#define BODY 4096u
#define GUARD 64u
#define CANARY 0xA5

static unsigned char plain[BODY];
static unsigned char victim[BODY];
static unsigned char guarded[GUARD + BODY + GUARD];

static int status_known(kb_status st)
{
    switch (st) {
    case KB_OK:
    case KB_ERR_TRUNCATED:
    case KB_ERR_TOO_LARGE:
    case KB_ERR_INVALID:
    case KB_ERR_LZ4_ZERO_OFFSET:
    case KB_ERR_LZ4_OFFSET_RANGE:
    case KB_ERR_UNSUPPORTED:
    case KB_ERR_OVERFLOW:
    case KB_ERR_NOT_FOUND:
    case KB_ERR_LIMIT:
    case KB_ERR_TIMEOUT:
    case KB_ERR_CLOSED:
    case KB_ERR_STATE:
        return 1;
    default:
        break;
    }
    return 0;
}

static void arm_guards(void)
{
    memset(guarded, CANARY, sizeof guarded);
}

static int guards_intact(void)
{
    size_t i;
    for (i = 0; i < GUARD; i++) {
        if (guarded[i] != CANARY) {
            return 0;
        }
        if (guarded[GUARD + BODY + i] != CANARY) {
            return 0;
        }
    }
    return 1;
}

static unsigned char *sandbox(void)
{
    return guarded + GUARD;
}

/* ─── Сессия ───────────────────────────────────────────────────────────── */

static unsigned char fz_rx[16384];
static unsigned char fz_tx[16384];
static unsigned char fz_scratch[16384];
static kb_pending fz_pending[8];

static void fuzz_session(kb_rng *rng)
{
    kb_session s;
    kb_session_config config;
    kb_session_buffers bufs;
    uint64_t now = 1000;
    size_t round;

    memset(&config, 0, sizeof config);
    config.handshake.instance_id = "fuzz";
    config.handshake.device_id = "fuzz";
    config.handshake.client_session_id = 1;
    config.handshake.user_agent.device_type = "ANDROID";
    config.handshake.user_agent.app_version = "1.0";
    config.handshake.user_agent.os_version = "Android 14";
    config.handshake.user_agent.timezone = "UTC";
    config.handshake.user_agent.screen = "1x1";
    config.handshake.user_agent.push_device_type = "GCM";
    config.handshake.user_agent.arch = "arm64-v8a";
    config.handshake.user_agent.locale = "ru";
    config.handshake.user_agent.build_number = 1;
    config.handshake.user_agent.device_name = "fuzz";
    config.handshake.user_agent.device_locale = "ru";
    config.ping_interval_ms = 1000;
    config.ping_interactive = 1;
    config.request_timeout_ms = 2000;
    config.auto_reconnect = 1;

    bufs.rx = fz_rx;
    bufs.rx_capacity = sizeof fz_rx;
    bufs.tx = fz_tx;
    bufs.tx_capacity = sizeof fz_tx;
    bufs.scratch = fz_scratch;
    bufs.scratch_capacity = sizeof fz_scratch;
    bufs.pending = fz_pending;
    bufs.pending_capacity = sizeof fz_pending / sizeof fz_pending[0];

    if (!CHECK_EQ(kb_session_init(&s, &config, &bufs), KB_OK)) {
        return;
    }
    kb_session_on_connecting(&s, now);
    kb_session_on_connected(&s, now);

    for (round = 0; round < 20000; round++) {
        unsigned long action = kb_rng_below(rng, 10);
        kb_event event;
        kb_status st;

        now += kb_rng_below(rng, 900);

        switch (action) {
        case 0:
        case 1:
        case 2: {
            /* Случайные байты «из сокета». */
            size_t len = (size_t)kb_rng_below(rng, 200) + 1;
            size_t i;
            for (i = 0; i < len; i++) {
                plain[i] = kb_rng_byte(rng);
            }
            st = kb_session_feed(&s, plain, len);
            if (!CHECK(status_known(st))) {
                return;
            }
            break;
        }
        case 3: {
            /* Правдоподобный пакет со случайными полями. */
            size_t body_len = (size_t)kb_rng_below(rng, 120);
            size_t written = 0;
            kb_gen_payload((int)(round % KB_SHAPE_COUNT), body_len, plain, rng);
            if (kb_encode((uint8_t)kb_rng_below(rng, 4),
                          (uint16_t)kb_rng_below(rng, 300),
                          (uint16_t)kb_rng_below(rng, 8), plain, body_len,
                          victim, sizeof victim, &written) == KB_OK) {
                st = kb_session_feed(&s, victim, written);
                if (!CHECK(status_known(st))) {
                    return;
                }
            }
            break;
        }
        case 4:
            if (!CHECK(status_known(kb_session_tick(&s, now)))) {
                return;
            }
            break;
        case 5: {
            uint16_t seq = 0;
            st = kb_session_request(&s, (uint16_t)kb_rng_below(rng, 300), NULL, 0,
                                    &seq);
            if (!CHECK(status_known(st))) {
                return;
            }
            break;
        }
        case 6: {
            size_t len = 0;
            const uint8_t *out = kb_session_pull_out(&s, &len);
            if (out != NULL) {
                /* Иногда подтверждаем не всё — сокет вправе принять часть. */
                size_t taken = (size_t)kb_rng_below(rng, (unsigned long)len) + 1;
                if (!CHECK_EQ(kb_session_consume_out(&s, taken), KB_OK)) {
                    return;
                }
            }
            break;
        }
        case 7:
            if (!CHECK(status_known(kb_session_on_disconnected(&s, now)))) {
                return;
            }
            break;
        case 8:
            kb_session_on_connecting(&s, now);
            if (!CHECK(status_known(kb_session_on_connected(&s, now)))) {
                return;
            }
            break;
        default:
            break;
        }

        /* События должны разбираться до конца и не выдумывать статусов. */
        while (kb_session_poll_event(&s, &event) == KB_OK &&
               event.type != KB_EVENT_NONE) {
            if (event.type == KB_EVENT_ERROR ||
                event.type == KB_EVENT_REQUEST_FAILED) {
                if (!CHECK(status_known(event.status))) {
                    return;
                }
            }
            if (event.payload != NULL) {
                if (!CHECK(event.payload >= fz_scratch &&
                           event.payload + event.payload_len <=
                               fz_scratch + sizeof fz_scratch)) {
                    return;
                }
            }
        }
        (void)kb_session_next_deadline(&s);
    }
}

void kb_tests_fuzz(void)
{
    kb_rng rng;
    size_t round;

    kb_rng_seed(&rng, 0xF0F0F0ul);

    KB_TEST("порча сжатых блоков");
    for (round = 0; round < 4000; round++) {
        size_t len = (size_t)kb_rng_below(&rng, 512) + 1;
        size_t comp_len = 0;
        size_t out_len = 0;
        size_t mutations;
        size_t m;
        kb_status st;

        kb_gen_payload((int)(round % KB_SHAPE_COUNT), len, plain, &rng);
        if (kb_lz4_compress(plain, len, victim, sizeof victim, &comp_len) != KB_OK) {
            continue;
        }

        /* От одной до восьми случайных правок, иногда с обрезкой хвоста. */
        mutations = (size_t)kb_rng_below(&rng, 8) + 1;
        for (m = 0; m < mutations && comp_len > 0; m++) {
            size_t at = (size_t)kb_rng_below(&rng, (unsigned long)comp_len);
            victim[at] = kb_rng_byte(&rng);
        }
        if (kb_rng_below(&rng, 3) == 0 && comp_len > 1) {
            comp_len -= (size_t)kb_rng_below(&rng, (unsigned long)comp_len);
        }

        arm_guards();
        st = kb_lz4_decompress(victim, comp_len, sandbox(), BODY, &out_len);
        if (!CHECK(status_known(st))) {
            continue;
        }
        if (!CHECK(guards_intact())) {
            return;
        }
        if (st == KB_OK) {
            CHECK(out_len <= BODY);
        }
    }

    KB_TEST("совершенно случайные блоки");
    for (round = 0; round < 4000; round++) {
        size_t len = (size_t)kb_rng_below(&rng, 256) + 1;
        size_t out_len = 0;
        size_t i;
        kb_status st;

        for (i = 0; i < len; i++) {
            victim[i] = kb_rng_byte(&rng);
        }

        arm_guards();
        st = kb_lz4_decompress(victim, len, sandbox(), BODY, &out_len);
        if (!CHECK(status_known(st))) {
            continue;
        }
        if (!CHECK(guards_intact())) {
            return;
        }
    }

    KB_TEST("порча пакетов");
    for (round = 0; round < 4000; round++) {
        size_t len = (size_t)kb_rng_below(&rng, 400);
        size_t packet_len = 0;
        size_t out_len = 0;
        kb_header header;
        size_t mutations;
        size_t m;
        kb_status st;

        kb_gen_payload((int)(round % KB_SHAPE_COUNT), len, plain, &rng);
        if (kb_encode(KB_CMD_REQUEST, 64, 1, plain, len, victim, sizeof victim,
                      &packet_len) != KB_OK) {
            continue;
        }

        mutations = (size_t)kb_rng_below(&rng, 6) + 1;
        for (m = 0; m < mutations; m++) {
            size_t at = (size_t)kb_rng_below(&rng, (unsigned long)packet_len);
            victim[at] = kb_rng_byte(&rng);
        }
        if (kb_rng_below(&rng, 4) == 0 && packet_len > 1) {
            packet_len -= (size_t)kb_rng_below(&rng, (unsigned long)packet_len);
        }

        arm_guards();
        st = kb_decode(victim, packet_len, &header, sandbox(), BODY, &out_len);
        if (!CHECK(status_known(st))) {
            continue;
        }
        if (!CHECK(guards_intact())) {
            return;
        }
    }

    KB_TEST("порча msgpack-документов");
    for (round = 0; round < 4000; round++) {
        kb_mp_writer w;
        kb_mp_reader r;
        kb_mp_value value;
        size_t doc_len;
        size_t mutations;
        size_t m;
        kb_status st;

        /* Небольшой документ со вложенностью, потом портим. */
        kb_mp_writer_init(&w, victim, sizeof victim);
        kb_mp_write_map(&w, 3);
        kb_mp_write_cstr(&w, "chatId");
        kb_mp_write_i64(&w, (int64_t)kb_rng_below(&rng, 1000000) - 500000);
        kb_mp_write_cstr(&w, "elements");
        kb_mp_write_array(&w, 2);
        kb_mp_write_map(&w, 1);
        kb_mp_write_cstr(&w, "type");
        kb_mp_write_cstr(&w, "STRONG");
        kb_mp_write_nil(&w);
        kb_mp_write_cstr(&w, "mode");
        kb_gen_payload((int)(round % KB_SHAPE_COUNT), 64, plain, &rng);
        kb_mp_write_bin(&w, plain, 64);
        if (kb_mp_writer_status(&w) != KB_OK) {
            continue;
        }
        doc_len = kb_mp_writer_len(&w);

        mutations = (size_t)kb_rng_below(&rng, 6) + 1;
        for (m = 0; m < mutations; m++) {
            size_t at = (size_t)kb_rng_below(&rng, (unsigned long)doc_len);
            victim[at] = kb_rng_byte(&rng);
        }
        if (kb_rng_below(&rng, 3) == 0 && doc_len > 1) {
            doc_len -= (size_t)kb_rng_below(&rng, (unsigned long)doc_len);
        }

        /* Ни чтение, ни пропуск, ни поиск не должны падать или зависать. */
        kb_mp_reader_init(&r, victim, doc_len);
        while ((st = kb_mp_read(&r, &value)) == KB_OK) {
            if (!CHECK(status_known(st))) {
                return;
            }
            /* Строки и бинарники обязаны лежать внутри буфера. */
            if (value.type == KB_MP_STR || value.type == KB_MP_BIN) {
                if (!CHECK(value.data >= victim &&
                           value.data + value.len <= victim + doc_len)) {
                    return;
                }
            }
        }
        if (!CHECK(status_known(st))) {
            return;
        }

        kb_mp_reader_init(&r, victim, doc_len);
        if (!CHECK(status_known(kb_mp_skip(&r)))) {
            return;
        }
        if (!CHECK(status_known(kb_mp_map_get(victim, doc_len, "mode", &value)))) {
            return;
        }
        (void)kb_mp_map_has(victim, doc_len, "chatId");
    }

    KB_TEST("совершенно случайный msgpack");
    for (round = 0; round < 4000; round++) {
        kb_mp_reader r;
        kb_mp_value value;
        size_t len = (size_t)kb_rng_below(&rng, 128) + 1;
        size_t i;
        kb_status st;

        for (i = 0; i < len; i++) {
            victim[i] = kb_rng_byte(&rng);
        }

        kb_mp_reader_init(&r, victim, len);
        st = kb_mp_skip(&r);
        if (!CHECK(status_known(st))) {
            return;
        }
        kb_mp_reader_init(&r, victim, len);
        st = kb_mp_read(&r, &value);
        if (!CHECK(status_known(st))) {
            return;
        }
        if (!CHECK(status_known(kb_mp_map_get(victim, len, "x", &value)))) {
            return;
        }
    }

    KB_TEST("случайная жизнь сессии");
    fuzz_session(&rng);

    KB_TEST("мусор в реассемблере");
    {
        static unsigned char buffer[8192];
        kb_rx rx;
        unsigned char chunk[97];

        CHECK_EQ(kb_rx_init(&rx, buffer, sizeof buffer), KB_OK);
        for (round = 0; round < 4000; round++) {
            size_t n = (size_t)kb_rng_below(&rng, sizeof chunk) + 1;
            size_t i;
            size_t got_len = 0;
            kb_status st;

            for (i = 0; i < n; i++) {
                chunk[i] = kb_rng_byte(&rng);
            }
            st = kb_rx_push(&rx, chunk, n);
            if (!CHECK(st == KB_OK || st == KB_ERR_OVERFLOW)) {
                return;
            }
            while (kb_rx_next(&rx, &got_len) != NULL) {
                if (!CHECK(got_len >= KB_HEADER_SIZE)) {
                    return;
                }
            }
        }
    }
    KB_TEST("случайный JSON");
    for (round = 0; round < 8000; round++) {
        size_t len = (size_t)kb_rng_below(&rng, 200) + 1;
        size_t i;
        kb_json_value root;

        for (i = 0; i < len; i++) {
            /* Смесь структурных символов и мусора — так чаще получаются
               почти-валидные документы, а не сплошной шум. */
            if (kb_rng_below(&rng, 2) == 0) {
                static const char sym[] = "{}[]\":,0123456789.-+eEtfn \\\"";
                victim[i] = (unsigned char)sym[kb_rng_below(&rng, sizeof sym - 1)];
            } else {
                victim[i] = kb_rng_byte(&rng);
            }
        }
        if (kb_json_parse((const char *)victim, len, &root) != KB_OK) {
            continue;
        }
        /* Разобралось — хождение по узлу не должно падать. */
        if (root.type == KB_JSON_OBJECT) {
            kb_json_value v;
            (void)kb_json_object_get(&root, "type", &v);
            (void)kb_json_object_get(&root, "notification", &v);
        } else if (root.type == KB_JSON_ARRAY) {
            kb_json_value it, elem;
            int guard = 0;
            kb_json_array_iter_init(&it, &root);
            while (kb_json_array_next(&it, &elem) && guard++ < 1000) {
                int64_t n;
                (void)kb_json_as_i64(&elem, &n);
            }
        } else if (root.type == KB_JSON_STRING) {
            size_t out_len = 0;
            arm_guards();
            if (!CHECK(status_known(kb_json_str_copy(
                    &root, (char *)(guarded + GUARD), BODY, &out_len)))) {
                return;
            }
            if (!CHECK(guards_intact())) {
                return;
            }
        } else if (root.type == KB_JSON_NUMBER) {
            int64_t n;
            (void)kb_json_as_i64(&root, &n);
        }
    }

    KB_TEST("мусорный vcp и события ws2");
    for (round = 0; round < 8000; round++) {
        size_t len = (size_t)kb_rng_below(&rng, 160) + 1;
        size_t i;
        kb_vcp vcp;
        kb_call_event ev;
        int64_t seq;

        for (i = 0; i < len; i++) {
            if (kb_rng_below(&rng, 3) == 0) {
                static const char sym[] = "0123456789:{}[]\",\"AB+/=";
                victim[i] = (unsigned char)sym[kb_rng_below(&rng, sizeof sym - 1)];
            } else {
                victim[i] = kb_rng_byte(&rng);
            }
        }
        arm_guards();
        if (!CHECK(status_known(kb_vcp_decode((const char *)victim, len,
                                              guarded + GUARD, BODY, &vcp)))) {
            return;
        }
        if (!CHECK(guards_intact())) {
            return;
        }
        /* Классификация и разбор события на том же мусоре. */
        (void)kb_ws2_classify((const char *)victim, len, &seq);
        (void)kb_call_event_parse((const char *)victim, len, &ev);
    }

    KB_TEST("round-trip msgpack через JSON");
    for (round = 0; round < 6000; round++) {
        kb_mp_writer mw;
        char jbuf[BODY];
        static unsigned char rebuilt[BODY];
        static unsigned char sc[BODY];
        size_t doc_len, jlen = 0, back = 0;

        /* Документ из значений, чьи кодировки стабильны через JSON. */
        kb_mp_writer_init(&mw, victim, sizeof victim);
        kb_mp_write_map(&mw, 3);
        kb_mp_write_cstr(&mw, "s");
        kb_gen_payload((int)(round % KB_SHAPE_COUNT), 32, plain, &rng);
        kb_mp_write_bin(&mw, plain, (size_t)kb_rng_below(&rng, 33));
        kb_mp_write_cstr(&mw, "n");
        kb_mp_write_i64(&mw, (int64_t)kb_rng_below(&rng, 100000) - 50000);
        kb_mp_write_cstr(&mw, "a");
        kb_mp_write_array(&mw, 2);
        kb_mp_write_bool(&mw, (int)kb_rng_below(&rng, 2));
        kb_mp_write_nil(&mw);
        if (kb_mp_writer_status(&mw) != KB_OK) {
            continue;
        }
        doc_len = kb_mp_writer_len(&mw);

        if (!CHECK(status_known(kb_mp_to_json(victim, doc_len, 1, jbuf,
                                              sizeof jbuf, &jlen)))) {
            return;
        }
        if (!CHECK(status_known(kb_json_to_mp(jbuf, jlen, sc, sizeof sc,
                                              rebuilt, sizeof rebuilt,
                                              &back)))) {
            return;
        }
        /* tagged round-trip обязан быть побайтовым. */
        if (!CHECK(back == doc_len && memcmp(rebuilt, victim, doc_len) == 0)) {
            return;
        }
    }

    KB_TEST("мусор в мост msgpack↔JSON");
    for (round = 0; round < 6000; round++) {
        static unsigned char out[BODY];
        static unsigned char sc[BODY];
        size_t len = (size_t)kb_rng_below(&rng, 128) + 1;
        size_t i;
        size_t got = 0;

        for (i = 0; i < len; i++) {
            victim[i] = kb_rng_byte(&rng);
        }
        /* Ни мусорный msgpack, ни мусорный JSON не должны падать. */
        if (!CHECK(status_known(
                kb_mp_to_json(victim, len, round & 1, (char *)out, sizeof out,
                              &got)))) {
            return;
        }
        if (!CHECK(status_known(kb_json_to_mp((const char *)victim, len, sc,
                                              sizeof sc, out, sizeof out,
                                              &got)))) {
            return;
        }
    }

}
