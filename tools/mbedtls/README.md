# Сборка mbedTLS для kolibri

TLS-бэкенд `src/kolibri_tls_mbedtls.c` работает с обычным mbedTLS 3.6.x, но
для `api2.oneme.ru` нужен один патч и правильный конфиг. Оба здесь.

```sh
curl -sL https://github.com/Mbed-TLS/mbedtls/archive/refs/tags/mbedtls-3.6.2.tar.gz | tar xz
cd mbedtls-mbedtls-3.6.2
patch -p0 < ../numericstring.patch
make -j4 lib CFLAGS="-Oz -ffunction-sections -fdata-sections"
```

Затем собери клиент:

```sh
cc -Iinclude -I<mbedtls>/include -DKB_WITH_MBEDTLS -Oz \
   -o client examples/c/client.c src/*.c \
   <mbedtls>/library/libmbedtls.a <mbedtls>/library/libmbedx509.a \
   <mbedtls>/library/libmbedcrypto.a
```

## Зачем патч

В Subject сертификата `*.oneme.ru` ИНН (`1.2.643.100.4`) и ОГРН закодированы
типом **NumericString** (тег `0x12`). Штатный mbedTLS этот тип в именах не
принимает и отвергает сертификат ещё до проверки цепочки, отвечая невнятным
«ASN1 tag was of an unexpected value». OpenSSL его принимает — потому
OpenSSL-бэкенд и работал без правок.

Патч добавляет `0x12` в список допустимых тегов. Одна строка условия.

## Грабли, на которые уже наступили

Всё проверено на живом `api2.oneme.ru`; без любого из пунктов рукопожатие
падает:

* **TLS 1.3 обязателен.** Сервер согласует именно его и подписывает
  `rsa_pss_rsae_sha256`. Сборка только с 1.2 получает
  `MBEDTLS_ERR_RSA_VERIFY_FAILED`.
* **ChaCha20-Poly1305 обязателен для 1.2.** При понижении до TLS 1.2 сервер
  выбирает `ECDHE-RSA-CHACHA20-POLY1305`, а не AES-GCM.
* **С `MBEDTLS_USE_PSA_CRYPTO` нужен отдельный PSA-конфиг.** Иначе обычные
  `MBEDTLS_*_C` не включают алгоритмы, список подписей в ClientHello уходит
  пустым и сервер отвечает fatal alert 40.
* **`check_config.h` нельзя подключать вручную** начиная с mbedTLS 3.0: он
  отработает раньше внутренних корректировок и не увидит включённые модули.
* **Комментарии в PEM.** Файл корней Минцифры начинается со строки `#`, а
  mbedTLS требует, чтобы данные начинались с `-----BEGIN`. Адаптер это уже
  учитывает: пропускает всё до первого маркера и считает длину от него,
  вместе с завершающим нулём.

## Размер

Урезанный конфиг (`config-minimal.h`) даёт около 490 КБ архивов; слинкованный
клиент со вшитыми mbedTLS и zstd — примерно 200 КБ против 4,3 МБ на OpenSSL.

Урезание, впрочем, не обязательно: со штатным конфигом всё тоже работает,
просто крупнее.
