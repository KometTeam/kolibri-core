/*
 * Anti-spoof fingerprint для авторизации.
 *
 * 96 байт: три SHA-256, каждый от `digest || int64_be(calls_seed) ||
 * utf8(device_id)`. Кладётся в поле `mode` запроса кода (опкод 17) и в
 * `chatCacheFingerprint` при входе (опкод 19).
 *
 * ПОРЯДОК БЛОКОВ: signature, dex, so.
 *
 * Именно с этим порядком живой сервер реально шлёт SMS — проверено запросом
 * кода на настоящий номер. Он совпадает с рабочим клиентом kolibri на Rust
 * (kolibri-net/src/auth.rs). Раньше здесь стоял signature, so, dex: я так
 * прочитал дизассемблер libmax.so и ошибся — с тем порядком сервер возвращал
 * токен, но код не приходил.
 *
 * Сами дайджесты считает хост: они зависят от конкретной сборки приложения и
 * в ядре им взяться неоткуда.
 */

#include <string.h>

#include "kolibri.h"

/* Один блок: sha256(digest || seed_be || device_id). */
static void seed_hash(const uint8_t *digest, size_t digest_len,
                      const uint8_t seed_be[8], const char *device_id,
                      size_t device_len, uint8_t out[KB_SHA256_SIZE])
{
    kb_sha256 ctx;

    kb_sha256_init(&ctx);
    kb_sha256_update(&ctx, digest, digest_len);
    kb_sha256_update(&ctx, seed_be, 8);
    kb_sha256_update(&ctx, (const uint8_t *)device_id, device_len);
    kb_sha256_final(&ctx, out);
}

kb_status kb_chat_cache_fingerprint(const kb_app_digests *digests,
                                    int64_t calls_seed, const char *device_id,
                                    uint8_t out[KB_FINGERPRINT_SIZE])
{
    uint8_t seed_be[8];
    uint64_t raw;
    size_t device_len;
    size_t i;

    if (digests == NULL || device_id == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (digests->signature == NULL || digests->so == NULL ||
        digests->dex == NULL) {
        return KB_ERR_INVALID;
    }
    if (digests->signature_len == 0 || digests->so_len == 0 ||
        digests->dex_len == 0) {
        return KB_ERR_INVALID;
    }

    /* Порядок байт сети: старший первым, как int64 big-endian. */
    raw = (uint64_t)calls_seed;
    for (i = 0; i < 8; i++) {
        seed_be[i] = (uint8_t)(raw >> (56 - i * 8));
    }
    device_len = strlen(device_id);

    /* Порядок блоков — signature, dex, so, — как в рабочем клиенте: с этим
       порядком сервер реально шлёт SMS. Прежний sig/so/dex был моей ошибкой
       чтения дизассемблера; живой сервер её опроверг. */
    seed_hash(digests->signature, digests->signature_len, seed_be, device_id,
              device_len, out);
    seed_hash(digests->dex, digests->dex_len, seed_be, device_id, device_len,
              out + KB_SHA256_SIZE);
    seed_hash(digests->so, digests->so_len, seed_be, device_id, device_len,
              out + KB_SHA256_SIZE * 2);
    return KB_OK;
}
