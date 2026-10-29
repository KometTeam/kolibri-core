"""Обёртка над ядром kolibri на ctypes.

Ядро sans-IO: оно разбирает и собирает пакеты, а сокет и TLS остаются на
стороне Python — обычные `socket` и `ssl`. Это тот же расклад, что у `h11`
и `h2`, только разбор написан на C.

    import kolibri_core as kb

    packet = kb.encode(opcode=64, seq=1, payload=msgpack_bytes)
    sock.sendall(packet)

    rx = kb.Receiver()
    rx.push(sock.recv(65536))
    for raw in rx:
        header, payload = kb.decode(raw)

Библиотека ищется в `build/`, рядом с модулем, в `KOLIBRI_LIBRARY`
и в системных путях. Собрать: `make shared`.
"""

import ctypes
import ctypes.util
import json as _json
try:
    from kolibri_ops import Ops as _Ops  # @generated tools/gen_python.py
except Exception:
    class _Ops(object):
        pass
import os
import pathlib
import platform
import sys

__all__ = [
    "KolibriError", "Header", "Receiver",
    "encode", "decode", "packet_total_len",
    "lz4_compress", "lz4_decompress", "decompress",
    "HEADER_SIZE", "PROTOCOL_VERSION", "COMPRESSION_THRESHOLD",
    "CMD_REQUEST", "CMD_PUSH", "CMD_OK", "CMD_NOT_FOUND", "CMD_ERROR",
    "library_path",
    "Session", "UserAgent", "Event", "State", "EventType",
    "OPCODE_PING", "OPCODE_SESSION_INIT", "backoff_ms",
    "COMPRESSION_NONE", "COMPRESSION_ZSTD", "zstd_enable", "zstd_available",
    "to_msgpack", "to_json", "fingerprint", "Connection",
]

HEADER_SIZE = 10
PROTOCOL_VERSION = 10
COMPRESSION_THRESHOLD = 32
MAX_BODY_LEN = 0xFFFFFF
MAX_DECOMPRESSED_SIZE = 32 * 1024 * 1024
MAX_BUFFER_SIZE = 16 * 1024 * 1024

# Старший байт packedLen — дискриминатор формата тела, а не «сжато/нет»:
# 0x00 как есть, 0xFF zstd, всё прочее LZ4 block с оценкой размера.
COMPRESSION_NONE = 0x00
COMPRESSION_ZSTD = 0xFF

CMD_REQUEST = 0
CMD_PUSH = 0
CMD_OK = 1
CMD_NOT_FOUND = 2
CMD_ERROR = 3

KB_OK = 0
KB_ERR_TOO_LARGE = -2
KB_ERR_OVERFLOW = -7
KB_ERR_LIMIT = -9


class KolibriError(RuntimeError):
    """Ядро вернуло ненулевой код."""

    def __init__(self, status, where):
        self.status = status
        text = _lib.kb_status_str(status).decode("utf-8", "replace")
        super().__init__("%s: %s (%d)" % (where, text, status))


def _candidates():
    system = platform.system()
    if system == "Darwin":
        names = ["libkolibri.dylib"]
    elif system == "Windows":
        names = ["kolibri.dll", "libkolibri.dll"]
    else:
        names = ["libkolibri.so"]

    env = os.environ.get("KOLIBRI_LIBRARY")
    if env:
        yield env

    here = pathlib.Path(__file__).resolve().parent
    for base in (here, here.parent / "build", here / "build", pathlib.Path.cwd() / "build"):
        for name in names:
            yield str(base / name)

    found = ctypes.util.find_library("kolibri")
    if found:
        yield found


def _load():
    tried = []
    for path in _candidates():
        tried.append(path)
        try:
            return ctypes.CDLL(path), path
        except OSError:
            continue
    raise ImportError(
        "не нашёл разделяемую библиотеку kolibri; собери `make shared` "
        "или укажи путь в KOLIBRI_LIBRARY. Пробовал:\n  " + "\n  ".join(tried)
    )


_lib, _library_path = _load()


def library_path():
    """Путь к загруженной разделяемой библиотеке."""
    return _library_path


class _CHeader(ctypes.Structure):
    _fields_ = [
        ("ver", ctypes.c_uint8),
        ("cmd", ctypes.c_uint8),
        ("seq", ctypes.c_uint16),
        ("opcode", ctypes.c_uint16),
        ("flag", ctypes.c_uint8),
        ("body_len", ctypes.c_uint32),
    ]


class _CRx(ctypes.Structure):
    _fields_ = [
        ("buf", ctypes.POINTER(ctypes.c_uint8)),
        ("cap", ctypes.c_size_t),
        ("start", ctypes.c_size_t),
        ("end", ctypes.c_size_t),
    ]


_u8p = ctypes.POINTER(ctypes.c_uint8)
_sizep = ctypes.POINTER(ctypes.c_size_t)

