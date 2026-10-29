/*
 * Загрузка медиа на CDN.
 *
 * Sans-IO, как и всё остальное: модуль строит HTTP-запрос и разбирает ответ, а
 * сокетом занимается хост — своим или через kolibri_transport.
 *
 * Полноценного HTTP-клиента тут нет и не нужно: CDN ждёт заголовки строго
 * определённой формы, и три режима загрузки отличаются именно ими.
 *
 *   файл  — один POST, Content-Range на всё тело;
 *   фото  — multipart/form-data, из ответа берут photoToken;
 *   видео — GET для узнавания смещения докачки, затем POST по кускам,
 *           с заголовком X-Uploading-Mode: parallel.
 */

#ifndef KOLIBRI_MEDIA_H
#define KOLIBRI_MEDIA_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define KB_URL_HOST_MAX 256
#define KB_URL_PATH_MAX 1024

typedef struct {
    int https;
    char host[KB_URL_HOST_MAX];
    uint16_t port;
    char path[KB_URL_PATH_MAX];
} kb_url;

/*
 * Разобрать `scheme://host[:port][/path]`. Схемы http и https; порт по
 * умолчанию 443 или 80. Пустой путь превращается в "/".
 */
kb_status kb_url_parse(const char *url, kb_url *out);

/*
 * Percent-кодирование как у Uri.encodeComponent в Dart: без изменений
 * проходят буквы, цифры и `-_.!~*'()`. Нужно для заголовка User-Agent, где
 * встречаются пробелы и скобки.
 *
 * Возвращает длину без нулевого байта, либо 0, если не хватило места.
 */
size_t kb_percent_encode(const char *input, char *out, size_t out_cap);

/* Тип содержимого по расширению имени файла; по умолчанию image/jpeg. */
const char *kb_content_type_for_filename(const char *filename);

/* ─── Построение запроса ───────────────────────────────────────────────── */

typedef struct {
    const char *name;
    const char *value;
} kb_http_header;

/*
 * Собрать строку запроса и заголовки, включая завершающую пустую строку.
 * Тело дописывает вызывающая сторона — его может и не быть в памяти целиком.
 */
kb_status kb_http_build_head(const char *method, const char *path,
                             const kb_http_header *headers, size_t count,
                             uint8_t *out, size_t cap, size_t *out_len);

/* ─── Разбор ответа ────────────────────────────────────────────────────── */

/*
 * Машина разбора ответа: корми байтами из сокета, пока не скажет «готово».
 * Понимает Content-Length, chunked и закрытие соединения как конец тела.
 *
 * Буфер даёт вызывающая сторона; в нём же остаётся тело, поэтому он должен
 * вмещать ответ целиком.
 */
typedef struct {
    uint8_t *buf;
    size_t cap;
    size_t len;

    int done;
    int header_parsed;
    size_t header_end;
    uint16_t status;
    /* -1 — заголовка не было */
    long content_length;
    int chunked;

    /* Тело после раскодирования chunked; указывает внутрь buf. */
    size_t body_at;
    size_t body_len;
} kb_http_response;

kb_status kb_http_response_init(kb_http_response *r, uint8_t *buffer,
                                size_t capacity);

/*
 * Скормить очередной кусок. Когда ответ дочитан, kb_http_response_done вернёт 1.
 *
 * Если сервер закрыл соединение, вызови kb_http_response_finish: для ответа без
 * Content-Length и без chunked именно закрытие и означает конец тела.
 */
kb_status kb_http_response_feed(kb_http_response *r, const uint8_t *data,
                                size_t len);

kb_status kb_http_response_finish(kb_http_response *r);

int kb_http_response_done(const kb_http_response *r);
uint16_t kb_http_response_status(const kb_http_response *r);
const uint8_t *kb_http_response_body(const kb_http_response *r, size_t *len);

/* ─── Заголовки трёх режимов загрузки ──────────────────────────────────── */

/*
 * Заголовки одиночной загрузки файла: Content-Range покрывает всё тело.
 * `user_agent` кодируется percent-кодированием сам.
 *
 * Заполняет массив headers (нужно не меньше KB_MEDIA_HEADERS_MAX элементов) и
 * пишет значения в storage. Возвращает количество через out_count.
 */
#define KB_MEDIA_HEADERS_MAX 10
#define KB_MEDIA_STORAGE_MIN 2048

kb_status kb_media_file_headers(const kb_url *url, const char *filename,
                                size_t body_len, const char *user_agent,
                                kb_http_header *headers, size_t *out_count,
                                char *storage, size_t storage_cap);

/*
 * Заголовки и обрамление multipart для фото.
 *
 * Тело = prologue + сами байты файла + epilogue, а Content-Length равен их
 * сумме. Граница генерируется из переданного числа: ядро не знает времени, и
 * подсовывать ему часы ради уникальности незачем.
 */
kb_status kb_media_photo_headers(const kb_url *url, const char *filename,
                                 size_t file_len, const char *user_agent,
                                 uint64_t boundary_seed, kb_http_header *headers,
                                 size_t *out_count, char *storage,
                                 size_t storage_cap, char *prologue,
                                 size_t prologue_cap, size_t *prologue_len,
                                 char *epilogue, size_t epilogue_cap,
                                 size_t *epilogue_len);

/*
 * Заголовки куска видео. content_range вида "bytes 0-1023/4096"; передай NULL
 * для GET-запроса, узнающего смещение докачки.
 */
kb_status kb_media_chunk_headers(const kb_url *url, const char *filename,
                                 size_t body_len, const char *content_range,
                                 kb_http_header *headers, size_t *out_count,
                                 char *storage, size_t storage_cap);

/* Собрать значение Content-Range: "bytes <from>-<to>/<total>". */
kb_status kb_media_content_range(uint64_t from, uint64_t to, uint64_t total,
                                 char *out, size_t cap);

/*
 * Раскладка большого файла на куски.
 *
 * Ядро не ходит в сеть и не заводит потоков: план лишь раздаёт диапазоны и
 * считает подтверждённые. Один поток вызывает next в цикле; несколько потоков
 * могут дёргать тот же план, если вызывающий сам защитит его замком, — так
 * получается та же параллельная загрузка, что и в эталоне.
 */
typedef struct {
    uint64_t total;       /* размер файла целиком */
    uint64_t chunk_size;  /* сколько байт в куске */
    uint64_t offset;      /* с чего начали: сервер мог принять часть раньше */
    uint64_t next;        /* первый ещё не розданный байт */
    uint64_t sent;        /* подтверждено сервером, включая offset */
} kb_media_plan;

/*
 * Разобрать ответ сервера на GET: тело — десятичное число уже принятых байт.
 * Мусор в теле или число больше файла означают «начинай сначала», а не ошибку:
 * так же ведёт себя официальный клиент.
 */
uint64_t kb_media_parse_resume(const uint8_t *body, size_t len, uint64_t total);

kb_status kb_media_plan_init(kb_media_plan *plan, uint64_t total,
                             uint64_t chunk_size, uint64_t start_offset);

/* Выдать следующий диапазон. 1 — выдан, 0 — раздавать больше нечего. */
int kb_media_plan_next(kb_media_plan *plan, uint64_t *from, uint64_t *to);

/* Отметить кусок принятым. Возвращает сколько байт залито всего. */
uint64_t kb_media_plan_ack(kb_media_plan *plan, uint64_t from, uint64_t to);

/* Весь файл подтверждён сервером. */
int kb_media_plan_done(const kb_media_plan *plan);

/* Код ответа, который CDN считает успехом для куска. */
int kb_media_status_ok(int status);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_MEDIA_H */
