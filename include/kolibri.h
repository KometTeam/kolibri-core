/*
 * kolibri — переносимое ядро бинарного протокола Komet.
 *
 * C89 + <stdint.h>. Ни аллокаций, ни сокетов, ни потоков, ни файлов: на входе
 * байты, на выходе байты, все буферы даёт вызывающая сторона. Собирается везде,
 * где есть компилятор C, — от clang 3.x под armv7 до MSVC 2013 под Windows
 * Phone 8.1.
 *
 * Формат пакета, 10-байтный заголовок big-endian:
 *
 *   [0]      ver        версия протокола (10)
 *   [1]      cmd        тип команды
 *   [2..4]   seq        номер запроса
 *   [4..6]   opcode     код операции
 *   [6..10]  packedLen  старший байт = флаг сжатия, младшие 24 бита = длина тела
 *   [10..]   body       MessagePack, возможно сжатый
 */

#ifndef KOLIBRI_H
#define KOLIBRI_H

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define KB_VERSION_MAJOR 0
#define KB_VERSION_MINOR 1
#define KB_VERSION_PATCH 0

/* ─── Константы протокола ──────────────────────────────────────────────── */

#define KB_HEADER_SIZE 10
#define KB_PROTOCOL_VERSION 10

/* Тип команды. Запрос и push оба несут 0 — их различает направление. */
#define KB_CMD_REQUEST 0
#define KB_CMD_PUSH 0
#define KB_CMD_OK 1
#define KB_CMD_NOT_FOUND 2
#define KB_CMD_ERROR 3

/* Тело короче этого уходит без сжатия. */
#define KB_COMPRESSION_THRESHOLD 32

/*
 * Старший байт packedLen — это дискриминатор формата тела, а не просто
 * «сжато/нет». Так его трактует официальный клиент (класс zfb в 26.28):
 *
 *   0x00           тело как есть
 *   0xFF           тело сжато zstd
 *   что-то другое  тело сжато LZ4 block, а значение — оценка степени сжатия
 *
 * Исходящее сжимается только LZ4 block: zstd-компрессора у клиента нет вовсе,
 * его libzstd экспортирует единственную функцию распаковки.
 */
#define KB_COMPRESSION_NONE 0x00
#define KB_COMPRESSION_ZSTD 0xFF

/* Длина тела занимает 24 бита. */
#define KB_MAX_BODY_LEN 0xFFFFFFu

/* Потолок буфера реассемблера, как в референсном ядре. */
#define KB_MAX_BUFFER_SIZE (16u * 1024u * 1024u)

/*
 * Потолок для одного распакованного тела: защита от zip-бомбы.
 *
 * Применяется к НАКОПЛЕННОМУ выходу распаковки, как в эталонной реализации:
 * превышение даёт KB_ERR_LIMIT независимо от того, насколько велик буфер
 * вызывающей стороны. Длина тела на проводе занимает 24 бита, то есть не
 * больше 16 МБ, так что 32 МБ распакованного — это уже вдвое больше любого
 * честного пакета.
 */
#define KB_MAX_DECOMPRESSED_SIZE (32u * 1024u * 1024u)

/* ─── Коды возврата ────────────────────────────────────────────────────── */

typedef enum {
    KB_OK = 0,
    /* Данных меньше, чем нужно: обрезанный заголовок, тело или LZ4-поток. */
    KB_ERR_TRUNCATED = -1,
    /* Выходной буфер мал. Дай больше и повтори. */
    KB_ERR_TOO_LARGE = -2,
    /* Нулевой указатель или бессмысленные аргументы. */
    KB_ERR_INVALID = -3,
    /* LZ4: нулевое смещение матча. */
    KB_ERR_LZ4_ZERO_OFFSET = -4,
    /* LZ4: смещение указывает до начала выхода. */
    KB_ERR_LZ4_OFFSET_RANGE = -5,
    /* Тело сжато форматом, которого это ядро пока не умеет (zstd, LZ4 frame). */
    KB_ERR_UNSUPPORTED = -6,
    /* Реассемблер переполнен: поток не бьётся на пакеты. */
    KB_ERR_OVERFLOW = -7,
    /* Запрошенного ключа в map нет. Это не повреждение данных. */
    KB_ERR_NOT_FOUND = -8,
    /*
     * Данные распаковываются во что-то крупнее KB_MAX_DECOMPRESSED_SIZE.
     * В отличие от KB_ERR_TOO_LARGE, повтор с бОльшим буфером не поможет:
     * это потолок протокола, а не размер буфера.
     */
    KB_ERR_LIMIT = -9,
    /* Ответ на запрос не пришёл в отведённое время. */
    KB_ERR_TIMEOUT = -10,
    /* Соединение оборвалось; запрос остался без ответа. */
    KB_ERR_CLOSED = -11,
    /* Действие не имеет смысла в текущем состоянии сессии. */
    KB_ERR_STATE = -12
} kb_status;

