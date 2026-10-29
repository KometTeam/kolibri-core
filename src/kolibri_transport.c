/*
 * Транспорт: TCP, прокси и подключение TLS-бэкенда.
 *
 * Единственный файл в проекте, который знает про операционную систему. Всё
 * остальное — чистые байты, поэтому портировать на новую платформу означает
 * поправить только этот файл.
 *
 * Синтаксис по-прежнему C89: MSVC 2013 и clang эпохи iOS 6 его принимают.
 */

#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <windows.h>
#  define KB_INVALID_SOCKET ((intptr_t)INVALID_SOCKET)
#  define kb_close_socket(fd) closesocket((SOCKET)(fd))
/*
 * На Win64 SOCKET — это UINT_PTR, и документация Winsock прямо отказывается
 * гарантировать, что значение влезает в int. Практически Windows раздаёт малые
 * дескрипторы, поэтому приведение к int почти всегда сходит с рук — и потому
 * это тихая мина, а не заметный сбой. Держим тип честно.
 */
#  define KB_SOCK(fd) ((SOCKET)(fd))
/* Winsock игнорирует nfds: набор хранится массивом, а не битовой маской. */
#  define KB_NFDS(fd) 0
#  define kb_last_error() WSAGetLastError()
#  define KB_EWOULDBLOCK WSAEWOULDBLOCK
#  define KB_EINPROGRESS WSAEWOULDBLOCK
#  define KB_EINTR WSAEINTR
#else
#  ifndef _POSIX_C_SOURCE
#    define _POSIX_C_SOURCE 200112L
#  endif
#  if defined(__APPLE__)
#    define _DARWIN_C_SOURCE 1
#  endif
#  include <errno.h>
#  include <fcntl.h>
#  include <netdb.h>
#  include <netinet/in.h>
#  include <netinet/tcp.h>
#  include <sys/select.h>
#  include <sys/socket.h>
#  include <sys/time.h>
#  include <sys/types.h>
#  include <time.h>
#  include <unistd.h>
#  if defined(__APPLE__)
/* Darwin получил clock_gettime только в macOS 10.12 / iOS 10, а
   mach_absolute_time есть с самой первой версии. */
#    include <mach/mach_time.h>
#  endif
#  define KB_INVALID_SOCKET ((intptr_t)-1)
#  define kb_close_socket(fd) close((int)(fd))
#  define KB_SOCK(fd) ((int)(fd))
#  define KB_NFDS(fd) ((int)(fd) + 1)
#  define kb_last_error() errno
#  define KB_EWOULDBLOCK EWOULDBLOCK
#  define KB_EINPROGRESS EINPROGRESS
#  define KB_EINTR EINTR
#endif

#include <stdio.h>
#include <string.h>

/*
 * Запись в закрытый пир на POSIX доставляет SIGPIPE, а его действие по
 * умолчанию — прибить процесс. Это библиотека внутри чужого приложения, и
 * трогать глобальную обработку сигналов она не имеет права: signal(SIGPIPE,
 * SIG_IGN) сломал бы хосту его собственную логику.
 *
 * Поэтому подавляем сигнал ПОСОКЕТНО: на Linux флагом при отправке, на macOS
 * и BSD — опцией сокета. Обе меры локальны и ничего чужого не задевают.
 */
#if defined(MSG_NOSIGNAL)
#  define KB_SEND_FLAGS MSG_NOSIGNAL
#else
#  define KB_SEND_FLAGS 0
#endif

#include "kolibri.h"
#include "kolibri_transport.h"

/* ─── Мелочи платформы ─────────────────────────────────────────────────── */

kb_status kb_net_startup(void)
{
#if defined(_WIN32)
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        return KB_ERR_STATE;
    }
#endif
    return KB_OK;
}

void kb_net_cleanup(void)
{
#if defined(_WIN32)
    WSACleanup();
#endif
}

