/*
 * Адаптер к libzstd.
 *
 * Собирается только при KB_WITH_ZSTD — ядро от zstd не зависит и знать про
 * него не хочет, ровно как и про TLS. Это образец: свой распаковщик
 * подключается одной функцией через kb_set_zstd_decompressor.
 *
 * Зачем это вообще нужно: в теле пакета сервер вправе прислать zstd, и
 * официальный клиент это распаковывает — при флаге 0xFF в заголовке. Без
 * адаптера такие пакеты дадут KB_ERR_UNSUPPORTED.
 *
 * Сжатие не нужно ни здесь, ни в клиенте: исходящее всегда LZ4 block, а
 * libzstd в самом приложении экспортирует единственную функцию распаковки.
 */

#ifndef KOLIBRI_ZSTD_H
#define KOLIBRI_ZSTD_H

#include "kolibri.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * Поставить распаковщик zstd на базе libzstd.
 *
 * Зовётся один раз при старте, до работы с сессиями: указатель глобальный.
 * Возвращает KB_ERR_UNSUPPORTED, если ядро собрано без KB_WITH_ZSTD.
 */
kb_status kb_zstd_enable(void);

/* Версия libzstd, с которой собрано; NULL, если поддержки нет. */
const char *kb_zstd_version(void);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_ZSTD_H */