_lib.kb_status_str.restype = ctypes.c_char_p
_lib.kb_status_str.argtypes = [ctypes.c_int]

_lib.kb_encode_bound.restype = ctypes.c_size_t
_lib.kb_encode_bound.argtypes = [ctypes.c_size_t]

_lib.kb_encode.restype = ctypes.c_int
_lib.kb_encode.argtypes = [
    ctypes.c_uint8, ctypes.c_uint16, ctypes.c_uint16,
    ctypes.c_char_p, ctypes.c_size_t,
    ctypes.c_char_p, ctypes.c_size_t, _sizep,
]

_lib.kb_decode.restype = ctypes.c_int
_lib.kb_decode.argtypes = [
    ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(_CHeader),
    ctypes.c_char_p, ctypes.c_size_t, _sizep,
]

_lib.kb_packet_total_len.restype = ctypes.c_size_t
_lib.kb_packet_total_len.argtypes = [ctypes.c_char_p, ctypes.c_size_t]

_lib.kb_lz4_compress_bound.restype = ctypes.c_size_t
_lib.kb_lz4_compress_bound.argtypes = [ctypes.c_size_t]

_lib.kb_lz4_compress.restype = ctypes.c_int
_lib.kb_lz4_compress.argtypes = [
    ctypes.c_char_p, ctypes.c_size_t, ctypes.c_char_p, ctypes.c_size_t, _sizep,
]

_lib.kb_lz4_decompress.restype = ctypes.c_int
_lib.kb_lz4_decompress.argtypes = _lib.kb_lz4_compress.argtypes

_lib.kb_decompress.restype = ctypes.c_int
_lib.kb_decompress.argtypes = _lib.kb_lz4_compress.argtypes

_lib.kb_rx_init.restype = ctypes.c_int
_lib.kb_rx_init.argtypes = [ctypes.POINTER(_CRx), _u8p, ctypes.c_size_t]

_lib.kb_rx_push.restype = ctypes.c_int
_lib.kb_rx_push.argtypes = [ctypes.POINTER(_CRx), ctypes.c_char_p, ctypes.c_size_t]

_lib.kb_rx_next.restype = _u8p
_lib.kb_rx_next.argtypes = [ctypes.POINTER(_CRx), _sizep]

_lib.kb_rx_buffered.restype = ctypes.c_size_t
_lib.kb_rx_buffered.argtypes = [ctypes.POINTER(_CRx)]

_lib.kb_rx_reset.restype = None
_lib.kb_rx_reset.argtypes = [ctypes.POINTER(_CRx)]


def zstd_available():
    """Собрана ли библиотека с поддержкой zstd."""
    try:
        _lib.kb_zstd_version.restype = ctypes.c_char_p
        _lib.kb_zstd_version.argtypes = []
        return _lib.kb_zstd_version() is not None
    except AttributeError:
        return False


def zstd_enable():
    """Включить распаковку zstd.

    Сервер вправе прислать тело, сжатое zstd (флаг 0xFF); без этого такие
    пакеты дадут ошибку «unsupported compression format». Исходящее всегда
    LZ4 block, компрессор zstd не нужен.
    """
    try:
        _lib.kb_zstd_enable.restype = ctypes.c_int
        _lib.kb_zstd_enable.argtypes = []
    except AttributeError:
        raise KolibriError(-6, "kb_zstd_enable")
    _check(_lib.kb_zstd_enable(), "kb_zstd_enable")


class Header(object):
    """Разобранный заголовок пакета."""

    __slots__ = ("ver", "cmd", "seq", "opcode", "flag", "body_len")

    def __init__(self, ver, cmd, seq, opcode, flag, body_len):
        self.ver = ver
        self.cmd = cmd
        self.seq = seq
        self.opcode = opcode
        self.flag = flag
        self.body_len = body_len

    @property
    def compressed(self):
        return self.flag != 0

    def __repr__(self):
        return ("Header(ver=%d, cmd=%d, seq=%d, opcode=%d, flag=%d, body_len=%d)"
                % (self.ver, self.cmd, self.seq, self.opcode, self.flag,
                   self.body_len))

    def __eq__(self, other):
        if not isinstance(other, Header):
            return NotImplemented
        return all(getattr(self, f) == getattr(other, f) for f in self.__slots__)


def _check(status, where):
    if status != KB_OK:
        raise KolibriError(status, where)


def encode(opcode, seq, payload=b"", cmd=CMD_REQUEST):
    """Собрать пакет из готового msgpack-тела."""
    payload = bytes(payload)
    if len(payload) > MAX_BODY_LEN:
        raise ValueError("тело длиннее 24 бит длины")
    cap = _lib.kb_encode_bound(len(payload))
    out = ctypes.create_string_buffer(cap)
    written = ctypes.c_size_t(0)
    _check(_lib.kb_encode(cmd, opcode, seq, payload, len(payload), out, cap,
                          ctypes.byref(written)), "kb_encode")
    return out.raw[:written.value]


