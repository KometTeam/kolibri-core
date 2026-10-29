#!/usr/bin/env python3
"""schema/*.toml -> include/kolibri_ops.h + src/kolibri_ops.c

    python3 tools/gen_schema.py
    python3 tools/gen_schema.py --check

Из описания операции получаются структура, кодировщик и разборщик. Пишутся
они один раз здесь, а не 168 раз руками -- ровно ради этого схема и заводилась.

Отображение типов выбрано под sans-IO: разбор ничего не копирует, строки и
бинарники указывают внутрь исходного буфера и живут ровно столько же.

    string  -> kb_str    данные и длина
    i64/i32 -> int64_t / int32_t
    bool    -> int
    f64     -> double
    bytes   -> kb_bin
    any     -> kb_raw    сырой кусок msgpack
    [T] {T} -> kb_raw + count: элементы обходит вызывающий, курсором
    Name    -> kb_raw:   вложенное разбирается своей функцией
    T?      -> поле плюс has_<имя>

Массивы и вложенные map намеренно не разворачиваются в структуры: без
аллокаций пришлось бы гадать про максимальные размеры, а курсор из
kolibri_msgpack.h обходит их без единого лишнего байта.
"""

import pathlib
import sys
import tomllib

ROOT = pathlib.Path(__file__).resolve().parent.parent
SCHEMA = ROOT / "schema"
OUT_H = ROOT / "include" / "kolibri_ops.h"
OUT_C = ROOT / "src" / "kolibri_ops.c"
OUT_API_H = ROOT / "include" / "kolibri_api.h"
OUT_API_C = ROOT / "src" / "kolibri_api.c"

BANNER = (
    "/*\n"
    " * @generated tools/gen_schema.py -- не редактировать руками.\n"
    " *\n"
    " * Источник: файлы schema, расширение toml\n"
    " * Перегенерация: python3 tools/gen_schema.py\n"
    " */\n"
)

# Ключевые слова C и C++. Заголовок объявлен extern "C" и вполне может попасть
# в C++-код, поэтому экранируются и слова, зарезервированные только там:
# class, new, template, operator и прочие. Поле "type" ключевым не является
# ни в одном из языков и остаётся как есть.
KEYWORDS = {"auto", "break", "case", "char", "const", "continue", "default",
            "do", "double", "else", "enum", "extern", "float", "for", "goto",
            "if", "int", "long", "register", "return", "short", "signed",
            "sizeof", "static", "struct", "switch", "typedef", "union",
            "unsigned", "void", "volatile", "while",
            "bool", "catch", "class", "delete", "explicit", "export", "false",
            "friend", "inline", "mutable", "namespace", "new", "operator",
            "private", "protected", "public", "template", "this", "throw",
            "true", "try", "typename", "using", "virtual"}


def snake(name):
    out = []
    for i, ch in enumerate(name):
        if ch.isupper():
            prev_lower = i > 0 and (name[i - 1].islower() or name[i - 1].isdigit())
            next_lower = i + 1 < len(name) and name[i + 1].islower()
            prev_upper = i > 0 and name[i - 1].isupper()
            if prev_lower or (prev_upper and next_lower):
                out.append("_")
            out.append(ch.lower())
        else:
            out.append(ch)
    result = "".join(out)
    return result + "_" if result in KEYWORDS else result


class Ty:
    def __init__(self, raw):
        self.optional = raw.endswith("?")
        base = raw[:-1] if self.optional else raw
        if base.startswith("[") and base.endswith("]"):
            self.kind = "array"
        elif base.startswith("{") and base.endswith("}"):
            self.kind = "map"
        elif base in ("string", "i64", "i32", "f64", "bool", "bytes", "any"):
            self.kind = base
        else:
            self.kind = "named"

    @property
    def c_type(self):
        return {"string": "kb_str", "i64": "int64_t", "i32": "int32_t",
                "f64": "double", "bool": "int", "bytes": "kb_bin",
                "any": "kb_raw", "array": "kb_raw", "map": "kb_raw",
                "named": "kb_raw"}[self.kind]

    @property
    def counted(self):
        return self.kind in ("array", "map")

    @property
    def is_raw(self):
        return self.kind in ("array", "map", "named", "any")


