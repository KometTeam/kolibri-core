/*
 * LZ4 block: распаковка и упаковка.
 *
 * Формат без заголовка кадра и без префикса длины — ровно то, что ходит в теле
 * пакета. Распаковщик перенесён из референсной реализации (kolibri-net,
 * src/protocol/compress.rs) один в один, включая порядок проверок: сначала
 * лимит выходного буфера, потом обрезанность входа. Это важно для
 * дифференциального тестирования — коды ошибок должны совпадать.
 */

#include <string.h>

#include "kolibri.h"

#define KB_LZ4_HASH_LOG 12
#define KB_LZ4_HASH_SIZE (1 << KB_LZ4_HASH_LOG)
#define KB_LZ4_MIN_MATCH 4
#define KB_LZ4_LAST_LITERALS 5
#define KB_LZ4_MF_LIMIT 12
#define KB_LZ4_MAX_OFFSET 65535u

/* ─── Распаковка ───────────────────────────────────────────────────────── */

/*
 * Читает цепочку продолжения длины: байты 255 складываются, первый не-255
 * завершает. Верхняя граница нужна, чтобы на 32-битной платформе сумма не
 * переполнила size_t на злонамеренном входе.
 */
static kb_status kb_read_ext_len(const uint8_t *src, size_t src_len, size_t *pos,
                                 size_t *len)
{
    while (*pos < src_len) {
        uint8_t b = src[*pos];
        (*pos)++;
        if (*len > (size_t)KB_MAX_DECOMPRESSED_SIZE - b) {
            return KB_ERR_LIMIT;
        }
        *len += b;
        if (b != 255) {
            break;
        }
    }
    return KB_OK;
}

kb_status kb_lz4_decompress(const uint8_t *src, size_t src_len, uint8_t *dst,
                            size_t dst_cap, size_t *dst_len)
{
    size_t pos = 0;
    size_t out = 0;

    if (src == NULL || (dst == NULL && dst_cap != 0)) {
        return KB_ERR_INVALID;
    }

    if (dst_cap > (size_t)KB_MAX_DECOMPRESSED_SIZE) {
        /* Буфер крупнее потолка ничего не даёт: лишнее всё равно не примем. */
        dst_cap = (size_t)KB_MAX_DECOMPRESSED_SIZE;
    }

    while (pos < src_len) {
        uint8_t token = src[pos];
        size_t lit_len = (size_t)(token >> 4);
        size_t match_len;
        size_t offset;
        size_t start;
        size_t i;
        kb_status st;

        pos++;

        if (lit_len == 15) {
            st = kb_read_ext_len(src, src_len, &pos, &lit_len);
            if (st != KB_OK) {
                return st;
            }
        }

        if (lit_len > 0) {
            /* Потолок протокола проверяется до буфера: он не зависит от того,
               сколько памяти дал вызывающий, и совпадает с поведением эталона. */
            if (lit_len > (size_t)KB_MAX_DECOMPRESSED_SIZE - out) {
                return KB_ERR_LIMIT;
            }
            if (lit_len > dst_cap - out) {
                return KB_ERR_TOO_LARGE;
            }
            if (lit_len > src_len - pos) {
                return KB_ERR_TRUNCATED;
            }
            memcpy(dst + out, src + pos, lit_len);
            out += lit_len;
            pos += lit_len;
        }

        /* Последняя последовательность состоит из одних литералов. */
        if (pos >= src_len) {
            break;
        }

        if (src_len - pos < 2) {
            return KB_ERR_TRUNCATED;
        }
        offset = (size_t)src[pos] | ((size_t)src[pos + 1] << 8);
        pos += 2;
        if (offset == 0) {
            return KB_ERR_LZ4_ZERO_OFFSET;
        }

        match_len = (size_t)(token & 0x0F) + KB_LZ4_MIN_MATCH;
        if ((token & 0x0F) == 0x0F) {
            st = kb_read_ext_len(src, src_len, &pos, &match_len);
            if (st != KB_OK) {
                return st;
            }
        }

        if (match_len > (size_t)KB_MAX_DECOMPRESSED_SIZE - out) {
            return KB_ERR_LIMIT;
        }
        if (match_len > dst_cap - out) {
            return KB_ERR_TOO_LARGE;
        }
        if (offset > out) {
            return KB_ERR_LZ4_OFFSET_RANGE;
        }

        /* Копия может перекрываться (offset < match_len) — только побайтно. */
        start = out - offset;
        for (i = 0; i < match_len; i++) {
            dst[out] = dst[start + i];
            out++;
        }
    }

    if (dst_len != NULL) {
        *dst_len = out;
    }
    return KB_OK;
}

