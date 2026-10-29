// Пример на Dart: dart:ffi читает разделяемую библиотеку напрямую.
//
//     make -C ../.. shared && dart pub get && dart run demo.dart
//
// Биндинги ниже выписаны руками, чтобы их было видно. В настоящем проекте их
// генерирует ffigen прямо из include/kolibri.h — см. ffigen.yaml.

import 'dart:ffi';
import 'dart:io';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';

// ── Разметка структур ────────────────────────────────────────────────────

final class KbHeader extends Struct {
  @Uint8()
  external int ver;
  @Uint8()
  external int cmd;
  @Uint16()
  external int seq;
  @Uint16()
  external int opcode;
  @Uint8()
  external int flag;
  @Uint32()
  external int bodyLen;
}

final class KbRx extends Struct {
  external Pointer<Uint8> buf;
  @Size()
  external int cap;
  @Size()
  external int start;
  @Size()
  external int end;
}

// ── Сигнатуры ────────────────────────────────────────────────────────────

typedef _StatusStrC = Pointer<Utf8> Function(Int32);
typedef _StatusStr = Pointer<Utf8> Function(int);

typedef _EncodeBoundC = Size Function(Size);
typedef _EncodeBound = int Function(int);

typedef _EncodeC = Int32 Function(Uint8, Uint16, Uint16, Pointer<Uint8>, Size,
    Pointer<Uint8>, Size, Pointer<Size>);
typedef _Encode = int Function(
    int, int, int, Pointer<Uint8>, int, Pointer<Uint8>, int, Pointer<Size>);

typedef _DecodeC = Int32 Function(Pointer<Uint8>, Size, Pointer<KbHeader>,
    Pointer<Uint8>, Size, Pointer<Size>);
typedef _Decode = int Function(
    Pointer<Uint8>, int, Pointer<KbHeader>, Pointer<Uint8>, int, Pointer<Size>);

typedef _RxInitC = Int32 Function(Pointer<KbRx>, Pointer<Uint8>, Size);
typedef _RxInit = int Function(Pointer<KbRx>, Pointer<Uint8>, int);

typedef _RxPushC = Int32 Function(Pointer<KbRx>, Pointer<Uint8>, Size);
typedef _RxPush = int Function(Pointer<KbRx>, Pointer<Uint8>, int);

typedef _RxNextC = Pointer<Uint8> Function(Pointer<KbRx>, Pointer<Size>);
typedef _RxNext = Pointer<Uint8> Function(Pointer<KbRx>, Pointer<Size>);

typedef _RxBufferedC = Size Function(Pointer<KbRx>);
typedef _RxBuffered = int Function(Pointer<KbRx>);

// ── Обёртка ──────────────────────────────────────────────────────────────

class KolibriException implements Exception {
  KolibriException(this.status, this.where, this.message);
  final int status;
  final String where;
  final String message;
  @override
  String toString() => '$where: $message ($status)';
}

class Header {
  Header(this.ver, this.cmd, this.seq, this.opcode, this.flag, this.bodyLen);
  final int ver, cmd, seq, opcode, flag, bodyLen;
  bool get compressed => flag != 0;
}

class Kolibri {
  Kolibri(DynamicLibrary lib)
      : _statusStr = lib.lookupFunction<_StatusStrC, _StatusStr>('kb_status_str'),
        _encodeBound =
            lib.lookupFunction<_EncodeBoundC, _EncodeBound>('kb_encode_bound'),
        _encode = lib.lookupFunction<_EncodeC, _Encode>('kb_encode'),
        _decode = lib.lookupFunction<_DecodeC, _Decode>('kb_decode'),
        rxInit = lib.lookupFunction<_RxInitC, _RxInit>('kb_rx_init'),
        rxPush = lib.lookupFunction<_RxPushC, _RxPush>('kb_rx_push'),
        rxNext = lib.lookupFunction<_RxNextC, _RxNext>('kb_rx_next'),
        rxBuffered =
            lib.lookupFunction<_RxBufferedC, _RxBuffered>('kb_rx_buffered');

  final _StatusStr _statusStr;
  final _EncodeBound _encodeBound;
  final _Encode _encode;
  final _Decode _decode;
  final _RxInit rxInit;
  final _RxPush rxPush;
  final _RxNext rxNext;
  final _RxBuffered rxBuffered;

  factory Kolibri.open([String? path]) {
    final name = Platform.isMacOS
        ? 'libkolibri.dylib'
        : Platform.isWindows
            ? 'kolibri.dll'
            : 'libkolibri.so';
    return Kolibri(DynamicLibrary.open(path ?? '../../build/$name'));
  }

  void check(int status, String where) {
    if (status != 0) {
      throw KolibriException(
          status, where, _statusStr(status).toDartString());
    }
  }