uint64_t kb_now_ms(void)
{
    /*
     * Ноль возвращать нельзя: машина сессии держит в этом же типе дедлайны, и
     * ноль у неё значит «дедлайна нет». Если часы вдруг отказали, отдаём
     * монотонно растущий запасной счётчик — время пойдёт неверно, но
     * последовательность сохранится и таймауты не обнулятся.
     */
    static uint64_t fallback = 1;

#if defined(_WIN32)
    /*
     * GetTickCount64 требует Vista, и одной её записи в таблице импорта хватает,
     * чтобы бинарь не запустился на XP: загрузчик откажет ещё до main, даже
     * если функцию никто не вызовет. Поэтому собираем 64-битное значение из
     * 32-битного GetTickCount, который есть с Windows 95.
     *
     * Счётчик переполняется каждые 49.7 суток; переход через ноль ловится
     * сравнением с прошлым значением. Условие одно — звать чаще, чем раз в
     * 49 суток, что для клиента с keepalive выполняется с огромным запасом.
     */
    {
        static uint32_t last_ticks = 0;
        static uint64_t wraps = 0;
        uint32_t now = (uint32_t)GetTickCount();

        if (now < last_ticks) {
            wraps += ((uint64_t)1 << 32);
        }
        last_ticks = now;
        return wraps + (uint64_t)now + 1u;
    }
#elif defined(__APPLE__)
    /*
     * Проверять CLOCK_MONOTONIC тут нельзя: современный SDK его определяет, но
     * при deployment target ниже 10.12 / iOS 10 сам clock_gettime слабо
     * слинкован, и на старой системе прямой вызов разыменует NULL. mach —
     * единственный способ получить монотонные часы на всей линейке.
     *
     * gettimeofday не подходит принципиально: это стенные часы, и шаг NTP
     * назад останавливает пинги и таймауты сессии до конца скачка.
     */
    {
        static mach_timebase_info_data_t timebase;
        uint64_t ticks;
        uint64_t nanos;

        if (timebase.denom == 0) {
            if (mach_timebase_info(&timebase) != 0 || timebase.denom == 0) {
                fallback++;
                return fallback;
            }
        }
        ticks = mach_absolute_time();
        /* Делим до умножения, иначе произведение переполнится за годы аптайма. */
        nanos = (ticks / timebase.denom) * timebase.numer +
                ((ticks % timebase.denom) * timebase.numer) / timebase.denom;
        return nanos / 1000000u + 1u;
    }
#else
#  if defined(CLOCK_MONOTONIC)
    {
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
            return (uint64_t)ts.tv_sec * 1000u +
                   (uint64_t)(ts.tv_nsec / 1000000) + 1u;
        }
    }
#  endif
    {
        struct timeval tv;
        if (gettimeofday(&tv, NULL) == 0) {
            return (uint64_t)tv.tv_sec * 1000u + (uint64_t)(tv.tv_usec / 1000) + 1u;
        }
    }
    fallback++;
    return fallback;
#endif
}

static void note(kb_transport *t, const char *text)
{
    size_t len;
    if (t == NULL || text == NULL) {
        return;
    }
    len = strlen(text);
    if (len >= sizeof t->detail) {
        len = sizeof t->detail - 1;
    }
    memcpy(t->detail, text, len);
    t->detail[len] = '\0';
}

const char *kb_transport_detail(const kb_transport *transport)
{
    if (transport == NULL || transport->detail[0] == '\0') {
        return "ok";
    }
    return transport->detail;
}

