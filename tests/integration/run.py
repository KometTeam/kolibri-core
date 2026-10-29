#!/usr/bin/env python3
"""Интеграционная проверка транспорта: клиент на C против сервера на Python.

    make integration

Поднимает на localhost сервер, говорящий на протоколе, и — для проверки
прокси — фальшивые HTTP CONNECT и SOCKS5. Затем запускает собранный
build/client и сверяет его вывод.

Сервер разбирает пакеты вручную: тут нарочно не используется питоновская
обёртка, чтобы тест проверял C-код, а не сам себя.
"""

import os
import socket
import ssl
import struct
import subprocess
import sys
import tempfile
import threading

HEADER = 10
CMD_OK = 1
CMD_PUSH = 0
OPCODE_SESSION_INIT = 6
OPCODE_PING = 1
CALLS_SEED = 4242


def build_packet(cmd, opcode, seq, payload=b""):
    return (bytes([10, cmd]) + struct.pack(">H", seq) + struct.pack(">H", opcode)
            + struct.pack(">I", len(payload)) + payload)


def split_packet(buf):
    """Вернуть (header, payload, остаток) или None, если пакет не набрался."""
    if len(buf) < HEADER:
        return None
    body_len = struct.unpack(">I", buf[6:10])[0] & 0xFFFFFF
    total = HEADER + body_len
    if len(buf) < total:
        return None
    header = {
        "ver": buf[0],
        "cmd": buf[1],
        "seq": struct.unpack(">H", buf[2:4])[0],
        "opcode": struct.unpack(">H", buf[4:6])[0],
        "flag": buf[6],
        "body_len": body_len,
    }
    return header, buf[HEADER:total], buf[total:]


def msgpack_map(pairs):
    out = bytearray([0x80 | len(pairs)])
    for key, value in pairs:
        key = key.encode()
        out += bytes([0xA0 | len(key)]) + key
        if isinstance(value, int):
            if value <= 0xFFFF:
                out += bytes([0xCD]) + struct.pack(">H", value)
            else:
                out += bytes([0xCE]) + struct.pack(">I", value)
        else:
            data = value.encode()
            out += bytes([0xA0 | len(data)]) + data
    return bytes(out)


class ProtocolServer(threading.Thread):
    """Отвечает на handshake, шлёт пуш, отвечает на запрос."""

    def __init__(self, tls_files=None):
        super().__init__(daemon=True)
        self.sock = socket.socket()
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(("127.0.0.1", 0))
        self.sock.listen(1)
        self.port = self.sock.getsockname()[1]
        self.seen = []
        self.error = None
        self.tls_context = None
        if tls_files is not None:
            cert, key = tls_files
            self.tls_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
            self.tls_context.load_cert_chain(cert, key)

    def run(self):
        try:
            conn, _ = self.sock.accept()
            if self.tls_context is not None:
                conn = self.tls_context.wrap_socket(conn, server_side=True)
            with conn:
                buf = b""
                while True:
                    chunk = conn.recv(65536)
                    if not chunk:
                        return
                    buf += chunk
                    while True:
                        parsed = split_packet(buf)
                        if parsed is None:
                            break
                        header, payload, buf = parsed
                        self.seen.append(header)
                        self.handle(conn, header, payload)
        except OSError as error:
            self.error = error

    def handle(self, conn, header, payload):
        if header["opcode"] == OPCODE_SESSION_INIT:
            # Тело handshake длиннее порога, значит пришло сжатым.
            assert header["flag"] != 0, "handshake должен быть сжат"
            conn.sendall(build_packet(
                CMD_OK, OPCODE_SESSION_INIT, header["seq"],
                msgpack_map([("callsSeed", CALLS_SEED)])))
            # Пуш приходит без спроса и со своим seq.
            conn.sendall(build_packet(CMD_PUSH, 128, 999,
                                      msgpack_map([("type", "MSG")])))
        elif header["opcode"] == OPCODE_PING:
            pass  # пинг ответа не ждёт
        else:
            conn.sendall(build_packet(CMD_OK, header["opcode"], header["seq"],
                                      msgpack_map([("ok", 1)])))


def pipe(a, b):
    try:
        while True:
            data = a.recv(65536)
            if not data:
                break
            b.sendall(data)
    except OSError:
        pass
    finally:
        for s in (a, b):
            try:
                s.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass


class ProxyServer(threading.Thread):
    """Фальшивый прокси: HTTP CONNECT или SOCKS5, с авторизацией или без."""

    def __init__(self, kind, username=None, password=None):
        super().__init__(daemon=True)
        self.kind = kind
        self.username = username
        self.password = password
        self.sock = socket.socket()
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(("127.0.0.1", 0))
        self.sock.listen(1)
        self.port = self.sock.getsockname()[1]
        self.target = None
        self.error = None

    def run(self):
        try:
            conn, _ = self.sock.accept()
            if self.kind == "http":
                target = self.http_connect(conn)
            else:
                target = self.socks5(conn)
            if target is None:
                conn.close()
                return
            upstream = socket.create_connection(target)
            threading.Thread(target=pipe, args=(conn, upstream), daemon=True).start()
            threading.Thread(target=pipe, args=(upstream, conn), daemon=True).start()
        except OSError as error:
            self.error = error

    def http_connect(self, conn):
        buf = b""
        while b"\r\n\r\n" not in buf:
            chunk = conn.recv(4096)
            if not chunk:
                return None
            buf += chunk
        head = buf.split(b"\r\n")[0].decode()
        assert head.startswith("CONNECT "), head
        authority = head.split()[1]
        host, port = authority.rsplit(":", 1)

        if self.username is not None:
            assert b"Proxy-Authorization: Basic " in buf, "нет авторизации"
        conn.sendall(b"HTTP/1.1 200 Connection established\r\n\r\n")
        self.target = (host, int(port))
        return self.target

    def socks5(self, conn):
        greeting = conn.recv(8)
        assert greeting[0] == 5, greeting
        methods = set(greeting[2:2 + greeting[1]])

        if self.username is not None:
            assert 2 in methods, "клиент не предложил авторизацию"
            conn.sendall(bytes([5, 2]))
            head = conn.recv(2)
            assert head[0] == 1
            user = conn.recv(head[1]).decode()
            plen = conn.recv(1)[0]
            password = conn.recv(plen).decode()
            assert (user, password) == (self.username, self.password), (user, password)
            conn.sendall(bytes([1, 0]))
        else:
            conn.sendall(bytes([5, 0]))

        request = conn.recv(4)
        assert request[:2] == bytes([5, 1]), request
        assert request[3] == 3, "цель должна идти доменным именем"
        host_len = conn.recv(1)[0]
        host = conn.recv(host_len).decode()
        port = struct.unpack(">H", conn.recv(2))[0]

        conn.sendall(bytes([5, 0, 0, 1]) + bytes([127, 0, 0, 1])
                     + struct.pack(">H", port))
        self.target = (host, port)
        return self.target


def run_client(client, args, label):
    result = subprocess.run([client] + args, capture_output=True, text=True,
                            timeout=30)
    lines = [line for line in result.stdout.splitlines() if line]
    if result.returncode != 0:
        print("  вывод:", lines)
        print("  ошибки:", result.stderr.strip())
        raise SystemExit("%s: клиент вернул %d" % (label, result.returncode))
    return lines


def check(label, lines, server, via_proxy=False):
    required = [
        "connected via=proxy" if via_proxy else "connected",
        "state=connecting",
        "state=connected",
        "handshake callsSeed=%d" % CALLS_SEED,
        "state=online",
        "request seq=2",
        "done",
    ]
    for want in required:
        if want not in lines:
            print("  получено:", lines)
            raise SystemExit("%s: в выводе нет %r" % (label, want))

    if not any(line.startswith("response seq=2 opcode=64") for line in lines):
        print("  получено:", lines)
        raise SystemExit("%s: не пришёл ответ на запрос" % label)
    if not any(line.startswith("push opcode=128") for line in lines):
        print("  получено:", lines)
        raise SystemExit("%s: не пришёл пуш" % label)

    opcodes = [h["opcode"] for h in server.seen]
    if opcodes[:2] != [OPCODE_SESSION_INIT, 64]:
        raise SystemExit("%s: сервер увидел %r" % (label, opcodes))
    print("  %s: ок (%d строк вывода)" % (label, len(lines)))


