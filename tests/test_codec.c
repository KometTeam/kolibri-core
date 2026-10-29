#include <stdlib.h>
#include <string.h>

#include "kb_test.h"
#include "kolibri.h"

#define MAX_PLAIN 65536u
#define MAX_PACKET (MAX_PLAIN + MAX_PLAIN / 255u + 64u)

static unsigned char payload[MAX_PLAIN];
static unsigned char packet[MAX_PACKET];
static unsigned char decoded[MAX_PLAIN];

static void roundtrip(int shape, size_t len, kb_rng *rng)
{
    kb_header header;
    size_t packet_len = 0;
    size_t out_len = 0;

    kb_gen_payload(shape, len, payload, rng);

    if (!CHECK_EQ(kb_encode(KB_CMD_REQUEST, 64, 7, payload, len, packet,
                            sizeof packet, &packet_len),
                  KB_OK)) {
        return;
    }
    CHECK(packet_len <= kb_encode_bound(len));
    CHECK_EQ(kb_packet_total_len(packet, packet_len), packet_len);

    if (!CHECK_EQ(kb_decode(packet, packet_len, &header, decoded, sizeof decoded,
                            &out_len),
                  KB_OK)) {
        return;
    }
    CHECK_EQ(header.ver, KB_PROTOCOL_VERSION);
    CHECK_EQ(header.cmd, KB_CMD_REQUEST);
    CHECK_EQ(header.opcode, 64);
    CHECK_EQ(header.seq, 7);
    CHECK_EQ(header.flag != 0, len >= KB_COMPRESSION_THRESHOLD);
    CHECK_MEM(decoded, out_len, payload, len);
}

