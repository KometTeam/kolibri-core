/* Кадры WebSocket. См. include/kolibri_ws.h. */

#include "kolibri_ws.h"

kb_status kb_ws_build(uint8_t opcode, const uint8_t *payload,
                      size_t payload_len, uint32_t mask_key, uint8_t *out,
                      size_t cap, size_t *out_len)
{
    uint8_t mask[4];
    size_t header;
    size_t need;
    size_t i;

    if (out == NULL || (payload == NULL && payload_len != 0)) {
        return KB_ERR_INVALID;
    }
    /* Размер поля длины. */
    if (payload_len < 126) {
        header = 2;
    } else if (payload_len <= 0xFFFF) {
        header = 4;
    } else {
        header = 10;
    }
    header += 4; /* маска-ключ */
    need = header + payload_len;
    if (need > cap) {
        return KB_ERR_TOO_LARGE;
    }

    out[0] = (uint8_t)(0x80 | (opcode & 0x0F)); /* FIN=1 */
    if (payload_len < 126) {
        out[1] = (uint8_t)(0x80 | (uint8_t)payload_len);
        i = 2;
    } else if (payload_len <= 0xFFFF) {
        out[1] = (uint8_t)(0x80 | 126);
        out[2] = (uint8_t)((payload_len >> 8) & 0xFF);
        out[3] = (uint8_t)(payload_len & 0xFF);
        i = 4;
    } else {
        int s;
        out[1] = (uint8_t)(0x80 | 127);
        for (s = 0; s < 8; s++) {
            out[2 + s] =
                (uint8_t)(((uint64_t)payload_len >> (56 - s * 8)) & 0xFF);
        }
        i = 10;
    }

    mask[0] = (uint8_t)((mask_key >> 24) & 0xFF);
    mask[1] = (uint8_t)((mask_key >> 16) & 0xFF);
    mask[2] = (uint8_t)((mask_key >> 8) & 0xFF);
    mask[3] = (uint8_t)(mask_key & 0xFF);
    out[i++] = mask[0];
    out[i++] = mask[1];
    out[i++] = mask[2];
    out[i++] = mask[3];

    {
        size_t k;
        for (k = 0; k < payload_len; k++) {
            out[i + k] = (uint8_t)(payload[k] ^ mask[k & 3]);
        }
    }
    if (out_len != NULL) {
        *out_len = need;
    }
    return KB_OK;
}

kb_status kb_ws_build_text(const char *text, size_t len, uint32_t mask_key,
                           uint8_t *out, size_t cap, size_t *out_len)
{
    return kb_ws_build(KB_WS_TEXT, (const uint8_t *)text, len, mask_key, out,
                       cap, out_len);
}

kb_status kb_ws_parse(const uint8_t *buf, size_t len, kb_ws_frame *frame,
                      size_t *consumed)
{
    uint64_t payload_len;
    size_t pos;
    int masked;

    if (buf == NULL || frame == NULL) {
        return KB_ERR_INVALID;
    }
    if (len < 2) {
        return KB_ERR_TRUNCATED;
    }
    frame->fin = (buf[0] & 0x80) ? 1 : 0;
    frame->opcode = (uint8_t)(buf[0] & 0x0F);
    /* RSV1-3 должны быть нулём (расширений не согласовывали). */
    if (buf[0] & 0x70) {
        return KB_ERR_INVALID;
    }
    masked = (buf[1] & 0x80) ? 1 : 0;
    if (masked) {
        return KB_ERR_INVALID; /* сервер не должен маскировать */
    }
    payload_len = (uint64_t)(buf[1] & 0x7F);
    pos = 2;
    if (payload_len == 126) {
        if (len < 4) {
            return KB_ERR_TRUNCATED;
        }
        payload_len = ((uint64_t)buf[2] << 8) | (uint64_t)buf[3];
        pos = 4;
    } else if (payload_len == 127) {
        int s;
        if (len < 10) {
            return KB_ERR_TRUNCATED;
        }
        payload_len = 0;
        for (s = 0; s < 8; s++) {
            payload_len = (payload_len << 8) | (uint64_t)buf[2 + s];
        }
        pos = 10;
    }
    if (payload_len > (uint64_t)(len - pos)) {
        return KB_ERR_TRUNCATED;
    }
    frame->payload = buf + pos;
    frame->payload_len = (size_t)payload_len;
    if (consumed != NULL) {
        *consumed = pos + (size_t)payload_len;
    }
    return KB_OK;
}
