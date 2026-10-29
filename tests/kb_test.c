#include <stdio.h>
#include <string.h>

#include "kb_test.h"

long kb_test_checks = 0;
long kb_test_failures = 0;

static const char *kb_test_current = "(без имени)";

void kb_test_begin(const char *name)
{
    kb_test_current = name;
    printf("  %s\n", name);
}

int kb_test_check(int ok, const char *file, int line, const char *expr)
{
    kb_test_checks++;
    if (!ok) {
        kb_test_failures++;
        printf("    ПРОВАЛ %s:%d [%s]: %s\n", file, line, kb_test_current, expr);
    }
    return ok;
}

int kb_test_eq_long(long got, long want, const char *file, int line,
                    const char *what)
{
    kb_test_checks++;
    if (got != want) {
        kb_test_failures++;
        printf("    ПРОВАЛ %s:%d [%s]: %s = %ld, ожидалось %ld\n", file, line,
               kb_test_current, what, got, want);
        return 0;
    }
    return 1;
}

static void dump_around(const unsigned char *data, size_t len, size_t at)
{
    size_t from = at > 4 ? at - 4 : 0;
    size_t to = at + 5 < len ? at + 5 : len;
    size_t i;
    printf("      ");
    for (i = from; i < to; i++) {
        printf("%s%02x", i == at ? ">" : " ", data[i]);
    }
    printf("\n");
}

int kb_test_eq_mem(const void *got, size_t got_len, const void *want,
                   size_t want_len, const char *file, int line,
                   const char *what)
{
    const unsigned char *g = (const unsigned char *)got;
    const unsigned char *w = (const unsigned char *)want;
    size_t i;

    kb_test_checks++;

    if (got_len != want_len) {
        kb_test_failures++;
        printf("    ПРОВАЛ %s:%d [%s]: длина %s = %lu, ожидалось %lu\n", file,
               line, kb_test_current, what, (unsigned long)got_len,
               (unsigned long)want_len);
        return 0;
    }
    for (i = 0; i < got_len; i++) {
        if (g[i] != w[i]) {
            kb_test_failures++;
            printf("    ПРОВАЛ %s:%d [%s]: %s расходится на байте %lu: "
                   "%02x вместо %02x\n",
                   file, line, kb_test_current, what, (unsigned long)i, g[i],
                   w[i]);
            printf("      получено:\n");
            dump_around(g, got_len, i);
            printf("      ожидалось:\n");
            dump_around(w, want_len, i);
            return 0;
        }
    }
    return 1;
}

/*
 * xorshift64 на паре 32-битных половин: на платформе без 64-битного целого
 * (а такие среди целевых есть) арифметика всё равно сойдётся байт в байт.
 */
void kb_rng_seed(kb_rng *rng, unsigned long seed)
{
    rng->lo = (seed ^ 0x9E3779B9ul) & 0xFFFFFFFFul;
    rng->hi = (seed ^ 0x7F4A7C15ul) & 0xFFFFFFFFul;
    if (rng->lo == 0 && rng->hi == 0) {
        rng->lo = 1;
    }
}

static void kb_rng_step(kb_rng *rng)
{
    unsigned long lo = rng->lo;
    unsigned long hi = rng->hi;

    /* x ^= x << 13 */
    hi = (hi ^ ((hi << 13) | (lo >> 19))) & 0xFFFFFFFFul;
    lo = (lo ^ (lo << 13)) & 0xFFFFFFFFul;
    /* x ^= x >> 7 */
    lo = (lo ^ ((lo >> 7) | (hi << 25))) & 0xFFFFFFFFul;
    hi = (hi ^ (hi >> 7)) & 0xFFFFFFFFul;
    /* x ^= x << 17 */
    hi = (hi ^ ((hi << 17) | (lo >> 15))) & 0xFFFFFFFFul;
    lo = (lo ^ (lo << 17)) & 0xFFFFFFFFul;

    rng->lo = lo;
    rng->hi = hi;
}

unsigned char kb_rng_byte(kb_rng *rng)
{
    kb_rng_step(rng);
    return (unsigned char)((rng->hi >> 5) & 0xFFul);
}

unsigned long kb_rng_below(kb_rng *rng, unsigned long bound)
{
    kb_rng_step(rng);
    if (bound == 0) {
        return 0;
    }
    return ((rng->hi >> 3) ^ rng->lo) % bound;
}

/* ─── Генератор полезной нагрузки ──────────────────────────────────────── */

const char *kb_shape_name(int shape)
{
    switch (shape) {
    case 0: return "random";
    case 1: return "zeros";
    case 2: return "repeat";
    case 3: return "text";
    case 4: return "runs";
    case 5: return "msgpack";
    default: break;
    }
    return "?";
}

size_t kb_gen_payload(int shape, size_t len, unsigned char *out, kb_rng *rng)
{
    static const char *const words[] = {"chat ", "message ", "send ",
                                        "user ", "id ", "time "};
    size_t i;

    switch (shape) {
    case 0:
        for (i = 0; i < len; i++) {
            out[i] = kb_rng_byte(rng);
        }
        break;
    case 1:
        memset(out, 0, len);
        break;
    case 2:
        for (i = 0; i < len; i++) {
            out[i] = (unsigned char)"kolibri"[i % 7];
        }
        break;
    case 3:
        i = 0;
        while (i < len) {
            const char *w = words[(i / 5) % 6];
            size_t wl = strlen(w);
            size_t k;
            for (k = 0; k < wl && i < len; k++) {
                out[i++] = (unsigned char)w[k];
            }
        }
        break;
    case 4:
        i = 0;
        while (i < len) {
            unsigned char b = kb_rng_byte(rng);
            unsigned long run = 1 + kb_rng_below(rng, 40);
            unsigned long k;
            for (k = 0; k < run && i < len; k++) {
                out[i++] = b;
            }
        }
        break;
    default:
        i = 0;
        if (len > 0) {
            out[i++] = 0x8A;
        }
        while (i < len) {
            static const unsigned char key[] = {0xA6, 'c', 'h', 'a', 't',
                                                'I', 'd'};
            size_t k;
            for (k = 0; k < sizeof key && i < len; k++) {
                out[i++] = key[k];
            }
            if (i < len) {
                out[i++] = 0xCD;
            }
            if (i < len) {
                out[i++] = kb_rng_byte(rng);
            }
            if (i < len) {
                out[i++] = kb_rng_byte(rng);
            }
        }
        break;
    }
    return len;
}
