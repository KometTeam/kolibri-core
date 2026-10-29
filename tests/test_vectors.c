/*
 * Прогон векторов, снятых с эталонной реализации (kolibri-net).
 *
 * Это и есть дифференциальный тест: данные породил Rust, разбирает их C, и
 * расхождение означает, что порт разъехался с оригиналом.
 */

#include <stdio.h>
#include <string.h>

#include "kb_test.h"
#include "kolibri.h"
#include "vectors_data.h"

#define SCRATCH 262144u

static unsigned char out[SCRATCH];
static unsigned char packet[SCRATCH];
static unsigned char doc[SCRATCH];

/*
 * Значения для handshake — те же, что зашиты в дампер оракула, поэтому байты
 * обязаны совпасть. Собирает их сама библиотека, а не тест.
 */
static size_t build_handshake(unsigned char *buffer, size_t capacity)
{
    kb_handshake_config config;
    size_t len = 0;

    memset(&config, 0, sizeof config);
    config.instance_id = "kolibri-core-0001";
    config.device_id = "kolibri-core-0001";
    config.client_session_id = 1755730000000LL;
    config.user_agent.device_type = "ANDROID";
    config.user_agent.app_version = "26.25.0";
    config.user_agent.os_version = "Android 14";
    config.user_agent.timezone = "Europe/Moscow";
    config.user_agent.screen = "480dpi 480dpi 1344x2992";
    config.user_agent.push_device_type = "GCM";
    config.user_agent.arch = "arm64-v8a";
    config.user_agent.locale = "ru";
    config.user_agent.build_number = 6790;
    config.user_agent.device_name = "Google Pixel 8 Pro";
    config.user_agent.device_locale = "ru";

    if (kb_build_handshake_payload(&config, buffer, capacity, &len) != KB_OK) {
        return 0;
    }
    return len;
}

static size_t build_ping(unsigned char *buffer, size_t capacity, int interactive)
{
    size_t len = 0;
    if (kb_build_ping_payload(interactive, buffer, capacity, &len) != KB_OK) {
        return 0;
    }
    return len;
}

static kb_status status_for(const char *name)
{
    if (strcmp(name, "truncated") == 0) {
        return KB_ERR_TRUNCATED;
    }
    if (strcmp(name, "zero_offset") == 0) {
        return KB_ERR_LZ4_ZERO_OFFSET;
    }
    if (strcmp(name, "offset_range") == 0) {
        return KB_ERR_LZ4_OFFSET_RANGE;
    }
    if (strcmp(name, "limit") == 0) {
        return KB_ERR_LIMIT;
    }
    return KB_ERR_INVALID;
}