void kb_tests_codec(void)
{
    static const size_t sizes[] = {0,   1,   2,    31,   32,    33,
                                   64,  255, 256,  1000, 4096,  65536};
    kb_rng rng;
    kb_header header;
    size_t packet_len = 0;
    size_t out_len = 0;
    size_t si;
    int shape;

    KB_TEST("раскладка заголовка");
    {
        /* ver, cmd, seq=42, opcode=64, packedLen=2, тело "hi" */
        static const unsigned char expected[] = {0x0A, 0x00, 0x00, 0x2A, 0x00,
                                                 0x40, 0x00, 0x00, 0x00, 0x02,
                                                 'h',  'i'};
        CHECK_EQ(kb_encode(KB_CMD_REQUEST, 64, 42, (const unsigned char *)"hi", 2,
                           packet, sizeof packet, &packet_len),
                 KB_OK);
        CHECK_MEM(packet, packet_len, expected, sizeof expected);
    }

    KB_TEST("заголовок туда и обратно");
    {
        kb_header src;
        unsigned char raw[KB_HEADER_SIZE];

        src.ver = KB_PROTOCOL_VERSION;
        src.cmd = KB_CMD_ERROR;
        src.seq = 65535;
        src.opcode = 4096;
        src.flag = 7;
        src.body_len = 0xFFFFFFu;

        CHECK_EQ(kb_header_encode(&src, raw, sizeof raw), KB_OK);
        CHECK_EQ(kb_header_decode(raw, sizeof raw, &header), KB_OK);
        CHECK_EQ(header.ver, src.ver);
        CHECK_EQ(header.cmd, src.cmd);
        CHECK_EQ(header.seq, src.seq);
        CHECK_EQ(header.opcode, src.opcode);
        CHECK_EQ(header.flag, src.flag);
        CHECK_EQ(header.body_len, (long)src.body_len);

        /* тело длиннее 24 бит не кодируется */
        src.body_len = 0x1000000u;
        CHECK_EQ(kb_header_encode(&src, raw, sizeof raw), KB_ERR_INVALID);
        CHECK_EQ(kb_header_encode(&src, raw, KB_HEADER_SIZE - 1), KB_ERR_TOO_LARGE);
    }

    KB_TEST("длина пакета по началу буфера");
    {
        static const unsigned char head[] = {0x0A, 0x00, 0x00, 0x01, 0x00, 0x40,
                                             0x03, 0x00, 0x00, 0x05};
        CHECK_EQ(kb_packet_total_len(head, sizeof head), KB_HEADER_SIZE + 5);
        /* неполный заголовок -> 0 */
        CHECK_EQ(kb_packet_total_len(head, KB_HEADER_SIZE - 1), 0);
        CHECK_EQ(kb_packet_total_len(NULL, 100), 0);
    }

    KB_TEST("пустое тело");
    {
        CHECK_EQ(kb_encode(KB_CMD_REQUEST, 1, 0, NULL, 0, packet, sizeof packet,
                           &packet_len),
                 KB_OK);
        CHECK_EQ(packet_len, KB_HEADER_SIZE);
        CHECK_EQ(kb_decode(packet, packet_len, &header, decoded, sizeof decoded,
                           &out_len),
                 KB_OK);
        CHECK_EQ(header.body_len, 0);
        CHECK_EQ(out_len, 0);
    }

    KB_TEST("обрезанный пакет");
    {
        CHECK_EQ(kb_encode(KB_CMD_REQUEST, 64, 1, (const unsigned char *)"hello",
                           5, packet, sizeof packet, &packet_len),
                 KB_OK);
        CHECK_EQ(kb_decode(packet, packet_len - 1, &header, decoded,
                           sizeof decoded, &out_len),
                 KB_ERR_TRUNCATED);
        CHECK_EQ(kb_decode(packet, KB_HEADER_SIZE - 1, &header, decoded,
                           sizeof decoded, &out_len),
                 KB_ERR_TRUNCATED);
    }

    KB_TEST("только заголовок, без тела");
    {
        CHECK_EQ(kb_encode(KB_CMD_REQUEST, 64, 1, (const unsigned char *)"hello",
                           5, packet, sizeof packet, &packet_len),
                 KB_OK);
        CHECK_EQ(kb_decode(packet, packet_len, &header, NULL, 0, &out_len), KB_OK);
        CHECK_EQ(header.opcode, 64);
        CHECK_EQ(header.body_len, 5);
    }

    KB_TEST("тесный выходной буфер");
    {
        CHECK_EQ(kb_encode(KB_CMD_REQUEST, 64, 1, (const unsigned char *)"hello",
                           5, packet, KB_HEADER_SIZE + 4, &packet_len),
                 KB_ERR_TOO_LARGE);
        CHECK_EQ(kb_encode(KB_CMD_REQUEST, 64, 1, (const unsigned char *)"hello",
                           5, packet, KB_HEADER_SIZE - 1, &packet_len),
                 KB_ERR_TOO_LARGE);

        CHECK_EQ(kb_encode(KB_CMD_REQUEST, 64, 1, (const unsigned char *)"hello",
                           5, packet, sizeof packet, &packet_len),
                 KB_OK);
        CHECK_EQ(kb_decode(packet, packet_len, &header, decoded, 4, &out_len),
                 KB_ERR_TOO_LARGE);
    }

    KB_TEST("аргументы");
    {
        CHECK_EQ(kb_encode(KB_CMD_REQUEST, 1, 0, NULL, 5, packet, sizeof packet,
                           &packet_len),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_encode(KB_CMD_REQUEST, 1, 0, payload, 4, NULL, 100,
                           &packet_len),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_decode(NULL, 20, &header, decoded, sizeof decoded, &out_len),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_header_decode(packet, KB_HEADER_SIZE, NULL), KB_ERR_INVALID);
    }

    KB_TEST("порог сжатия");
    {
        kb_rng r;
        kb_rng_seed(&r, 11);

        kb_gen_payload(1, KB_COMPRESSION_THRESHOLD - 1, payload, &r);
        CHECK_EQ(kb_encode(KB_CMD_REQUEST, 64, 1, payload,
                           KB_COMPRESSION_THRESHOLD - 1, packet, sizeof packet,
                           &packet_len),
                 KB_OK);
        CHECK_EQ(packet[6], 0);

        kb_gen_payload(1, KB_COMPRESSION_THRESHOLD, payload, &r);
        CHECK_EQ(kb_encode(KB_CMD_REQUEST, 64, 1, payload,
                           KB_COMPRESSION_THRESHOLD, packet, sizeof packet,
                           &packet_len),
                 KB_OK);
        CHECK(packet[6] != 0);
    }

    KB_TEST("исходящий флаг не занимает значения-сигналы");
    {
        /*
         * 0x00 значит «не сжато», 0xFF — «zstd». Формула (ratio+1) даёт 0xFF
         * при степени сжатия 254, и сервер прочитал бы наш LZ4-блок как
         * zstd-кадр. Проверяем на сильно сжимаемых телах разного размера.
         */
        static const size_t big[] = {200000u, 1000000u, 2000000u};
        static unsigned char *zeros;
        size_t k;

        zeros = (unsigned char *)calloc(2000000u, 1);
        if (CHECK(zeros != NULL)) {
            for (k = 0; k < sizeof big / sizeof big[0]; k++) {
                size_t written = 0;
                unsigned char *wire =
                    (unsigned char *)malloc(kb_encode_bound(big[k]));
                if (!CHECK(wire != NULL)) {
                    break;
                }
                CHECK_EQ(kb_encode(KB_CMD_REQUEST, 64, 1, zeros, big[k], wire,
                                   kb_encode_bound(big[k]), &written),
                         KB_OK);
                CHECK(wire[6] != 0x00);
                CHECK(wire[6] != 0xFF);
                /* И тело обязано остаться читаемым. */
                CHECK_EQ(kb_decode(wire, written, &header, NULL, 0, &out_len),
                         KB_OK);
                free(wire);
            }
        }
        free(zeros);
    }

    KB_TEST("round-trip по формам и размерам");
    kb_rng_seed(&rng, 0xBEEFul);
    for (si = 0; si < sizeof sizes / sizeof sizes[0]; si++) {
        for (shape = 0; shape < KB_SHAPE_COUNT; shape++) {
            roundtrip(shape, sizes[si], &rng);
        }
    }
}