def packet_total_len(buf):
    """Объявленная полная длина пакета или 0, если заголовок ещё не набрался."""
    buf = bytes(buf)
    return _lib.kb_packet_total_len(buf, len(buf))


def decode(packet):
    """Разобрать целый пакет -> (Header, payload)."""
    packet = bytes(packet)
    header = _CHeader()
    written = ctypes.c_size_t(0)

    # Для сжатого тела итоговый размер заранее неизвестен: начинаем с оценки и
    # растём, пока ядро не скажет, что хватило. Оценку обязательно зажимаем
    # потолком: иначе на крупном пакете первый же буфер окажется больше лимита,
    # и лимит никогда не сработает.
    cap = min(max(len(packet) * 4, 4096), MAX_DECOMPRESSED_SIZE)
    while True:
        out = ctypes.create_string_buffer(cap)
        status = _lib.kb_decode(packet, len(packet), ctypes.byref(header), out,
                                cap, ctypes.byref(written))
        if status == KB_ERR_TOO_LARGE and cap < MAX_DECOMPRESSED_SIZE:
            cap = min(cap * 4, MAX_DECOMPRESSED_SIZE)
            continue
        _check(status, "kb_decode")
        break

    parsed = Header(header.ver, header.cmd, header.seq, header.opcode,
                    header.flag, header.body_len)
    return parsed, out.raw[:written.value]


def lz4_compress(data):
    data = bytes(data)
    cap = _lib.kb_lz4_compress_bound(len(data))
    out = ctypes.create_string_buffer(cap)
    written = ctypes.c_size_t(0)
    _check(_lib.kb_lz4_compress(data, len(data), out, cap,
                                ctypes.byref(written)), "kb_lz4_compress")
    return out.raw[:written.value]


def _grow_decompress(fn, data, max_out, where):
    data = bytes(data)
    if max_out < 1:
        raise ValueError("max_out должен быть положительным")
    # Зажимаем стартовую оценку потолком, иначе max_out ни на что не влияет,
    # когда ядро уложилось в первый же буфер.
    cap = min(max(len(data) * 4, 4096), max_out)
    while True:
        out = ctypes.create_string_buffer(cap)
        written = ctypes.c_size_t(0)
        status = fn(data, len(data), out, cap, ctypes.byref(written))
        if status == KB_ERR_TOO_LARGE and cap < max_out:
            cap = min(cap * 4, max_out)
            continue
        _check(status, where)
        return out.raw[:written.value]


def lz4_decompress(data, max_out=MAX_DECOMPRESSED_SIZE):
    """Распаковать LZ4-block."""
    return _grow_decompress(_lib.kb_lz4_decompress, data, max_out,
                            "kb_lz4_decompress")


def decompress(data, max_out=MAX_DECOMPRESSED_SIZE):
    """Распаковать тело, определив формат по магическому числу."""
    return _grow_decompress(_lib.kb_decompress, data, max_out, "kb_decompress")


# ─── Машина сессии ─────────────────────────────────────────────────────────

OPCODE_PING = 1
OPCODE_SESSION_INIT = 6

_SIZEOF_SESSION = 0
_SIZEOF_CONFIG = 1
_SIZEOF_BUFFERS = 2
_SIZEOF_EVENT = 3
_SIZEOF_PENDING = 4
_SIZEOF_USER_AGENT = 5
_SIZEOF_HANDSHAKE = 6


class State(object):
    DISCONNECTED = 0
    CONNECTING = 1
    CONNECTED = 2
    ONLINE = 3

    _NAMES = {0: "disconnected", 1: "connecting", 2: "connected", 3: "online"}

    @classmethod
    def name(cls, value):
        return cls._NAMES.get(value, "?")


class EventType(object):
    NONE = 0
    STATE = 1
    HANDSHAKE = 2
    RESPONSE = 3
    PUSH = 4
    REQUEST_FAILED = 5
    ERROR = 6

    _NAMES = {0: "none", 1: "state", 2: "handshake", 3: "response",
              4: "push", 5: "request_failed", 6: "error"}

    @classmethod
    def name(cls, value):
        return cls._NAMES.get(value, "?")


class UserAgent(object):
    """Поля `userAgent` в payload sessionInit."""

    __slots__ = ("device_type", "app_version", "os_version", "timezone",
                 "screen", "push_device_type", "arch", "locale",
                 "build_number", "device_name", "device_locale",
                 "header_user_agent", "is_pwa")

    def __init__(self, device_type="ANDROID", app_version="26.25.0",
                 os_version="Android 14", timezone="Europe/Moscow",
                 screen="480dpi 480dpi 1344x2992", push_device_type="GCM",
                 arch="arm64-v8a", locale="ru", build_number=6790,
                 device_name="Google Pixel 8 Pro", device_locale="ru",
                 header_user_agent=None, is_pwa=None):
        self.device_type = device_type
        self.app_version = app_version
        self.os_version = os_version
        self.timezone = timezone
        self.screen = screen
        self.push_device_type = push_device_type
        self.arch = arch
        self.locale = locale
        self.build_number = build_number
        self.device_name = device_name
        self.device_locale = device_locale
        # Опциональны (веб-клиент MAX). None -> в payload не попадают.
        self.header_user_agent = header_user_agent
        self.is_pwa = is_pwa

    def http_user_agent(self):
        """User-Agent для заголовка загрузки медиа — как UserAgent::http_user_agent
        в эталоне: OKMessages/<app> (<os>; <device>; <screen>)."""
        return "OKMessages/%s (%s; %s; %s)" % (
            self.app_version, self.os_version, self.device_name, self.screen)


