/*
 * SHA-256 (FIPS 180-4).
 *
 * Своя реализация, потому что тащить ради одного хеша целый криптостек
 * бессмысленно: authRequest и login требуют трёх хешей, и это всё, что нужно
 * протоколу от криптографии сверх TLS.
 *
 * Константы и раунды — прямо из стандарта; проверяется он векторами оттуда же.
 */

#include <string.h>

#include "kolibri.h"

static const uint32_t K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu,
    0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u, 0xd807aa98u, 0x12835b01u,
    0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u,
    0xc19bf174u, 0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau, 0x983e5152u,
    0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u,
    0x06ca6351u, 0x14292967u, 0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu,
    0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u,
    0xd6990624u, 0xf40e3585u, 0x106aa070u, 0x19a4c116u, 0x1e376c08u,
    0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu,
    0x682e6ff3u, 0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define BSIG0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define BSIG1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SSIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SSIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

static void sha256_block(kb_sha256 *ctx, const uint8_t *block)
{
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    size_t i;

    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)block[i * 4] << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8) | (uint32_t)block[i * 4 + 3];
    }
    for (i = 16; i < 64; i++) {
        w[i] = SSIG1(w[i - 2]) + w[i - 7] + SSIG0(w[i - 15]) + w[i - 16];
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; i++) {
        uint32_t t1 = h + BSIG1(e) + CH(e, f, g) + K[i] + w[i];
        uint32_t t2 = BSIG0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;

    /* Промежуточные значения — тоже материал ключа: не оставляем на стеке. */
    memset(w, 0, sizeof w);
}

void kb_sha256_init(kb_sha256 *ctx)
{
    if (ctx == NULL) {
        return;
    }
    ctx->state[0] = 0x6a09e667u;
    ctx->state[1] = 0xbb67ae85u;
    ctx->state[2] = 0x3c6ef372u;
    ctx->state[3] = 0xa54ff53au;
    ctx->state[4] = 0x510e527fu;
    ctx->state[5] = 0x9b05688cu;
    ctx->state[6] = 0x1f83d9abu;
    ctx->state[7] = 0x5be0cd19u;
    ctx->length = 0;
    ctx->buffered = 0;
    memset(ctx->buffer, 0, sizeof ctx->buffer);
}

void kb_sha256_update(kb_sha256 *ctx, const uint8_t *data, size_t len)
{
    size_t at = 0;

    if (ctx == NULL || (data == NULL && len != 0)) {
        return;
    }
    ctx->length += (uint64_t)len;

    if (ctx->buffered > 0) {
        size_t need = 64 - ctx->buffered;
        size_t take = len < need ? len : need;
        memcpy(ctx->buffer + ctx->buffered, data, take);
        ctx->buffered += take;
        at = take;
        if (ctx->buffered < 64) {
            return;
        }
        sha256_block(ctx, ctx->buffer);
        ctx->buffered = 0;
    }

    while (len - at >= 64) {
        sha256_block(ctx, data + at);
        at += 64;
    }
    if (at < len) {
        ctx->buffered = len - at;
        memcpy(ctx->buffer, data + at, ctx->buffered);
    }
}

void kb_sha256_final(kb_sha256 *ctx, uint8_t out[KB_SHA256_SIZE])
{
    uint64_t bits;
    uint8_t tail[72];
    size_t pad;
    size_t i;

    if (ctx == NULL || out == NULL) {
        return;
    }
    bits = ctx->length * 8u;

    /* 0x80, нули до 56 по модулю 64, затем длина в битах big-endian. */
    pad = (ctx->buffered < 56) ? (56 - ctx->buffered) : (120 - ctx->buffered);
    memset(tail, 0, sizeof tail);
    tail[0] = 0x80;
    for (i = 0; i < 8; i++) {
        tail[pad + i] = (uint8_t)(bits >> (56 - i * 8));
    }
    kb_sha256_update(ctx, tail, pad + 8);

    for (i = 0; i < 8; i++) {
        out[i * 4] = (uint8_t)(ctx->state[i] >> 24);
        out[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        out[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
        out[i * 4 + 3] = (uint8_t)ctx->state[i];
    }
    memset(ctx, 0, sizeof *ctx);
}

void kb_sha256_hash(const uint8_t *data, size_t len,
                    uint8_t out[KB_SHA256_SIZE])
{
    kb_sha256 ctx;
    kb_sha256_init(&ctx);
    kb_sha256_update(&ctx, data, len);
    kb_sha256_final(&ctx, out);
}
