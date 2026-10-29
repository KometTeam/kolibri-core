"""Пример на Python: ядро через ctypes, сокет и TLS остаются питоновскими.

    make -C ../.. shared && python3 demo.py

Обёртка живёт в python/kolibri_core.py — это и есть «биндинг»: 300 строк на
ctypes, без единой строчки связующего кода на C.
"""

import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent.parent / "python"))

import kolibri_core as kb

# msgpack: {"chatId": 42}
SMALL = bytes([0x81, 0xA6]) + b"chatId" + bytes([0xCD, 0x00, 0x2A])
BIG = (b"kolibri" * 200)[:1400]


def main():
    first = kb.encode(opcode=64, seq=1, payload=SMALL)
    print("маленький пакет (%d Б): %s" % (len(first), first.hex()))

    second = kb.encode(opcode=64, seq=2, payload=BIG)
    print("большой пакет: тело %d Б -> на проводе %d Б" % (len(BIG), len(second)))

    stream = first + second
    rx = kb.Receiver(capacity=8192)

    print("\nпоток %d Б, кусками по 7:" % len(stream))
    for offset in range(0, len(stream), 7):
        rx.push(stream[offset:offset + 7])
        for raw in rx:
            header, body = kb.decode(raw)
            state = "сжато" if header.compressed else "как есть"
            print("  seq=%d opcode=%d %s, тело %d Б"
                  % (header.seq, header.opcode, state, len(body)))

    print("\nв буфере осталось: %d Б" % rx.buffered)


if __name__ == "__main__":
    main()