def field_type(spec):
    return Ty(spec if isinstance(spec, str) else spec["type"])


def load():
    ops = {}
    types = {}
    for path in sorted(SCHEMA.glob("*.toml")):
        if path.name == "opcodes.toml":
            continue
        data = tomllib.loads(path.read_text())
        for tname, tbody in (data.get("types") or {}).items():
            types[tname] = {
                "doc": (tbody.get("doc") or "").strip(),
                "fields": {k: field_type(v)
                           for k, v in (tbody.get("fields") or {}).items()},
            }
        for name, body in (data.get("ops") or {}).items():
            ops[name] = {
                "opcode": body["opcode"],
                "wire": body.get("wire", ""),
                "doc": (body.get("doc") or "").strip(),
                "request": {k: field_type(v)
                            for k, v in (body.get("request") or {}).items()},
                "response": {k: field_type(v)
                             for k, v in (body.get("response") or {}).items()},
            }
    return ops, types


def count_name(field, names):
    """Имя companion-поля с числом элементов. Обычно <field>_count, но если оно
    сталкивается с реальным полем (напр. participants + participantsCount) — _len."""
    for suffix in ("_count", "_len", "_n"):
        cand = field + suffix
        if cand not in names:
            return cand
    return field + "_count_"


def emit_struct(name, fields, doc):
    out = []
    names = {snake(w) for w in fields}
    if doc:
        out.append("/*")
        for line in doc.splitlines():
            out.append(" * " + line if line.strip() else " *")
        out.append(" */")
    out.append("typedef struct {")
    for wire, ty in fields.items():
        field = snake(wire)
        out.append("    /* " + wire + " */")
        out.append("    %s %s;" % (ty.c_type, field))
        if ty.counted:
            out.append("    size_t %s;" % count_name(field, names))
        if ty.optional:
            out.append("    int has_%s;" % field)
    if not fields:
        out.append("    int reserved; /* операция без полей */")
    out.append("} kb_%s;" % snake(name))
    return "\n".join(out)


def emit_encoder(name, fields):
    fn = snake(name)
    required = sum(1 for t in fields.values() if not t.optional)
    body = ["kb_status kb_encode_%s(const kb_%s *v, uint8_t *out, size_t cap,"
            % (fn, fn),
            "                       size_t *out_len)", "{",
            "    kb_mp_writer w;", "    size_t count = %d;" % required, "",
            "    if (v == NULL || out == NULL) {",
            "        return KB_ERR_INVALID;", "    }"]
    for wire, ty in fields.items():
        if ty.optional:
            body.append("    if (v->has_%s) { count++; }" % snake(wire))
    body.append("")
    body.append("    kb_mp_writer_init(&w, out, cap);")
    body.append("    kb_mp_write_map(&w, count);")

    for wire, ty in fields.items():
        field = snake(wire)
        pad = "    "
        if ty.optional:
            body.append("    if (v->has_%s) {" % field)
            pad = "        "
        body.append('%skb_mp_write_cstr(&w, "%s");' % (pad, wire))
        if ty.kind == "string":
            body.append("%skb_mp_write_str(&w, v->%s.data, v->%s.len);"
                        % (pad, field, field))
        elif ty.kind in ("i64", "i32"):
            body.append("%skb_mp_write_i64(&w, (int64_t)v->%s);" % (pad, field))
        elif ty.kind == "bool":
            body.append("%skb_mp_write_bool(&w, v->%s);" % (pad, field))
        elif ty.kind == "f64":
            body.append("%skb_mp_write_f64(&w, v->%s);" % (pad, field))
        elif ty.kind == "bytes":
            body.append("%skb_mp_write_bin(&w, v->%s.data, v->%s.len);"
                        % (pad, field, field))
        else:
            body.append("%s/* готовый msgpack вставляется как есть */" % pad)
            body.append("%skb_mp_write_raw(&w, v->%s.data, v->%s.len);"
                        % (pad, field, field))
        if ty.optional:
            body.append("    }")

    body += ["", "    if (kb_mp_writer_status(&w) != KB_OK) {",
             "        return kb_mp_writer_status(&w);", "    }",
             "    if (out_len != NULL) {",
             "        *out_len = kb_mp_writer_len(&w);", "    }",
             "    return KB_OK;", "}"]
    return "\n".join(body)


