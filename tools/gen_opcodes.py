#!/usr/bin/env python3
"""schema/opcodes.toml -> include/kolibri_opcodes.h + src/kolibri_opcodes.c

    python3 tools/gen_opcodes.py
    python3 tools/gen_opcodes.py --check   # для CI: не отстало ли

Результат коммитится: пользователю ядра генератор не нужен, а изменения
таблицы видно в ревью.

Поиск имени по номеру делается двоичным поиском по отсортированной таблице —
switch на 168 веток компилятор превратил бы в такую же таблицу, только
руками её потом не почитать.
"""

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
SRC = ROOT / "schema" / "opcodes.toml"
OUT_H = ROOT / "include" / "kolibri_opcodes.h"
OUT_C = ROOT / "src" / "kolibri_opcodes.c"

BANNER = """/*
 * @generated tools/gen_opcodes.py — не редактировать руками.
 *
 * Источник: schema/opcodes.toml
 * Перегенерация: python3 tools/gen_opcodes.py
 */
"""


def parse():
    """Возвращает [(секция, имя, номер)] в порядке файла."""
    ops, section = [], "прочее"
    for line in SRC.read_text().splitlines():
        head = re.match(r"#\s*──\s*(.+?)\s*──", line)
        if head:
            section = head.group(1).strip()
            continue
        entry = re.match(r"([A-Z0-9_]+)\s*=\s*(\d+)\s*$", line)
        if entry:
            ops.append((section, entry.group(1), int(entry.group(2))))
    return ops


def emit_header(ops):
    out = [BANNER, """
#ifndef KOLIBRI_OPCODES_H
#define KOLIBRI_OPCODES_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif
"""]
    section = None
    for sec, name, value in ops:
        if sec != section:
            section = sec
            out.append("\n/* ── %s ── */" % sec)
        out.append("#define KB_OP_%-32s %d" % (name, value))

    out.append("""
/* Сколько опкодов знает таблица. */
#define KB_OPCODE_COUNT %d

/*
 * Имя опкода или NULL, если он таблице неизвестен.
 * Строка статическая, освобождать не нужно.
 */
const char *kb_opcode_name(uint16_t opcode);

/* Номер по имени; 0, если имя неизвестно (нулевого опкода в протоколе нет). */
uint16_t kb_opcode_by_name(const char *name);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_OPCODES_H */""" % len(ops))
    return "\n".join(out) + "\n"


def emit_source(ops):
    by_value = sorted(ops, key=lambda o: o[2])
    rows = ",\n".join('    { %d, "%s" }' % (v, n) for _, n, v in by_value)
    return BANNER + '''
#include <string.h>

#include "kolibri_opcodes.h"

typedef struct {
    uint16_t value;
    const char *name;
} kb_opcode_entry;

/* Отсортировано по номеру: поиск двоичный. */
static const kb_opcode_entry kb_opcodes[] = {
%s
};

#define KB_OPCODE_TABLE_SIZE (sizeof kb_opcodes / sizeof kb_opcodes[0])

const char *kb_opcode_name(uint16_t opcode)
{
    size_t low = 0;
    size_t high = KB_OPCODE_TABLE_SIZE;

    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (kb_opcodes[mid].value == opcode) {
            return kb_opcodes[mid].name;
        }
        if (kb_opcodes[mid].value < opcode) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return NULL;
}

uint16_t kb_opcode_by_name(const char *name)
{
    size_t i;

    if (name == NULL) {
        return 0;
    }
    /* Имён немного и ищут по ним редко — линейного прохода достаточно. */
    for (i = 0; i < KB_OPCODE_TABLE_SIZE; i++) {
        if (strcmp(kb_opcodes[i].name, name) == 0) {
            return kb_opcodes[i].value;
        }
    }
    return 0;
}
''' % rows


def main():
    ops = parse()
    if not ops:
        sys.exit("в schema/opcodes.toml не нашлось ни одного опкода")

    seen = {}
    for _, name, value in ops:
        if value in seen:
            sys.exit("опкод %d занят дважды: %s и %s" % (value, seen[value], name))
        seen[value] = name

    header, source = emit_header(ops), emit_source(ops)

    if "--check" in sys.argv[1:]:
        stale = [p.name for p, want in ((OUT_H, header), (OUT_C, source))
                 if not p.exists() or p.read_text() != want]
        if stale:
            sys.exit("устарело: %s — выполни python3 tools/gen_opcodes.py"
                     % ", ".join(stale))
        print("опкоды актуальны (%d штук)" % len(ops))
        return

    OUT_H.write_text(header)
    OUT_C.write_text(source)
    print("сгенерировано %d опкодов" % len(ops))


if __name__ == "__main__":
    main()
