#include <string.h>

#include "kb_test.h"
#include "kolibri.h"

static unsigned char buffer[64 * 1024];
static unsigned char scratch[4096];

/* Собрать три пакета и вернуть их общий поток. */
static size_t build_stream(unsigned char *out, size_t cap, size_t *lens)
{
    size_t total = 0;
    unsigned short seq;

    for (seq = 0; seq < 3; seq++) {
        unsigned char body[128];
        size_t body_len = (size_t)seq * 40;
        size_t written = 0;
        size_t i;

        for (i = 0; i < body_len; i++) {
            body[i] = (unsigned char)(i % 251);
        }
        if (kb_encode(KB_CMD_REQUEST, (unsigned short)(64 + seq), seq, body,
                      body_len, out + total, cap - total, &written) != KB_OK) {
            return 0;
        }
        lens[seq] = written;
        total += written;
    }
    return total;
}

void kb_tests_framing(void)
{
    kb_rx rx;
    unsigned char stream[512];
    size_t lens[3];
    size_t total;
    size_t got_len = 0;
    const unsigned char *got;

    total = build_stream(stream, sizeof stream, lens);
    CHECK(total > 0);

    KB_TEST("поток по одному байту");
    {
        size_t i;
        int seen = 0;

        CHECK_EQ(kb_rx_init(&rx, buffer, sizeof buffer), KB_OK);
        for (i = 0; i < total; i++) {
            CHECK_EQ(kb_rx_push(&rx, stream + i, 1), KB_OK);
            while ((got = kb_rx_next(&rx, &got_len)) != NULL) {
                CHECK_EQ(got_len, lens[seen]);
                seen++;
            }
        }
        CHECK_EQ(seen, 3);
        CHECK_EQ(kb_rx_buffered(&rx), 0);
    }

    KB_TEST("весь поток одним куском");
    {
        int seen = 0;
        size_t offset = 0;

        CHECK_EQ(kb_rx_init(&rx, buffer, sizeof buffer), KB_OK);
        CHECK_EQ(kb_rx_push(&rx, stream, total), KB_OK);
        while ((got = kb_rx_next(&rx, &got_len)) != NULL) {
            CHECK_MEM(got, got_len, stream + offset, lens[seen]);
            offset += lens[seen];
            seen++;
        }
        CHECK_EQ(seen, 3);
        CHECK_EQ(kb_rx_buffered(&rx), 0);
    }

    KB_TEST("неполный пакет ждёт продолжения");
    {
        CHECK_EQ(kb_rx_init(&rx, buffer, sizeof buffer), KB_OK);
        CHECK_EQ(kb_rx_push(&rx, stream, lens[0] + 3), KB_OK);
        CHECK(kb_rx_next(&rx, &got_len) != NULL); /* первый целый */
        CHECK(kb_rx_next(&rx, &got_len) == NULL); /* второй ещё нет */
        CHECK_EQ(kb_rx_buffered(&rx), 3);
        CHECK_EQ(kb_rx_push(&rx, stream + lens[0] + 3, lens[1] - 3), KB_OK);
        CHECK(kb_rx_next(&rx, &got_len) != NULL);
        CHECK_EQ(got_len, lens[1]);
    }

    KB_TEST("заголовок по частям");
    {
        CHECK_EQ(kb_rx_init(&rx, buffer, sizeof buffer), KB_OK);
        CHECK_EQ(kb_rx_push(&rx, stream, 5), KB_OK);
        CHECK(kb_rx_next(&rx, &got_len) == NULL);
        CHECK_EQ(kb_rx_push(&rx, stream + 5, lens[0] - 5), KB_OK);
        CHECK(kb_rx_next(&rx, &got_len) != NULL);
        CHECK_EQ(got_len, lens[0]);
    }

    KB_TEST("переполнение сбрасывает состояние");
    {
        unsigned char tiny[32];
        CHECK_EQ(kb_rx_init(&rx, tiny, sizeof tiny), KB_OK);
        CHECK_EQ(kb_rx_push(&rx, stream, 20), KB_OK);
        CHECK_EQ(kb_rx_push(&rx, stream, 20), KB_ERR_OVERFLOW);
        CHECK_EQ(kb_rx_buffered(&rx), 0);
    }

    KB_TEST("уплотнение буфера");
    {
        /* Буфер чуть больше двух пакетов: без сдвига хвоста третий не влезет. */
        unsigned char snug[160];
        size_t i;
        int seen = 0;

        CHECK_EQ(kb_rx_init(&rx, snug, sizeof snug), KB_OK);
        for (i = 0; i < total; i += 16) {
            size_t n = total - i < 16 ? total - i : 16;
            CHECK_EQ(kb_rx_push(&rx, stream + i, n), KB_OK);
            while (kb_rx_next(&rx, &got_len) != NULL) {
                seen++;
            }
        }
        CHECK_EQ(seen, 3);
    }

    KB_TEST("аргументы");
    {
        CHECK_EQ(kb_rx_init(NULL, buffer, sizeof buffer), KB_ERR_INVALID);
        CHECK_EQ(kb_rx_init(&rx, NULL, 100), KB_ERR_INVALID);
        CHECK_EQ(kb_rx_init(&rx, buffer, KB_HEADER_SIZE - 1), KB_ERR_INVALID);
        CHECK_EQ(kb_rx_init(&rx, buffer, sizeof buffer), KB_OK);
        CHECK_EQ(kb_rx_push(&rx, NULL, 5), KB_ERR_INVALID);
        CHECK_EQ(kb_rx_push(&rx, scratch, 0), KB_OK);
        CHECK(kb_rx_next(NULL, &got_len) == NULL);
        CHECK_EQ(kb_rx_buffered(NULL), 0);
    }

    KB_TEST("сброс");
    {
        CHECK_EQ(kb_rx_init(&rx, buffer, sizeof buffer), KB_OK);
        CHECK_EQ(kb_rx_push(&rx, stream, 15), KB_OK);
        kb_rx_reset(&rx);
        CHECK_EQ(kb_rx_buffered(&rx), 0);
        CHECK(kb_rx_next(&rx, &got_len) == NULL);
    }
}
