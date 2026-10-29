/*
 * Проверка адаптера к libzstd на настоящих данных.
 *
 *   make zstd-check ZSTD=/usr/local
 *
 * Юнит-тесты проверяют только маршрутизацию по флагу подставным распаковщиком;
 * здесь работает настоящий libzstd, и данные сжимает он же — то есть проверяем
 * ровно тот путь, по которому пойдёт тело от сервера.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kolibri.h"
#include "kolibri_zstd.h"

#if !defined(KB_WITH_ZSTD)
int main(void)
{
    printf("собрано без KB_WITH_ZSTD — пропускаю\n");
    return 0;
}
#else

#include <zstd.h>

/*
 * Библиотека может быть собрана только на распаковку (ZSTD_LIB_COMPRESSION=0) —
 * именно так её и стоит собирать для клиента: компрессор zstd не нужен, потому
 * что исходящее всегда LZ4 block. Тогда ZSTD_compress отсутствует, и тестовые
 * данные готовим внешним zstd, а не библиотекой.
 */
/* ZSTD_LIB_COMPRESSION — переменная сборки самой библиотеки, в заголовок она
   не попадает, поэтому признак передаётся флагом: -DKB_ZSTD_DECODE_ONLY. */
#if defined(KB_ZSTD_DECODE_ONLY)
#  define KB_HAVE_ZSTD_COMPRESS 0
#else
#  define KB_HAVE_ZSTD_COMPRESS 1
#endif

static int failures;

static void check(int ok, const char *what)
{
    printf("  %s %s\n", ok ? "ок  " : "ПРОВАЛ", what);
    if (!ok) {
        failures++;
    }
}

#define BODY (256u * 1024u)

#if !KB_HAVE_ZSTD_COMPRESS
/* Готовит кадр внешним zstd: библиотека собрана без компрессора. */
static size_t compress_externally(const unsigned char *src, size_t src_len,
                                  unsigned char *dst, size_t dst_cap)
{
    FILE *raw;
    FILE *pipe;
    size_t got;

    raw = fopen("/tmp/kb_zstd_in.bin", "wb");
    if (raw == NULL) {
        return 0;
    }
    fwrite(src, 1, src_len, raw);
    fclose(raw);

    if (system("zstd -q -f -3 /tmp/kb_zstd_in.bin -o /tmp/kb_zstd_in.zst") != 0) {
        return 0;
    }
    pipe = fopen("/tmp/kb_zstd_in.zst", "rb");
    if (pipe == NULL) {
        return 0;
    }
    got = fread(dst, 1, dst_cap, pipe);
    fclose(pipe);
    return got;
}
#endif

static unsigned char plain[BODY];
static unsigned char packed[BODY + 4096];
static unsigned char out[BODY * 2];
static unsigned char packet[BODY + 8192];