/* ─── Упаковка ─────────────────────────────────────────────────────────── */

size_t kb_lz4_compress_bound(size_t src_len)
{
    return src_len + (src_len / 255) + 16;
}

static uint32_t kb_read32(const uint8_t *p)
{
    uint32_t v;
    memcpy(&v, p, 4);
    return v;
}

/*
 * Порядок байт значения не имеет: хеш нужен лишь для того, чтобы одинаковые
 * четвёрки попадали в один слот, а совпадение всё равно перепроверяется.
 */
static uint32_t kb_lz4_hash(uint32_t sequence)
{
    return (sequence * 2654435761u) >> (32 - KB_LZ4_HASH_LOG);
}

static kb_status kb_write_ext_len(uint8_t **op, uint8_t *oend, size_t len)
{
    while (len >= 255) {
        if (*op >= oend) {
            return KB_ERR_TOO_LARGE;
        }
        **op = 255;
        (*op)++;
        len -= 255;
    }
    if (*op >= oend) {
        return KB_ERR_TOO_LARGE;
    }
    **op = (uint8_t)len;
    (*op)++;
    return KB_OK;
}

static kb_status kb_write_literals(uint8_t **op, uint8_t *oend,
                                   const uint8_t *from, size_t len)
{
    if ((size_t)(oend - *op) < len) {
        return KB_ERR_TOO_LARGE;
    }
    if (len > 0) {
        memcpy(*op, from, len);
        *op += len;
    }
    return KB_OK;
}

/*
 * Однопроходный компрессор с таблицей последних позиций. Степень сжатия слегка
 * уступает референсной библиотеке, но формат корректен: последние пять байт
 * всегда литералы, а последний матч начинается не позже чем за двенадцать байт
 * до конца — этих двух правил требует спецификация LZ4, и строгие декодеры её
 * проверяют.
 */
kb_status kb_lz4_compress(const uint8_t *src, size_t src_len, uint8_t *dst,
                          size_t dst_cap, size_t *dst_len)
{
    uint32_t table[KB_LZ4_HASH_SIZE];
    const uint8_t *ip;
    const uint8_t *anchor;
    const uint8_t *iend;
    uint8_t *op = dst;
    uint8_t *oend = dst + dst_cap;
    size_t lit_len;
    kb_status st;

    if (src == NULL || dst == NULL) {
        return KB_ERR_INVALID;
    }

    memset(table, 0, sizeof table);
    ip = src;
    anchor = src;
    iend = src + src_len;

    if (src_len > KB_LZ4_MF_LIMIT) {
        const uint8_t *mflimit = iend - KB_LZ4_MF_LIMIT;
        const uint8_t *matchlimit = iend - KB_LZ4_LAST_LITERALS;

        while (ip < mflimit) {
            uint32_t h = kb_lz4_hash(kb_read32(ip));
            uint32_t slot = table[h];
            const uint8_t *ref;

            /* В таблице хранится позиция + 1, ноль означает «пусто». */
            table[h] = (uint32_t)(ip - src) + 1;

            if (slot == 0) {
                ip++;
                continue;
            }
            ref = src + (slot - 1);
            if ((size_t)(ip - ref) > KB_LZ4_MAX_OFFSET ||
                kb_read32(ref) != kb_read32(ip)) {
                ip++;
                continue;
            }

            {
                const uint8_t *cur = ip + KB_LZ4_MIN_MATCH;
                const uint8_t *cref = ref + KB_LZ4_MIN_MATCH;
                uint16_t offset = (uint16_t)(ip - ref);
                size_t match_len;
                uint8_t token;

                while (cur < matchlimit && *cur == *cref) {
                    cur++;
                    cref++;
                }
                match_len = (size_t)(cur - ip) - KB_LZ4_MIN_MATCH;
                lit_len = (size_t)(ip - anchor);

                token = (uint8_t)((lit_len >= 15 ? 15u : lit_len) << 4);
                token |= (uint8_t)(match_len >= 15 ? 15u : match_len);

                if (op >= oend) {
                    return KB_ERR_TOO_LARGE;
                }
                *op = token;
                op++;

                if (lit_len >= 15) {
                    st = kb_write_ext_len(&op, oend, lit_len - 15);
                    if (st != KB_OK) {
                        return st;
                    }
                }
                st = kb_write_literals(&op, oend, anchor, lit_len);
                if (st != KB_OK) {
                    return st;
                }

                if ((size_t)(oend - op) < 2) {
                    return KB_ERR_TOO_LARGE;
                }
                *op = (uint8_t)(offset & 0xFF);
                op++;
                *op = (uint8_t)(offset >> 8);
                op++;

                if (match_len >= 15) {
                    st = kb_write_ext_len(&op, oend, match_len - 15);
                    if (st != KB_OK) {
                        return st;
                    }
                }

                ip = cur;
                anchor = cur;
            }
        }
    }

    /* Хвост всегда уходит литералами, без смещения матча. */
    lit_len = (size_t)(iend - anchor);
    if (op >= oend) {
        return KB_ERR_TOO_LARGE;
    }
    *op = (uint8_t)((lit_len >= 15 ? 15u : lit_len) << 4);
    op++;
    if (lit_len >= 15) {
        st = kb_write_ext_len(&op, oend, lit_len - 15);
        if (st != KB_OK) {
            return st;
        }
    }
    st = kb_write_literals(&op, oend, anchor, lit_len);
    if (st != KB_OK) {
        return st;
    }

    if (dst_len != NULL) {
        *dst_len = (size_t)(op - dst);
    }
    return KB_OK;
}