static int set_nonblocking(intptr_t fd)
{
#if defined(_WIN32)
    u_long on = 1;
    return ioctlsocket((SOCKET)fd, FIONBIO, &on) == 0 ? 0 : -1;
#else
    int flags = fcntl((int)fd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    return fcntl((int)fd, F_SETFL, flags | O_NONBLOCK) < 0 ? -1 : 0;
#endif
}

static int would_block(int err)
{
    return err == KB_EWOULDBLOCK || err == KB_EINPROGRESS
#if !defined(_WIN32)
           || err == EAGAIN
#endif
        ;
}

/* 0 — без ограничения; иначе абсолютная отметка. */
static uint64_t deadline_from(uint32_t timeout_ms)
{
    if (timeout_ms == 0 || timeout_ms == KB_WAIT_FOREVER) {
        return timeout_ms == 0 ? kb_now_ms() : 0;
    }
    return kb_now_ms() + (uint64_t)timeout_ms;
}

/*
 * Ждать готовности дескриптора до АБСОЛЮТНОГО момента.
 *
 * Раньше сюда передавался таймаут, и он начинался заново после каждого EINTR
 * и на каждой операции рукопожатия — в сумме враждебный прокси мог тянуть
 * подключение сколько угодно. Общий бюджет считается один раз снаружи.
 */
static int wait_until(intptr_t fd, int for_write, uint64_t deadline_ms)
{
    fd_set set;
#if defined(_WIN32)
    fd_set except;
#endif
    struct timeval tv;
    struct timeval *ptv = NULL;
    int ready;

    FD_ZERO(&set);
#if defined(_WIN32)
    FD_ZERO(&except);
    FD_SET((SOCKET)fd, &set);
    /*
     * Winsock сообщает о ПРОВАЛЕ неблокирующего connect только через
     * exceptfds: в writefds такой сокет не попадает никогда. Без этого
     * набора select досиживал бы весь таймаут вместо мгновенного отказа.
     */
    if (for_write) {
        FD_SET((SOCKET)fd, &except);
    }
#else
    if ((int)fd >= FD_SETSIZE) {
        /* select не умеет дескрипторы выше FD_SETSIZE — это не наш случай на
           целевых платформах, но молча портить стек нельзя. */
        return -1;
    }
    FD_SET((int)fd, &set);
#endif

    for (;;) {
        if (deadline_ms != 0) {
            uint64_t now = kb_now_ms();
            uint64_t left = now >= deadline_ms ? 0 : deadline_ms - now;
            tv.tv_sec = (long)(left / 1000u);
            tv.tv_usec = (long)((left % 1000u) * 1000u);
            ptv = &tv;
            if (left == 0) {
                return 0;
            }
        }

#if defined(_WIN32)
        ready = select(KB_NFDS(fd), for_write ? NULL : &set,
                       for_write ? &set : NULL, for_write ? &except : NULL, ptv);
#else
        ready = select(KB_NFDS(fd), for_write ? NULL : &set,
                       for_write ? &set : NULL, NULL, ptv);
#endif
        if (ready < 0 && kb_last_error() == KB_EINTR) {
            /* Сигнал не должен продлевать ожидание: остаток пересчитается. */
            continue;
        }
        /* Готов он или отвалился — решает вызывающий по SO_ERROR. */
        return ready > 0 ? 1 : ready;
    }
}

int kb_transport_wait(kb_transport *transport, int for_write,
                      uint32_t timeout_ms)
{
    if (transport == NULL || !transport->open) {
        return -1;
    }
    /* Хвост от прокси уже лежит в памяти: ждать нечего. Под TLS такого хвоста
       быть не может — см. kb_transport_set_tls. */
    if (!for_write && !transport->has_tls &&
        transport->carry_read < transport->carry_len) {
        return 1;
    }
    return wait_until(transport->fd, for_write, deadline_from(timeout_ms));
}

/* ─── Подключение ──────────────────────────────────────────────────────── */

static intptr_t tcp_connect(const char *host, uint16_t port,
                            uint64_t deadline_ms, kb_transport *t)
{
    struct addrinfo hints;
    struct addrinfo *list = NULL;
    struct addrinfo *item;
    char service[8];
    intptr_t fd = KB_INVALID_SOCKET;

    sprintf(service, "%u", (unsigned)port);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(host, service, &hints, &list) != 0 || list == NULL) {
        note(t, "не удалось разрешить имя");
        return KB_INVALID_SOCKET;
    }

    for (item = list; item != NULL; item = item->ai_next) {
        int result;
        int flag = 1;

        fd = (intptr_t)socket(item->ai_family, item->ai_socktype,
                              item->ai_protocol);
        if (fd == KB_INVALID_SOCKET) {
            continue;
        }
        if (set_nonblocking(fd) != 0) {
            kb_close_socket(fd);
            fd = KB_INVALID_SOCKET;
            continue;
        }
        setsockopt(KB_SOCK(fd), IPPROTO_TCP, TCP_NODELAY, (const char *)&flag,
                   (int)sizeof flag);
#if defined(SO_NOSIGPIPE)
        /* macOS и BSD: сигнал глушится опцией, флага при отправке там нет. */
        setsockopt(KB_SOCK(fd), SOL_SOCKET, SO_NOSIGPIPE, (const char *)&flag,
                   (int)sizeof flag);
#endif

        result = connect(KB_SOCK(fd), item->ai_addr, (int)item->ai_addrlen);
        if (result == 0) {
            break;
        }
        if (!would_block(kb_last_error())) {
            kb_close_socket(fd);
            fd = KB_INVALID_SOCKET;
            continue;
        }

        /* Неблокирующий connect: готовность к записи означает результат. */
        if (wait_until(fd, 1, deadline_ms) != 1) {
            note(t, "таймаут подключения");
            kb_close_socket(fd);
            fd = KB_INVALID_SOCKET;
            continue;
        }
        {
            int err = 0;
#if defined(_WIN32)
            int len = (int)sizeof err;
#else
            socklen_t len = (socklen_t)sizeof err;
#endif
            if (getsockopt(KB_SOCK(fd), SOL_SOCKET, SO_ERROR, (char *)&err,
                           &len) !=
                    0 ||
                err != 0) {
                kb_close_socket(fd);
                fd = KB_INVALID_SOCKET;
                continue;
            }
        }
        break;
    }

    freeaddrinfo(list);
    if (fd == KB_INVALID_SOCKET && t->detail[0] == '\0') {
        note(t, "не удалось подключиться");
    }
    return fd;
}