def emit_decoder(name, fields):
    names = {snake(w) for w in fields}
    fn = snake(name)
    body = ["kb_status kb_decode_%s(const uint8_t *body, size_t len, kb_%s *out)"
            % (fn, fn), "{",
            "    kb_mp_reader reader;", "    kb_mp_value head;",
            "    kb_mp_value key;",
            "    size_t i;", "    kb_status st;", "",
            "    if (out == NULL || (body == NULL && len != 0)) {",
            "        return KB_ERR_INVALID;", "    }",
            "    memset(out, 0, sizeof *out);", "",
            "    st = kb_mp_reader_init(&reader, body, len);",
            "    if (st != KB_OK) { return st; }",
            "    st = kb_mp_read(&reader, &head);",
            "    if (st != KB_OK) { return st; }",
            "    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }", "",
            "    for (i = 0; i < head.len; i++) {",
            "        size_t before = kb_mp_reader_pos(&reader);", "",
            "        st = kb_mp_read(&reader, &key);",
            "        if (st != KB_OK) { return st; }",
            "        if (key.type != KB_MP_STR) {",
            "            /* Ключ не строка: пропускаем пару целиком. */",
            "            reader.pos = before;",
            "            st = kb_mp_skip(&reader);",
            "            if (st != KB_OK) { return st; }",
            "            st = kb_mp_skip(&reader);",
            "            if (st != KB_OK) { return st; }",
            "            continue;", "        }", ""]

    first = True
    for wire, ty in fields.items():
        field = snake(wire)
        body.append("        %s (kb_mp_str_eq(&key, \"%s\")) {"
                    % ("if" if first else "} else if", wire))
        first = False
        if ty.is_raw:
            body.append("            size_t start = kb_mp_reader_pos(&reader);")
            body.append("            kb_mp_value value;")
            body.append("            st = kb_mp_read(&reader, &value);")
            body.append("            if (st != KB_OK) { return st; }")
            if ty.counted:
                body.append("            out->%s = value.len;"
                            % count_name(field, names))
            body.append("            reader.pos = start;")
            body.append("            st = kb_mp_skip(&reader);")
            body.append("            if (st != KB_OK) { return st; }")
            body.append("            out->%s.data = body + start;" % field)
            body.append("            out->%s.len = kb_mp_reader_pos(&reader) - start;"
                        % field)
        else:
            body.append("            kb_mp_value value;")
            body.append("            st = kb_mp_read(&reader, &value);")
            body.append("            if (st != KB_OK) { return st; }")
            if ty.kind == "string":
                body.append("            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }")
                body.append("            out->%s.data = (const char *)value.data;" % field)
                body.append("            out->%s.len = value.len;" % field)
            elif ty.kind in ("i64", "i32"):
                body.append("            if (value.type == KB_MP_INT) {")
                body.append("                out->%s = (%s)value.i;" % (field, ty.c_type))
                body.append("            } else if (value.type == KB_MP_UINT) {")
                body.append("                out->%s = (%s)value.u;" % (field, ty.c_type))
                body.append("            } else { return KB_ERR_INVALID; }")
            elif ty.kind == "bool":
                body.append("            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }")
                body.append("            out->%s = value.boolean;" % field)
            elif ty.kind == "f64":
                body.append("            if (value.type != KB_MP_FLOAT) { return KB_ERR_INVALID; }")
                body.append("            out->%s = value.f;" % field)
            elif ty.kind == "bytes":
                body.append("            if (value.type != KB_MP_BIN) { return KB_ERR_INVALID; }")
                body.append("            out->%s.data = value.data;" % field)
                body.append("            out->%s.len = value.len;" % field)
        if ty.optional:
            body.append("            out->has_%s = 1;" % field)

    if fields:
        body.append("        } else {")
    else:
        body.append("        {")
    body.append("            /* Полей, которых нет в схеме, сервер вправе")
    body.append("               прислать сколько угодно: молча пропускаем. */")
    body.append("            st = kb_mp_skip(&reader);")
    body.append("            if (st != KB_OK) { return st; }")
    body.append("        }")
    body.append("    }")
    body.append("    return KB_OK;")
    body.append("}")
    return "\n".join(body)