/* ─── Подключаемый zstd ────────────────────────────────────────────────── */

/*
 * Глобальный хук: ядро не зависит от libzstd, а хост при желании подставляет
 * свой распаковщик. Так же, как TLS подключается таблицей функций.
 */
static kb_zstd_fn kb_zstd_hook = NULL;

void kb_set_zstd_decompressor(kb_zstd_fn decompressor)
{
    kb_zstd_hook = decompressor;
}

kb_zstd_fn kb_get_zstd_decompressor(void)
{
    return kb_zstd_hook;
}

kb_status kb_decompress_body(uint8_t flag, const uint8_t *src, size_t src_len,
                             uint8_t *dst, size_t dst_cap, size_t *dst_len)
{
    if (src == NULL || (dst == NULL && dst_cap != 0)) {
        return KB_ERR_INVALID;
    }

    /* Потолок протокола действует и здесь: буфер крупнее него ничего не даёт. */
    if (dst_cap > (size_t)KB_MAX_DECOMPRESSED_SIZE) {
        dst_cap = (size_t)KB_MAX_DECOMPRESSED_SIZE;
    }

    if (flag == KB_COMPRESSION_NONE) {
        if (src_len > dst_cap) {
            return src_len > (size_t)KB_MAX_DECOMPRESSED_SIZE ? KB_ERR_LIMIT
                                                              : KB_ERR_TOO_LARGE;
        }
        if (src_len > 0) {
            memcpy(dst, src, src_len);
        }
        if (dst_len != NULL) {
            *dst_len = src_len;
        }
        return KB_OK;
    }

    if (flag == KB_COMPRESSION_ZSTD) {
        if (kb_zstd_hook == NULL) {
            return KB_ERR_UNSUPPORTED;
        }
        return kb_zstd_hook(src, src_len, dst, dst_cap, dst_len);
    }

    /* Любое другое значение флага — LZ4 block, а само оно лишь оценка. */
    return kb_lz4_decompress(src, src_len, dst, dst_cap, dst_len);
}

/* ─── Определение формата ──────────────────────────────────────────────── */

kb_status kb_decompress(const uint8_t *src, size_t src_len, uint8_t *dst,
                        size_t dst_cap, size_t *dst_len)
{
    if (src == NULL) {
        return KB_ERR_INVALID;
    }
    /* Zstandard: 28 B5 2F FD */
    if (src_len >= 4 && src[0] == 0x28 && src[1] == 0xB5 && src[2] == 0x2F &&
        src[3] == 0xFD) {
        return kb_decompress_body(KB_COMPRESSION_ZSTD, src, src_len, dst,
                                  dst_cap, dst_len);
    }
    /* LZ4 frame: 04 22 4D 18 */
    if (src_len >= 4 && src[0] == 0x04 && src[1] == 0x22 && src[2] == 0x4D &&
        src[3] == 0x18) {
        return KB_ERR_UNSUPPORTED;
    }
    return kb_lz4_decompress(src, src_len, dst, dst_cap, dst_len);
}