class Event(object):
    """Событие сессии. payload уже скопирован и живёт сколько угодно."""

    __slots__ = ("type", "state", "seq", "opcode", "cmd", "payload",
                 "has_calls_seed", "calls_seed", "status")

    def __init__(self, raw):
        self.type = raw.type
        self.state = raw.state
        self.seq = raw.seq
        self.opcode = raw.opcode
        self.cmd = raw.cmd
        self.payload = (ctypes.string_at(raw.payload, raw.payload_len)
                        if raw.payload else b"")
        self.has_calls_seed = bool(raw.has_calls_seed)
        self.calls_seed = raw.calls_seed
        self.status = raw.status

    def __repr__(self):
        bits = ["type=%s" % EventType.name(self.type)]
        if self.type == EventType.STATE:
            bits.append("state=%s" % State.name(self.state))
        if self.type in (EventType.RESPONSE, EventType.PUSH,
                         EventType.HANDSHAKE, EventType.REQUEST_FAILED):
            bits.append("seq=%d opcode=%d" % (self.seq, self.opcode))
        if self.payload:
            bits.append("payload=%d Б" % len(self.payload))
        if self.status:
            bits.append("status=%d" % self.status)
        return "Event(%s)" % ", ".join(bits)


class _CUserAgent(ctypes.Structure):
    _fields_ = [
        ("device_type", ctypes.c_char_p),
        ("app_version", ctypes.c_char_p),
        ("os_version", ctypes.c_char_p),
        ("timezone", ctypes.c_char_p),
        ("screen", ctypes.c_char_p),
        ("push_device_type", ctypes.c_char_p),
        ("arch", ctypes.c_char_p),
        ("locale", ctypes.c_char_p),
        ("build_number", ctypes.c_int64),
        ("device_name", ctypes.c_char_p),
        ("device_locale", ctypes.c_char_p),
        ("header_user_agent", ctypes.c_char_p),
        ("has_is_pwa", ctypes.c_int),
        ("is_pwa", ctypes.c_int),
    ]


class _CHandshake(ctypes.Structure):
    _fields_ = [
        ("instance_id", ctypes.c_char_p),
        ("device_id", ctypes.c_char_p),
        ("client_session_id", ctypes.c_int64),
        ("user_agent", _CUserAgent),
    ]


class _CConfig(ctypes.Structure):
    _fields_ = [
        ("handshake", _CHandshake),
        ("ping_interval_ms", ctypes.c_uint32),
        ("ping_interactive", ctypes.c_int),
        ("request_timeout_ms", ctypes.c_uint32),
        ("auto_reconnect", ctypes.c_int),
    ]


class _CPending(ctypes.Structure):
    _fields_ = [
        ("seq", ctypes.c_uint16),
        ("opcode", ctypes.c_uint16),
        ("deadline_ms", ctypes.c_uint64),
        ("in_use", ctypes.c_int),
        ("failed", ctypes.c_int),
    ]


class _CBuffers(ctypes.Structure):
    _fields_ = [
        ("rx", _u8p),
        ("rx_capacity", ctypes.c_size_t),
        ("tx", _u8p),
        ("tx_capacity", ctypes.c_size_t),
        ("scratch", _u8p),
        ("scratch_capacity", ctypes.c_size_t),
        ("pending", ctypes.POINTER(_CPending)),
        ("pending_capacity", ctypes.c_size_t),
    ]


class _CEvent(ctypes.Structure):
    _fields_ = [
        ("type", ctypes.c_int),
        ("state", ctypes.c_int),
        ("seq", ctypes.c_uint16),
        ("opcode", ctypes.c_uint16),
        ("cmd", ctypes.c_uint8),
        ("payload", _u8p),
        ("payload_len", ctypes.c_size_t),
        ("has_calls_seed", ctypes.c_int),
        ("calls_seed", ctypes.c_int64),
        ("status", ctypes.c_int),
    ]


_lib.kb_sizeof.restype = ctypes.c_size_t
_lib.kb_sizeof.argtypes = [ctypes.c_int]

