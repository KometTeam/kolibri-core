/*
 * Дамп того, что производит C-ядро, для проверки эталонной реализацией.
 *
 * Вектора в tests/ проверяют направление «Rust произвёл — C разобрал».
 * Обратное направление — «C произвёл — Rust разобрал» — проверяется этим
 * дампером: он печатает пары hex, а верификатор на Rust (живёт рядом с
 * kolibri-net, в этот репозиторий не входит) их читает.
 *
 *   make dump > /tmp/c_output.txt
 *
 * Формат: по строке на случай.
 *   lz4    <plain_hex> <compressed_hex>
 *   packet <payload_hex> <packet_hex>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kolibri.h"

#define MAX_PLAIN 262144u
#define MAX_OUT (MAX_PLAIN + MAX_PLAIN / 255u + 64u)

static unsigned char plain[MAX_PLAIN];
static unsigned char comp[MAX_OUT];
static unsigned char packet[MAX_OUT];

static unsigned long rng_state = 0x12345678ul;

static unsigned char next_byte(void)
{
    rng_state = (rng_state * 1103515245ul + 12345ul) & 0x7FFFFFFFul;
    return (unsigned char)((rng_state >> 16) & 0xFF);
}

static void fill(int shape, size_t len)
{
    size_t i;

    switch (shape) {
    case 0:
        for (i = 0; i < len; i++) {
            plain[i] = next_byte();
        }
        break;
    case 1:
        memset(plain, 0, len);
        break;
    case 2:
        for (i = 0; i < len; i++) {
            plain[i] = (unsigned char)"kolibri"[i % 7];
        }
        break;
    case 3:
        for (i = 0; i < len; i++) {
            plain[i] = (unsigned char)('a' + (i % 26));
        }
        break;
    default:
        i = 0;
        while (i < len) {
            unsigned char b = next_byte();
            int run = 1 + (int)(next_byte() % 40);
            int k;
            for (k = 0; k < run && i < len; k++) {
                plain[i++] = b;
            }
        }
        break;
    }
}

static void put_hex(const unsigned char *data, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
}

int main(void)
{
    static const size_t sizes[] = {1,   2,    4,    11,   12,   13,    15,
                                   16,  31,   32,   33,   47,   64,    100,
                                   127, 128,  255,  256,  511,  512,   1000,
                                   4096, 16384, 65536, 262144};
    size_t si;
    int shape;

    for (si = 0; si < sizeof sizes / sizeof sizes[0]; si++) {
        for (shape = 0; shape < 5; shape++) {
            size_t len = sizes[si];
            size_t comp_len = 0;
            size_t packet_len = 0;

            if (len > MAX_PLAIN) {
                continue;
            }
            fill(shape, len);

            if (kb_lz4_compress(plain, len, comp, sizeof comp, &comp_len) != KB_OK) {
                fprintf(stderr, "compress failed at len=%lu shape=%d\n",
                        (unsigned long)len, shape);
                return 1;
            }
            printf("lz4 ");
            put_hex(plain, len);
            printf(" ");
            put_hex(comp, comp_len);
            printf("\n");

            if (kb_encode(KB_CMD_REQUEST, 64, (unsigned short)(si * 5 + shape),
                          plain, len, packet, sizeof packet,
                          &packet_len) != KB_OK) {
                fprintf(stderr, "encode failed at len=%lu shape=%d\n",
                        (unsigned long)len, shape);
                return 1;
            }
            printf("packet ");
            put_hex(plain, len);
            printf(" ");
            put_hex(packet, packet_len);
            printf("\n");
        }
    }
    return 0;
}