/* Записать всё, дожидаясь готовности сокета. */
static kb_status write_all(intptr_t fd, const uint8_t *data, size_t len,
                           uint64_t deadline_ms)
{
    size_t sent = 0;
    while (sent < len) {
        long n;
#if defined(_WIN32)
        n = (long)send((SOCKET)fd, (const char *)(data + sent),
                       (int)(len - sent), 0);
#else
        n = (long)send((int)fd, data + sent, len - sent, KB_SEND_FLAGS);
#endif
        if (n > 0) {
            sent += (size_t)n;
            continue;
        }
        if (n < 0 && kb_last_error() == KB_EINTR) {
            continue;
        }
        if (n < 0 && would_block(kb_last_error())) {
            if (wait_until(fd, 1, deadline_ms) != 1) {
                return KB_ERR_TIMEOUT;
            }
            continue;
        }
        return KB_ERR_CLOSED;
    }
    return KB_OK;
}

/*
 * Прогнать рукопожатие с прокси.
 *
 * carry служит и приёмным буфером: прокси вправе склеить несколько своих
 * ответов в один сегмент, и байты, не разобранные на текущем шаге, обязаны
 * дождаться следующего — раньше они уезжали в хвост и машина зависала до
 * таймаута. Когда рукопожатие закончено, в carry остаётся ровно то, что уже
 * принадлежит туннелю.
 */
