/*
 * SHA-256 и anti-spoof fingerprint.
 *
 * Хеш проверяется векторами FIPS 180-4, fingerprint — значениями, которые
 * независимо считает scripts/fingerprint.py из репозитория разбора клиента.
 */

#include <string.h>

#include "kb_test.h"
#include "kolibri.h"

static void hex_to_bytes(const char *hex, unsigned char *out, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++) {
        unsigned char hi = (unsigned char)hex[i * 2];
        unsigned char lo = (unsigned char)hex[i * 2 + 1];
        hi = (unsigned char)(hi <= '9' ? hi - '0' : (hi | 32) - 'a' + 10);
        lo = (unsigned char)(lo <= '9' ? lo - '0' : (lo | 32) - 'a' + 10);
        out[i] = (unsigned char)((hi << 4) | lo);
    }
}

static void check_hash(const char *input, size_t len, const char *want,
                       const char *name)
{
    unsigned char digest[KB_SHA256_SIZE];
    unsigned char expected[KB_SHA256_SIZE];

    kb_sha256_hash((const unsigned char *)input, len, digest);
    hex_to_bytes(want, expected, KB_SHA256_SIZE);
    kb_test_eq_mem(digest, KB_SHA256_SIZE, expected, KB_SHA256_SIZE, __FILE__,
                   __LINE__, name);
}

void kb_tests_opcodes(void)
{
    KB_TEST("таблица опкодов");
    {
        /* Опкоды, которыми пользуется сама машина сессии, обязаны совпадать. */
        CHECK_EQ(KB_OP_PING, KB_OPCODE_PING);
        CHECK_EQ(KB_OP_SESSION_INIT, KB_OPCODE_SESSION_INIT);

        /* Точечная сверка с тем, что видно в эталоне и в клиенте. */
        CHECK_EQ(KB_OP_AUTH_REQUEST, 17);
        CHECK_EQ(KB_OP_AUTH, 18);
        CHECK_EQ(KB_OP_LOGIN, 19);
        CHECK_EQ(KB_OP_MSG_SEND, 64);
        CHECK_EQ(KB_OP_MSG_TYPING, 65);
        CHECK_EQ(KB_OP_AUTH_LOGIN_CHECK_PASSWORD, 115);

        CHECK_EQ(strcmp(kb_opcode_name(KB_OP_MSG_SEND), "MSG_SEND"), 0);
        CHECK_EQ(strcmp(kb_opcode_name(1), "PING"), 0);
        CHECK(kb_opcode_name(9999) == NULL);
        CHECK(kb_opcode_name(0) == NULL);

        CHECK_EQ(kb_opcode_by_name("LOGIN"), 19);
        CHECK_EQ(kb_opcode_by_name("нет такого"), 0);
        CHECK_EQ(kb_opcode_by_name(NULL), 0);
    }

    KB_TEST("имя и номер согласованы во всей таблице");
    {
        /* Двоичный поиск обязан находить каждую запись, а обратный поиск —
           возвращать тот же номер. Заодно ловится порядок сортировки. */
        int found = 0;
        long value;

        for (value = 0; value <= 0xFFFF; value++) {
            const char *name = kb_opcode_name((uint16_t)value);
            if (name == NULL) {
                continue;
            }
            found++;
            if (!CHECK_EQ(kb_opcode_by_name(name), value)) {
                break;
            }
        }
        CHECK_EQ(found, KB_OPCODE_COUNT);
    }
}

