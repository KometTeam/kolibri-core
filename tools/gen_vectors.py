#!/usr/bin/env python3
"""tests/vectors.json -> tests/vectors_data.{c,h}

Вектора снимаются с эталонной реализации (kolibri-net) отдельным дампером и
кладутся в репозиторий готовым JSON — поэтому здесь не нужен ни Rust, ни сеть.
Этот скрипт лишь превращает JSON в таблицы на C, чтобы тестам не понадобился
парсер JSON.

    python3 tools/gen_vectors.py
    python3 tools/gen_vectors.py --check   # для CI: убедиться, что не отстало
"""

import json
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
SRC = ROOT / "tests" / "vectors.json"
OUT_C = ROOT / "tests" / "vectors_data.c"
OUT_H = ROOT / "tests" / "vectors_data.h"

BANNER = """/*
 * @generated tools/gen_vectors.py — не редактировать руками.
 *
 * Источник: tests/vectors.json, снятый с эталонной реализации kolibri-net.
 * Перегенерация: python3 tools/gen_vectors.py
 */
"""

HEADER = BANNER + """
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
"""


class Blobs:
    """Собирает уникальные массивы байт и выдаёт на них ссылки."""

    def __init__(self):
        self.by_hex = {}
        self.lines = []

    def add(self, hex_str):
        if hex_str in self.by_hex:
            return self.by_hex[hex_str]
        data = bytes.fromhex(hex_str)
        name = "kb_b%d" % len(self.by_hex)
        self.by_hex[hex_str] = (name, len(data))
        body = []
        for i in range(0, max(len(data), 1), 12):
            row = data[i:i + 12]
            body.append("    " + " ".join("0x%02x," % b for b in row))
        if not data:
            # C89 не допускает пустой инициализатор; длина всё равно нулевая.
            body = ["    0x00"]
        self.lines.append("static const uint8_t %s[] = {\n%s\n};\n" % (name, "\n".join(body)))
        return self.by_hex[hex_str]

    def ref(self, hex_str):
        name, length = self.add(hex_str)
        return "{ %s, %d }" % (name, length)


def cstr(text):
    escaped = text.replace("\\", "\\\\").replace('"', '\\"')
    return '"%s"' % escaped


def generate(data):
    blobs = Blobs()
    parts = []

    enc = []
    for v in data["encode_plain"]:
        enc.append("    { %s, %d, %d, %d, %s, %s }," % (
            cstr(v["name"]), v["cmd"], v["opcode"], v["seq"],
            blobs.ref(v["payload"]), blobs.ref(v["packet"])))

    dec = []
    for v in data["decode"]:
        dec.append("    { %s, %s, %d, %d, %d, %d, %d, %du, %s }," % (
            cstr(v["name"]), blobs.ref(v["packet"]), v["ver"], v["cmd"],
            v["seq"], v["opcode"], v["flag"], v["body_len"],
            blobs.ref(v["payload"])))

    lz4 = []
    for v in data["lz4_block"]:
        lz4.append("    { %s, %s, %s }," % (
            cstr(v["name"]), blobs.ref(v["compressed"]), blobs.ref(v["plain"])))

    bad = []
    for v in data["lz4_error"]:
        bad.append("    { %s, %s, %s }," % (
            cstr(v["name"]), blobs.ref(v["input"]), cstr(v["error"])))

    KINDS = {"i64": 0, "u64": 1, "str": 2, "bin": 3, "bool": 4, "nil": 5}
    empty = blobs.ref("")
    mp = []
    for v in data["msgpack_scalar"]:
        kind = KINDS[v["kind"]]
        # -2^63 нельзя записать литералом: минус применяется к числу,
        # которое уже не влезает в int64_t.
        i = v.get("i", 0)
        i_lit = "(-9223372036854775807LL - 1)" if i == -(2 ** 63) else "%dLL" % i
        payload = blobs.ref(v["value"]) if v["kind"] in ("str", "bin") else empty
        mp.append("    { %s, %d, %s, %sULL, %d, %s, %s }," % (
            cstr(v["name"]), kind, i_lit, v.get("u", 0), v.get("b", 0),
            payload, blobs.ref(v["bytes"])))

    docs = []
    for v in data["msgpack_doc"]:
        docs.append("    { %s, %s }," % (cstr(v["name"]), blobs.ref(v["bytes"])))

    fr_tables = []
    fr = []
    for i, v in enumerate(data["framing"]):
        chunks = ",\n    ".join(blobs.ref(c) for c in v["chunks"])
        packets = ",\n    ".join(blobs.ref(p) for p in v["packets"])
        fr_tables.append("static const kb_blob kb_f%d_chunks[] = {\n    %s\n};\n" % (i, chunks))
        fr_tables.append("static const kb_blob kb_f%d_packets[] = {\n    %s\n};\n" % (i, packets))
        fr.append("    { %d, kb_f%d_chunks, %d, kb_f%d_packets, %d }," % (
            v["chunk"], i, len(v["chunks"]), i, len(v["packets"])))

    parts.append(BANNER)
    parts.append('\n#include "vectors_data.h"\n')
    parts.append("\n".join(blobs.lines))
    parts.append("\n".join(fr_tables))

    def table(kind, name, rows):
        return ("const %s %s[] = {\n%s\n};\nconst size_t %s_count = "
                "sizeof %s / sizeof %s[0];\n" % (
                    kind, name, "\n".join(rows), name, name, name))

    parts.append(table("kb_vec_encode", "kb_vectors_encode", enc))
    parts.append(table("kb_vec_decode", "kb_vectors_decode", dec))
    parts.append(table("kb_vec_lz4", "kb_vectors_lz4", lz4))
    parts.append(table("kb_vec_lz4_error", "kb_vectors_lz4_error", bad))
    parts.append(table("kb_vec_framing", "kb_vectors_framing", fr))
    parts.append(table("kb_vec_msgpack", "kb_vectors_msgpack", mp))
    parts.append(table("kb_vec_doc", "kb_vectors_doc", docs))

    return "\n".join(parts)


def main():
    check = "--check" in sys.argv[1:]
    data = json.loads(SRC.read_text())
    body = generate(data)

    if check:
        stale = []
        if not OUT_C.exists() or OUT_C.read_text() != body:
            stale.append(OUT_C.name)
        if not OUT_H.exists() or OUT_H.read_text() != HEADER:
            stale.append(OUT_H.name)
        if stale:
            sys.exit("устарело: %s — выполни python3 tools/gen_vectors.py"
                     % ", ".join(stale))
        print("vectors_data.{c,h} актуальны")
        return

    OUT_H.write_text(HEADER)
    OUT_C.write_text(body)
    counts = ", ".join("%s: %d" % (k, len(v)) for k, v in data.items())
    print("%s записан (%s)" % (OUT_C.name, counts))


if __name__ == "__main__":
    main()
