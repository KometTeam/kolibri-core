/*
 * Пример на C: собрать пакеты, прогнать их через реассемблер кусками, разобрать.
 *
 *   make examples-c
 *
 * Это полный цикл sans-IO: сокета тут нет, вместо него куски по 7 байт —
 * ровно так же выглядит поток из настоящего recv().
 */

#include <stdio.h>
#include <string.h>

#include "kolibri.h"

/* msgpack: {"chatId": 42} */
static const unsigned char SMALL[] = {0x81, 0xA6, 'c', 'h', 'a', 't',
                                      'I',  'd',  0xCD, 0x00, 0x2A};

static void print_hex(const char *label, const unsigned char *data, size_t len)
{
    size_t i;
    printf("%s (%lu Б): ", label, (unsigned long)len);
    for (i = 0; i < len && i < 24; i++) {
        printf("%02x", data[i]);
    }
    if (len > 24) {
        printf("…");
    }
    printf("\n");
}

int main(void)
{
    unsigned char big[1400];
    unsigned char stream[4096];
    unsigned char buffer[8192];
    unsigned char body[8192];
    size_t stream_len = 0;
    size_t written = 0;
    size_t offset;
    kb_rx rx;
    const unsigned char *raw;
    size_t raw_len = 0;
    size_t i;

    for (i = 0; i < sizeof big; i++) {
        big[i] = (unsigned char)"kolibri"[i % 7];
    }

    /* ── Сборка ────────────────────────────────────────────────────────── */

    if (kb_encode(KB_CMD_REQUEST, 64, 1, SMALL, sizeof SMALL, stream,
                  sizeof stream, &written) != KB_OK) {
        return 1;
    }
    print_hex("маленький пакет", stream, written);
    stream_len = written;

    if (kb_encode(KB_CMD_REQUEST, 64, 2, big, sizeof big, stream + stream_len,
                  sizeof stream - stream_len, &written) != KB_OK) {
        return 1;
    }
    printf("большой пакет: тело %lu Б -> на проводе %lu Б\n",
           (unsigned long)sizeof big, (unsigned long)written);
    stream_len += written;

    /* ── Разбор потока кусками по 7 байт ───────────────────────────────── */

    kb_rx_init(&rx, buffer, sizeof buffer);
    printf("\nпоток %lu Б, кусками по 7:\n", (unsigned long)stream_len);

    for (offset = 0; offset < stream_len; offset += 7) {
        size_t n = stream_len - offset < 7 ? stream_len - offset : 7;
        kb_status st = kb_rx_push(&rx, stream + offset, n);
        if (st != KB_OK) {
            printf("  ошибка: %s\n", kb_status_str(st));
            return 1;
        }

        while ((raw = kb_rx_next(&rx, &raw_len)) != NULL) {
            kb_header header;
            size_t body_len = 0;
            kb_status dec =
                kb_decode(raw, raw_len, &header, body, sizeof body, &body_len);

            if (dec != KB_OK) {
                printf("  разбор не удался: %s\n", kb_status_str(dec));
                return 1;
            }
            printf("  seq=%u opcode=%u %s, тело %lu Б\n", (unsigned)header.seq,
                   (unsigned)header.opcode,
                   header.flag ? "сжато" : "как есть", (unsigned long)body_len);
        }
    }

    printf("\nв буфере осталось: %lu Б\n", (unsigned long)kb_rx_buffered(&rx));
    return 0;
}
