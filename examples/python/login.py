"""Вход в аккаунт с сохранением сессии.

    make -C ../.. shared && python3 login.py +7XXXXXXXXXX   # первый вход (SMS)
    python3 login.py                                        # потом — без SMS

Первый вход идёт по SMS и сохраняет login-токен в файл. Дальше скрипт входит
этим токеном (handshake → login), не запрашивая код: именно так «залогинен»
официальный клиент. Само ядро ничего не сохраняет — это забота вызывающего.

ВНИМАНИЕ: файл сессии — это учётные данные, доступ к нему = доступ к аккаунту.
"""

import json
import os
import pathlib
import secrets
import sys
import uuid

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent.parent / "python"))
import kolibri_core as kb

# Дайджесты сборки и версия — единый комплект (26.20.2), иначе сервер отвергнет
# fingerprint. Значения из ChatCacheFingerprint официального клиента.
APP_VERSION, BUILD = "26.20.2", 6758
SIGNATURE = bytes.fromhex("1684414033eb263e2c615f8b7df5ed8793850a07656304997fbf07e9e21e1e93")
SO = bytes.fromhex("90e2fb8745b17b42a10182f8d8ac590e3fca5b311e2ce2d5144fa2c18cb3090d")
DEX = bytes.fromhex("0a6265f6e5d8231b9cba641f8c40475e6f3baeb06ed41b804b9bf7307aa4214e")

SESSION_FILE = pathlib.Path(os.environ.get("KOLIBRI_SESSION", "kolibri_session.json"))

# Идентичность устройства в формате реального клиента: device_id = 16 hex-символов
# (Android ID), instance_id = UUID. Генерится один раз на установку и хранится в
# файле сессии — device_id обязан быть стабильным (к нему привязаны отпечаток и
# аккаунт). Через env можно задать свои, чтобы прогон был воспроизводимым.
def new_device_identity():
    return (os.environ.get("KOLIBRI_DEVICE_ID", secrets.token_hex(8)),
            os.environ.get("KOLIBRI_INSTANCE_ID", str(uuid.uuid4())))

UA = kb.UserAgent(app_version=APP_VERSION, build_number=BUILD,
                  os_version="Android 14", device_name="Google Pixel 7")


def load_session():
    try:
        return json.loads(SESSION_FILE.read_text())
    except Exception:
        return None


def save_session(token, device_id, instance_id):
    SESSION_FILE.write_text(json.dumps(
        {"token": token, "deviceId": device_id, "instanceId": instance_id}))
    try:
        os.chmod(SESSION_FILE, 0o600)  # это учётные данные
    except OSError:
        pass


def do_login(c, device_id, token):
    """Шаг 3 (опкод 19): авторизовать соединение login-токеном. token тот же,
    что вернул код/2FA или лежит в файле. Возвращает (обновлённый_токен, resp)."""
    mode = kb.fingerprint(c.handshake_seed, device_id, SIGNATURE, SO, DEX)
    r = c.login(token=token, interactive=True, chatCacheFingerprint=mode,
                chatsSync=-1, presenceSync=-1)
    return (r.token or token), r


def resume(saved):
    """Войти сохранённым токеном без SMS. True, если получилось."""
    with kb.Connection("api.oneme.ru", device_id=saved["deviceId"],
                       instance_id=saved["instanceId"], user_agent=UA) as c:
        c.handshake_seed = c.handshake()
        try:
            token, r = do_login(c, saved["deviceId"], saved["token"])
        except kb.KolibriError:
            return False  # токен протух — нужен новый вход
        save_session(token, saved["deviceId"], saved["instanceId"])
        contact = (r.profile or {}).get("contact") or {}
        print("✓✓ ВХОД по сохранённому токену (без SMS)")
        print("   id:", contact.get("id"), "| чатов:", len(r.chats or []))
        return True


def fresh_login(phone):
    """Первый вход: код по SMS (+ 2FA), затем login и сохранение токена."""
    device_id, instance_id = new_device_identity()
    with kb.Connection("api.oneme.ru", device_id=device_id,
                       instance_id=instance_id, user_agent=UA) as c:
        c.handshake_seed = c.handshake()
        mode = kb.fingerprint(c.handshake_seed, device_id, SIGNATURE, SO, DEX)

        r = c.auth_request(phone=phone, type="START_AUTH", language="ru", mode=mode)
        print("✓ SMS отправлена (код из %s цифр)" % r.codeLength)

        r = c.auth_verify(token=r.token, authTokenType="CHECK_CODE",
                          verifyCode=input("Код из SMS: ").strip())
        login_token = (r.tokenAttrs or {}).get("LOGIN", {}).get("token")

        if login_token is None and r.passwordChallenge:
            ch = r.passwordChallenge
            print("→ 2FA." + (" Подсказка: " + ch["hint"] if ch.get("hint") else ""))
            r = c.auth_login_check_password(
                trackId=ch["trackId"], password=input("Облачный пароль: ").strip())
            login_token = (r.tokenAttrs or {}).get("LOGIN", {}).get("token")

        if login_token is None:
            print("Номер не зарегистрирован — это регистрация, не вход."
                  if (r.tokenAttrs or {}).get("REGISTER") else ("Нет login-токена: %s" % r.raw))
            return 1

        token, r = do_login(c, device_id, login_token)
        save_session(token, device_id, instance_id)
        contact = (r.profile or {}).get("contact") or {}
        print("\n✓✓ ВХОД ВЫПОЛНЕН, токен и устройство сохранены в", SESSION_FILE)
        print("   id:", contact.get("id"), "| чатов:", len(r.chats or []))
        return 0


def main():
    saved = load_session()
    if saved and resume(saved):
        return 0
    if saved:
        print("сохранённый токен не подошёл — вхожу заново по SMS")

    if len(sys.argv) < 2:
        print("нет сохранённой сессии. usage: login.py +7XXXXXXXXXX")
        return 2
    phone = "+" + "".join(c for c in sys.argv[1] if c.isdigit())
    return fresh_login(phone)


if __name__ == "__main__":
    sys.exit(main())
