/*
 * Реассемблер потока: байты из сокета обратно в пакеты.
 *
 * Буфер даёт вызывающая сторона. В отличие от референса, который наращивает
 * Vec до 16 МБ, здесь потолок задаётся ёмкостью переданного буфера: он должен
 * быть не меньше самого крупного ожидаемого пакета, иначе тот никогда не
 * соберётся и kb_rx_push вернёт KB_ERR_OVERFLOW.
 */

#include <string.h>

#include "kolibri.h"

kb_status kb_rx_init(kb_rx *rx, uint8_t *buffer, size_t capacity)
{
    if (rx == NULL || buffer == NULL || capacity < KB_HEADER_SIZE) {
        return KB_ERR_INVALID;
    }
    rx->buf = buffer;
    rx->cap = capacity;
    rx->start = 0;
    rx->end = 0;
    return KB_OK;
}

void kb_rx_reset(kb_rx *rx)
{
    if (rx == NULL) {
        return;
    }
    rx->start = 0;
    rx->end = 0;
}

size_t kb_rx_buffered(const kb_rx *rx)
{
    if (rx == NULL) {
        return 0;
    }
    return rx->end - rx->start;
}

kb_status kb_rx_push(kb_rx *rx, const uint8_t *data, size_t len)
{
    if (rx == NULL || rx->buf == NULL) {
        return KB_ERR_INVALID;
    }
    if (data == NULL && len != 0) {
        return KB_ERR_INVALID;
    }
    if (len == 0) {
        return KB_OK;
    }

    if (len > rx->cap - rx->end) {
        /* Сначала подтягиваем неразобранный хвост к началу буфера. */
        if (rx->start > 0) {
            size_t pending = rx->end - rx->start;
            if (pending > 0) {
                memmove(rx->buf, rx->buf + rx->start, pending);
            }
            rx->start = 0;
            rx->end = pending;
        }
        if (len > rx->cap - rx->end) {
            /* Как в референсе: переполнение сбрасывает состояние. */
            kb_rx_reset(rx);
            return KB_ERR_OVERFLOW;
        }
    }

    memcpy(rx->buf + rx->end, data, len);
    rx->end += len;
    return KB_OK;
}

const uint8_t *kb_rx_next(kb_rx *rx, size_t *packet_len)
{
    size_t available;
    size_t total;
    const uint8_t *packet;

    if (rx == NULL || rx->buf == NULL) {
        return NULL;
    }

    available = rx->end - rx->start;
    if (available < KB_HEADER_SIZE) {
        return NULL;
    }

    total = kb_packet_total_len(rx->buf + rx->start, available);
    if (total == 0 || total > available) {
        return NULL;
    }

    packet = rx->buf + rx->start;
    rx->start += total;
    if (rx->start == rx->end) {
        rx->start = 0;
        rx->end = 0;
    }

    if (packet_len != NULL) {
        *packet_len = total;
    }
    return packet;
}