void kb_tests_vectors(void)
{
    size_t i;
    size_t len = 0;

    printf("  (%lu encode, %lu decode, %lu lz4, %lu ошибок, %lu framing, "
           "%lu msgpack, %lu документов)\n",
           (unsigned long)kb_vectors_encode_count,
           (unsigned long)kb_vectors_decode_count,
           (unsigned long)kb_vectors_lz4_count,
           (unsigned long)kb_vectors_lz4_error_count,
           (unsigned long)kb_vectors_framing_count,
           (unsigned long)kb_vectors_msgpack_count,
           (unsigned long)kb_vectors_doc_count);

    KB_TEST("кодирование байт в байт");
    for (i = 0; i < kb_vectors_encode_count; i++) {
        const kb_vec_encode *v = &kb_vectors_encode[i];
        if (!CHECK_EQ(kb_encode(v->cmd, v->opcode, v->seq, v->payload.data,
                                v->payload.len, packet, sizeof packet, &len),
                      KB_OK)) {
            continue;
        }
        CHECK_MEM(packet, len, v->packet.data, v->packet.len);
    }

    KB_TEST("разбор пакетов эталона");
    for (i = 0; i < kb_vectors_decode_count; i++) {
        const kb_vec_decode *v = &kb_vectors_decode[i];
        kb_header header;

        if (!CHECK_EQ(kb_decode(v->packet.data, v->packet.len, &header, out,
                                sizeof out, &len),
                      KB_OK)) {
            continue;
        }
        CHECK_EQ(header.ver, v->ver);
        CHECK_EQ(header.cmd, v->cmd);
        CHECK_EQ(header.seq, v->seq);
        CHECK_EQ(header.opcode, v->opcode);
        CHECK_EQ(header.flag, v->flag);
        CHECK_EQ(header.body_len, (long)v->body_len);
        CHECK_MEM(out, len, v->payload.data, v->payload.len);
    }

    KB_TEST("распаковка LZ4 от эталона");
    for (i = 0; i < kb_vectors_lz4_count; i++) {
        const kb_vec_lz4 *v = &kb_vectors_lz4[i];
        if (!CHECK_EQ(kb_lz4_decompress(v->compressed.data, v->compressed.len,
                                        out, sizeof out, &len),
                      KB_OK)) {
            continue;
        }
        CHECK_MEM(out, len, v->plain.data, v->plain.len);
    }

    KB_TEST("коды ошибок совпадают с эталонными");
    for (i = 0; i < kb_vectors_lz4_error_count; i++) {
        const kb_vec_lz4_error *v = &kb_vectors_lz4_error[i];
        CHECK_EQ(kb_lz4_decompress(v->input.data, v->input.len, out, sizeof out,
                                   &len),
                 status_for(v->error));
    }

    KB_TEST("msgpack: скаляры кодируются как у эталона");
    for (i = 0; i < kb_vectors_msgpack_count; i++) {
        const kb_vec_msgpack *v = &kb_vectors_msgpack[i];
        kb_mp_writer w;
        kb_mp_value parsed;
        kb_mp_reader reader;

        kb_mp_writer_init(&w, out, sizeof out);
        switch (v->kind) {
        case KB_VEC_MP_I64:
            kb_mp_write_i64(&w, v->i);
            break;
        case KB_VEC_MP_U64:
            kb_mp_write_u64(&w, v->u);
            break;
        case KB_VEC_MP_STR:
            kb_mp_write_str(&w, (const char *)v->value.data, v->value.len);
            break;
        case KB_VEC_MP_BIN:
            kb_mp_write_bin(&w, v->value.data, v->value.len);
            break;
        case KB_VEC_MP_BOOL:
            kb_mp_write_bool(&w, v->b);
            break;
        default:
            kb_mp_write_nil(&w);
            break;
        }
        if (!CHECK_EQ(kb_mp_writer_status(&w), KB_OK)) {
            continue;
        }
        CHECK_MEM(out, kb_mp_writer_len(&w), v->bytes.data, v->bytes.len);

        /* И обратно: то, что написал эталон, должно читаться нашим курсором. */
        kb_mp_reader_init(&reader, v->bytes.data, v->bytes.len);
        if (!CHECK_EQ(kb_mp_read(&reader, &parsed), KB_OK)) {
            continue;
        }
        CHECK_EQ(kb_mp_reader_left(&reader), 0);
        switch (v->kind) {
        case KB_VEC_MP_I64:
            /* Неотрицательные эталон пишет беззнаковыми — это ожидаемо. */
            if (v->i >= 0) {
                CHECK_EQ(parsed.type, KB_MP_UINT);
            } else {
                CHECK_EQ(parsed.type, KB_MP_INT);
                CHECK(parsed.i == v->i);
            }
            break;
        case KB_VEC_MP_U64:
            CHECK_EQ(parsed.type, KB_MP_UINT);
            CHECK(parsed.u == v->u);
            break;
        case KB_VEC_MP_STR:
            CHECK_EQ(parsed.type, KB_MP_STR);
            CHECK_MEM(parsed.data, parsed.len, v->value.data, v->value.len);
            break;
        case KB_VEC_MP_BIN:
            CHECK_EQ(parsed.type, KB_MP_BIN);
            CHECK_MEM(parsed.data, parsed.len, v->value.data, v->value.len);
            break;
        case KB_VEC_MP_BOOL:
            CHECK_EQ(parsed.type, KB_MP_BOOL);
            CHECK_EQ(parsed.boolean != 0, v->b != 0);
            break;
        default:
            CHECK_EQ(parsed.type, KB_MP_NIL);
            break;
        }
    }

    KB_TEST("msgpack: handshake и ping байт в байт");
    for (i = 0; i < kb_vectors_doc_count; i++) {
        const kb_vec_doc *v = &kb_vectors_doc[i];
        size_t built;

        if (strcmp(v->name, "handshake") == 0) {
            built = build_handshake(doc, sizeof doc);
        } else if (strcmp(v->name, "ping_interactive") == 0) {
            built = build_ping(doc, sizeof doc, 1);
        } else {
            built = build_ping(doc, sizeof doc, 0);
        }
        if (!CHECK(built > 0)) {
            continue;
        }
        CHECK_MEM(doc, built, v->bytes.data, v->bytes.len);
    }

    KB_TEST("реассемблер повторяет разбивку эталона");
    for (i = 0; i < kb_vectors_framing_count; i++) {
        const kb_vec_framing *v = &kb_vectors_framing[i];
        static unsigned char buffer[8192];
        kb_rx rx;
        size_t c;
        size_t seen = 0;
        const unsigned char *got;
        size_t got_len = 0;

        CHECK_EQ(kb_rx_init(&rx, buffer, sizeof buffer), KB_OK);
        for (c = 0; c < v->chunk_count; c++) {
            if (!CHECK_EQ(kb_rx_push(&rx, v->chunks[c].data, v->chunks[c].len),
                          KB_OK)) {
                break;
            }
            while ((got = kb_rx_next(&rx, &got_len)) != NULL) {
                if (seen < v->packet_count) {
                    CHECK_MEM(got, got_len, v->packets[seen].data,
                              v->packets[seen].len);
                }
                seen++;
            }
        }
        CHECK_EQ(seen, (long)v->packet_count);
        CHECK_EQ(kb_rx_buffered(&rx), 0);
    }
}
