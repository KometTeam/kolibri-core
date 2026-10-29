// Пример на Swift: заголовок ядра импортируется напрямую, шима нет вовсе.
//
//     make -C ../.. && swiftc -I ../../include demo.swift ../../build/libkolibri.a -o demo && ./demo
//
// Обёртка ниже и есть «биндинг»: идиоматичный слой на 50 строк с исключениями,
// массивами и автоматическим временем жизни.

import CKolibri

enum KolibriError: Error, CustomStringConvertible {
    case failed(kb_status, String)

    var description: String {
        if case let .failed(status, where_) = self {
            return "\(where_): \(String(cString: kb_status_str(status))) (\(status.rawValue))"
        }
        return "kolibri"
    }
}

@inline(__always)
func check(_ status: kb_status, _ where_: String) throws {
    guard status == KB_OK else { throw KolibriError.failed(status, where_) }
}

struct Header {
    let ver: UInt8
    let cmd: UInt8
    let seq: UInt16
    let opcode: UInt16
    let flag: UInt8
    let bodyLen: UInt32

    var compressed: Bool { flag != 0 }
}

/// Указатели на память Swift живут только на время вызова — ядро их не удерживает.
func encode(cmd: UInt8 = UInt8(KB_CMD_REQUEST), opcode: UInt16, seq: UInt16,
            payload: [UInt8]) throws -> [UInt8] {
    let capacity = kb_encode_bound(payload.count)
    var out = [UInt8](repeating: 0, count: capacity)
    var written = 0

    try payload.withUnsafeBufferPointer { input in
        try out.withUnsafeMutableBufferPointer { output in
            try check(kb_encode(cmd, opcode, seq,
                                input.baseAddress, payload.count,
                                output.baseAddress, capacity, &written),
                      "kb_encode")
        }
    }
    return Array(out[..<written])
}

func decode(_ packet: [UInt8]) throws -> (Header, [UInt8]) {
    var header = kb_header()
    var written = 0
    // Оценку зажимаем потолком, иначе он не сработает на крупном пакете.
    var capacity = min(max(packet.count * 4, 4096), 32 * 1024 * 1024)

    while true {
        var out = [UInt8](repeating: 0, count: capacity)
        let status: kb_status = packet.withUnsafeBufferPointer { input in
            out.withUnsafeMutableBufferPointer { output in
                kb_decode(input.baseAddress, packet.count, &header,
                          output.baseAddress, capacity, &written)
            }
        }
        if status == KB_ERR_TOO_LARGE && capacity < 32 * 1024 * 1024 {
            capacity *= 4
            continue
        }
        try check(status, "kb_decode")
        return (Header(ver: header.ver, cmd: header.cmd, seq: header.seq,
                       opcode: header.opcode, flag: header.flag,
                       bodyLen: header.body_len),
                Array(out[..<written]))
    }
}

/// Буфер выделен вручную: kb_rx хранит указатель на него между вызовами, так что
/// временный указатель на массив Swift сюда не годится.
final class Receiver {
    private var rx = kb_rx()
    private let buffer: UnsafeMutablePointer<UInt8>

    init(capacity: Int = 1 << 20) throws {
        buffer = UnsafeMutablePointer<UInt8>.allocate(capacity: capacity)
        try check(kb_rx_init(&rx, buffer, capacity), "kb_rx_init")
    }

    deinit { buffer.deallocate() }

    func push(_ data: [UInt8]) throws {
        guard !data.isEmpty else { return }
        try data.withUnsafeBufferPointer { input in
            try check(kb_rx_push(&rx, input.baseAddress, data.count), "kb_rx_push")
        }
    }

    /// Копия обязательна: указатель от ядра живёт лишь до следующего push.
    func next() -> [UInt8]? {
        var length = 0
        guard let raw = kb_rx_next(&rx, &length) else { return nil }
        return Array(UnsafeBufferPointer(start: raw, count: length))
    }

    var buffered: Int { kb_rx_buffered(&rx) }
}

// ── Демонстрация ─────────────────────────────────────────────────────────

/// Своя шестнадцатеричная печать, чтобы примеру не понадобился Foundation.
func hex(_ bytes: [UInt8]) -> String {
    let digits = Array("0123456789abcdef")
    var out = ""
    out.reserveCapacity(bytes.count * 2)
    for byte in bytes {
        out.append(digits[Int(byte >> 4)])
        out.append(digits[Int(byte & 0x0F)])
    }
    return out
}

// msgpack: {"chatId": 42}
let small: [UInt8] = [0x81, 0xA6, 0x63, 0x68, 0x61, 0x74, 0x49, 0x64, 0xCD, 0x00, 0x2A]
let big = (0..<1400).map { Array("kolibri".utf8)[$0 % 7] }

let first = try encode(opcode: 64, seq: 1, payload: small)
print("маленький пакет (\(first.count) Б): " + hex(first))

let second = try encode(opcode: 64, seq: 2, payload: big)
print("большой пакет: тело \(big.count) Б -> на проводе \(second.count) Б")

let stream = first + second
let rx = try Receiver(capacity: 8192)

print("\nпоток \(stream.count) Б, кусками по 7:")
var offset = 0
while offset < stream.count {
    let end = min(offset + 7, stream.count)
    try rx.push(Array(stream[offset..<end]))
    offset = end

    while let raw = rx.next() {
        let (header, body) = try decode(raw)
        let state = header.compressed ? "сжато" : "как есть"
        print("  seq=\(header.seq) opcode=\(header.opcode) \(state), тело \(body.count) Б")
    }
}
print("\nв буфере осталось: \(rx.buffered) Б")
