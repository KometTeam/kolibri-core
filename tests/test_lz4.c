#include <stdlib.h>
#include <string.h>

#include "kb_test.h"
#include "kolibri.h"

#define MAX_PLAIN 65536u
#define MAX_COMP (MAX_PLAIN + MAX_PLAIN / 255u + 64u)

static unsigned char plain[MAX_PLAIN];
static unsigned char comp[MAX_COMP];
static unsigned char back[MAX_PLAIN];

/* Подставной распаковщик: проверяет маршрутизацию, не таща зависимость. */
static int fake_zstd_calls;

static kb_status fake_zstd(const unsigned char *src, size_t src_len,
                           unsigned char *dst, size_t dst_cap, size_t *dst_len)
{
    (void)src;
    (void)src_len;
    fake_zstd_calls++;
    if (dst_cap < 4) {
        return KB_ERR_TOO_LARGE;
    }
    memcpy(dst, "zstd", 4);
    if (dst_len != NULL) {
        *dst_len = 4;
    }
    return KB_OK;
}

static void roundtrip(int shape, size_t len, kb_rng *rng)
{
    size_t comp_len = 0;
    size_t back_len = 0;

    kb_gen_payload(shape, len, plain, rng);

    if (!CHECK_EQ(kb_lz4_compress(plain, len, comp, sizeof comp, &comp_len), KB_OK)) {
        return;
    }
    CHECK(comp_len <= kb_lz4_compress_bound(len));
    if (!CHECK_EQ(kb_lz4_decompress(comp, comp_len, back, sizeof back, &back_len),
                  KB_OK)) {
        return;
    }
    CHECK_MEM(back, back_len, plain, len);
}