# Разметка структур обязана совпасть с той, что видит компилятор. Ошибка здесь
# молча портила бы память, поэтому проверяем на импорте, а не когда-нибудь.
for _name, _what, _cls in (
    ("kb_user_agent", _SIZEOF_USER_AGENT, _CUserAgent),
    ("kb_handshake_config", _SIZEOF_HANDSHAKE, _CHandshake),
    ("kb_session_config", _SIZEOF_CONFIG, _CConfig),
    ("kb_session_buffers", _SIZEOF_BUFFERS, _CBuffers),
    ("kb_pending", _SIZEOF_PENDING, _CPending),
    ("kb_event", _SIZEOF_EVENT, _CEvent),
):
    _expected = _lib.kb_sizeof(_what)
    if ctypes.sizeof(_cls) != _expected:
        raise ImportError(
            "разметка %s разошлась: ctypes видит %d байт, библиотека — %d"
            % (_name, ctypes.sizeof(_cls), _expected))

_lib.kb_session_init.restype = ctypes.c_int
_lib.kb_session_init.argtypes = [ctypes.c_void_p, ctypes.POINTER(_CConfig),
                                 ctypes.POINTER(_CBuffers)]
for _fn in ("kb_session_on_connecting", "kb_session_on_connected",
            "kb_session_on_disconnected", "kb_session_tick"):
    getattr(_lib, _fn).restype = ctypes.c_int
    getattr(_lib, _fn).argtypes = [ctypes.c_void_p, ctypes.c_uint64]

_lib.kb_session_feed.restype = ctypes.c_int
_lib.kb_session_feed.argtypes = [ctypes.c_void_p, ctypes.c_char_p,
                                 ctypes.c_size_t]

_lib.kb_session_pull_out.restype = _u8p
_lib.kb_session_pull_out.argtypes = [ctypes.c_void_p, _sizep]

_lib.kb_session_consume_out.restype = ctypes.c_int
_lib.kb_session_consume_out.argtypes = [ctypes.c_void_p, ctypes.c_size_t]

_lib.kb_session_poll_event.restype = ctypes.c_int
_lib.kb_session_poll_event.argtypes = [ctypes.c_void_p, ctypes.POINTER(_CEvent)]

for _fn in ("kb_session_request", "kb_session_send"):
    getattr(_lib, _fn).restype = ctypes.c_int
    getattr(_lib, _fn).argtypes = [ctypes.c_void_p, ctypes.c_uint16,
                                   ctypes.c_char_p, ctypes.c_size_t,
                                   ctypes.POINTER(ctypes.c_uint16)]

_lib.kb_session_next_deadline.restype = ctypes.c_uint64
_lib.kb_session_next_deadline.argtypes = [ctypes.c_void_p]

_lib.kb_session_get_state.restype = ctypes.c_int
_lib.kb_session_get_state.argtypes = [ctypes.c_void_p]

_lib.kb_session_pending_count.restype = ctypes.c_size_t
_lib.kb_session_pending_count.argtypes = [ctypes.c_void_p]

_lib.kb_session_set_ping_interactive.restype = None
_lib.kb_session_set_ping_interactive.argtypes = [ctypes.c_void_p, ctypes.c_int]

_lib.kb_session_backoff_ms.restype = ctypes.c_uint32
_lib.kb_session_backoff_ms.argtypes = [ctypes.c_uint32]


def backoff_ms(attempt):
    """Задержка перед попыткой номер attempt: 2, 4, 8, 15, 15... секунд."""
    return _lib.kb_session_backoff_ms(attempt)


