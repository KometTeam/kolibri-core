"""Тесты обёртки на ctypes.

Гоняют те же вектора эталонной реализации, что и тесты на C, — так что если
обёртка перепутает типы аргументов или разметку структуры, это видно сразу.

    make shared && python3 -m unittest discover -s python -v
"""

import json
import pathlib
import socket
import unittest

import kolibri_core as kb

VECTORS = json.loads(
    (pathlib.Path(__file__).resolve().parent.parent / "tests" / "vectors.json").read_text()
)


def unhex(s):
    return bytes.fromhex(s)


class TestRoundTrip(unittest.TestCase):
    def test_small_payload_is_not_compressed(self):
        packet = kb.encode(opcode=64, seq=42, payload=b"hi")
        self.assertEqual(
            packet, bytes([0x0A, 0, 0, 42, 0, 64, 0, 0, 0, 2]) + b"hi"
        )
        header, body = kb.decode(packet)
        self.assertEqual(body, b"hi")
        self.assertFalse(header.compressed)
        self.assertEqual(header.opcode, 64)

    def test_large_payload_is_compressed(self):
        payload = b"kolibri" * 500
        packet = kb.encode(opcode=64, seq=1, payload=payload)
        header, body = kb.decode(packet)
        self.assertTrue(header.compressed)
        self.assertLess(header.body_len, len(payload))
        self.assertEqual(body, payload)

    def test_empty_payload(self):
        packet = kb.encode(opcode=1, seq=0)
        self.assertEqual(len(packet), kb.HEADER_SIZE)
        header, body = kb.decode(packet)
        self.assertEqual(body, b"")
        self.assertEqual(header.body_len, 0)

    def test_packet_total_len(self):
        packet = kb.encode(opcode=64, seq=1, payload=b"hello")
        self.assertEqual(kb.packet_total_len(packet), len(packet))
        self.assertEqual(kb.packet_total_len(packet[:5]), 0)

    def test_lz4_round_trip(self):
        for payload in (b"", b"a", b"a" * 5000, bytes(range(256)) * 20):
            self.assertEqual(kb.lz4_decompress(kb.lz4_compress(payload)), payload)

    def test_errors_surface_as_exceptions(self):
        with self.assertRaises(kb.KolibriError) as ctx:
            kb.lz4_decompress(bytes([0x00, 0x00, 0x00]))
        self.assertIn("zero match offset", str(ctx.exception))

    def test_max_out_is_actually_a_ceiling(self):
        # Регрессия на находку аудита: раньше стартовый буфер выделялся без
        # оглядки на max_out, и лимит не срабатывал, если ядро в него уложилось.
        compressed = kb.lz4_compress(b"A" * 3000)
        self.assertEqual(len(kb.lz4_decompress(compressed)), 3000)
        for limit in (1, 100, 2999):
            with self.subTest(limit=limit):
                with self.assertRaises(kb.KolibriError):
                    kb.lz4_decompress(compressed, max_out=limit)
        self.assertEqual(len(kb.lz4_decompress(compressed, max_out=3000)), 3000)
        with self.assertRaises(ValueError):
            kb.lz4_decompress(compressed, max_out=0)

    def test_unsupported_format(self):
        with self.assertRaises(kb.KolibriError):
            kb.decompress(bytes([0x28, 0xB5, 0x2F, 0xFD, 0x00]))


class TestReferenceVectors(unittest.TestCase):
    def test_encode_matches_reference(self):
        for case in VECTORS["encode_plain"]:
            with self.subTest(case["name"]):
                got = kb.encode(
                    opcode=case["opcode"],
                    seq=case["seq"],
                    payload=unhex(case["payload"]),
                    cmd=case["cmd"],
                )
                self.assertEqual(got, unhex(case["packet"]))

    def test_decode_matches_reference(self):
        for case in VECTORS["decode"]:
            with self.subTest(case["name"]):
                header, body = kb.decode(unhex(case["packet"]))
                self.assertEqual(header.ver, case["ver"])
                self.assertEqual(header.cmd, case["cmd"])
                self.assertEqual(header.seq, case["seq"])
                self.assertEqual(header.opcode, case["opcode"])
                self.assertEqual(header.flag, case["flag"])
                self.assertEqual(header.body_len, case["body_len"])
                self.assertEqual(body, unhex(case["payload"]))

    def test_lz4_blocks_from_reference(self):
        for case in VECTORS["lz4_block"]:
            with self.subTest(case["name"]):
                self.assertEqual(
                    kb.lz4_decompress(unhex(case["compressed"])),
                    unhex(case["plain"]),
                )


