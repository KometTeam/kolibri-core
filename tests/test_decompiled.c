/*
 * Операции, извлечённые из декомпила 26.28 (schema/decompiled.toml).
 * Проверяем, что сгенерённые энкодеры/декодеры дают ожидаемый msgpack и что
 * опкод-константы на месте — то есть извлечённые структуры реально пригодны.
 */

#include <string.h>

#include "kb_test.h"
#include "kolibri.h"
#include "kolibri_ops.h"
#include "kolibri_msgpack.h"

static unsigned char wire[8192];

void kb_tests_decompiled(void)
{
    KB_TEST("новые опкоды на месте");
    {
        CHECK_EQ(KB_OPCODE_CHAT_HISTORY, 49);
        CHECK_EQ(KB_OPCODE_OK_TOKEN, 158);
        CHECK_EQ(KB_OPCODE_ORG_INFO, 256);
        CHECK_EQ(KB_OPCODE_SEND_VOTE, 304);
        /* Опкоды, которых не было в таблице до декомпила. */
        CHECK_EQ(KB_OP_CALL_HISTORY, 163);
        CHECK_EQ(KB_OP_ORG_INFO, 256);
        CHECK_EQ(KB_OP_BANNERS_GET, 302);
    }

    KB_TEST("ChatHistory: обязательные и опциональные поля");
    {
        kb_chat_history req;
        kb_mp_value v;
        size_t len = 0;

        memset(&req, 0, sizeof req);
        req.chat_id = 12345;
        req.from = 999;
        req.forward = 20;
        req.get_messages = 1;
        req.item_type.data = (const uint8_t *)"MESSAGE";
        req.item_type.len = 7;
        req.interactive = 1;
        /* postId и chatAccessToken опциональны — не выставлены. */

        CHECK_EQ(kb_encode_chat_history(&req, wire, sizeof wire, &len), KB_OK);
        CHECK_EQ(kb_mp_map_get(wire, len, "chatId", &v), KB_OK);
        CHECK(v.type == KB_MP_UINT ? v.u == 12345 : v.i == 12345);
        CHECK_EQ(kb_mp_map_get(wire, len, "from", &v), KB_OK);
        CHECK_EQ(kb_mp_map_get(wire, len, "forward", &v), KB_OK);
        CHECK_EQ(kb_mp_map_get(wire, len, "getMessages", &v), KB_OK);
        CHECK_EQ(v.type, KB_MP_BOOL);
        CHECK_EQ(kb_mp_map_get(wire, len, "itemType", &v), KB_OK);
        CHECK(kb_mp_str_eq(&v, "MESSAGE"));
        /* Незаданные опциональные не ушли в пакет. */
        CHECK_EQ(kb_mp_map_has(wire, len, "postId"), 0);
        CHECK_EQ(kb_mp_map_has(wire, len, "chatAccessToken"), 0);
    }

    KB_TEST("ChatHistory: опциональные попадают, если заданы");
    {
        kb_chat_history req;
        kb_mp_value v;
        size_t len = 0;

        memset(&req, 0, sizeof req);
        req.chat_id = 1;
        req.item_type.data = (const uint8_t *)"X";
        req.item_type.len = 1;
        req.has_post_id = 1;
        req.post_id = 7000;
        req.has_chat_access_token = 1;
        req.chat_access_token.data = (const uint8_t *)"tok";
        req.chat_access_token.len = 3;

        CHECK_EQ(kb_encode_chat_history(&req, wire, sizeof wire, &len), KB_OK);
        CHECK_EQ(kb_mp_map_get(wire, len, "postId", &v), KB_OK);
        CHECK(v.type == KB_MP_UINT ? v.u == 7000 : v.i == 7000);
        CHECK_EQ(kb_mp_map_get(wire, len, "chatAccessToken", &v), KB_OK);
        CHECK(kb_mp_str_eq(&v, "tok"));
    }

    KB_TEST("OkToken: энкод запроса и декод ответа");
    {
        kb_ok_token req;
        kb_ok_token_response resp;
        kb_mp_writer w;
        kb_mp_value v;
        size_t len = 0;

        memset(&req, 0, sizeof req);
        req.value.data = (const uint8_t *)"v1";
        req.value.len = 2;
        req.user_id = 42;
        CHECK_EQ(kb_encode_ok_token(&req, wire, sizeof wire, &len), KB_OK);
        CHECK_EQ(kb_mp_map_get(wire, len, "value", &v), KB_OK);
        CHECK(kb_mp_str_eq(&v, "v1"));
        CHECK_EQ(kb_mp_map_get(wire, len, "userId", &v), KB_OK);

        /* Ответ сервера: token + метки времени. */
        kb_mp_writer_init(&w, wire, sizeof wire);
        kb_mp_write_map(&w, 3);
        kb_mp_write_cstr(&w, "token");
        kb_mp_write_cstr(&w, "abc");
        kb_mp_write_cstr(&w, "token_refresh_ts");
        kb_mp_write_i64(&w, 111);
        kb_mp_write_cstr(&w, "token_lifetime_ts");
        kb_mp_write_i64(&w, 222);
        len = kb_mp_writer_len(&w);

        memset(&resp, 0, sizeof resp);
        CHECK_EQ(kb_decode_ok_token_response(wire, len, &resp), KB_OK);
        CHECK_MEM(resp.token.data, resp.token.len, "abc", 3);
        CHECK(resp.token_refresh_ts == 111);
        CHECK(resp.token_lifetime_ts == 222);
    }

    KB_TEST("Login обогащён полями из декомпила");
    {
        kb_login req;
        kb_mp_value v;
        size_t len = 0;

        memset(&req, 0, sizeof req);
        req.token.data = (const uint8_t *)"t";
        req.token.len = 1;
        req.interactive = 1;
        req.has_contacts_sync = 1;
        req.contacts_sync = 500;
        req.has_config_hash = 1;
        req.config_hash.data = (const uint8_t *)"h";
        req.config_hash.len = 1;
        req.has_calls_sync = 1;
        req.calls_sync = 7;

        CHECK_EQ(kb_encode_login(&req, wire, sizeof wire, &len), KB_OK);
        CHECK_EQ(kb_mp_map_get(wire, len, "contactsSync", &v), KB_OK);
        CHECK_EQ(kb_mp_map_get(wire, len, "configHash", &v), KB_OK);
        CHECK_EQ(kb_mp_map_get(wire, len, "callsSync", &v), KB_OK);
        /* Не заданные новые поля не ушли. */
        CHECK_EQ(kb_mp_map_has(wire, len, "lastLogin"), 0);
        CHECK_EQ(kb_mp_map_has(wire, len, "bannersSync"), 0);
    }
    KB_TEST("типы из живого захвата: ErrorEnvelope, Contact, Message");
    {
        kb_mp_writer w;
        size_t len;
        kb_error_envelope err;
        kb_contact ct;
        kb_message msg;

        /* Конверт ошибки. */
        kb_mp_writer_init(&w, wire, sizeof wire);
        kb_mp_write_map(&w, 3);
        kb_mp_write_cstr(&w, "error");   kb_mp_write_cstr(&w, "chat.denied");
        kb_mp_write_cstr(&w, "message"); kb_mp_write_cstr(&w, "no access");
        kb_mp_write_cstr(&w, "title");   kb_mp_write_cstr(&w, "Ошибка");
        len = kb_mp_writer_len(&w);
        CHECK_EQ(kb_decode_error_envelope(wire, len, &err), KB_OK);
        CHECK_MEM(err.error.data, err.error.len, "chat.denied", 11);
        CHECK_MEM(err.message.data, err.message.len, "no access", 9);
        CHECK_EQ(err.localized_message.len, (size_t)0); /* не прислали */

        /* Контакт: id, country, names[] (raw+count), опциональный phone. */
        kb_mp_writer_init(&w, wire, sizeof wire);
        kb_mp_write_map(&w, 3);
        kb_mp_write_cstr(&w, "id");      kb_mp_write_i64(&w, 10000001);
        kb_mp_write_cstr(&w, "country"); kb_mp_write_cstr(&w, "RU");
        kb_mp_write_cstr(&w, "names");   kb_mp_write_array(&w, 1);
        kb_mp_write_map(&w, 1);
        kb_mp_write_cstr(&w, "name");    kb_mp_write_cstr(&w, "аа");
        len = kb_mp_writer_len(&w);
        CHECK_EQ(kb_decode_contact(wire, len, &ct), KB_OK);
        CHECK(ct.id == 10000001);
        CHECK_MEM(ct.country.data, ct.country.len, "RU", 2);
        CHECK_EQ(ct.names_count, (size_t)1);
        CHECK_EQ(ct.has_phone, 0);

        /* Сообщение: обязательные + опциональный sender. */
        kb_mp_writer_init(&w, wire, sizeof wire);
        kb_mp_write_map(&w, 5);
        kb_mp_write_cstr(&w, "id");      kb_mp_write_i64(&w, 117133258591395264LL);
        kb_mp_write_cstr(&w, "time");    kb_mp_write_i64(&w, 1787311685049LL);
        kb_mp_write_cstr(&w, "type");    kb_mp_write_cstr(&w, "USER");
        kb_mp_write_cstr(&w, "text");    kb_mp_write_cstr(&w, "привет");
        kb_mp_write_cstr(&w, "sender");  kb_mp_write_i64(&w, 543835);
        len = kb_mp_writer_len(&w);
        CHECK_EQ(kb_decode_message(wire, len, &msg), KB_OK);
        CHECK(msg.id == 117133258591395264LL);
        CHECK_MEM(msg.type.data, msg.type.len, "USER", 4);
        CHECK(msg.sender == 543835);   /* sender обязателен в живой модели */
        CHECK_EQ(msg.has_cid, 0);      /* опциональный cid не прислан */
    }

    KB_TEST("тип Chat: participants (map) отдельно от participantsCount");
    {
        kb_mp_writer w;
        size_t len;
        kb_chat chat;
        kb_message last;

        kb_mp_writer_init(&w, wire, sizeof wire);
        kb_mp_write_map(&w, 6);
        kb_mp_write_cstr(&w, "id");     kb_mp_write_i64(&w, 20000002);
        kb_mp_write_cstr(&w, "type");   kb_mp_write_cstr(&w, "CHAT");
        kb_mp_write_cstr(&w, "status"); kb_mp_write_cstr(&w, "ACTIVE");
        /* participants: map из 2 записей id->joinTime. */
        kb_mp_write_cstr(&w, "participants"); kb_mp_write_map(&w, 2);
        kb_mp_write_cstr(&w, "10000001"); kb_mp_write_i64(&w, 1700000000000LL);
        kb_mp_write_cstr(&w, "543835");   kb_mp_write_i64(&w, 1700000000000LL);
        /* отдельное числовое поле participantsCount. */
        kb_mp_write_cstr(&w, "participantsCount"); kb_mp_write_i64(&w, 2);
        /* вложенное сообщение. */
        kb_mp_write_cstr(&w, "lastMessage"); kb_mp_write_map(&w, 4);
        kb_mp_write_cstr(&w, "id");   kb_mp_write_i64(&w, 999);
        kb_mp_write_cstr(&w, "time"); kb_mp_write_i64(&w, 111);
        kb_mp_write_cstr(&w, "type"); kb_mp_write_cstr(&w, "USER");
        kb_mp_write_cstr(&w, "text"); kb_mp_write_cstr(&w, "hi");
        len = kb_mp_writer_len(&w);

        CHECK_EQ(kb_decode_chat(wire, len, &chat), KB_OK);
        CHECK(chat.id == 20000002);
        CHECK_MEM(chat.type.data, chat.type.len, "CHAT", 4);
        /* companion карты participants — participants_len (2 записи), а не
           participants_count: последнее занято реальным полем. */
        CHECK_EQ(chat.participants_len, (size_t)2);
        CHECK_EQ(chat.has_participants_count, 1);
        CHECK(chat.participants_count == 2);
        /* lastMessage лежит сырым куском — разбираем своим декодером. */
        CHECK(chat.last_message.len > 0);
        CHECK_EQ(kb_decode_message(chat.last_message.data,
                                   chat.last_message.len, &last), KB_OK);
        CHECK(last.id == 999);
        CHECK_MEM(last.text.data, last.text.len, "hi", 2);
    }
    KB_TEST("союз Attach: PHOTO и CALL по _type");
    {
        kb_mp_writer w;
        size_t len;
        kb_attach a;

        /* PHOTO. */
        kb_mp_writer_init(&w, wire, sizeof wire);
        kb_mp_write_map(&w, 4);
        kb_mp_write_cstr(&w, "_type");   kb_mp_write_cstr(&w, "PHOTO");
        kb_mp_write_cstr(&w, "photoId"); kb_mp_write_i64(&w, 28757217240LL);
        kb_mp_write_cstr(&w, "width");   kb_mp_write_i64(&w, 1024);
        kb_mp_write_cstr(&w, "height");  kb_mp_write_i64(&w, 780);
        len = kb_mp_writer_len(&w);
        CHECK_EQ(kb_decode_attach(wire, len, &a), KB_OK);
        CHECK_MEM(a._type.data, a._type.len, "PHOTO", 5);
        CHECK_EQ(a.has_photo_id, 1);
        CHECK(a.photo_id == 28757217240LL);
        CHECK_EQ(a.has_width, 1);
        CHECK_EQ(a.has_conversation_id, 0); /* поле CALL — не пришло */

        /* CALL — другой набор полей под тем же союзом. */
        kb_mp_writer_init(&w, wire, sizeof wire);
        kb_mp_write_map(&w, 3);
        kb_mp_write_cstr(&w, "_type");          kb_mp_write_cstr(&w, "CALL");
        kb_mp_write_cstr(&w, "callType");       kb_mp_write_cstr(&w, "AUDIO");
        kb_mp_write_cstr(&w, "hangupType");     kb_mp_write_cstr(&w, "MISSED");
        len = kb_mp_writer_len(&w);
        CHECK_EQ(kb_decode_attach(wire, len, &a), KB_OK);
        CHECK_MEM(a._type.data, a._type.len, "CALL", 4);
        CHECK_EQ(a.has_call_type, 1);
        CHECK_MEM(a.call_type.data, a.call_type.len, "AUDIO", 5);
        CHECK_EQ(a.has_photo_id, 0);          /* поле PHOTO — не пришло */
    }
}