class Session(object):
    """Машина сессии: handshake, мультиплекс, keepalive, таймауты.

    Сокета внутри нет — его ведёт вызывающий:

        s = Session(device_id="...", user_agent=UserAgent())
        s.on_connected(now_ms())
        sock.sendall(s.take_outgoing())
        s.feed(sock.recv(65536))
        for event in s.events():
            ...
    """

    def __init__(self, device_id, user_agent=None, instance_id=None,
                 client_session_id=0, ping_interval_ms=30000,
                 ping_interactive=True, request_timeout_ms=30000,
                 auto_reconnect=True, rx_capacity=1 << 20,
                 tx_capacity=1 << 18, scratch_capacity=2 << 20,
                 max_pending=64):
        ua = user_agent or UserAgent()

        # Строки принадлежат нам: ядро их не копирует, а хранит указатели.
        self._strings = {}

        def keep(text):
            data = (text or "").encode("utf-8")
            self._strings[id(data)] = data
            return data

        self._config = _CConfig()
        self._config.handshake.instance_id = keep(instance_id or device_id)
        self._config.handshake.device_id = keep(device_id)
        self._config.handshake.client_session_id = client_session_id
        c_ua = self._config.handshake.user_agent
        c_ua.device_type = keep(ua.device_type)
        c_ua.app_version = keep(ua.app_version)
        c_ua.os_version = keep(ua.os_version)
        c_ua.timezone = keep(ua.timezone)
        c_ua.screen = keep(ua.screen)
        c_ua.push_device_type = keep(ua.push_device_type)
        c_ua.arch = keep(ua.arch)
        c_ua.locale = keep(ua.locale)
        c_ua.build_number = ua.build_number
        c_ua.device_name = keep(ua.device_name)
        c_ua.device_locale = keep(ua.device_locale)
        # Опциональные web-поля: переносим только если заданы.
        if getattr(ua, "header_user_agent", None) is not None:
            c_ua.header_user_agent = keep(ua.header_user_agent)
        if getattr(ua, "is_pwa", None) is not None:
            c_ua.has_is_pwa = 1
            c_ua.is_pwa = 1 if ua.is_pwa else 0
        self._config.ping_interval_ms = ping_interval_ms
        self._config.ping_interactive = 1 if ping_interactive else 0
        self._config.request_timeout_ms = request_timeout_ms
        self._config.auto_reconnect = 1 if auto_reconnect else 0

        self._rx = (ctypes.c_uint8 * rx_capacity)()
        self._tx = (ctypes.c_uint8 * tx_capacity)()
        self._scratch = (ctypes.c_uint8 * scratch_capacity)()
        self._pending = (_CPending * max_pending)()

        self._buffers = _CBuffers()
        self._buffers.rx = ctypes.cast(self._rx, _u8p)
        self._buffers.rx_capacity = rx_capacity
        self._buffers.tx = ctypes.cast(self._tx, _u8p)
        self._buffers.tx_capacity = tx_capacity
        self._buffers.scratch = ctypes.cast(self._scratch, _u8p)
        self._buffers.scratch_capacity = scratch_capacity
        self._buffers.pending = ctypes.cast(self._pending,
                                            ctypes.POINTER(_CPending))
        self._buffers.pending_capacity = max_pending

        # Сессия — непрозрачный блок нужного размера: разметку не повторяем.
        self._storage = ctypes.create_string_buffer(
            _lib.kb_sizeof(_SIZEOF_SESSION))
        self._ptr = ctypes.cast(self._storage, ctypes.c_void_p)

        _check(_lib.kb_session_init(self._ptr, ctypes.byref(self._config),
                                    ctypes.byref(self._buffers)),
               "kb_session_init")

    # ── Жизненный цикл ────────────────────────────────────────────────────

    def on_connecting(self, now_ms):
        _check(_lib.kb_session_on_connecting(self._ptr, now_ms),
               "kb_session_on_connecting")

    def on_connected(self, now_ms):
        _check(_lib.kb_session_on_connected(self._ptr, now_ms),
               "kb_session_on_connected")

    def on_disconnected(self, now_ms):
        _check(_lib.kb_session_on_disconnected(self._ptr, now_ms),
               "kb_session_on_disconnected")

    # ── Поток ─────────────────────────────────────────────────────────────

    def feed(self, data):
        data = bytes(data)
        if data:
            _check(_lib.kb_session_feed(self._ptr, data, len(data)),
                   "kb_session_feed")

    def tick(self, now_ms):
        _check(_lib.kb_session_tick(self._ptr, now_ms), "kb_session_tick")

    def take_outgoing(self):
        """Всё, что сессия хочет отправить; b"" — если нечего."""
        chunks = []
        while True:
            length = ctypes.c_size_t(0)
            raw = _lib.kb_session_pull_out(self._ptr, ctypes.byref(length))
            if not raw:
                break
            chunks.append(ctypes.string_at(raw, length.value))
            _check(_lib.kb_session_consume_out(self._ptr, length.value),
                   "kb_session_consume_out")
        return b"".join(chunks)

    def poll_event(self):
        raw = _CEvent()
        _check(_lib.kb_session_poll_event(self._ptr, ctypes.byref(raw)),
               "kb_session_poll_event")
        if raw.type == EventType.NONE:
            return None
        return Event(raw)

    def events(self):
        while True:
            event = self.poll_event()
            if event is None:
                return
            yield event

    # ── Запросы ───────────────────────────────────────────────────────────

    def request(self, opcode, payload=b""):
        payload = bytes(payload)
        seq = ctypes.c_uint16(0)
        _check(_lib.kb_session_request(self._ptr, opcode, payload,
                                       len(payload), ctypes.byref(seq)),
               "kb_session_request")
        return seq.value

    def send(self, opcode, payload=b""):
        payload = bytes(payload)
        seq = ctypes.c_uint16(0)
        _check(_lib.kb_session_send(self._ptr, opcode, payload, len(payload),
                                    ctypes.byref(seq)),
               "kb_session_send")
        return seq.value

    # ── Справки ───────────────────────────────────────────────────────────

    @property
    def state(self):
        return _lib.kb_session_get_state(self._ptr)

    @property
    def pending_count(self):
        return _lib.kb_session_pending_count(self._ptr)

    def next_deadline(self):
        """Когда разбудить сессию; 0 — можно ждать на сокете сколько угодно."""
        return _lib.kb_session_next_deadline(self._ptr)

    def set_ping_interactive(self, interactive):
        _lib.kb_session_set_ping_interactive(self._ptr, 1 if interactive else 0)