static kb_status run_proxy(kb_transport *t, const kb_proxy_config *proxy,
                           const char *host, uint16_t port,
                           uint64_t deadline_ms)
{
    kb_proxy machine;
    kb_status st;

    /* kb_proxy_init может выйти ранним return, не тронув структуру, поэтому
       обнуляем до вызова: иначе detail читался бы из мусора на стеке. */
    memset(&machine, 0, sizeof machine);
    st = kb_proxy_init(&machine, proxy, host, port);
    if (st != KB_OK) {
        note(t, machine.detail[0] != '\0' ? machine.detail
                                          : "прокси не настроился");
        return st;
    }

    for (;;) {
        size_t len = 0;
        const uint8_t *out;
        long n;
        size_t consumed = 0;

        if (kb_proxy_get_step(&machine) == KB_PROXY_STEP_DONE) {
            return KB_OK;
        }
        if (kb_proxy_get_step(&machine) == KB_PROXY_STEP_FAILED) {
            note(t, kb_proxy_detail(&machine));
            return kb_proxy_error(&machine);
        }

        out = kb_proxy_pull_out(&machine, &len);
        if (out != NULL) {
            st = write_all(t->fd, out, len, deadline_ms);
            if (st != KB_OK) {
                note(t, "прокси не принял запрос");
                return st;
            }
            kb_proxy_consume_out(&machine, len);
            continue;
        }

        /* Сначала доедаем то, что уже прочитано, но на прошлом шаге не пошло. */
        if (t->carry_read < t->carry_len) {
            st = kb_proxy_feed(&machine, t->carry + t->carry_read,
                               t->carry_len - t->carry_read, &consumed);
            t->carry_read += consumed;
            if (t->carry_read == t->carry_len) {
                t->carry_len = 0;
                t->carry_read = 0;
            }
            if (st != KB_OK) {
                note(t, kb_proxy_detail(&machine));
                return st;
            }
            if (consumed > 0) {
                continue;
            }
        }

        /* Уплотняем разобранное и читаем ещё. */
        if (t->carry_read > 0) {
            size_t left = t->carry_len - t->carry_read;
            if (left > 0) {
                memmove(t->carry, t->carry + t->carry_read, left);
            }
            t->carry_len = left;
            t->carry_read = 0;
        }
        if (t->carry_len >= sizeof t->carry) {
            note(t, "прокси прислал больше, чем помещается");
            return KB_ERR_OVERFLOW;
        }

        if (wait_until(t->fd, 0, deadline_ms) != 1) {
            note(t, "прокси не ответил");
            return KB_ERR_TIMEOUT;
        }
#if defined(_WIN32)
        n = (long)recv((SOCKET)t->fd, (char *)(t->carry + t->carry_len),
                       (int)(sizeof t->carry - t->carry_len), 0);
#else
        n = (long)recv((int)t->fd, t->carry + t->carry_len,
                       sizeof t->carry - t->carry_len, 0);
#endif
        if (n == 0) {
            note(t, "прокси закрыл соединение");
            return KB_ERR_CLOSED;
        }
        if (n < 0) {
            if (would_block(kb_last_error()) || kb_last_error() == KB_EINTR) {
                continue;
            }
            note(t, "ошибка чтения от прокси");
            return KB_ERR_CLOSED;
        }
        t->carry_len += (size_t)n;
    }
}

/*
 * Подключить TLS-бэкенд к уже открытому сокету.
 *
 * Отказывает, если прокси успел дописать байты за своим ответом. Под TLS их
 * там быть не может: клиент ещё не отправил ClientHello, поэтому всё, что
 * пришло раньше, сочинил сам прокси. Отдать их приложению как расшифрованные
 * данные значило бы дать прокси подделывать трафик «от сервера» при
 * безупречно проверенном сертификате.
 */
kb_status kb_transport_set_tls(kb_transport *transport, const kb_tls *tls)
{
    if (transport == NULL || tls == NULL || tls->vt == NULL) {
        return KB_ERR_INVALID;
    }
    if (!transport->open) {
        return KB_ERR_STATE;
    }
    if (transport->carry_read < transport->carry_len) {
        note(transport, "прокси дописал байты до TLS — соединение отвергнуто");
        return KB_ERR_STATE;
    }
    transport->tls = *tls;
    transport->has_tls = 1;
    return KB_OK;
}

kb_status kb_transport_connect(kb_transport *transport,
                               const kb_transport_config *config)
{
    kb_status st;
    uint64_t deadline;

    if (transport == NULL || config == NULL || config->host == NULL) {
        return KB_ERR_INVALID;
    }
    memset(transport, 0, sizeof *transport);
    transport->fd = KB_INVALID_SOCKET;

    /* Один бюджет на всё: разрешение имени, каждый адрес и рукопожатие с
       прокси. Иначе враждебный прокси растягивает подключение бесконечно. */
    deadline = deadline_from(config->connect_timeout_ms);

    if (config->proxy != NULL && config->proxy->kind != KB_PROXY_NONE) {
        transport->fd = tcp_connect(config->proxy->host, config->proxy->port,
                                    deadline, transport);
        if (transport->fd == KB_INVALID_SOCKET) {
            return KB_ERR_CLOSED;
        }
        transport->open = 1;
        st = run_proxy(transport, config->proxy, config->host, config->port,
                       deadline);
        if (st != KB_OK) {
            kb_transport_close(transport);
            return st;
        }
    } else {
        transport->fd = tcp_connect(config->host, config->port, deadline,
                                    transport);
        if (transport->fd == KB_INVALID_SOCKET) {
            return KB_ERR_CLOSED;
        }
        transport->open = 1;
    }

    if (config->tls != NULL && config->tls->vt != NULL) {
        kb_status tls_st = kb_transport_set_tls(transport, config->tls);
        if (tls_st != KB_OK) {
            kb_transport_close(transport);
            return tls_st;
        }
    }
    note(transport, "ok");
    return KB_OK;
}