def scenario(label, client, proxy=None):
    server = ProtocolServer()
    server.start()
    args = ["127.0.0.1", str(server.port)]

    if proxy is not None:
        proxy.start()
        credentials = ""
        if proxy.username is not None:
            credentials = "%s:%s@" % (proxy.username, proxy.password)
        scheme = "http" if proxy.kind == "http" else "socks5"
        args += ["--proxy", "%s://%s127.0.0.1:%d" % (scheme, credentials,
                                                     proxy.port)]

    lines = run_client(client, args, label)
    server.join(timeout=5)
    check(label, lines, server, via_proxy=proxy is not None)

    if proxy is not None:
        if proxy.error is not None:
            raise SystemExit("%s: прокси упал: %s" % (label, proxy.error))
        if proxy.target != ("127.0.0.1", server.port):
            raise SystemExit("%s: прокси вёл не туда: %r" % (label, proxy.target))


class InjectingProxy(threading.Thread):
    """Прокси, дописывающий свои байты сразу за ответом на CONNECT.

    Ровно то, чем аудит пробил TLS: прокси недоверенный по модели угроз, и
    открытый текст, приклеенный к его ответу, не должен становиться данными
    «от сервера».
    """

    def __init__(self, injection):
        super().__init__(daemon=True)
        self.injection = injection
        self.sock = socket.socket()
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(("127.0.0.1", 0))
        self.sock.listen(1)
        self.port = self.sock.getsockname()[1]
        self.error = None

    def run(self):
        try:
            conn, _ = self.sock.accept()
            buf = b""
            while b"\r\n\r\n" not in buf:
                chunk = conn.recv(4096)
                if not chunk:
                    return
                buf += chunk
            head = buf.split(b"\r\n")[0].decode()
            host, port = head.split()[1].rsplit(":", 1)
            # Ответ и подделка уходят одним сегментом.
            conn.sendall(b"HTTP/1.1 200 Connection established\r\n\r\n"
                         + self.injection)
            upstream = socket.create_connection((host, int(port)))
            threading.Thread(target=pipe, args=(conn, upstream), daemon=True).start()
            threading.Thread(target=pipe, args=(upstream, conn), daemon=True).start()
        except OSError as error:
            self.error = error


class PipeliningSocks5(threading.Thread):
    """SOCKS5, отправляющий оба своих ответа заранее, одним куском.

    Так делать нельзя, но сломанный или враждебный прокси может. Раньше хвост
    сегмента уезжал мимо машины рукопожатия и клиент висел до таймаута.
    """

    def __init__(self):
        super().__init__(daemon=True)
        self.sock = socket.socket()
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(("127.0.0.1", 0))
        self.sock.listen(1)
        self.port = self.sock.getsockname()[1]
        self.target_port = None
        self.error = None

    def run(self):
        try:
            conn, _ = self.sock.accept()
            greeting = conn.recv(8)
            assert greeting[0] == 5, greeting
            # Выбор метода и ответ на CONNECT — вместе, до самого запроса.
            conn.sendall(bytes([5, 0]) + bytes([5, 0, 0, 1, 127, 0, 0, 1])
                         + struct.pack(">H", self.target_port))
            request = conn.recv(4)
            host_len = conn.recv(1)[0]
            host = conn.recv(host_len).decode()
            conn.recv(2)
            assert request[:2] == bytes([5, 1]), request
            upstream = socket.create_connection((host, self.target_port))
            threading.Thread(target=pipe, args=(conn, upstream), daemon=True).start()
            threading.Thread(target=pipe, args=(upstream, conn), daemon=True).start()
        except OSError as error:
            self.error = error


def push_frame():
    """Кадр PUSH, который сервер не отправлял."""
    return build_packet(CMD_PUSH, 777, 999, msgpack_map([("fake", 1)]))