/* Короткое описание кода. Всегда непустая строка, освобождать не нужно. */
const char *kb_status_str(kb_status status);

/* ─── Заголовок ────────────────────────────────────────────────────────── */

typedef struct {
    uint8_t ver;
    uint8_t cmd;
    uint16_t seq;
    uint16_t opcode;
    /* Старший байт packedLen. Ненулевой = тело сжато; само значение —
       подсказка о степени сжатия, на разбор не влияет. */
    uint8_t flag;
    /* Длина тела на проводе, до распаковки. */
    uint32_t body_len;
} kb_header;

/* Разобрать 10 байт заголовка. Нужно ровно KB_HEADER_SIZE доступных байт. */
kb_status kb_header_decode(const uint8_t *src, size_t len, kb_header *out);

/* Записать заголовок. Поля ver/cmd/seq/opcode/flag/body_len берутся как есть. */
kb_status kb_header_encode(const kb_header *header, uint8_t *out, size_t out_cap);

/*
 * Полная длина пакета (заголовок + тело), объявленная в начале буфера.
 * Возвращает 0, если доступно меньше KB_HEADER_SIZE байт.
 */
size_t kb_packet_total_len(const uint8_t *src, size_t len);

/* ─── Кодирование ──────────────────────────────────────────────────────── */

/* Верхняя оценка размера пакета: столько байт хватит kb_encode гарантированно. */
size_t kb_encode_bound(size_t payload_len);

/*
 * Собрать пакет из готового MessagePack-тела.
 *
 * Тело длиной от KB_COMPRESSION_THRESHOLD сжимается LZ4-block, и в флаг
 * попадает подсказка (payload_len / compressed_len) + 1 — ровно как в
 * референсном ядре.
 *
 * out_len можно передать NULL, если длина не нужна.
 */
kb_status kb_encode(uint8_t cmd, uint16_t opcode, uint16_t seq,
                    const uint8_t *payload, size_t payload_len,
                    uint8_t *out, size_t out_cap, size_t *out_len);

/* ─── Декодирование ────────────────────────────────────────────────────── */

/*
 * Разобрать целый пакет: заголовок в header, распакованное тело в out.
 *
 * header и out можно передавать NULL по отдельности, если нужна только одна
 * часть. Если тела не хватило, вернётся KB_ERR_TRUNCATED; если out мал —
 * KB_ERR_TOO_LARGE (для несжатого тела нужный размер уже лежит в
 * header->body_len, для сжатого — пробуй больше).
 */
kb_status kb_decode(const uint8_t *packet, size_t packet_len, kb_header *header,
                    uint8_t *out, size_t out_cap, size_t *out_len);

/* ─── LZ4 block ────────────────────────────────────────────────────────── */

/* Верхняя оценка размера сжатого блока. */
size_t kb_lz4_compress_bound(size_t src_len);

kb_status kb_lz4_compress(const uint8_t *src, size_t src_len, uint8_t *dst,
                          size_t dst_cap, size_t *dst_len);

/*
 * Распаковать LZ4-block: без заголовка кадра и без префикса длины, ровно то,
 * что кладёт в тело сервер.
 *
 * KB_ERR_TOO_LARGE — не хватило dst_cap, стоит повторить с бОльшим буфером.
 * KB_ERR_LIMIT — выход перевалил за KB_MAX_DECOMPRESSED_SIZE, повтор бесполезен.
 */
kb_status kb_lz4_decompress(const uint8_t *src, size_t src_len, uint8_t *dst,
                            size_t dst_cap, size_t *dst_len);

/*
 * Распаковщик zstd, который ставит хост.
 *
 * Ядро намеренно не тянет libzstd: как и с TLS, формат подключается снаружи.
 * Готовый адаптер — src/kolibri_zstd.c, собирается при KB_WITH_ZSTD и
 * включается вызовом kb_zstd_enable().
 *
 * Ставить один раз до начала работы: указатель глобальный и не защищён от
 * одновременной установки из нескольких потоков.
 */
typedef kb_status (*kb_zstd_fn)(const uint8_t *src, size_t src_len,
                                uint8_t *dst, size_t dst_cap, size_t *dst_len);

void kb_set_zstd_decompressor(kb_zstd_fn decompressor);
kb_zstd_fn kb_get_zstd_decompressor(void);

/*
 * Распаковать тело пакета по флагу из заголовка — так же, как это делает
 * официальный клиент. Именно этим пользуется kb_decode.
 *
 * KB_ERR_UNSUPPORTED означает, что тело сжато zstd, а распаковщик не поставлен.
 */