class Receiver(object):
    """Собирает поток байт из сокета обратно в пакеты.

        rx = Receiver()
        rx.push(sock.recv(65536))
        for packet in rx:
            header, payload = decode(packet)

    Ядро работает на буфере фиксированного размера, который ему даёт вызывающая
    сторона. Здесь буфер растёт по мере надобности до `max_capacity`, потому что
    длина тела на проводе занимает 24 бита и один пакет законно бывает до 16 МБ:
    приёмник с жёстким мегабайтом молча ломался бы на крупном вложении.
    """

    def __init__(self, capacity=64 * 1024, max_capacity=MAX_BUFFER_SIZE):
        if capacity < HEADER_SIZE:
            raise ValueError("буфер меньше заголовка")
        if max_capacity < capacity:
            raise ValueError("max_capacity меньше начальной ёмкости")
        self._capacity = capacity
        self._max_capacity = max_capacity
        self._storage = (ctypes.c_uint8 * capacity)()
        self._rx = _CRx()
        _check(_lib.kb_rx_init(ctypes.byref(self._rx), self._storage, capacity),
               "kb_rx_init")

    def _pending(self):
        """Ещё не разобранные байты, скопированные наружу."""
        start, end = self._rx.start, self._rx.end
        if end <= start:
            return b""
        return ctypes.string_at(
            ctypes.byref(self._storage, start), end - start)

    def _grow(self, needed):
        capacity = self._capacity
        while capacity < needed:
            capacity = min(capacity * 2, self._max_capacity)
            if capacity == self._max_capacity:
                break
        if capacity < needed:
            raise KolibriError(KB_ERR_OVERFLOW, "kb_rx_push")

        # Переносим хвост в новый буфер: kb_rx_push при переполнении сбрасывает
        # состояние, так что данные надо спасать до, а не после.
        pending = self._pending()
        self._capacity = capacity
        self._storage = (ctypes.c_uint8 * capacity)()
        _check(_lib.kb_rx_init(ctypes.byref(self._rx), self._storage, capacity),
               "kb_rx_init")
        if pending:
            _check(_lib.kb_rx_push(ctypes.byref(self._rx), pending, len(pending)),
                   "kb_rx_push")

    def push(self, data):
        data = bytes(data)
        if not data:
            return
        needed = self.buffered + len(data)
        if needed > self._capacity:
            self._grow(needed)
        _check(_lib.kb_rx_push(ctypes.byref(self._rx), data, len(data)),
               "kb_rx_push")

    def next_packet(self):
        """Следующий целый пакет или None."""
        length = ctypes.c_size_t(0)
        ptr = _lib.kb_rx_next(ctypes.byref(self._rx), ctypes.byref(length))
        if not ptr:
            return None
        # Копия обязательна: указатель живёт лишь до следующего push.
        return ctypes.string_at(ctypes.cast(ptr, ctypes.c_void_p), length.value)

    def __iter__(self):
        while True:
            packet = self.next_packet()
            if packet is None:
                return
            yield packet

    @property
    def buffered(self):
        return _lib.kb_rx_buffered(ctypes.byref(self._rx))

    @property
    def capacity(self):
        return self._capacity

    def reset(self):
        _lib.kb_rx_reset(ctypes.byref(self._rx))



# ── Мост msgpack↔JSON и anti-spoof fingerprint (функции ядра) ───────────────

_lib.kb_json_to_mp.restype = ctypes.c_int
_lib.kb_json_to_mp.argtypes = [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_char_p,
                               ctypes.c_size_t, ctypes.c_char_p, ctypes.c_size_t,
                               _sizep]
_lib.kb_mp_to_json.restype = ctypes.c_int
_lib.kb_mp_to_json.argtypes = [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int,
                               ctypes.c_char_p, ctypes.c_size_t, _sizep]


class _CDigests(ctypes.Structure):
    _fields_ = [("signature", ctypes.c_char_p), ("signature_len", ctypes.c_size_t),
                ("so", ctypes.c_char_p), ("so_len", ctypes.c_size_t),
                ("dex", ctypes.c_char_p), ("dex_len", ctypes.c_size_t)]


_lib.kb_chat_cache_fingerprint.restype = ctypes.c_int
_lib.kb_chat_cache_fingerprint.argtypes = [ctypes.POINTER(_CDigests),
                                           ctypes.c_int64, ctypes.c_char_p,
                                           ctypes.c_char_p]


def to_msgpack(obj, scratch_cap=1 << 16, out_cap=1 << 16):
    """Python-объект → msgpack. Байты передавать тегом {"$bin": base64}."""
    src = _json.dumps(obj).encode("utf-8")
    scratch = ctypes.create_string_buffer(scratch_cap)
    out = ctypes.create_string_buffer(out_cap)
    n = ctypes.c_size_t(0)
    _check(_lib.kb_json_to_mp(src, len(src), scratch, scratch_cap, out, out_cap,
                              ctypes.byref(n)), "kb_json_to_mp")
    return out.raw[:n.value]


