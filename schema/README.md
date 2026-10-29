# Схема протокола

Единственный источник правды о форме payload'ов. Из этих файлов
`kolibri-codegen` генерирует `kolibri-api/src/generated.rs`; позже отсюда же
будут генерироваться стабы для Python, Dart, Kotlin и Swift.

## Синтаксис типов

| Запись      | Rust                        | На проводе (MessagePack) |
|-------------|-----------------------------|--------------------------|
| `string`    | `String`                    | str                      |
| `i64` `i32` | `i64` / `i32`               | int                      |
| `f64`       | `f64`                       | float                    |
| `bool`      | `bool`                      | bool                     |
| `bytes`     | `Vec<u8>`                   | **bin**, не массив int   |
| `any`       | `rmpv::Value`               | что угодно               |
| `[T]`       | `Vec<T>`                    | array                    |
| `{T}`       | `BTreeMap<String, T>`       | map со строковыми ключами|
| `Name`      | ссылка на тип из `types`    | map                      |
| `T?`        | `Option<T>`                 | ключ может отсутствовать |

Необязательные поля не сериализуются вовсе, когда они `None` — сервер
отличает отсутствие ключа от `null`, так что это важно.

## Форма файла

```toml
[types.Element]
doc = "Кусок форматирования в тексте."
[types.Element.fields]
type   = "string"
from   = "i64"
length = { type = "i64", doc = "Длина в UTF-16 code units." }

[ops.MsgSend]
opcode = 64
doc = "Отправить сообщение в чат."
[ops.MsgSend.request]
chatId  = "i64"
message = "OutgoingMessage"
[ops.MsgSend.response]
message = "SentMessage?"
```

Имя таблицы — имя типа/операции в Rust. Ключи внутри — имена полей **как на
проводе**; генератор сам переводит их в snake_case и вешает `#[serde(rename)]`.

## Откуда берутся типы

Поля выверены по референсному клиенту (`~/Komet/lib/backend/modules/`), а не
угаданы. Если тип поля неизвестен или сервер отдаёт то int, то string —
ставь `any` и разбирай в прикладном коде, это честнее, чем угадать неверно.