int main(void)
{
    size_t packed_len;
    size_t out_len = 0;
    size_t packet_len;
    kb_header header;
    unsigned i;

    printf("libzstd %s\n", kb_zstd_version());
    printf("распаковка zstd:\n");

    check(kb_get_zstd_decompressor() == NULL, "до включения распаковщика нет");
    check(kb_zstd_enable() == KB_OK, "kb_zstd_enable");
    check(kb_get_zstd_decompressor() != NULL, "распаковщик поставлен");

    /* Тело, похожее на настоящее: повторяющийся msgpack-подобный мусор. */
    for (i = 0; i < BODY; i++) {
        plain[i] = (unsigned char)("chatId\xcd\x00\x2a"[i % 9]);
    }

#if KB_HAVE_ZSTD_COMPRESS
    packed_len = ZSTD_compress(packed, sizeof packed, plain, BODY, 3);
    check(!ZSTD_isError(packed_len), "libzstd сжал тело");
#else
    /* Сжимаем внешней утилитой: в распаковочной сборке компрессора нет. */
    packed_len = compress_externally(plain, BODY, packed, sizeof packed);
    check(packed_len > 0, "внешний zstd подготовил кадр");
    if (packed_len == 0) {
        printf("нужен исполняемый zstd в PATH\n");
        return 1;
    }
#endif
    printf("       %u Б -> %lu Б\n", BODY, (unsigned long)packed_len);

    check(kb_decompress_body(KB_COMPRESSION_ZSTD, packed, packed_len, out,
                             sizeof out, &out_len) == KB_OK,
          "kb_decompress_body распаковал");
    check(out_len == BODY && memcmp(out, plain, BODY) == 0,
          "данные совпали байт в байт");

    /* Целый пакет с флагом 0xFF — путь, по которому пойдёт сессия. */
    packet[0] = KB_PROTOCOL_VERSION;
    packet[1] = KB_CMD_OK;
    packet[2] = 0x00;
    packet[3] = 0x2A;
    packet[4] = 0x00;
    packet[5] = 0x40;
    packet[6] = KB_COMPRESSION_ZSTD;
    packet[7] = (unsigned char)((packed_len >> 16) & 0xFF);
    packet[8] = (unsigned char)((packed_len >> 8) & 0xFF);
    packet[9] = (unsigned char)(packed_len & 0xFF);
    memcpy(packet + 10, packed, packed_len);
    packet_len = 10 + packed_len;

    out_len = 0;
    check(kb_decode(packet, packet_len, &header, out, sizeof out, &out_len) ==
              KB_OK,
          "kb_decode разобрал пакет с флагом 0xFF");
    check(header.flag == 0xFF && header.opcode == 64 && header.seq == 42,
          "заголовок разобран верно");
    check(out_len == BODY && memcmp(out, plain, BODY) == 0,
          "тело пакета совпало");

    /* Тесный буфер — отличимая ошибка, а не порча. */
    check(kb_decompress_body(KB_COMPRESSION_ZSTD, packed, packed_len, out, 16,
                             &out_len) == KB_ERR_TOO_LARGE,
          "тесный буфер даёт KB_ERR_TOO_LARGE");

    /*
     * Порча ВНУТРИ блока не обязана быть замечена: zstd по умолчанию не пишет
     * контрольную сумму кадра (Content_Checksum_flag сброшен), и libzstd
     * распакует такой кадр в отличающиеся данные, не сообщив об ошибке. Это
     * свойство формата, а не адаптера — проверено на libzstd напрямую.
     *
     * Поэтому требуем от адаптера единственного, что он обязан гарантировать:
     * не выйти за буфер и вернуть внятный статус. Целостность тела в этом
     * протоколе обеспечивает TLS, а не сжатие.
     */
    packed[packed_len / 2] ^= 0xFF;
    {
        kb_status st = kb_decompress_body(KB_COMPRESSION_ZSTD, packed,
                                          packed_len, out, sizeof out,
                                          &out_len);
        check(st == KB_OK || st == KB_ERR_TRUNCATED || st == KB_ERR_TOO_LARGE ||
                  st == KB_ERR_LIMIT,
              "битый кадр даёт внятный статус, а не срыв");
        check(st != KB_OK || out_len <= sizeof out,
              "и в любом случае не пишет за буфер");
    }

    /* А вот порча ЗАГОЛОВКА кадра ловится всегда: магия проверяется. */
    packed[packed_len / 2] ^= 0xFF; /* вернули как было */
    packed[0] ^= 0xFF;
    check(kb_decompress_body(KB_COMPRESSION_ZSTD, packed, packed_len, out,
                             sizeof out, &out_len) != KB_OK,
          "кадр с испорченной магией отвергнут");
    packed[0] ^= 0xFF;

    /* Не-zstd под флагом zstd — тоже отказ. */
    check(kb_decompress_body(KB_COMPRESSION_ZSTD,
                             (const unsigned char *)"не кадр вовсе", 13, out,
                             sizeof out, &out_len) != KB_OK,
          "мусор вместо кадра отвергнут");

    kb_set_zstd_decompressor(NULL);
    printf("%s\n", failures == 0 ? "все проверки прошли" : "есть провалы");
    return failures == 0 ? 0 : 1;
}
#endif