class TestReceiver(unittest.TestCase):
    def _stream(self):
        packets = [
            kb.encode(opcode=64 + i, seq=i, payload=bytes(range(i * 40 % 251)))
            for i in range(3)
        ]
        return packets, b"".join(packets)

    def test_byte_at_a_time(self):
        packets, stream = self._stream()
        rx = kb.Receiver()
        got = []
        for i in range(len(stream)):
            rx.push(stream[i:i + 1])
            got.extend(rx)
        self.assertEqual(got, packets)
        self.assertEqual(rx.buffered, 0)

    def test_all_at_once(self):
        packets, stream = self._stream()
        rx = kb.Receiver()
        rx.push(stream)
        self.assertEqual(list(rx), packets)

    def test_matches_reference_framing(self):
        for case in VECTORS["framing"]:
            with self.subTest(chunk=case["chunk"]):
                rx = kb.Receiver()
                got = []
                for chunk in case["chunks"]:
                    rx.push(unhex(chunk))
                    got.extend(rx)
                self.assertEqual(got, [unhex(p) for p in case["packets"]])
                self.assertEqual(rx.buffered, 0)

    def test_buffer_grows_for_large_packets(self):
        # Протокол допускает тело до 16 МБ, поэтому приёмник обязан дорасти,
        # а не сломаться на пакете крупнее начального буфера.
        payload = bytes(range(256)) * 2000  # 512 000 Б
        packet = kb.encode(opcode=64, seq=1, payload=payload)
        rx = kb.Receiver(capacity=1024)
        for offset in range(0, len(packet), 4096):
            rx.push(packet[offset:offset + 4096])
        got = list(rx)
        self.assertEqual(got, [packet])
        self.assertGreater(rx.capacity, 1024)
        header, body = kb.decode(got[0])
        self.assertEqual(body, payload)

    def test_overflow_past_max_capacity(self):
        rx = kb.Receiver(capacity=32, max_capacity=64)
        rx.push(b"x" * 20)
        with self.assertRaises(kb.KolibriError):
            rx.push(b"x" * 100)

    def test_reset(self):
        rx = kb.Receiver()
        rx.push(b"12345")
        self.assertEqual(rx.buffered, 5)
        rx.reset()
        self.assertEqual(rx.buffered, 0)


def msgpack_map(pairs):
    """Крошечный кодировщик msgpack для тестового «сервера»."""
    out = bytearray([0x80 | len(pairs)])
    for key, value in pairs:
        key = key.encode("utf-8")
        out += bytes([0xA0 | len(key)]) + key
        if isinstance(value, bool):
            out += bytes([0xC3 if value else 0xC2])
        elif isinstance(value, int):
            if 0 <= value < 0x80:
                out += bytes([value])
            elif value <= 0xFFFF:
                out += bytes([0xCD, value >> 8, value & 0xFF])
            else:
                out += bytes([0xCE]) + value.to_bytes(4, "big")
        else:
            data = value.encode("utf-8")
            out += bytes([0xA0 | len(data)]) + data
    return bytes(out)


def recv_packet(sock):
    """Дочитать ровно один пакет, чего бы ни отдал recv."""
    buf = b""
    while True:
        buf += sock.recv(65536)
        total = kb.packet_total_len(buf)
        if total and len(buf) >= total:
            return buf[:total], buf[total:]


