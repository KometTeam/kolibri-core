/*
 * @generated tools/gen_vectors.py — не редактировать руками.
 *
 * Источник: tests/vectors.json, снятый с эталонной реализации kolibri-net.
 * Перегенерация: python3 tools/gen_vectors.py
 */

#ifndef KB_VECTORS_DATA_H
#define KB_VECTORS_DATA_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const uint8_t *data;
    size_t len;
} kb_blob;

typedef struct {
    const char *name;
    uint8_t cmd;
    uint16_t opcode;
    uint16_t seq;
    kb_blob payload;
    kb_blob packet;
} kb_vec_encode;

typedef struct {
    const char *name;
    kb_blob packet;
    uint8_t ver;
    uint8_t cmd;
    uint16_t seq;
    uint16_t opcode;
    uint8_t flag;
    uint32_t body_len;
    kb_blob payload;
} kb_vec_decode;

typedef struct {
    const char *name;
    kb_blob compressed;
    kb_blob plain;
} kb_vec_lz4;

typedef struct {
    const char *name;
    kb_blob input;
    /* "truncated" | "zero_offset" | "offset_range" | "limit" */
    const char *error;
} kb_vec_lz4_error;

typedef struct {
    int chunk;
    const kb_blob *chunks;
    size_t chunk_count;
    const kb_blob *packets;
    size_t packet_count;
} kb_vec_framing;

#define KB_VEC_MP_I64 0
#define KB_VEC_MP_U64 1
#define KB_VEC_MP_STR 2
#define KB_VEC_MP_BIN 3
#define KB_VEC_MP_BOOL 4
#define KB_VEC_MP_NIL 5

typedef struct {
    const char *name;
    int kind;
    int64_t i;
    uint64_t u;
    int b;
    /* содержимое строки или бинарника */
    kb_blob value;
    /* как это кодирует эталон */
    kb_blob bytes;
} kb_vec_msgpack;

/* Целый документ, собранный эталоном: handshake, ping. */
typedef struct {
    const char *name;
    kb_blob bytes;
} kb_vec_doc;

extern const kb_vec_encode kb_vectors_encode[];
extern const size_t kb_vectors_encode_count;

extern const kb_vec_decode kb_vectors_decode[];
extern const size_t kb_vectors_decode_count;

extern const kb_vec_lz4 kb_vectors_lz4[];
extern const size_t kb_vectors_lz4_count;

extern const kb_vec_lz4_error kb_vectors_lz4_error[];
extern const size_t kb_vectors_lz4_error_count;

extern const kb_vec_framing kb_vectors_framing[];
extern const size_t kb_vectors_framing_count;

extern const kb_vec_msgpack kb_vectors_msgpack[];
extern const size_t kb_vectors_msgpack_count;

extern const kb_vec_doc kb_vectors_doc[];
extern const size_t kb_vectors_doc_count;

#endif /* KB_VECTORS_DATA_H */
