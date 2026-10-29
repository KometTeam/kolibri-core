/*
 * Проверки, которым нужен настоящий сокет и настоящий обрыв.
 *
 *   make socket-checks
 *
 * Юнит-тестам такое не под силу: SIGPIPE приходит только при записи в
 * по-настоящему закрытый пир.
 */

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
int main(void)
{
    printf("пропущено: проверка SIGPIPE имеет смысл только на POSIX\n");
    return 0;
}
#else

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "kolibri.h"
#include "kolibri_transport.h"

static int failures = 0;

static void check(int ok, const char *what)
{
    printf("  %s %s\n", ok ? "ок  " : "ПРОВАЛ", what);
    if (!ok) {
        failures++;
    }
}

/*
 * Регрессия аудита: send() без MSG_NOSIGNAL и без SO_NOSIGPIPE доставлял
 * SIGPIPE, и приложение-хозяин молча умирало с кодом 141 вместо того, чтобы
 * получить ошибку и переподключиться. Если починка отвалится, этот тест не
 * провалится, а будет убит сигналом — что тоже заметно.
 */
static void sigpipe_does_not_kill_the_host(void)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof addr;
    kb_transport transport;
    kb_transport_config config;
    struct linger hard;
    unsigned char payload[4096];
    char host[32];
    int listener;
    int accepted;
    int got_error = 0;
    int i;

    printf("запись в оборванное соединение:\n");

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        check(0, "не создался слушающий сокет");
        return;
    }
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    if (bind(listener, (struct sockaddr *)&addr, sizeof addr) != 0 ||
        listen(listener, 1) != 0 ||
        getsockname(listener, (struct sockaddr *)&addr, &addr_len) != 0) {
        check(0, "слушающий сокет не поднялся");
        close(listener);
        return;
    }
    sprintf(host, "127.0.0.1");

    memset(&config, 0, sizeof config);
    config.host = host;
    config.port = ntohs(addr.sin_port);
    config.connect_timeout_ms = 5000;

    if (kb_transport_connect(&transport, &config) != KB_OK) {
        check(0, "клиент не подключился");
        close(listener);
        return;
    }

    accepted = accept(listener, NULL, NULL);
    if (accepted < 0) {
        check(0, "соединение не принялось");
        kb_transport_close(&transport);
        close(listener);
        return;
    }

    /* Жёсткий обрыв: RST вместо аккуратного FIN. */
    hard.l_onoff = 1;
    hard.l_linger = 0;
    setsockopt(accepted, SOL_SOCKET, SO_LINGER, &hard, sizeof hard);
    close(accepted);
    close(listener);

    memset(payload, 'x', sizeof payload);
    for (i = 0; i < 200; i++) {
        long written = kb_transport_write(&transport, payload, sizeof payload);
        if (written < 0 && written != KB_IO_WANT_WRITE) {
            got_error = 1;
            break;
        }
        if (written == KB_IO_WANT_WRITE) {
            /* Буфер полон — значит пир ещё не сбросил, продолжаем. */
            continue;
        }
    }

    check(1, "процесс не убит сигналом");
    check(got_error, "запись вернула ошибку, а не молчание");

    kb_transport_close(&transport);
    /* Повторное закрытие обязано быть безобидным. */
    kb_transport_close(&transport);
    check(1, "повторное закрытие безопасно");
}

int main(void)
{
    sigpipe_does_not_kill_the_host();
    printf("%s\n", failures == 0 ? "все проверки прошли" : "есть провалы");
    return failures == 0 ? 0 : 1;
}
#endif