def generate(ops, types):
    head = [BANNER, "", "#ifndef KOLIBRI_OPS_H", "#define KOLIBRI_OPS_H", "",
            "#include <stddef.h>", "#include <stdint.h>", "",
            '#include "kolibri.h"', "",
            "#if defined(__cplusplus)", 'extern "C" {', "#endif", "",
            "/*", " * Строка msgpack: без завершающего нуля, указывает внутрь",
            " * исходного буфера и живёт ровно столько же.", " */",
            "typedef struct {", "    const char *data;", "    size_t len;",
            "} kb_str;", "",
            "/* Бинарное поле — так же, без копирования. */",
            "typedef struct {", "    const uint8_t *data;", "    size_t len;",
            "} kb_bin;", "",
            "/*", " * Кусок msgpack как есть: массив, map или вложенная структура.",
            " * Разбирается курсором из kolibri_msgpack.h, когда понадобится.",
            " */", "typedef struct {", "    const uint8_t *data;",
            "    size_t len;", "} kb_raw;", ""]

    src = [BANNER, "", "#include <string.h>", "",
           '#include "kolibri_ops.h"', ""]

    for name in sorted(ops):
        op = ops[name]
        doc = op["doc"] + ("\n\nОпкод %d" % op["opcode"])
        if op["wire"]:
            doc += " (" + op["wire"] + ")"
        doc += "."
        head.append(emit_struct(name, op["request"], doc))
        head.append("")
        head.append(emit_struct(name + "Response", op["response"],
                                "Ответ на " + name + "."))
        head.append("")
        head.append("#define KB_OPCODE_%s %d"
                    % (snake(name).upper(), op["opcode"]))
        head.append("")
        head.append("kb_status kb_encode_%s(const kb_%s *v, uint8_t *out,"
                    % (snake(name), snake(name)))
        head.append("                       size_t cap, size_t *out_len);")
        head.append("kb_status kb_decode_%s(const uint8_t *body, size_t len,"
                    % snake(name + "Response"))
        head.append("                       kb_%s *out);"
                    % snake(name + "Response"))
        head.append("")

        src.append(emit_encoder(name, op["request"]))
        src.append("")
        src.append(emit_decoder(name + "Response", op["response"]))
        src.append("")

    # Общие типы: структура + кодек на каждый [types.X]. В родительских
    # структурах именованный тип остаётся kb_raw (курсор), а тут — свой декодер.
    for name in sorted(types):
        t = types[name]
        head.append(emit_struct(name, t["fields"], t["doc"]))
        head.append("")
        head.append("kb_status kb_encode_%s(const kb_%s *v, uint8_t *out,"
                    % (snake(name), snake(name)))
        head.append("                       size_t cap, size_t *out_len);")
        head.append("kb_status kb_decode_%s(const uint8_t *body, size_t len,"
                    % snake(name))
        head.append("                       kb_%s *out);" % snake(name))
        head.append("")
        src.append(emit_encoder(name, t["fields"]))
        src.append("")
        src.append(emit_decoder(name, t["fields"]))
        src.append("")

    head += ["#if defined(__cplusplus)", "}", "#endif", "",
             "#endif /* KOLIBRI_OPS_H */"]
    return "\n".join(head) + "\n", "\n".join(src)



