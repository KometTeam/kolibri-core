/*
 * Кодек пакета: 10-байтный заголовок big-endian плюс тело.
 *
 * Перенесено из референсной реализации (kolibri-net, src/protocol/codec.rs).
 * Расхождение ровно одно, оно помечено ниже.
 */

#include <string.h>

#include "kolibri.h"

kb_status kb_header_decode(const uint8_t *src, size_t len, kb_header *out)
{
    uint32_t packed;

    if (src == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (len < KB_HEADER_SIZE) {
        return KB_ERR_TRUNCATED;
    }

    packed = ((uint32_t)src[6] << 24) | ((uint32_t)src[7] << 16) |
             ((uint32_t)src[8] << 8) | (uint32_t)src[9];

    out->ver = src[0];
    out->cmd = src[1];
    out->seq = (uint16_t)(((uint16_t)src[2] << 8) | (uint16_t)src[3]);
    out->opcode = (uint16_t)(((uint16_t)src[4] << 8) | (uint16_t)src[5]);
    out->flag = (uint8_t)(packed >> 24);
    out->body_len = packed & KB_MAX_BODY_LEN;
    return KB_OK;
}

kb_status kb_header_encode(const kb_header *header, uint8_t *out, size_t out_cap)
{
    uint32_t packed;

    if (header == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (out_cap < KB_HEADER_SIZE) {
        return KB_ERR_TOO_LARGE;
    }
    if (header->body_len > KB_MAX_BODY_LEN) {
        return KB_ERR_INVALID;
    }

    packed = ((uint32_t)header->flag << 24) | (header->body_len & KB_MAX_BODY_LEN);

    out[0] = header->ver;
    out[1] = header->cmd;
    out[2] = (uint8_t)(header->seq >> 8);
    out[3] = (uint8_t)(header->seq & 0xFF);
    out[4] = (uint8_t)(header->opcode >> 8);
    out[5] = (uint8_t)(header->opcode & 0xFF);
    out[6] = (uint8_t)(packed >> 24);
    out[7] = (uint8_t)((packed >> 16) & 0xFF);
    out[8] = (uint8_t)((packed >> 8) & 0xFF);
    out[9] = (uint8_t)(packed & 0xFF);
    return KB_OK;
}

size_t kb_packet_total_len(const uint8_t *src, size_t len)
{
    uint32_t body_len;

    if (src == NULL || len < KB_HEADER_SIZE) {
        return 0;
    }
    body_len = (((uint32_t)src[6] << 24) | ((uint32_t)src[7] << 16) |
                ((uint32_t)src[8] << 8) | (uint32_t)src[9]) &
               KB_MAX_BODY_LEN;
    return (size_t)KB_HEADER_SIZE + (size_t)body_len;
}

size_t kb_encode_bound(size_t payload_len)
{
    if (payload_len < KB_COMPRESSION_THRESHOLD) {
        return (size_t)KB_HEADER_SIZE + payload_len;
    }
    /* Сжатые данные бывают длиннее исходных — берём верхнюю оценку LZ4. */
    return (size_t)KB_HEADER_SIZE + kb_lz4_compress_bound(payload_len);
}

kb_status kb_encode(uint8_t cmd, uint16_t opcode, uint16_t seq,
                    const uint8_t *payload, size_t payload_len, uint8_t *out,
                    size_t out_cap, size_t *out_len)
{
    kb_header header;
    size_t body_len;
    kb_status st;

    if (out == NULL) {
        return KB_ERR_INVALID;
    }
    if (payload == NULL && payload_len != 0) {
        return KB_ERR_INVALID;
    }
    if (payload_len > KB_MAX_BODY_LEN) {
        return KB_ERR_INVALID;
    }
    if (out_cap < KB_HEADER_SIZE) {
        return KB_ERR_TOO_LARGE;
    }

    header.ver = KB_PROTOCOL_VERSION;
    header.cmd = cmd;
    header.seq = seq;
    header.opcode = opcode;

    if (payload_len < KB_COMPRESSION_THRESHOLD) {
        if (payload_len > out_cap - KB_HEADER_SIZE) {
            return KB_ERR_TOO_LARGE;
        }
        if (payload_len > 0) {
            memcpy(out + KB_HEADER_SIZE, payload, payload_len);
        }
        body_len = payload_len;
        header.flag = 0;
    } else {
        size_t ratio;

        st = kb_lz4_compress(payload, payload_len, out + KB_HEADER_SIZE,
                             out_cap - KB_HEADER_SIZE, &body_len);
        if (st != KB_OK) {
            return st;
        }
        if (body_len > KB_MAX_BODY_LEN) {
            return KB_ERR_TOO_LARGE;
        }

        /*
         * Флаг — подсказка о степени сжатия; на разбор влияет только то, ноль он
         * или нет. Формула та же, что в референсе: (payload_len / body_len) + 1,
         * усечённое до байта.
         *
         * У усечения есть теоретический край: при степени сжатия ровно 255 флаг
         * схлопнется в 0 и сжатое тело будет помечено как несжатое. На LZ4 это
         * недостижимо — отношение асимптотически подходит к 255 снизу, но
         * фиксированные накладные расходы держат целую часть на 254 (проверено
         * до 16 МБ нулей). Зажим ниже — страховка на случай другого
         * компрессора, а не заплатка на живой баг.
         */
        ratio = payload_len / (body_len == 0 ? 1 : body_len);
        header.flag = (uint8_t)((ratio + 1) & 0xFF);
        /*
         * Флаг обязан остаться в диапазоне LZ4: 0x00 значит «не сжато», а
         * 0xFF — «тело сжато zstd». Формула даёт 0xFF при степени сжатия 254
         * (проверено: мегабайт нулей), и тогда сервер попытался бы разобрать
         * наш LZ4-блок как zstd-кадр. Значение всё равно лишь подсказка о
         * размере, поэтому зажимаем без потерь.
         */
        if (header.flag == KB_COMPRESSION_NONE ||
            header.flag == KB_COMPRESSION_ZSTD) {
            header.flag = 0xFE;
        }
    }

    header.body_len = (uint32_t)body_len;
    st = kb_header_encode(&header, out, out_cap);
    if (st != KB_OK) {
        return st;
    }
    if (out_len != NULL) {
        *out_len = (size_t)KB_HEADER_SIZE + body_len;
    }
    return KB_OK;
}

kb_status kb_decode(const uint8_t *packet, size_t packet_len, kb_header *header,
                    uint8_t *out, size_t out_cap, size_t *out_len)
{
    kb_header local;
    const uint8_t *body;
    kb_status st;

    if (packet == NULL) {
        return KB_ERR_INVALID;
    }

    st = kb_header_decode(packet, packet_len, &local);
    if (st != KB_OK) {
        return st;
    }
    if (header != NULL) {
        *header = local;
    }

    if (local.body_len == 0) {
        if (out_len != NULL) {
            *out_len = 0;
        }
        return KB_OK;
    }

    if ((size_t)local.body_len > packet_len - KB_HEADER_SIZE) {
        return KB_ERR_TRUNCATED;
    }
    body = packet + KB_HEADER_SIZE;

    if (out == NULL) {
        /* Заголовок уже отдан; тело вызывающей стороне не нужно. */
        return KB_OK;
    }

    /* Формат тела решает флаг, а не магия: так же трактует его клиент. */
    return kb_decompress_body(local.flag, body, (size_t)local.body_len, out,
                              out_cap, out_len);
}