class TestSessionLoopback(unittest.TestCase):
    """Сквозной прогон через настоящий сокет: сессия на C, транспорт на Python."""

    def setUp(self):
        self.client_sock, self.server_sock = socket.socketpair()
        self.addCleanup(self.client_sock.close)
        self.addCleanup(self.server_sock.close)
        self.now = 1000
        self.session = kb.Session(
            device_id="loopback",
            client_session_id=1755730000000,
            ping_interval_ms=10000,
            request_timeout_ms=5000,
        )

    def flush(self):
        data = self.session.take_outgoing()
        if data:
            self.client_sock.sendall(data)
        return data

    def pump(self):
        """Прочитать один пакет с клиентской стороны и скормить сессии."""
        packet, rest = recv_packet(self.client_sock)
        self.assertEqual(rest, b"")
        self.session.feed(packet)
        return list(self.session.events())

    def handshake(self, calls_seed=4242):
        self.session.on_connecting(self.now)
        self.session.on_connected(self.now)
        self.assertTrue(self.flush())

        packet, rest = recv_packet(self.server_sock)
        self.assertEqual(rest, b"")
        header, payload = kb.decode(packet)
        self.assertEqual(header.opcode, kb.OPCODE_SESSION_INIT)
        self.assertEqual(header.seq, 1)
        self.assertEqual(payload[:1], b"\x84")  # map из четырёх полей

        self.server_sock.sendall(kb.encode(
            opcode=kb.OPCODE_SESSION_INIT, seq=header.seq, cmd=kb.CMD_OK,
            payload=msgpack_map([("callsSeed", calls_seed),
                                 ("device_name", "Pixel")])))
        return self.pump()

    def test_handshake_brings_session_online(self):
        events = self.handshake(calls_seed=4242)
        kinds = [e.type for e in events]
        self.assertIn(kb.EventType.HANDSHAKE, kinds)

        shake = next(e for e in events if e.type == kb.EventType.HANDSHAKE)
        self.assertTrue(shake.has_calls_seed)
        self.assertEqual(shake.calls_seed, 4242)
        self.assertEqual(self.session.state, kb.State.ONLINE)
        self.assertEqual(self.session.pending_count, 0)

    def test_request_response_roundtrip(self):
        self.handshake()

        seq = self.session.request(64, msgpack_map([("chatId", 42)]))
        self.assertEqual(seq, 2)
        self.assertEqual(self.session.pending_count, 1)
        self.flush()

        packet, rest = recv_packet(self.server_sock)
        self.assertEqual(rest, b"")
        header, payload = kb.decode(packet)
        self.assertEqual(header.opcode, 64)
        self.assertEqual(header.seq, seq)
        self.assertEqual(payload, msgpack_map([("chatId", 42)]))

        self.server_sock.sendall(kb.encode(
            opcode=64, seq=seq, cmd=kb.CMD_OK,
            payload=msgpack_map([("ok", True)])))

        events = self.pump()
        response = next(e for e in events if e.type == kb.EventType.RESPONSE)
        self.assertEqual(response.seq, seq)
        self.assertEqual(response.cmd, kb.CMD_OK)
        self.assertEqual(response.payload, msgpack_map([("ok", True)]))
        self.assertEqual(self.session.pending_count, 0)

    def test_server_push_is_not_confused_with_response(self):
        self.handshake()
        # Пуш приходит с cmd == 0 и своим seq, к нашим запросам не относящимся.
        self.server_sock.sendall(kb.encode(
            opcode=128, seq=999, cmd=kb.CMD_PUSH,
            payload=msgpack_map([("type", "MESSAGE")])))

        events = self.pump()
        push = next(e for e in events if e.type == kb.EventType.PUSH)
        self.assertEqual(push.opcode, 128)
        self.assertEqual(push.seq, 999)
        self.assertEqual(push.payload, msgpack_map([("type", "MESSAGE")]))

    def test_keepalive_goes_out_on_schedule(self):
        self.handshake()
        self.session.tick(self.now + 9999)
        self.assertEqual(self.session.take_outgoing(), b"")

        self.session.tick(self.now + 10000)
        self.flush()
        packet, rest = recv_packet(self.server_sock)
        self.assertEqual(rest, b"")
        header, payload = kb.decode(packet)
        self.assertEqual(header.opcode, kb.OPCODE_PING)
        self.assertEqual(payload, msgpack_map([("interactive", True)]))

    def test_disconnect_fails_pending_and_schedules_retry(self):
        self.handshake()
        seq = self.session.request(64)
        self.flush()
        recv_packet(self.server_sock)

        self.session.on_disconnected(self.now + 500)
        events = list(self.session.events())
        failed = next(e for e in events
                      if e.type == kb.EventType.REQUEST_FAILED)
        self.assertEqual(failed.seq, seq)
        self.assertEqual(failed.status, -11)  # KB_ERR_CLOSED
        self.assertEqual(self.session.state, kb.State.DISCONNECTED)
        self.assertEqual(self.session.next_deadline(),
                         self.now + 500 + kb.backoff_ms(0))

    def test_request_before_online_is_rejected(self):
        with self.assertRaises(kb.KolibriError):
            self.session.request(64)


class TypedOpsAndBridge(unittest.TestCase):
    """Мост msgpack<->JSON и типизированный слой операций."""

    def test_bridge_round_trip(self):
        mp = kb.to_msgpack({"phone": "+70000000000", "n": 5,
                            "b": {"$bin": "3q0="}})
        back = kb.to_json(mp)
        self.assertEqual(back["phone"], "+70000000000")
        self.assertEqual(back["n"], 5)
        self.assertEqual(back["b"]["$bin"], "3q0=")  # bytes вернулись тегом

    def test_fingerprint_is_96_bytes(self):
        fp = kb.fingerprint(1234567890123, "dev-abc",
                            b"\x01" * 32, b"\x02" * 32, b"\x03" * 32)
        self.assertEqual(len(fp), 96)

    def test_typed_op_builds_and_validates(self):
        import kolibri_ops

        class Stub(kolibri_ops.Ops):
            def request(self, opcode, payload):
                self.seen = (opcode, payload)
                return {"token": "TOK", "codeLength": 6}

        s = Stub()
        r = s.auth_request(phone="+70000000000", type="START_AUTH",
                           mode=b"\x01" * 96)
        opcode, payload = s.seen
        self.assertEqual(opcode, 17)                       # опкод подставлен
        self.assertIn("$bin", payload["mode"])             # bytes -> $bin
        self.assertEqual(r.token, "TOK")                   # ответ типизирован
        self.assertEqual(r.codeLength, 6)                  # серверный extra виден

        with self.assertRaises(TypeError):                 # опечатка в поле
            s.auth_request(phone="x", tpye="A")
        with self.assertRaises(TypeError):                 # нет обязательного
            s.auth_request(type="START_AUTH")


if __name__ == "__main__":
    unittest.main()
