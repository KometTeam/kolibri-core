/*
 * Адаптер к libzstd: распаковка тела пакета при флаге 0xFF.
 *
 * Файл собирается только при KB_WITH_ZSTD, поэтому обычная сборка ядра
 * остаётся без единой внешней зависимости.
 */

#include "kolibri.h"
#include "kolibri_zstd.h"

#if defined(KB_WITH_ZSTD)

#include <zstd.h>
/* ZSTD_getErrorCode и коды ошибок живут здесь; zstd.h подключает этот заголовок
   сам только начиная с 1.5.5, а Debian 12 ещё на 1.5.4. */
#include <zstd_errors.h>

static kb_status zstd_decompress(const uint8_t *src, size_t src_len,
                                 uint8_t *dst, size_t dst_cap, size_t *dst_len)
{
    size_t produced;
    unsigned long long declared;

    if (src == NULL || dst == NULL) {
        return KB_ERR_INVALID;
    }

    /*
     * Заявленный размер проверяем ДО распаковки: zstd-кадр несёт его в
     * заголовке, и это шанс отбить бомбу, не потратив на неё память.
     * ZSTD_CONTENTSIZE_UNKNOWN означает, что размер не записан — тогда
     * ограничением остаётся размер буфера.
     */
    declared = ZSTD_getFrameContentSize(src, src_len);
    if (declared == ZSTD_CONTENTSIZE_ERROR) {
        return KB_ERR_TRUNCATED;
    }
    if (declared != ZSTD_CONTENTSIZE_UNKNOWN) {
        if (declared > (unsigned long long)KB_MAX_DECOMPRESSED_SIZE) {
            return KB_ERR_LIMIT;
        }
        if (declared > (unsigned long long)dst_cap) {
            return KB_ERR_TOO_LARGE;
        }
    }

    produced = ZSTD_decompress(dst, dst_cap, src, src_len);
    if (ZSTD_isError(produced)) {
        /* Тесный буфер отличаем от порчи данных: вызывающий вправе повторить. */
        if (ZSTD_getErrorCode(produced) == ZSTD_error_dstSize_tooSmall) {
            return KB_ERR_TOO_LARGE;
        }
        return KB_ERR_TRUNCATED;
    }
    if (produced > (size_t)KB_MAX_DECOMPRESSED_SIZE) {
        return KB_ERR_LIMIT;
    }
    if (dst_len != NULL) {
        *dst_len = produced;
    }
    return KB_OK;
}

kb_status kb_zstd_enable(void)
{
    kb_set_zstd_decompressor(zstd_decompress);
    return KB_OK;
}

const char *kb_zstd_version(void)
{
    return ZSTD_versionString();
}

#else /* без KB_WITH_ZSTD */

kb_status kb_zstd_enable(void)
{
    return KB_ERR_UNSUPPORTED;
}

const char *kb_zstd_version(void)
{
    return NULL;
}

#endif /* KB_WITH_ZSTD */