kb_status kb_transport_handshake(kb_transport *transport, int *want_write)
{
    int result;

    if (want_write != NULL) {
        *want_write = 0;
    }
    if (transport == NULL || !transport->open) {
        return KB_ERR_INVALID;
    }
    if (!transport->has_tls) {
        return KB_OK;
    }
    result = transport->tls.vt->handshake(transport->tls.ctx);
    if (result == 0) {
        return KB_OK;
    }
    if (result == KB_IO_WANT_READ || result == KB_IO_WANT_WRITE) {
        if (want_write != NULL) {
            *want_write = result == KB_IO_WANT_WRITE;
        }
        return KB_ERR_TRUNCATED;
    }
    if (transport->tls.vt->detail != NULL) {
        note(transport, transport->tls.vt->detail(transport->tls.ctx));
    } else {
        note(transport, "TLS-рукопожатие не удалось");
    }
    return KB_ERR_STATE;
}

/* ─── Чтение и запись ──────────────────────────────────────────────────── */

long kb_transport_read(kb_transport *transport, uint8_t *buf, size_t len)
{
    long n;

    if (transport == NULL || buf == NULL) {
        return KB_IO_FAILED;
    }
    if (!transport->open) {
        return KB_IO_CLOSED;
    }
    if (len == 0) {
        return 0;
    }

    /* Сначала отдаём то, что прилетело вместе с ответом прокси. */
    if (transport->carry_read < transport->carry_len) {
        size_t left;
        if (transport->has_tls) {
            /* Досюда доходить не должно: kb_transport_set_tls такое соединение
               уже отверг. Но открытый текст мимо TLS не отдаём никогда. */
            return KB_IO_FAILED;
        }
        left = transport->carry_len - transport->carry_read;
        size_t take = left < len ? left : len;
        memcpy(buf, transport->carry + transport->carry_read, take);
        transport->carry_read += take;
        if (transport->carry_read == transport->carry_len) {
            transport->carry_len = 0;
            transport->carry_read = 0;
        }
        return (long)take;
    }

    if (transport->has_tls) {
        return transport->tls.vt->read(transport->tls.ctx, buf, len);
    }
#if defined(_WIN32)
    n = (long)recv((SOCKET)transport->fd, (char *)buf, (int)len, 0);
#else
    n = (long)recv((int)transport->fd, buf, len, 0);
#endif
    if (n > 0) {
        return n;
    }
    if (n == 0) {
        return KB_IO_CLOSED;
    }
    if (would_block(kb_last_error()) || kb_last_error() == KB_EINTR) {
        return KB_IO_WANT_READ;
    }
    return KB_IO_FAILED;
}

long kb_transport_write(kb_transport *transport, const uint8_t *buf, size_t len)
{
    long n;

    if (transport == NULL || buf == NULL) {
        return KB_IO_FAILED;
    }
    if (!transport->open) {
        return KB_IO_CLOSED;
    }
    if (len == 0) {
        return 0;
    }
    if (transport->has_tls) {
        return transport->tls.vt->write(transport->tls.ctx, buf, len);
    }
#if defined(_WIN32)
    n = (long)send((SOCKET)transport->fd, (const char *)buf, (int)len, 0);
#else
    n = (long)send((int)transport->fd, buf, len, KB_SEND_FLAGS);
#endif
    if (n >= 0) {
        return n;
    }
    if (would_block(kb_last_error()) || kb_last_error() == KB_EINTR) {
        return KB_IO_WANT_WRITE;
    }
    return KB_IO_FAILED;
}

void kb_transport_close(kb_transport *transport)
{
    if (transport == NULL || !transport->open) {
        return;
    }
    if (transport->has_tls && transport->tls.vt->close != NULL) {
        transport->tls.vt->close(transport->tls.ctx);
    }
    if (transport->fd != KB_INVALID_SOCKET) {
        kb_close_socket(transport->fd);
    }
    transport->fd = KB_INVALID_SOCKET;
    transport->open = 0;
    transport->has_tls = 0;
    transport->carry_len = 0;
    transport->carry_read = 0;
}