def generate_api(ops):
    """Типизированный слой вызовов: kb_session_<op> (async) и kb_client_<op>
    (блокирующий). Прячут опкод, кодирование, I/O и разбор ответа."""
    h = [BANNER, "", "#ifndef KOLIBRI_API_H", "#define KOLIBRI_API_H", "",
         '#include "kolibri_ops.h"', '#include "kolibri_session.h"',
         '#include "kolibri_client.h"', "",
         "#if defined(__cplusplus)", 'extern "C" {', "#endif", "",
         "/*", " * По методу на операцию. kb_session_<op> кодирует запрос из",
         " * типизированной структуры и шлёт его (I/O ведёшь сам, event-loop);",
         " * kb_call_<op> вдобавок сам качает I/O до ответа и разбирает его.",
         " * Пейлоады руками не пишешь. Ответ kb_call_<op> живёт до следующего",
         " * вызова (kb_str/kb_raw смотрят внутрь буфера клиента).",
         " */", ""]
    c = [BANNER, "", "#include <string.h>", "", '#include "kolibri_api.h"', ""]
    for name in sorted(ops, key=lambda n: ops[n]["opcode"]):
        op = ops[name]
        fn = snake(name)
        opc = "KB_OPCODE_%s" % fn.upper()
        has_req = bool(op["request"])
        sess_args = ("kb_session *s, const kb_%s *v, uint16_t *seq" % fn) if has_req \
                    else ("kb_session *s, uint16_t *seq")
        cli_args = ("kb_client *c, const kb_%s *v, kb_%s_response *resp" % (fn, fn)) if has_req \
                   else ("kb_client *c, kb_%s_response *resp" % fn)
        h.append("kb_status kb_send_%s(%s);" % (fn, sess_args))
        h.append("kb_status kb_call_%s(%s);" % (fn, cli_args))
        h.append("")
        # kb_session_<op>
        c.append("kb_status kb_send_%s(%s)" % (fn, sess_args))
        c.append("{")
        c.append("    size_t n = 0;")
        c.append("    kb_status st;")
        if not has_req:
            c.append("    kb_%s v;" % fn)
        c.append("    if (s == NULL%s) {" % ("" if not has_req else " || v == NULL"))
        c.append("        return KB_ERR_INVALID;")
        c.append("    }")
        if not has_req:
            c.append("    memset(&v, 0, sizeof v);")
            enc_arg = "&v"
        else:
            enc_arg = "v"
        c.append("    st = kb_encode_%s(%s, s->bufs.scratch, s->bufs.scratch_capacity, &n);"
                 % (fn, enc_arg))
        c.append("    if (st != KB_OK) {")
        c.append("        return st;")
        c.append("    }")
        c.append("    return kb_session_request(s, %s, s->bufs.scratch, n, seq);" % opc)
        c.append("}")
        c.append("")
        # kb_client_<op>
        c.append("kb_status kb_call_%s(%s)" % (fn, cli_args))
        c.append("{")
        c.append("    size_t n = 0, rlen = 0;")
        c.append("    const uint8_t *r = NULL;")
        c.append("    kb_status st;")
        if not has_req:
            c.append("    kb_%s v;" % fn)
        c.append("    if (c == NULL%s) {" % ("" if not has_req else " || v == NULL"))
        c.append("        return KB_ERR_INVALID;")
        c.append("    }")
        if not has_req:
            c.append("    memset(&v, 0, sizeof v);")
        c.append("    st = kb_encode_%s(%s, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);"
                 % (fn, enc_arg))
        c.append("    if (st != KB_OK) {")
        c.append("        return st;")
        c.append("    }")
        c.append("    st = kb_client_call(c, %s, c->session.bufs.scratch, n, &r, &rlen);" % opc)
        c.append("    if (st != KB_OK) {")
        c.append("        return st;")
        c.append("    }")
        c.append("    return resp != NULL ? kb_decode_%s_response(r, rlen, resp) : KB_OK;" % fn)
        c.append("}")
        c.append("")
    h += ["#if defined(__cplusplus)", "}", "#endif", "",
          "#endif /* KOLIBRI_API_H */"]
    return "\n".join(h) + "\n", "\n".join(c)


def main():
    ops, types = load()
    if not ops:
        sys.exit("в schema/ не нашлось ни одной операции")
    header, source = generate(ops, types)
    api_h, api_c = generate_api(ops)

    if "--check" in sys.argv[1:]:
        stale = [p.name for p, want in ((OUT_H, header), (OUT_C, source),
                                        (OUT_API_H, api_h), (OUT_API_C, api_c))
                 if not p.exists() or p.read_text() != want]
        if stale:
            sys.exit("устарело: %s -- выполни python3 tools/gen_schema.py"
                     % ", ".join(stale))
        print("kolibri_ops актуален (%d операций, %d типов)" % (len(ops), len(types)))
        return

    OUT_H.write_text(header)
    OUT_C.write_text(source)
    OUT_API_H.write_text(api_h)
    OUT_API_C.write_text(api_c)
    print("сгенерировано операций: %d, типов: %d (+ kolibri_api)" % (len(ops), len(types)))


if __name__ == "__main__":
    main()