  Uint8List encode(
      {required int opcode,
      required int seq,
      required Uint8List payload,
      int cmd = 0}) {
    final capacity = _encodeBound(payload.length);
    final input = calloc<Uint8>(payload.length == 0 ? 1 : payload.length);
    final output = calloc<Uint8>(capacity);
    final written = calloc<Size>();
    try {
      input.asTypedList(payload.length).setAll(0, payload);
      check(
          _encode(cmd, opcode, seq, input, payload.length, output, capacity,
              written),
          'kb_encode');
      return Uint8List.fromList(output.asTypedList(written.value));
    } finally {
      calloc.free(input);
      calloc.free(output);
      calloc.free(written);
    }
  }

  (Header, Uint8List) decode(Uint8List packet) {
    final input = calloc<Uint8>(packet.length);
    final header = calloc<KbHeader>();
    final written = calloc<Size>();
    // Оценку зажимаем потолком, иначе он не сработает на крупном пакете.
    var capacity = packet.length * 4 < 4096 ? 4096 : packet.length * 4;
    if (capacity > 32 * 1024 * 1024) capacity = 32 * 1024 * 1024;
    try {
      input.asTypedList(packet.length).setAll(0, packet);
      while (true) {
        final output = calloc<Uint8>(capacity);
        try {
          final status = _decode(
              input, packet.length, header, output, capacity, written);
          // -2 = KB_ERR_TOO_LARGE: сжатое тело оказалось больше, чем ожидали.
          if (status == -2 && capacity < 32 * 1024 * 1024) {
            capacity *= 4;
            continue;
          }
          check(status, 'kb_decode');
          final h = header.ref;
          return (
            Header(h.ver, h.cmd, h.seq, h.opcode, h.flag, h.bodyLen),
            Uint8List.fromList(output.asTypedList(written.value)),
          );
        } finally {
          calloc.free(output);
        }
      }
    } finally {
      calloc.free(input);
      calloc.free(header);
      calloc.free(written);
    }
  }
}

/// Буфер выделен в нативной памяти: kb_rx хранит указатель на него между
/// вызовами, а память, которой владеет Dart, для этого не годится.
class Receiver {
  Receiver(this._kb, {int capacity = 1 << 20})
      : _rx = calloc<KbRx>(),
        _buffer = calloc<Uint8>(capacity),
        _chunk = calloc<Uint8>(65536),
        _length = calloc<Size>() {
    _kb.check(_kb.rxInit(_rx, _buffer, capacity), 'kb_rx_init');
  }

  final Kolibri _kb;
  final Pointer<KbRx> _rx;
  final Pointer<Uint8> _buffer;
  final Pointer<Uint8> _chunk;
  final Pointer<Size> _length;

  void push(Uint8List data) {
    if (data.isEmpty) return;
    _chunk.asTypedList(data.length).setAll(0, data);
    _kb.check(_kb.rxPush(_rx, _chunk, data.length), 'kb_rx_push');
  }

  /// Копия обязательна: указатель от ядра живёт лишь до следующего push.
  Uint8List? next() {
    final raw = _kb.rxNext(_rx, _length);
    if (raw == nullptr) return null;
    return Uint8List.fromList(raw.asTypedList(_length.value));
  }

  int get buffered => _kb.rxBuffered(_rx);

  void dispose() {
    calloc.free(_rx);
    calloc.free(_buffer);
    calloc.free(_chunk);
    calloc.free(_length);
  }
}

// ── Демонстрация ─────────────────────────────────────────────────────────

String hex(Uint8List bytes) =>
    bytes.map((b) => b.toRadixString(16).padLeft(2, '0')).join();

void main() {
  final kb = Kolibri.open();

  // msgpack: {"chatId": 42}
  final small = Uint8List.fromList(
      [0x81, 0xA6, 0x63, 0x68, 0x61, 0x74, 0x49, 0x64, 0xCD, 0x00, 0x2A]);
  final big = Uint8List.fromList(
      List.generate(1400, (i) => 'kolibri'.codeUnitAt(i % 7)));

  final first = kb.encode(opcode: 64, seq: 1, payload: small);
  print('маленький пакет (${first.length} Б): ${hex(first)}');

  final second = kb.encode(opcode: 64, seq: 2, payload: big);
  print('большой пакет: тело ${big.length} Б -> на проводе ${second.length} Б');

  final stream = Uint8List.fromList([...first, ...second]);
  final rx = Receiver(kb, capacity: 8192);

  print('\nпоток ${stream.length} Б, кусками по 7:');
  for (var offset = 0; offset < stream.length; offset += 7) {
    final end = (offset + 7).clamp(0, stream.length);
    rx.push(Uint8List.sublistView(stream, offset, end));

    for (var raw = rx.next(); raw != null; raw = rx.next()) {
      final (header, body) = kb.decode(raw);
      final state = header.compressed ? 'сжато' : 'как есть';
      print('  seq=${header.seq} opcode=${header.opcode} $state, '
          'тело ${body.length} Б');
    }
  }
  print('\nв буфере осталось: ${rx.buffered} Б');
  rx.dispose();
}
