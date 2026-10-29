"""Настоящий клиент: TLS и сокет на Python, весь протокол — в C-ядре.

    make -C ../.. shared && python3 client_loop.py

Скрипт реально ходит в сеть, поэтому запускать его надо осознанно. Он только
подключается, делает handshake и слушает пуши — ничего не отправляет и никуда
не логинится.

Ядро sans-IO, поэтому здесь видно всю схему целиком: сокет, TLS, часы и сон —
питоновские, а фрейминг, сжатие, handshake, нумерация seq, keepalive и таймауты
живут в C. Ровно этот же цикл на Go, Swift или Kotlin выглядит так же — меняется
только способ читать из сокета.
"""

import pathlib
import select
import socket
import ssl
import sys
import time

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent.parent / "python"))

import kolibri_core as kb

HOST = "api.oneme.ru"
PORT = 443
DEVICE_ID = "kolibri-core-demo-0001"


def now_ms():
    """Монотонные часы: важна только разность, не абсолютное время."""
    return int(time.monotonic() * 1000)


def connect_tls():
    """
    Обычный питоновский TLS. Для api2.oneme.ru сюда нужен корень Минцифры —
    ядро в доверии не участвует вовсе, это забота хоста.
    """
    context = ssl.create_default_context()
    raw = socket.create_connection((HOST, PORT), timeout=15)
    return context.wrap_socket(raw, server_hostname=HOST)


def flush(session, sock):
    data = session.take_outgoing()
    if data:
        sock.sendall(data)


def handle(event):
    if event.type == kb.EventType.STATE:
        print("состояние:", kb.State.name(event.state))
    elif event.type == kb.EventType.HANDSHAKE:
        seed = event.calls_seed if event.has_calls_seed else "нет"
        print("handshake принят, callsSeed =", seed)
        print("  тело ответа:", len(event.payload), "Б")
    elif event.type == kb.EventType.RESPONSE:
        print("ответ на seq=%d opcode=%d: %d Б"
              % (event.seq, event.opcode, len(event.payload)))
    elif event.type == kb.EventType.PUSH:
        print("пуш opcode=%d: %d Б" % (event.opcode, len(event.payload)))
    elif event.type == kb.EventType.REQUEST_FAILED:
        print("запрос seq=%d не дождался ответа (%d)" % (event.seq, event.status))
    elif event.type == kb.EventType.ERROR:
        print("поток испорчен:", event.status)
        return False
    return True


def run_connection(session, sock):
    """Крутится, пока соединение живо."""
    while True:
        session.tick(now_ms())
        flush(session, sock)

        # Спим ровно до ближайшего дела сессии: пинга или таймаута.
        deadline = session.next_deadline()
        timeout = None if deadline == 0 else max(0.0, (deadline - now_ms()) / 1000.0)
        ready, _, _ = select.select([sock], [], [], timeout)

        if ready:
            chunk = sock.recv(65536)
            if not chunk:
                print("сервер закрыл соединение")
                return
            session.feed(chunk)

        for event in session.events():
            if not handle(event):
                return


def main():
    session = kb.Session(
        device_id=DEVICE_ID,
        user_agent=kb.UserAgent(),
        client_session_id=now_ms(),
        ping_interval_ms=30000,
        request_timeout_ms=30000,
        auto_reconnect=True,
    )

    while True:
        session.on_connecting(now_ms())
        try:
            sock = connect_tls()
        except OSError as error:
            print("не подключился:", error)
            sock = None

        if sock is not None:
            session.on_connected(now_ms())
            try:
                run_connection(session, sock)
            except OSError as error:
                print("сокет отвалился:", error)
            finally:
                sock.close()

        session.on_disconnected(now_ms())

        # Расписание переподключения считает сессия — та же лестница, что в
        # эталонной реализации: 2, 4, 8, 15, 15 секунд.
        deadline = session.next_deadline()
        if deadline == 0:
            return
        pause = max(0.0, (deadline - now_ms()) / 1000.0)
        print("переподключение через %.0f с" % pause)
        time.sleep(pause)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nвыход")
