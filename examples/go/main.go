// Пример на Go: cgo напрямую по заголовку ядра, без промежуточного шима.
//
//	make -C ../.. && go run .
//
// Обёртка ниже — это и есть «биндинг»: идиоматичный слой на 60 строк.
// Отдельного FFI-слоя между Go и ядром нет, cgo читает kolibri.h как есть.
package main

/*
#cgo CFLAGS: -I${SRCDIR}/../../include
#cgo LDFLAGS: ${SRCDIR}/../../build/libkolibri.a
#include <stdlib.h>
#include "kolibri.h"
*/
import "C"

import (
	"errors"
	"fmt"
	"unsafe"
)

const maxDecompressed = 32 * 1024 * 1024

type Header struct {
	Ver     uint8
	Cmd     uint8
	Seq     uint16
	Opcode  uint16
	Flag    uint8
	BodyLen uint32
}

func (h Header) Compressed() bool { return h.Flag != 0 }

func check(st C.kb_status, where string) error {
	if st == C.KB_OK {
		return nil
	}
	return fmt.Errorf("%s: %s (%d)", where, C.GoString(C.kb_status_str(st)), int(st))
}

// Encode собирает пакет. Указатели на память Go живут только на время вызова —
// ядро их не сохраняет, поэтому правила cgo не нарушаются.
func Encode(cmd uint8, opcode, seq uint16, payload []byte) ([]byte, error) {
	size := C.kb_encode_bound(C.size_t(len(payload)))
	out := make([]byte, int(size))

	var in *C.uint8_t
	if len(payload) > 0 {
		in = (*C.uint8_t)(unsafe.Pointer(&payload[0]))
	}

	var written C.size_t
	st := C.kb_encode(C.uint8_t(cmd), C.uint16_t(opcode), C.uint16_t(seq),
		in, C.size_t(len(payload)),
		(*C.uint8_t)(unsafe.Pointer(&out[0])), size, &written)
	if err := check(st, "kb_encode"); err != nil {
		return nil, err
	}
	return out[:written], nil
}

// Decode разбирает целый пакет. Для сжатого тела итоговый размер заранее
// неизвестен, поэтому буфер растёт, пока ядро не скажет, что хватило.
func Decode(packet []byte) (Header, []byte, error) {
	if len(packet) == 0 {
		return Header{}, nil, errors.New("пустой пакет")
	}
	var h C.kb_header
	var written C.size_t

	size := len(packet) * 4
	if size < 4096 {
		size = 4096
	}
	// Оценку зажимаем потолком: иначе на крупном пакете первый же буфер будет
	// больше лимита и лимит никогда не сработает.
	if size > maxDecompressed {
		size = maxDecompressed
	}
	for {
		out := make([]byte, size)
		st := C.kb_decode((*C.uint8_t)(unsafe.Pointer(&packet[0])),
			C.size_t(len(packet)), &h,
			(*C.uint8_t)(unsafe.Pointer(&out[0])), C.size_t(size), &written)

		if st == C.KB_ERR_TOO_LARGE && size < maxDecompressed {
			size *= 4
			continue
		}
		if err := check(st, "kb_decode"); err != nil {
			return Header{}, nil, err
		}
		return Header{
			Ver: uint8(h.ver), Cmd: uint8(h.cmd),
			Seq: uint16(h.seq), Opcode: uint16(h.opcode),
			Flag: uint8(h.flag), BodyLen: uint32(h.body_len),
		}, out[:written], nil
	}
}

// Receiver собирает поток из сокета обратно в пакеты.
//
// Буфер выделен в C, а не в Go, и это не мелочь: kb_rx хранит указатель на него
// между вызовами, а правила cgo запрещают C удерживать указатели на память Go.
// Альтернатива — runtime.Pinner, но тогда всё равно нужен явный Unpin.
type Receiver struct {
	rx  C.kb_rx
	buf unsafe.Pointer
}

func NewReceiver(capacity int) (*Receiver, error) {
	buf := C.malloc(C.size_t(capacity))
	if buf == nil {
		return nil, errors.New("не хватило памяти под буфер приёма")
	}
	r := &Receiver{buf: buf}
	if err := check(C.kb_rx_init(&r.rx, (*C.uint8_t)(buf), C.size_t(capacity)),
		"kb_rx_init"); err != nil {
		C.free(buf)
		return nil, err
	}
	return r, nil
}

func (r *Receiver) Close() {
	if r.buf != nil {
		C.free(r.buf)
		r.buf = nil
	}
}

func (r *Receiver) Push(data []byte) error {
	if len(data) == 0 {
		return nil
	}
	return check(C.kb_rx_push(&r.rx, (*C.uint8_t)(unsafe.Pointer(&data[0])),
		C.size_t(len(data))), "kb_rx_push")
}

// Next возвращает следующий целый пакет или nil. Копия обязательна: указатель
// от ядра живёт лишь до следующего Push.
func (r *Receiver) Next() []byte {
	var length C.size_t
	p := C.kb_rx_next(&r.rx, &length)
	if p == nil {
		return nil
	}
	return C.GoBytes(unsafe.Pointer(p), C.int(length))
}

func (r *Receiver) Buffered() int { return int(C.kb_rx_buffered(&r.rx)) }

func main() {
	// msgpack: {"chatId": 42}
	small := []byte{0x81, 0xA6, 'c', 'h', 'a', 't', 'I', 'd', 0xCD, 0x00, 0x2A}
	big := make([]byte, 1400)
	for i := range big {
		big[i] = "kolibri"[i%7]
	}

	first, err := Encode(C.KB_CMD_REQUEST, 64, 1, small)
	if err != nil {
		panic(err)
	}
	fmt.Printf("маленький пакет (%d Б): %x\n", len(first), first)

	second, err := Encode(C.KB_CMD_REQUEST, 64, 2, big)
	if err != nil {
		panic(err)
	}
	fmt.Printf("большой пакет: тело %d Б -> на проводе %d Б\n", len(big), len(second))

	stream := append(append([]byte{}, first...), second...)

	rx, err := NewReceiver(8192)
	if err != nil {
		panic(err)
	}
	defer rx.Close()

	fmt.Printf("\nпоток %d Б, кусками по 7:\n", len(stream))
	for offset := 0; offset < len(stream); offset += 7 {
		end := offset + 7
		if end > len(stream) {
			end = len(stream)
		}
		if err := rx.Push(stream[offset:end]); err != nil {
			panic(err)
		}
		for raw := rx.Next(); raw != nil; raw = rx.Next() {
			header, body, err := Decode(raw)
			if err != nil {
				panic(err)
			}
			state := "как есть"
			if header.Compressed() {
				state = "сжато"
			}
			fmt.Printf("  seq=%d opcode=%d %s, тело %d Б\n",
				header.Seq, header.Opcode, state, len(body))
		}
	}
	fmt.Printf("\nв буфере осталось: %d Б\n", rx.Buffered())
}
