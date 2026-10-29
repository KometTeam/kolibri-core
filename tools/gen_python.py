#!/usr/bin/env python3
"""schema/*.toml -> python/kolibri_ops.py

    python3 tools/gen_python.py
    python3 tools/gen_python.py --check

Типизированный слой поверх схемы: по методу на операцию. Вызываешь
`client.auth_request(phone=..., type=...)` — имена полей проверяются по схеме,
опкод подставляется сам, bytes-поля сами оборачиваются в {"$bin": ...}, а ответ
приходит объектом с атрибутами-полями. Никаких ручных пейлоадов и номеров.
"""

import pathlib
import sys
import tomllib

ROOT = pathlib.Path(__file__).resolve().parent.parent
SCHEMA = ROOT / "schema"
OUT = ROOT / "python" / "kolibri_ops.py"

PYKW = {"from", "import", "class", "def", "return", "for", "in", "and", "or",
        "not", "if", "else", "elif", "while", "with", "as", "lambda", "global",
        "None", "True", "False", "pass", "yield", "del", "raise", "try",
        "except", "finally", "is", "assert", "async", "await", "nonlocal"}


def snake(name):
    out = []
    for i, ch in enumerate(name):
        if ch.isupper() and i > 0 and (name[i - 1].islower() or name[i - 1].isdigit()):
            out.append("_")
        out.append(ch.lower())
    return "".join(out)


def base_type(spec):
    t = spec if isinstance(spec, str) else spec["type"]
    return t


def load():
    ops = {}
    for path in sorted(SCHEMA.glob("*.toml")):
        if path.name == "opcodes.toml":
            continue
        data = tomllib.loads(path.read_text())
        for name, body in (data.get("ops") or {}).items():
            ops[name] = {
                "opcode": body["opcode"],
                "wire": body.get("wire", name),
                "doc": (body.get("doc") or "").strip().splitlines()[0:1],
                "request": {k: base_type(v) for k, v in (body.get("request") or {}).items()},
                "response": list((body.get("response") or {}).keys()),
            }
    return ops


HEADER = '''"""@generated tools/gen_python.py -- не редактировать руками.

Типизированный слой над схемой протокола. Подмешивается в kolibri_core.Connection:
у клиента появляется по методу на операцию.

    with kb.Connection("api.oneme.ru", device_id=..., user_agent=ua) as c:
        c.handshake()
        r = c.auth_request(phone="+7...", type="START_AUTH", mode=fp)  # fp: bytes
        token = r.token

Имена полей сверяются со схемой (опечатка -> TypeError), bytes-поля сами уходят
как {"$bin": base64}, ответ -- объект с атрибутами-полями (плюс .raw с сырым dict).
"""

import base64
import types


def _build(op, spec, fields):
    out = {}
    for k, v in fields.items():
        if k not in spec:
            raise TypeError("%s: неизвестное поле %r; есть: %s"
                            % (op, k, ", ".join(spec) or "нет"))
        if v is None:
            continue
        if spec[k].rstrip("?") == "bytes" and isinstance(v, (bytes, bytearray)):
            v = {"$bin": base64.b64encode(bytes(v)).decode()}
        out[k] = v
    for k, t in spec.items():
        if not t.endswith("?") and k not in out:
            raise TypeError("%s: не хватает обязательного поля %r" % (op, k))
    return out


def _resp(d, names):
    obj = types.SimpleNamespace()
    for n in names:
        setattr(obj, n, None)
    if isinstance(d, dict):
        for k, v in d.items():
            if isinstance(k, str) and k.isidentifier():
                setattr(obj, k, v)
    obj.raw = d
    return obj


class Ops(object):
    """Методы-операции. Требует self.request(opcode, dict) -> dict."""
'''


def generate(ops):
    lines = [HEADER]
    opcodes = {}
    for name in sorted(ops, key=lambda n: ops[n]["opcode"]):
        op = ops[name]
        method = snake(name)
        if method in PYKW:
            method += "_"
        opcodes[op["wire"]] = op["opcode"]
        req = op["request"]
        # сигнатура-подсказка в докстроке
        flds = ", ".join("%s:%s" % (k, t) for k, t in req.items()) or "—"
        doc = op["doc"][0] if op["doc"] else ""
        lines.append("")
        lines.append("    def %s(self, **fields):" % method)
        lines.append('        """%s (%d). Поля: %s.%s"""'
                     % (op["wire"], op["opcode"], flds,
                        (" " + doc) if doc else ""))
        lines.append("        _spec = %r" % req)
        lines.append("        d = self.request(%d, _build(%r, _spec, fields))"
                     % (op["opcode"], name))
        lines.append("        return _resp(d, %r)" % (op["response"],))

    lines.append("")
    lines.append("")
    lines.append("# Опкод по wire-имени, если нужен номер напрямую.")
    lines.append("OPCODES = %r" % dict(sorted(opcodes.items())))
    lines.append("")
    return "\n".join(lines)


def main():
    ops = load()
    text = generate(ops)
    if "--check" in sys.argv:
        current = OUT.read_text() if OUT.exists() else ""
        if current != text:
            print("kolibri_ops.py устарел — запусти python3 tools/gen_python.py")
            sys.exit(1)
        print("kolibri_ops.py актуален (%d операций)" % len(ops))
        return
    OUT.write_text(text)
    print("сгенерировано операций: %d" % len(ops))


if __name__ == "__main__":
    main()