void kb_tests_auth(void)
{
    static unsigned char big[1000000];

    KB_TEST("SHA-256: векторы FIPS 180-4");
    {
        check_hash("", 0,
                   "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b785"
                   "2b855",
                   "пустая строка");
        check_hash("abc", 3,
                   "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20"
                   "015ad",
                   "abc");
        check_hash("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
                   56,
                   "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419d"
                   "b06c1",
                   "448 бит");
        memset(big, 'a', sizeof big);
        check_hash((const char *)big, sizeof big,
                   "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc71"
                   "12cd0",
                   "миллион 'a'");
    }

    KB_TEST("SHA-256: потоковая подача равна разовой");
    {
        unsigned char whole[KB_SHA256_SIZE];
        unsigned char streamed[KB_SHA256_SIZE];
        kb_sha256 ctx;
        size_t chunk;

        kb_sha256_hash(big, 100000, whole);
        for (chunk = 1; chunk <= 200; chunk += 37) {
            size_t at = 0;
            kb_sha256_init(&ctx);
            while (at < 100000) {
                size_t take = 100000 - at < chunk ? 100000 - at : chunk;
                kb_sha256_update(&ctx, big + at, take);
                at += take;
            }
            kb_sha256_final(&ctx, streamed);
            CHECK_MEM(streamed, KB_SHA256_SIZE, whole, KB_SHA256_SIZE);
        }
    }

    KB_TEST("fingerprint: порядок signature, dex, so");
    {
        /*
         * Порядок signature, dex, so подтверждён живым сервером: только с ним
         * приходит SMS. Совпадает с рабочим Rust-клиентом. Значения ниже
         * посчитаны sha256 в этом же порядке.
         */
        static const char *sig_hex =
            "1684414033eb263e2c615f8b7df5ed8793850a07656304997fbf07e9e21e1e93";
        static const char *dex_hex =
            "8db68fcc0e85e8f041fe4a875c0a9bcfe542a8f679603728c651ed81b64dd684";
        static const char *so_hex =
            "634ecc42b246784d975f180b4fecf903df235cdf0476da47163a85630eb1a6a8";
        static const char *want_hex =
            "710c2334a53626b6b179b59ff1f306865c68c37abb9b1a996a99a3067e60aa84"
            "9d2a744f021de9c241ed9bde48203c0a544b604ec0f558ab7ded56f4a544bbf1"
            "45e65c545e98e6ca9f8d1194decacf506dc844fa9d0be9e56d6645340a0c50d1";

        unsigned char sig[32], dex[32], so[32];
        unsigned char got[KB_FINGERPRINT_SIZE];
        unsigned char want[KB_FINGERPRINT_SIZE];
        kb_app_digests digests;

        hex_to_bytes(sig_hex, sig, 32);
        hex_to_bytes(dex_hex, dex, 32);
        hex_to_bytes(so_hex, so, 32);
        hex_to_bytes(want_hex, want, KB_FINGERPRINT_SIZE);

        digests.signature = sig;
        digests.signature_len = 32;
        digests.so = so;
        digests.so_len = 32;
        digests.dex = dex;
        digests.dex_len = 32;

        CHECK_EQ(kb_chat_cache_fingerprint(&digests, 1234567890123LL,
                                           "kolibri-core-0001", got),
                 KB_OK);
        CHECK_MEM(got, KB_FINGERPRINT_SIZE, want, KB_FINGERPRINT_SIZE);

        /* Перестановка so и dex обязана дать другой результат — иначе тест
           ничего не проверяет. */
        digests.so = dex;
        digests.dex = so;
        CHECK_EQ(kb_chat_cache_fingerprint(&digests, 1234567890123LL,
                                           "kolibri-core-0001", got),
                 KB_OK);
        CHECK(memcmp(got, want, KB_FINGERPRINT_SIZE) != 0);
    }

    KB_TEST("fingerprint: seed и device_id входят в результат");
    {
        unsigned char sig[32], a[KB_FINGERPRINT_SIZE], b[KB_FINGERPRINT_SIZE];
        kb_app_digests d;

        memset(sig, 0x11, sizeof sig);
        d.signature = sig; d.signature_len = 32;
        d.so = sig; d.so_len = 32;
        d.dex = sig; d.dex_len = 32;

        kb_chat_cache_fingerprint(&d, 1, "dev", a);
        kb_chat_cache_fingerprint(&d, 2, "dev", b);
        CHECK(memcmp(a, b, KB_FINGERPRINT_SIZE) != 0);

        kb_chat_cache_fingerprint(&d, 1, "dev2", b);
        CHECK(memcmp(a, b, KB_FINGERPRINT_SIZE) != 0);

        /* Отрицательный seed — обычное дело: сервер шлёт его знаковым. */
        CHECK_EQ(kb_chat_cache_fingerprint(&d, -8775114916663495426LL, "dev", b),
                 KB_OK);
        CHECK(memcmp(a, b, KB_FINGERPRINT_SIZE) != 0);
    }

    KB_TEST("fingerprint: аргументы");
    {
        unsigned char sig[32], out[KB_FINGERPRINT_SIZE];
        kb_app_digests d;

        memset(sig, 1, sizeof sig);
        d.signature = sig; d.signature_len = 32;
        d.so = sig; d.so_len = 32;
        d.dex = sig; d.dex_len = 32;

        CHECK_EQ(kb_chat_cache_fingerprint(NULL, 1, "d", out), KB_ERR_INVALID);
        CHECK_EQ(kb_chat_cache_fingerprint(&d, 1, NULL, out), KB_ERR_INVALID);
        CHECK_EQ(kb_chat_cache_fingerprint(&d, 1, "d", NULL), KB_ERR_INVALID);
        d.so = NULL;
        CHECK_EQ(kb_chat_cache_fingerprint(&d, 1, "d", out), KB_ERR_INVALID);
        d.so = sig; d.dex_len = 0;
        CHECK_EQ(kb_chat_cache_fingerprint(&d, 1, "d", out), KB_ERR_INVALID);
    }
}