kb_status kb_decompress_body(uint8_t flag, const uint8_t *src, size_t src_len,
                             uint8_t *dst, size_t dst_cap, size_t *dst_len);

/*
 * Распаковать, определив формат по магическому числу: zstd (28 B5 2F FD),
 * LZ4 frame (04 22 4D 18) или, если магии нет, LZ4 block.
 *
 * Протокол опирается на флаг, а не на магию, поэтому для разбора пакетов нужна
 * kb_decompress_body. Эта функция остаётся для данных вне протокола.
 * LZ4 frame ядро не умеет: в протоколе он не встречается — проверено по
 * официальному клиенту, там frame-классы есть в библиотеке, но не вызываются.
 */
kb_status kb_decompress(const uint8_t *src, size_t src_len, uint8_t *dst,
                        size_t dst_cap, size_t *dst_len);

/* ─── Реассемблер потока ───────────────────────────────────────────────── */

/*
 * Собирает поток байт из сокета обратно в пакеты. Буфер даёт вызывающая
 * сторона и владеет им сама.
 *
 *   kb_rx rx;
 *   kb_rx_init(&rx, buffer, sizeof buffer);
 *   kb_rx_push(&rx, chunk, n);
 *   while ((packet = kb_rx_next(&rx, &len)) != NULL) { ... }
 *
 * Указатель, возвращённый kb_rx_next, живёт до следующего kb_rx_push или
 * kb_rx_reset.
 */
typedef struct {
    uint8_t *buf;
    size_t cap;
    /* Смещение первого неразобранного байта. */
    size_t start;
    /* Смещение за последним записанным байтом. */
    size_t end;
} kb_rx;

kb_status kb_rx_init(kb_rx *rx, uint8_t *buffer, size_t capacity);

/* Дописать кусок из сокета. KB_ERR_OVERFLOW — буфер не вмещает пакет. */
kb_status kb_rx_push(kb_rx *rx, const uint8_t *data, size_t len);

/* Следующий целый пакет или NULL, если пока не набралось. */
const uint8_t *kb_rx_next(kb_rx *rx, size_t *packet_len);

/* Сколько байт лежит неразобранными. */
size_t kb_rx_buffered(const kb_rx *rx);

void kb_rx_reset(kb_rx *rx);

#if defined(__cplusplus)
}
#endif

/* ─── SHA-256 ──────────────────────────────────────────────────────────── */

/*
 * Нужен протоколу ровно для одного: anti-spoof fingerprint при авторизации.
 * Ради этого тащить криптостек незачем, поэтому реализация своя.
 */
#define KB_SHA256_SIZE 32

typedef struct {
    uint32_t state[8];
    uint64_t length;
    uint8_t buffer[64];
    size_t buffered;
} kb_sha256;

void kb_sha256_init(kb_sha256 *ctx);
void kb_sha256_update(kb_sha256 *ctx, const uint8_t *data, size_t len);
/* Забирает результат и затирает состояние. */
void kb_sha256_final(kb_sha256 *ctx, uint8_t out[KB_SHA256_SIZE]);

/* Всё сразу, если данные уже целиком в памяти. */
void kb_sha256_hash(const uint8_t *data, size_t len,
                    uint8_t out[KB_SHA256_SIZE]);

/* ─── Anti-spoof fingerprint ───────────────────────────────────────────── */

#define KB_FINGERPRINT_SIZE 96

/*
 * Дайджесты сборки приложения. Считает их хост: они зависят от конкретного
 * APK, и ядру взять их неоткуда.
 *
 * Как они получаются в официальном клиенте:
 *   signature — sha256 DER-сертификата подписи (схема v2/v3, не META-INF);
 *   so        — sha256 содержимого библиотек lib/<abi>, порядок по имени;
 *   dex       — sha256 первых 0x14 байт каждого .dex, порядок по имени.
 *
 * Дайджесты и версия приложения в handshake — один комплект: смешаешь от
 * разных сборок, и сервер отвергнет fingerprint.
 */
typedef struct {
    const uint8_t *signature;
    size_t signature_len;
    const uint8_t *so;
    size_t so_len;
    const uint8_t *dex;
    size_t dex_len;
} kb_app_digests;

/*
 * Собрать 96-байтный fingerprint: поле `mode` в опкоде 17 и
 * `chatCacheFingerprint` в опкоде 19.
 *
 * calls_seed приходит в ответе на handshake.
 */
kb_status kb_chat_cache_fingerprint(const kb_app_digests *digests,
                                    int64_t calls_seed, const char *device_id,
                                    uint8_t out[KB_FINGERPRINT_SIZE]);

/* Отдельные модули, но один публичный заголовок на всё. */
#include "kolibri_msgpack.h"
#include "kolibri_opcodes.h"
#include "kolibri_proxy.h"
#include "kolibri_session.h"

#endif /* KOLIBRI_H */