void kb_tests_lz4(void)
{
    static const size_t sizes[] = {0,  1,   2,   3,   4,    5,    12,  13,
                                   15, 16,  31,  32,  33,   64,   127, 128,
                                   255, 256, 511, 1000, 4096, 16384, 65536};
    kb_rng rng;
    size_t out_len = 0;
    size_t comp_len = 0;
    size_t si;
    int shape;

    KB_TEST("только литералы");
    {
        static const unsigned char src[] = {0x50, 'h', 'e', 'l', 'l', 'o'};
        CHECK_EQ(kb_lz4_decompress(src, sizeof src, back, sizeof back, &out_len),
                 KB_OK);
        CHECK_MEM(back, out_len, "hello", 5);
    }

    KB_TEST("копия матча");
    {
        /* token 0x10: 1 литерал 'a', матч длиной 4 со смещением 1 -> "aaaaa" */
        static const unsigned char src[] = {0x10, 'a', 0x01, 0x00};
        CHECK_EQ(kb_lz4_decompress(src, sizeof src, back, sizeof back, &out_len),
                 KB_OK);
        CHECK_MEM(back, out_len, "aaaaa", 5);
    }

    KB_TEST("перекрывающаяся копия");
    {
        /* 2 литерала "ab", матч длиной 5 со смещением 2 -> "ababab" + "a" */
        static const unsigned char src[] = {0x21, 'a', 'b', 0x02, 0x00};
        CHECK_EQ(kb_lz4_decompress(src, sizeof src, back, sizeof back, &out_len),
                 KB_OK);
        CHECK_MEM(back, out_len, "abababa", 7);
    }

    KB_TEST("пустой вход");
    {
        CHECK_EQ(kb_lz4_decompress((const unsigned char *)"", 0, back,
                                   sizeof back, &out_len),
                 KB_OK);
        CHECK_EQ(out_len, 0);
        CHECK_EQ(kb_lz4_compress((const unsigned char *)"", 0, comp, sizeof comp,
                                 &comp_len),
                 KB_OK);
        CHECK_EQ(kb_lz4_decompress(comp, comp_len, back, sizeof back, &out_len),
                 KB_OK);
        CHECK_EQ(out_len, 0);
    }

    KB_TEST("битые потоки");
    {
        static const unsigned char zero_offset[] = {0x00, 0x00, 0x00};
        static const unsigned char out_of_range[] = {0x10, 'a', 0x05, 0x00};
        static const unsigned char short_literals[] = {0x50, 'a', 'b'};
        static const unsigned char short_offset[] = {0x00, 0x01};

        CHECK_EQ(kb_lz4_decompress(zero_offset, sizeof zero_offset, back,
                                   sizeof back, &out_len),
                 KB_ERR_LZ4_ZERO_OFFSET);
        CHECK_EQ(kb_lz4_decompress(out_of_range, sizeof out_of_range, back,
                                   sizeof back, &out_len),
                 KB_ERR_LZ4_OFFSET_RANGE);
        CHECK_EQ(kb_lz4_decompress(short_literals, sizeof short_literals, back,
                                   sizeof back, &out_len),
                 KB_ERR_TRUNCATED);
        CHECK_EQ(kb_lz4_decompress(short_offset, sizeof short_offset, back,
                                   sizeof back, &out_len),
                 KB_ERR_TRUNCATED);
    }

    KB_TEST("тесный выходной буфер не переполняется");
    {
        static const unsigned char src[] = {0x50, 'h', 'e', 'l', 'l', 'o'};
        unsigned char guarded[8];
        memset(guarded, 0xEE, sizeof guarded);
        CHECK_EQ(kb_lz4_decompress(src, sizeof src, guarded, 4, &out_len),
                 KB_ERR_TOO_LARGE);
        /* байты за пределами разрешённых четырёх остались нетронутыми */
        CHECK_EQ(guarded[4], 0xEE);
        CHECK_EQ(guarded[5], 0xEE);
    }

    KB_TEST("сжатие в тесный буфер");
    {
        kb_rng r;
        kb_rng_seed(&r, 7);
        kb_gen_payload(0, 4096, plain, &r);
        CHECK_EQ(kb_lz4_compress(plain, 4096, comp, 16, &comp_len),
                 KB_ERR_TOO_LARGE);
        CHECK_EQ(kb_lz4_compress(plain, 4096, comp, 0, &comp_len),
                 KB_ERR_TOO_LARGE);
    }

    KB_TEST("аргументы");
    {
        CHECK_EQ(kb_lz4_compress(NULL, 4, comp, sizeof comp, &comp_len),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_lz4_decompress(NULL, 4, back, sizeof back, &out_len),
                 KB_ERR_INVALID);
    }

    KB_TEST("round-trip по формам и размерам");
    kb_rng_seed(&rng, 0xC0FFEEul);
    for (si = 0; si < sizeof sizes / sizeof sizes[0]; si++) {
        for (shape = 0; shape < KB_SHAPE_COUNT; shape++) {
            roundtrip(shape, sizes[si], &rng);
        }
    }

    KB_TEST("round-trip на случайных длинах");
    for (si = 0; si < 300; si++) {
        size_t len = (size_t)kb_rng_below(&rng, 5000);
        roundtrip((int)(si % KB_SHAPE_COUNT), len, &rng);
    }

    KB_TEST("потолок распаковки не зависит от размера буфера");
    {
        /*
         * Регрессия на находку аудита: KB_MAX_DECOMPRESSED_SIZE был объявлен
         * защитой от zip-бомбы, но проверялся только для одного поля длины, а
         * суммарный выход ограничивал лишь буфер вызывающей стороны. Эталон
         * проверяет накопленный размер на каждой последовательности.
         *
         * Блок: один литерал, затем матч со смещением 1 и длиной ~40 МБ,
         * набранной цепочкой байтов 255. На входе 153 КБ.
         */
        const size_t target = 40000000u;
        const size_t chain = (target - 19u) / 255u;
        const size_t rest = (target - 19u) % 255u;
        const size_t src_len = 4u + chain + 1u;
        unsigned char *bomb = (unsigned char *)malloc(src_len);
        unsigned char *room = (unsigned char *)malloc(48u * 1024u * 1024u);
        size_t produced = 0;

        if (CHECK(bomb != NULL && room != NULL)) {
            bomb[0] = 0x1F; /* 1 литерал, длина матча через расширение */
            bomb[1] = 'A';
            bomb[2] = 0x01; /* смещение 1 */
            bomb[3] = 0x00;
            memset(bomb + 4, 0xFF, chain);
            bomb[4 + chain] = (unsigned char)rest;

            /* Буфер вдвое больше потолка — и всё равно отказ. */
            CHECK_EQ(kb_lz4_decompress(bomb, src_len, room,
                                       48u * 1024u * 1024u, &produced),
                     KB_ERR_LIMIT);
            /* И через определение формата тоже. */
            CHECK_EQ(kb_decompress(bomb, src_len, room, 48u * 1024u * 1024u,
                                   &produced),
                     KB_ERR_LIMIT);
        }
        free(bomb);
        free(room);
    }

    KB_TEST("ровно на потолке распаковка проходит");
    {
        /* Матч длиной чуть меньше потолка обязан пройти. */
        const size_t target = 32u * 1024u * 1024u - 1000u;
        const size_t chain = (target - 19u) / 255u;
        const size_t rest = (target - 19u) % 255u;
        const size_t src_len = 4u + chain + 1u;
        unsigned char *block = (unsigned char *)malloc(src_len);
        unsigned char *room = (unsigned char *)malloc(33u * 1024u * 1024u);
        size_t produced = 0;

        if (CHECK(block != NULL && room != NULL)) {
            block[0] = 0x1F;
            block[1] = 'A';
            block[2] = 0x01;
            block[3] = 0x00;
            memset(block + 4, 0xFF, chain);
            block[4 + chain] = (unsigned char)rest;

            CHECK_EQ(kb_lz4_decompress(block, src_len, room,
                                       33u * 1024u * 1024u, &produced),
                     KB_OK);
            CHECK_EQ(produced, 1u + target);
            CHECK(produced <= KB_MAX_DECOMPRESSED_SIZE);
        }
        free(block);
        free(room);
    }

    KB_TEST("разбор тела по флагу, как у официального клиента");
    {
        /*
         * Флаг в заголовке — дискриминатор формата, а не «сжато/нет»:
         * 0x00 — как есть, 0xFF — zstd, всё прочее — LZ4 block.
         * Так его трактует клиент (класс zfb в 26.28).
         */
        static const unsigned char literal[] = {0x50, 'h', 'e', 'l', 'l', 'o'};

        /* 0x00: тело копируется как есть */
        CHECK_EQ(kb_decompress_body(KB_COMPRESSION_NONE, literal, sizeof literal,
                                    back, sizeof back, &out_len),
                 KB_OK);
        CHECK_MEM(back, out_len, literal, sizeof literal);

        /* произвольный ненулевой флаг: LZ4 block */
        CHECK_EQ(kb_decompress_body(0x0C, literal, sizeof literal, back,
                                    sizeof back, &out_len),
                 KB_OK);
        CHECK_MEM(back, out_len, "hello", 5);
        CHECK_EQ(kb_decompress_body(0x01, literal, sizeof literal, back,
                                    sizeof back, &out_len),
                 KB_OK);

        /* 0xFF без поставленного распаковщика — честный отказ */
        CHECK(kb_get_zstd_decompressor() == NULL);
        CHECK_EQ(kb_decompress_body(KB_COMPRESSION_ZSTD, literal,
                                    sizeof literal, back, sizeof back,
                                    &out_len),
                 KB_ERR_UNSUPPORTED);

        /* с поставленным — управление уходит в него */
        kb_set_zstd_decompressor(fake_zstd);
        fake_zstd_calls = 0;
        CHECK_EQ(kb_decompress_body(KB_COMPRESSION_ZSTD, literal,
                                    sizeof literal, back, sizeof back,
                                    &out_len),
                 KB_OK);
        CHECK_EQ(fake_zstd_calls, 1);
        CHECK_MEM(back, out_len, "zstd", 4);

        /* и только при 0xFF, а не при любом сжатии */
        fake_zstd_calls = 0;
        CHECK_EQ(kb_decompress_body(0x0C, literal, sizeof literal, back,
                                    sizeof back, &out_len),
                 KB_OK);
        CHECK_EQ(fake_zstd_calls, 0);

        kb_set_zstd_decompressor(NULL);
        CHECK(kb_get_zstd_decompressor() == NULL);
    }

    KB_TEST("пакет с флагом zstd проходит через kb_decode");
    {
        unsigned char packet[64];
        kb_header header;
        size_t body_len = 0;

        /* Собираем пакет вручную: флаг 0xFF, тело — что угодно. */
        packet[0] = KB_PROTOCOL_VERSION;
        packet[1] = KB_CMD_OK;
        packet[2] = 0x00;
        packet[3] = 0x07;
        packet[4] = 0x00;
        packet[5] = 0x40;
        packet[6] = KB_COMPRESSION_ZSTD;
        packet[7] = 0x00;
        packet[8] = 0x00;
        packet[9] = 0x03;
        packet[10] = 'a';
        packet[11] = 'b';
        packet[12] = 'c';

        /* Без распаковщика — понятный отказ, а не мусор. */
        CHECK_EQ(kb_decode(packet, 13, &header, back, sizeof back, &body_len),
                 KB_ERR_UNSUPPORTED);
        CHECK_EQ(header.flag, 0xFF);

        kb_set_zstd_decompressor(fake_zstd);
        CHECK_EQ(kb_decode(packet, 13, &header, back, sizeof back, &body_len),
                 KB_OK);
        CHECK_MEM(back, body_len, "zstd", 4);
        kb_set_zstd_decompressor(NULL);
    }

    KB_TEST("определение формата");
    {
        static const unsigned char zstd_magic[] = {0x28, 0xB5, 0x2F, 0xFD, 0x00};
        static const unsigned char lz4f_magic[] = {0x04, 0x22, 0x4D, 0x18, 0x00};
        CHECK_EQ(kb_decompress(zstd_magic, sizeof zstd_magic, back, sizeof back,
                               &out_len),
                 KB_ERR_UNSUPPORTED);
        CHECK_EQ(kb_decompress(lz4f_magic, sizeof lz4f_magic, back, sizeof back,
                               &out_len),
                 KB_ERR_UNSUPPORTED);
    }
}