def to_json(payload, tagged=True, out_cap=1 << 20):
    """msgpack → Python-объект. При tagged Binary приходит как {"$bin": base64}."""
    payload = bytes(payload)
    buf = ctypes.create_string_buffer(out_cap)
    n = ctypes.c_size_t(0)
    _check(_lib.kb_mp_to_json(payload, len(payload), 1 if tagged else 0, buf,
                              out_cap, ctypes.byref(n)), "kb_mp_to_json")
    return _json.loads(buf.raw[:n.value])


def fingerprint(calls_seed, device_id, signature, so, dex):
    """96-байтный fingerprint для поля mode (опкод 17) и chatCacheFingerprint."""
    d = _CDigests(bytes(signature), len(signature), bytes(so), len(so),
                  bytes(dex), len(dex))
    out = (ctypes.c_uint8 * 96)()
    _check(_lib.kb_chat_cache_fingerprint(
        ctypes.byref(d), ctypes.c_int64(calls_seed), device_id.encode(),
        ctypes.cast(out, ctypes.c_char_p)), "kb_chat_cache_fingerprint")
    return bytes(out)


# ── Блокирующий клиент для скриптов ─────────────────────────────────────────

class Connection(_Ops):
    """TLS-сокет + сессия + запрос-ответ по seq — для скриптов и утилит.

    НЕ sans-IO: держит сокет и сам крутит цикл. Ядро остаётся чистым; это лишь
    удобная обёртка, чтобы пример входа умещался в десяток строк.

        with kb.Connection("api.oneme.ru", device_id="...", user_agent=ua) as c:
            seed = c.handshake()
            resp = c.request(17, {"phone": "+7...", "type": "START_AUTH"})
    """

    def __init__(self, host, port=443, device_id=None, user_agent=None,
                 instance_id=None, client_session_id=0, timeout=15,
                 ssl_context=None):
        import socket
        import ssl
        self._ssl = ssl
        self._select = __import__("select")
        self._time = __import__("time")
        self.session = Session(device_id=device_id, user_agent=user_agent,
                               instance_id=instance_id,
                               client_session_id=client_session_id,
                               auto_reconnect=False, ping_interval_ms=10000)
        ctx = ssl_context or ssl.create_default_context()
        raw = socket.create_connection((host, port), timeout=timeout)
        self.sock = ctx.wrap_socket(raw, server_hostname=host)
        self.sock.setblocking(False)

    def _now(self):
        return int(self._time.monotonic() * 1000)

    def _flush(self):
        data = self.session.take_outgoing()
        if data:
            self.sock.sendall(data)

    def _pump(self, deadline):
        self.session.tick(self._now())
        self._flush()
        wait = max(0.0, deadline - self._time.time())
        ready, _, _ = self._select.select([self.sock], [], [], wait)
        if ready:
            try:
                chunk = self.sock.recv(65536)
            except self._ssl.SSLWantReadError:
                return []
            if not chunk:
                raise ConnectionError("сервер закрыл соединение")
            self.session.feed(chunk)
        return list(self.session.events())

    def handshake(self, timeout=30):
        """Провести handshake, вернуть callsSeed (или None)."""
        self.session.on_connected(self._now())
        self._flush()
        deadline = self._time.time() + timeout
        while self._time.time() < deadline:
            for ev in self._pump(deadline):
                if ev.type == EventType.HANDSHAKE:
                    return ev.calls_seed if ev.has_calls_seed else None
                if ev.type == EventType.ERROR:
                    raise KolibriError(ev.status, "session")
        raise TimeoutError("handshake timeout")

    def request(self, opcode, obj=None, timeout=30):
        """Отправить запрос (dict → msgpack) и вернуть ответ (msgpack → dict)."""
        payload = to_msgpack(obj) if obj is not None else b""
        seq = self.session.request(opcode, payload)
        self._flush()
        deadline = self._time.time() + timeout
        while self._time.time() < deadline:
            for ev in self._pump(deadline):
                if ev.seq == seq and ev.type == EventType.RESPONSE:
                    return to_json(ev.payload)
                if ev.seq == seq and ev.type == EventType.REQUEST_FAILED:
                    raise KolibriError(ev.status, "opcode %d" % opcode)
                if ev.type == EventType.ERROR:
                    raise KolibriError(ev.status, "session")
        raise TimeoutError("response timeout for opcode %d" % opcode)

    def close(self):
        try:
            self.sock.close()
        except Exception:
            pass

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()


if __name__ == "__main__":
    sys.stdout.write("kolibri-core: %s\n" % library_path())
    demo = encode(opcode=64, seq=1, payload=b"\x81\xa4text\xa6\xd0\xbf\xd1\x80\xd0\xb8\xd0\xb2")
    head, body = decode(demo)
    sys.stdout.write("%r -> %r\n" % (head, body))