def injection_scenarios(client, with_tls):
    # 1. Без TLS хвост от прокси — законные данные туннеля и теряться не должен.
    server = ProtocolServer()
    server.start()
    proxy = InjectingProxy(push_frame())
    proxy.target = None
    proxy.start()
    lines = run_client(client, ["127.0.0.1", str(server.port), "--proxy",
                                "http://127.0.0.1:%d" % proxy.port],
                       "хвост прокси без TLS")
    server.join(timeout=5)
    if not any(line.startswith("push opcode=777") for line in lines):
        print("  получено:", lines)
        raise SystemExit("хвост прокси без TLS: данные туннеля потерялись")
    print("  хвост прокси без TLS доходит: ок")

    if not with_tls:
        print("  подделка под TLS пропущена: собери с OPENSSL")
        return

    # 2. С TLS тот же хвост — подделка, и соединение обязано быть отвергнуто.
    with tempfile.TemporaryDirectory() as directory:
        cert, key = make_self_signed(directory)
        server = ProtocolServer(tls_files=(cert, key))
        server.start()
        proxy = InjectingProxy(push_frame())
        proxy.start()
        result = subprocess.run(
            [client, "localhost", str(server.port), "--tls", "--ca", cert,
             "--proxy", "http://127.0.0.1:%d" % proxy.port],
            capture_output=True, text=True, timeout=30)
        if result.returncode == 0:
            print("  вывод:", result.stdout.splitlines())
            raise SystemExit("подделка под TLS: клиент принял соединение")
        if "push opcode=777" in result.stdout:
            raise SystemExit("подделка под TLS: подделанный кадр дошёл до приложения")
        print("  подделка под TLS отвергнута: ок")


def pipelining_scenario(client):
    server = ProtocolServer()
    server.start()
    proxy = PipeliningSocks5()
    proxy.target_port = server.port
    proxy.start()
    lines = run_client(client, ["127.0.0.1", str(server.port), "--proxy",
                                "socks5://127.0.0.1:%d" % proxy.port],
                       "SOCKS5 с конвейером")
    server.join(timeout=5)
    check("SOCKS5 с конвейером", lines, server, via_proxy=True)


def make_self_signed(directory):
    """Свой корень для проверки: ровно так же подключают корень Минцифры."""
    cert = os.path.join(directory, "cert.pem")
    key = os.path.join(directory, "key.pem")
    result = subprocess.run([
        "openssl", "req", "-x509", "-newkey", "rsa:2048", "-nodes",
        "-keyout", key, "-out", cert, "-days", "1",
        "-subj", "/CN=localhost",
        "-addext", "subjectAltName=DNS:localhost,IP:127.0.0.1",
    ], capture_output=True, text=True)
    if result.returncode != 0:
        raise SystemExit("openssl не сделал сертификат: %s" % result.stderr)
    return cert, key


def tls_scenario(client):
    with tempfile.TemporaryDirectory() as directory:
        cert, key = make_self_signed(directory)
        server = ProtocolServer(tls_files=(cert, key))
        server.start()

        # Имя берём то же, что в сертификате: проверка должна пройти честно.
        args = ["localhost", str(server.port), "--tls", "--ca", cert]
        lines = run_client(client, args, "поверх TLS")
        server.join(timeout=5)
        if "tls=ok" not in lines:
            print("  получено:", lines)
            raise SystemExit("поверх TLS: рукопожатие не состоялось")
        check("поверх TLS", lines, server)

        # А теперь без доверия к этому корню — соединение обязано не встать.
        server2 = ProtocolServer(tls_files=(cert, key))
        server2.start()
        result = subprocess.run(
            [client, "localhost", str(server2.port), "--tls"],
            capture_output=True, text=True, timeout=30)
        if result.returncode == 0:
            raise SystemExit("поверх TLS: клиент принял недоверенный сертификат")
        print("  недоверенный сертификат отвергнут: ок")


def main():
    if len(sys.argv) < 2:
        raise SystemExit("использование: run.py <путь к client> [--tls]")
    client = sys.argv[1]
    with_tls = "--tls" in sys.argv[2:]
    if not os.path.exists(client):
        raise SystemExit("не нашёл %s — собери `make client`" % client)

    print("интеграция транспорта:")
    scenario("напрямую", client)
    scenario("через HTTP CONNECT", client, ProxyServer("http"))
    scenario("через HTTP CONNECT с авторизацией", client,
             ProxyServer("http", "bob", "secret"))
    scenario("через SOCKS5", client, ProxyServer("socks5"))
    scenario("через SOCKS5 с авторизацией", client,
             ProxyServer("socks5", "bob", "secret"))
    pipelining_scenario(client)
    injection_scenarios(client, with_tls)
    if with_tls:
        tls_scenario(client)
    else:
        print("  TLS пропущен: собери `make client OPENSSL=<префикс>`")
    print("все сценарии прошли")


if __name__ == "__main__":
    main()
