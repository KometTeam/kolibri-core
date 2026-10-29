/*
 * @generated tools/gen_schema.py -- не редактировать руками.
 *
 * Источник: файлы schema, расширение toml
 * Перегенерация: python3 tools/gen_schema.py
 */


#include <string.h>

#include "kolibri_ops.h"

kb_status kb_encode_audio_transcription(const kb_audio_transcription *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 3;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "mediaId");
    kb_mp_write_i64(&w, (int64_t)v->media_id);
    kb_mp_write_cstr(&w, "messageId");
    kb_mp_write_i64(&w, (int64_t)v->message_id);
    kb_mp_write_cstr(&w, "chatId");
    kb_mp_write_i64(&w, (int64_t)v->chat_id);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_audio_transcription_response(const uint8_t *body, size_t len, kb_audio_transcription_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "transcription")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->transcription.data = (const char *)value.data;
            out->transcription.len = value.len;
        } else if (kb_mp_str_eq(&key, "transcriptionStatus")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->transcription_status = (int32_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->transcription_status = (int32_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_auth2fa_details(const kb_auth2fa_details *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_track_id) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    if (v->has_track_id) {
        kb_mp_write_cstr(&w, "trackId");
        kb_mp_write_str(&w, v->track_id.data, v->track_id.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_auth2fa_details_response(const uint8_t *body, size_t len, kb_auth2fa_details_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_auth_check_email(const kb_auth_check_email *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "trackId");
    kb_mp_write_str(&w, v->track_id.data, v->track_id.len);
    kb_mp_write_cstr(&w, "verifyCode");
    kb_mp_write_str(&w, v->verify_code.data, v->verify_code.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_auth_check_email_response(const uint8_t *body, size_t len, kb_auth_check_email_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "trackId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->track_id.data = (const char *)value.data;
            out->track_id.len = value.len;
            out->has_track_id = 1;
        } else if (kb_mp_str_eq(&key, "email")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->email.data = (const char *)value.data;
            out->email.len = value.len;
            out->has_email = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_auth_check_password(const kb_auth_check_password *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_auth_check_password_response(const uint8_t *body, size_t len, kb_auth_check_password_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "trackId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->track_id.data = (const char *)value.data;
            out->track_id.len = value.len;
            out->has_track_id = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_auth_create_track(const kb_auth_create_track *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "type");
    kb_mp_write_i64(&w, (int64_t)v->type);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_auth_create_track_response(const uint8_t *body, size_t len, kb_auth_create_track_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "trackId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->track_id.data = (const char *)value.data;
            out->track_id.len = value.len;
            out->has_track_id = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_auth_login_check_password(const kb_auth_login_check_password *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "trackId");
    kb_mp_write_str(&w, v->track_id.data, v->track_id.len);
    kb_mp_write_cstr(&w, "password");
    kb_mp_write_str(&w, v->password.data, v->password.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_auth_login_check_password_response(const uint8_t *body, size_t len, kb_auth_login_check_password_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "tokenAttrs")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->token_attrs_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->token_attrs.data = body + start;
            out->token_attrs.len = kb_mp_reader_pos(&reader) - start;
            out->has_token_attrs = 1;
        } else if (kb_mp_str_eq(&key, "profile")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->profile.data = body + start;
            out->profile.len = kb_mp_reader_pos(&reader) - start;
            out->has_profile = 1;
        } else if (kb_mp_str_eq(&key, "error")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->error.data = body + start;
            out->error.len = kb_mp_reader_pos(&reader) - start;
            out->has_error = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_auth_login_restore_password(const kb_auth_login_restore_password *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_auth_login_restore_password_response(const uint8_t *body, size_t len, kb_auth_login_restore_password_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "tokenAttrs")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->token_attrs_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->token_attrs.data = body + start;
            out->token_attrs.len = kb_mp_reader_pos(&reader) - start;
            out->has_token_attrs = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_auth_request(const kb_auth_request *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_language) { count++; }
    if (v->has_mode) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "phone");
    kb_mp_write_str(&w, v->phone.data, v->phone.len);
    kb_mp_write_cstr(&w, "type");
    kb_mp_write_str(&w, v->type.data, v->type.len);
    if (v->has_language) {
        kb_mp_write_cstr(&w, "language");
        kb_mp_write_str(&w, v->language.data, v->language.len);
    }
    if (v->has_mode) {
        kb_mp_write_cstr(&w, "mode");
        kb_mp_write_bin(&w, v->mode.data, v->mode.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_auth_request_response(const uint8_t *body, size_t len, kb_auth_request_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "token")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->token.data = (const char *)value.data;
            out->token.len = value.len;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_auth_set2fa(const kb_auth_set2fa *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_password) { count++; }
    if (v->has_hint) { count++; }
    if (v->has_remove2fa) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "trackId");
    kb_mp_write_str(&w, v->track_id.data, v->track_id.len);
    if (v->has_password) {
        kb_mp_write_cstr(&w, "password");
        kb_mp_write_str(&w, v->password.data, v->password.len);
    }
    if (v->has_hint) {
        kb_mp_write_cstr(&w, "hint");
        kb_mp_write_str(&w, v->hint.data, v->hint.len);
    }
    if (v->has_remove2fa) {
        kb_mp_write_cstr(&w, "remove2fa");
        kb_mp_write_bool(&w, v->remove2fa);
    }
    kb_mp_write_cstr(&w, "expectedCapabilities");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->expected_capabilities.data, v->expected_capabilities.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_auth_set2fa_response(const uint8_t *body, size_t len, kb_auth_set2fa_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_auth_verify(const kb_auth_verify *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 3;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "token");
    kb_mp_write_str(&w, v->token.data, v->token.len);
    kb_mp_write_cstr(&w, "verifyCode");
    kb_mp_write_str(&w, v->verify_code.data, v->verify_code.len);
    kb_mp_write_cstr(&w, "authTokenType");
    kb_mp_write_str(&w, v->auth_token_type.data, v->auth_token_type.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_auth_verify_response(const uint8_t *body, size_t len, kb_auth_verify_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "tokenAttrs")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->token_attrs_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->token_attrs.data = body + start;
            out->token_attrs.len = kb_mp_reader_pos(&reader) - start;
            out->has_token_attrs = 1;
        } else if (kb_mp_str_eq(&key, "profile")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->profile.data = body + start;
            out->profile.len = kb_mp_reader_pos(&reader) - start;
            out->has_profile = 1;
        } else if (kb_mp_str_eq(&key, "passwordChallenge")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->password_challenge.data = body + start;
            out->password_challenge.len = kb_mp_reader_pos(&reader) - start;
            out->has_password_challenge = 1;
        } else if (kb_mp_str_eq(&key, "presetAvatars")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->preset_avatars.data = body + start;
            out->preset_avatars.len = kb_mp_reader_pos(&reader) - start;
            out->has_preset_avatars = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_auth_verify_email(const kb_auth_verify_email *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_email) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "trackId");
    kb_mp_write_str(&w, v->track_id.data, v->track_id.len);
    if (v->has_email) {
        kb_mp_write_cstr(&w, "email");
        kb_mp_write_str(&w, v->email.data, v->email.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_auth_verify_email_response(const uint8_t *body, size_t len, kb_auth_verify_email_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_banners_get(const kb_banners_get *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "bannersSync");
    kb_mp_write_i64(&w, (int64_t)v->banners_sync);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_banners_get_response(const uint8_t *body, size_t len, kb_banners_get_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_bot_info(const kb_bot_info *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_bot_info_response(const uint8_t *body, size_t len, kb_bot_info_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "contact")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->contact.data = body + start;
            out->contact.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "startMessage")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->start_message.data = body + start;
            out->start_message.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "commands")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->commands_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->commands.data = body + start;
            out->commands.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_call_history(const kb_call_history *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "callHistorySync");
    kb_mp_write_i64(&w, (int64_t)v->call_history_sync);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_call_history_response(const uint8_t *body, size_t len, kb_call_history_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "reset")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->reset = value.boolean;
        } else if (kb_mp_str_eq(&key, "callHistoryItems")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->call_history_items_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->call_history_items.data = body + start;
            out->call_history_items.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "callHistorySync")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->call_history_sync = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->call_history_sync = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_chat_history(const kb_chat_history *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 10;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_post_id) { count++; }
    if (v->has_chat_access_token) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "chatId");
    kb_mp_write_i64(&w, (int64_t)v->chat_id);
    if (v->has_post_id) {
        kb_mp_write_cstr(&w, "postId");
        kb_mp_write_i64(&w, (int64_t)v->post_id);
    }
    kb_mp_write_cstr(&w, "from");
    kb_mp_write_i64(&w, (int64_t)v->from);
    kb_mp_write_cstr(&w, "forward");
    kb_mp_write_i64(&w, (int64_t)v->forward);
    kb_mp_write_cstr(&w, "forwardTime");
    kb_mp_write_i64(&w, (int64_t)v->forward_time);
    kb_mp_write_cstr(&w, "backward");
    kb_mp_write_i64(&w, (int64_t)v->backward);
    kb_mp_write_cstr(&w, "backwardTime");
    kb_mp_write_i64(&w, (int64_t)v->backward_time);
    kb_mp_write_cstr(&w, "getChat");
    kb_mp_write_bool(&w, v->get_chat);
    kb_mp_write_cstr(&w, "getMessages");
    kb_mp_write_bool(&w, v->get_messages);
    if (v->has_chat_access_token) {
        kb_mp_write_cstr(&w, "chatAccessToken");
        kb_mp_write_str(&w, v->chat_access_token.data, v->chat_access_token.len);
    }
    kb_mp_write_cstr(&w, "itemType");
    kb_mp_write_str(&w, v->item_type.data, v->item_type.len);
    kb_mp_write_cstr(&w, "interactive");
    kb_mp_write_bool(&w, v->interactive);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_chat_history_response(const uint8_t *body, size_t len, kb_chat_history_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_chat_info(const kb_chat_info *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "chatIds");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->chat_ids.data, v->chat_ids.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_chat_info_response(const uint8_t *body, size_t len, kb_chat_info_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_chat_join(const kb_chat_join *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_chat_join_response(const uint8_t *body, size_t len, kb_chat_join_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "chat")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->chat.data = body + start;
            out->chat.len = kb_mp_reader_pos(&reader) - start;
            out->has_chat = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_chat_livestream_info(const kb_chat_livestream_info *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_chat_livestream_info_response(const uint8_t *body, size_t len, kb_chat_livestream_info_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "liveStreams")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->live_streams_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->live_streams.data = body + start;
            out->live_streams.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_chat_mark(const kb_chat_mark *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_chat_mark_response(const uint8_t *body, size_t len, kb_chat_mark_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "timestamp")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->timestamp = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->timestamp = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_chat_media(const kb_chat_media *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_attach_types) { count++; }
    if (v->has_forward) { count++; }
    if (v->has_backward) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "chatId");
    kb_mp_write_i64(&w, (int64_t)v->chat_id);
    kb_mp_write_cstr(&w, "messageId");
    kb_mp_write_i64(&w, (int64_t)v->message_id);
    if (v->has_attach_types) {
        kb_mp_write_cstr(&w, "attachTypes");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->attach_types.data, v->attach_types.len);
    }
    if (v->has_forward) {
        kb_mp_write_cstr(&w, "forward");
        kb_mp_write_i64(&w, (int64_t)v->forward);
    }
    if (v->has_backward) {
        kb_mp_write_cstr(&w, "backward");
        kb_mp_write_i64(&w, (int64_t)v->backward);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_chat_media_response(const uint8_t *body, size_t len, kb_chat_media_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_chat_members(const kb_chat_members *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_type) { count++; }
    if (v->has_marker) { count++; }
    if (v->has_count) { count++; }
    if (v->has_query) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "chatId");
    kb_mp_write_i64(&w, (int64_t)v->chat_id);
    if (v->has_type) {
        kb_mp_write_cstr(&w, "type");
        kb_mp_write_str(&w, v->type.data, v->type.len);
    }
    if (v->has_marker) {
        kb_mp_write_cstr(&w, "marker");
        kb_mp_write_i64(&w, (int64_t)v->marker);
    }
    if (v->has_count) {
        kb_mp_write_cstr(&w, "count");
        kb_mp_write_i64(&w, (int64_t)v->count);
    }
    if (v->has_query) {
        kb_mp_write_cstr(&w, "query");
        kb_mp_write_str(&w, v->query.data, v->query.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_chat_members_response(const uint8_t *body, size_t len, kb_chat_members_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "members")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->members_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->members.data = body + start;
            out->members.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "marker")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->marker = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->marker = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_chat_personal_config(const kb_chat_personal_config *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_chat_personal_config_response(const uint8_t *body, size_t len, kb_chat_personal_config_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "chat")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->chat.data = body + start;
            out->chat.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_chat_reactions_settings_set(const kb_chat_reactions_settings_set *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_chat_reactions_settings_set_response(const uint8_t *body, size_t len, kb_chat_reactions_settings_set_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "chatReactionsSettings")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->chat_reactions_settings.data = body + start;
            out->chat_reactions_settings.len = kb_mp_reader_pos(&reader) - start;
            out->has_chat_reactions_settings = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_chat_suggest(const kb_chat_suggest *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_folder_id) { count++; }
    if (v->has_user_chat_ids) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    if (v->has_folder_id) {
        kb_mp_write_cstr(&w, "folderId");
        kb_mp_write_str(&w, v->folder_id.data, v->folder_id.len);
    }
    if (v->has_user_chat_ids) {
        kb_mp_write_cstr(&w, "userChatIds");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->user_chat_ids.data, v->user_chat_ids.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_chat_suggest_response(const uint8_t *body, size_t len, kb_chat_suggest_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "chats")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->chats_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->chats.data = body + start;
            out->chats.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_complain(const kb_complain *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 3;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_parent_id) { count++; }
    if (v->has_post_id) { count++; }
    if (v->has_details) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "typeId");
    kb_mp_write_i64(&w, (int64_t)v->type_id);
    kb_mp_write_cstr(&w, "reasonId");
    kb_mp_write_i64(&w, (int64_t)v->reason_id);
    kb_mp_write_cstr(&w, "ids");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->ids.data, v->ids.len);
    if (v->has_parent_id) {
        kb_mp_write_cstr(&w, "parentId");
        kb_mp_write_i64(&w, (int64_t)v->parent_id);
    }
    if (v->has_post_id) {
        kb_mp_write_cstr(&w, "postId");
        kb_mp_write_i64(&w, (int64_t)v->post_id);
    }
    if (v->has_details) {
        kb_mp_write_cstr(&w, "details");
        kb_mp_write_str(&w, v->details.data, v->details.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_complain_response(const uint8_t *body, size_t len, kb_complain_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "success")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->success = value.boolean;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_complain_reasons_get(const kb_complain_reasons_get *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_complain_reasons_get_response(const uint8_t *body, size_t len, kb_complain_reasons_get_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "complainSync")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->complain_sync = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->complain_sync = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "complains")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->complains_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->complains.data = body + start;
            out->complains.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_config(const kb_config *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_push_token) { count++; }
    if (v->has_push_tokens) { count++; }
    if (v->has_push_options) { count++; }
    if (v->has_settings) { count++; }
    if (v->has_reset) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    if (v->has_push_token) {
        kb_mp_write_cstr(&w, "pushToken");
        kb_mp_write_str(&w, v->push_token.data, v->push_token.len);
    }
    if (v->has_push_tokens) {
        kb_mp_write_cstr(&w, "pushTokens");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->push_tokens.data, v->push_tokens.len);
    }
    if (v->has_push_options) {
        kb_mp_write_cstr(&w, "pushOptions");
        kb_mp_write_i64(&w, (int64_t)v->push_options);
    }
    if (v->has_settings) {
        kb_mp_write_cstr(&w, "settings");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->settings.data, v->settings.len);
    }
    if (v->has_reset) {
        kb_mp_write_cstr(&w, "reset");
        kb_mp_write_bool(&w, v->reset);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_config_response(const uint8_t *body, size_t len, kb_config_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "hash")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->hash.data = (const char *)value.data;
            out->hash.len = value.len;
            out->has_hash = 1;
        } else if (kb_mp_str_eq(&key, "user")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->user.data = body + start;
            out->user.len = kb_mp_reader_pos(&reader) - start;
            out->has_user = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_contact_info(const kb_contact_info *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_chat_id) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "contactIds");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->contact_ids.data, v->contact_ids.len);
    if (v->has_chat_id) {
        kb_mp_write_cstr(&w, "chat_id");
        kb_mp_write_i64(&w, (int64_t)v->chat_id);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_contact_info_response(const uint8_t *body, size_t len, kb_contact_info_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "contacts")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->contacts_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->contacts.data = body + start;
            out->contacts.len = kb_mp_reader_pos(&reader) - start;
            out->has_contacts = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_contact_info_by_phone(const kb_contact_info_by_phone *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_contact_info_by_phone_response(const uint8_t *body, size_t len, kb_contact_info_by_phone_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "contact")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->contact.data = body + start;
            out->contact.len = kb_mp_reader_pos(&reader) - start;
            out->has_contact = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_contact_presence(const kb_contact_presence *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_contact_presence_response(const uint8_t *body, size_t len, kb_contact_presence_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "presence")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->presence.data = body + start;
            out->presence.len = kb_mp_reader_pos(&reader) - start;
            out->has_presence = 1;
        } else if (kb_mp_str_eq(&key, "time")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_external_callback(const kb_external_callback *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_external_callback_response(const uint8_t *body, size_t len, kb_external_callback_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "botId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->bot_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->bot_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_bot_id = 1;
        } else if (kb_mp_str_eq(&key, "startParam")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->start_param.data = (const char *)value.data;
            out->start_param.len = value.len;
            out->has_start_param = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_file_download(const kb_file_download *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 3;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "fileId");
    kb_mp_write_i64(&w, (int64_t)v->file_id);
    kb_mp_write_cstr(&w, "chatId");
    kb_mp_write_i64(&w, (int64_t)v->chat_id);
    kb_mp_write_cstr(&w, "messageId");
    kb_mp_write_i64(&w, (int64_t)v->message_id);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_file_download_response(const uint8_t *body, size_t len, kb_file_download_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "url")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->url.data = (const char *)value.data;
            out->url.len = value.len;
        } else if (kb_mp_str_eq(&key, "unsafe")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->unsafe = value.boolean;
            out->has_unsafe = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_folders_delete(const kb_folders_delete *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "folderIds");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->folder_ids.data, v->folder_ids.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_folders_delete_response(const uint8_t *body, size_t len, kb_folders_delete_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "folderSync")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->folder_sync = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->folder_sync = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_folder_sync = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_folders_get(const kb_folders_get *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "folderSync");
    kb_mp_write_i64(&w, (int64_t)v->folder_sync);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_folders_get_response(const uint8_t *body, size_t len, kb_folders_get_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "folderSync")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->folder_sync = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->folder_sync = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "folders")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->folders_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->folders.data = body + start;
            out->folders.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "foldersOrder")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->folders_order_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->folders_order.data = body + start;
            out->folders_order.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "allFilterExcludeFolders")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->all_filter_exclude_folders_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->all_filter_exclude_folders.data = body + start;
            out->all_filter_exclude_folders.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_folders_get_by_id(const kb_folders_get_by_id *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_folders_get_by_id_response(const uint8_t *body, size_t len, kb_folders_get_by_id_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "folders")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->folders_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->folders.data = body + start;
            out->folders.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_folders_reorder(const kb_folders_reorder *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "foldersOrder");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->folders_order.data, v->folders_order.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_folders_reorder_response(const uint8_t *body, size_t len, kb_folders_reorder_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "folderSync")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->folder_sync = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->folder_sync = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_folder_sync = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_folders_update(const kb_folders_update *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_include) { count++; }
    if (v->has_favorites) { count++; }
    if (v->has_filters) { count++; }
    if (v->has_options) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "id");
    kb_mp_write_str(&w, v->id.data, v->id.len);
    kb_mp_write_cstr(&w, "title");
    kb_mp_write_str(&w, v->title.data, v->title.len);
    if (v->has_include) {
        kb_mp_write_cstr(&w, "include");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->include.data, v->include.len);
    }
    if (v->has_favorites) {
        kb_mp_write_cstr(&w, "favorites");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->favorites.data, v->favorites.len);
    }
    if (v->has_filters) {
        kb_mp_write_cstr(&w, "filters");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->filters.data, v->filters.len);
    }
    if (v->has_options) {
        kb_mp_write_cstr(&w, "options");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->options.data, v->options.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_folders_update_response(const uint8_t *body, size_t len, kb_folders_update_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "folder")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->folder.data = body + start;
            out->folder.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "folderSync")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->folder_sync = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->folder_sync = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "foldersOrder")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->folders_order_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->folders_order.data = body + start;
            out->folders_order.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_get_comments_updates(const kb_get_comments_updates *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_get_comments_updates_response(const uint8_t *body, size_t len, kb_get_comments_updates_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "commentsInfoUpdates")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->comments_info_updates_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->comments_info_updates.data = body + start;
            out->comments_info_updates.len = kb_mp_reader_pos(&reader) - start;
            out->has_comments_info_updates = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_get_poll_updates(const kb_get_poll_updates *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "chatId");
    kb_mp_write_i64(&w, (int64_t)v->chat_id);
    kb_mp_write_cstr(&w, "polls");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->polls.data, v->polls.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_get_poll_updates_response(const uint8_t *body, size_t len, kb_get_poll_updates_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "polls")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->polls_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->polls.data = body + start;
            out->polls.len = kb_mp_reader_pos(&reader) - start;
            out->has_polls = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_login(const kb_login *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_exp) { count++; }
    if (v->has_chat_cache_fingerprint) { count++; }
    if (v->has_presence_sync) { count++; }
    if (v->has_chats_sync) { count++; }
    if (v->has_contacts_sync) { count++; }
    if (v->has_calls_sync) { count++; }
    if (v->has_last_login) { count++; }
    if (v->has_banners_sync) { count++; }
    if (v->has_config_hash) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "token");
    kb_mp_write_str(&w, v->token.data, v->token.len);
    kb_mp_write_cstr(&w, "interactive");
    kb_mp_write_bool(&w, v->interactive);
    if (v->has_exp) {
        kb_mp_write_cstr(&w, "exp");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->exp.data, v->exp.len);
    }
    if (v->has_chat_cache_fingerprint) {
        kb_mp_write_cstr(&w, "chatCacheFingerprint");
        kb_mp_write_bin(&w, v->chat_cache_fingerprint.data, v->chat_cache_fingerprint.len);
    }
    if (v->has_presence_sync) {
        kb_mp_write_cstr(&w, "presenceSync");
        kb_mp_write_i64(&w, (int64_t)v->presence_sync);
    }
    if (v->has_chats_sync) {
        kb_mp_write_cstr(&w, "chatsSync");
        kb_mp_write_i64(&w, (int64_t)v->chats_sync);
    }
    if (v->has_contacts_sync) {
        kb_mp_write_cstr(&w, "contactsSync");
        kb_mp_write_i64(&w, (int64_t)v->contacts_sync);
    }
    if (v->has_calls_sync) {
        kb_mp_write_cstr(&w, "callsSync");
        kb_mp_write_i64(&w, (int64_t)v->calls_sync);
    }
    if (v->has_last_login) {
        kb_mp_write_cstr(&w, "lastLogin");
        kb_mp_write_i64(&w, (int64_t)v->last_login);
    }
    if (v->has_banners_sync) {
        kb_mp_write_cstr(&w, "bannersSync");
        kb_mp_write_i64(&w, (int64_t)v->banners_sync);
    }
    if (v->has_config_hash) {
        kb_mp_write_cstr(&w, "configHash");
        kb_mp_write_str(&w, v->config_hash.data, v->config_hash.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_login_response(const uint8_t *body, size_t len, kb_login_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "token")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->token.data = (const char *)value.data;
            out->token.len = value.len;
            out->has_token = 1;
        } else if (kb_mp_str_eq(&key, "profile")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->profile.data = body + start;
            out->profile.len = kb_mp_reader_pos(&reader) - start;
            out->has_profile = 1;
        } else if (kb_mp_str_eq(&key, "chats")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->chats_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->chats.data = body + start;
            out->chats.len = kb_mp_reader_pos(&reader) - start;
            out->has_chats = 1;
        } else if (kb_mp_str_eq(&key, "time")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_time = 1;
        } else if (kb_mp_str_eq(&key, "config")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->config.data = body + start;
            out->config.len = kb_mp_reader_pos(&reader) - start;
            out->has_config = 1;
        } else if (kb_mp_str_eq(&key, "contacts")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->contacts.data = body + start;
            out->contacts.len = kb_mp_reader_pos(&reader) - start;
            out->has_contacts = 1;
        } else if (kb_mp_str_eq(&key, "presence")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->presence.data = body + start;
            out->presence.len = kb_mp_reader_pos(&reader) - start;
            out->has_presence = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_logout(const kb_logout *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_logout_response(const uint8_t *body, size_t len, kb_logout_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_msg_get(const kb_msg_get *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "chatId");
    kb_mp_write_i64(&w, (int64_t)v->chat_id);
    kb_mp_write_cstr(&w, "messageIds");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->message_ids.data, v->message_ids.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_msg_get_response(const uint8_t *body, size_t len, kb_msg_get_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "chatId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->chat_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->chat_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "messages")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->messages_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->messages.data = body + start;
            out->messages.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_msg_get_reactions(const kb_msg_get_reactions *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_msg_get_reactions_response(const uint8_t *body, size_t len, kb_msg_get_reactions_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "messagesReactions")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->messages_reactions_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->messages_reactions.data = body + start;
            out->messages_reactions.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_msg_send(const kb_msg_send *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_notify) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "chatId");
    kb_mp_write_i64(&w, (int64_t)v->chat_id);
    kb_mp_write_cstr(&w, "message");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->message.data, v->message.len);
    if (v->has_notify) {
        kb_mp_write_cstr(&w, "notify");
        kb_mp_write_bool(&w, v->notify);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_msg_send_response(const uint8_t *body, size_t len, kb_msg_send_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "chatId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->chat_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->chat_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_chat_id = 1;
        } else if (kb_mp_str_eq(&key, "message")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->message.data = body + start;
            out->message.len = kb_mp_reader_pos(&reader) - start;
            out->has_message = 1;
        } else if (kb_mp_str_eq(&key, "unread")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->unread = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->unread = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_unread = 1;
        } else if (kb_mp_str_eq(&key, "mark")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->mark = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->mark = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_mark = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_msg_typing(const kb_msg_typing *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "chatId");
    kb_mp_write_i64(&w, (int64_t)v->chat_id);
    kb_mp_write_cstr(&w, "type");
    kb_mp_write_str(&w, v->type.data, v->type.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_msg_typing_response(const uint8_t *body, size_t len, kb_msg_typing_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_notif_call_history(const kb_notif_call_history *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_notif_call_history_response(const uint8_t *body, size_t len, kb_notif_call_history_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "historyIds")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->history_ids_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->history_ids.data = body + start;
            out->history_ids.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "callHistoryItems")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->call_history_items_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->call_history_items.data = body + start;
            out->call_history_items.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "prevCallHistorySync")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->prev_call_history_sync = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->prev_call_history_sync = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "callHistorySync")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->call_history_sync = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->call_history_sync = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "action")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->action.data = (const char *)value.data;
            out->action.len = value.len;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_notif_folders(const kb_notif_folders *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_notif_folders_response(const uint8_t *body, size_t len, kb_notif_folders_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "folderSync")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->folder_sync = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->folder_sync = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "folders")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->folders_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->folders.data = body + start;
            out->folders.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "foldersOrder")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->folders_order_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->folders_order.data = body + start;
            out->folders_order.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "allFilterExcludeFolders")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->all_filter_exclude_folders_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->all_filter_exclude_folders.data = body + start;
            out->all_filter_exclude_folders.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_notif_msg_delayed(const kb_notif_msg_delayed *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_notif_msg_delayed_response(const uint8_t *body, size_t len, kb_notif_msg_delayed_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "message")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->message.data = body + start;
            out->message.len = kb_mp_reader_pos(&reader) - start;
            out->has_message = 1;
        } else if (kb_mp_str_eq(&key, "userId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->user_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->user_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "updateTypeId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->update_type_id = (int32_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->update_type_id = (int32_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "lastDelayedUpdateTime")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->last_delayed_update_time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->last_delayed_update_time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_last_delayed_update_time = 1;
        } else if (kb_mp_str_eq(&key, "chatId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->chat_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->chat_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "messageIds")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->message_ids_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->message_ids.data = body + start;
            out->message_ids.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_notif_msg_delete(const kb_notif_msg_delete *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_notif_msg_delete_response(const uint8_t *body, size_t len, kb_notif_msg_delete_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "chat")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->chat.data = body + start;
            out->chat.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "ttl")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->ttl = value.boolean;
        } else if (kb_mp_str_eq(&key, "messageIds")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->message_ids_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->message_ids.data = body + start;
            out->message_ids.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_notif_profile(const kb_notif_profile *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_notif_profile_response(const uint8_t *body, size_t len, kb_notif_profile_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "profile")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->profile.data = body + start;
            out->profile.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_notif_stories_update(const kb_notif_stories_update *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_notif_stories_update_response(const uint8_t *body, size_t len, kb_notif_stories_update_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "storiesPreview")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->stories_preview.data = body + start;
            out->stories_preview.len = kb_mp_reader_pos(&reader) - start;
            out->has_stories_preview = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_ok_token(const kb_ok_token *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "value");
    kb_mp_write_str(&w, v->value.data, v->value.len);
    kb_mp_write_cstr(&w, "userId");
    kb_mp_write_i64(&w, (int64_t)v->user_id);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_ok_token_response(const uint8_t *body, size_t len, kb_ok_token_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "token")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->token.data = (const char *)value.data;
            out->token.len = value.len;
        } else if (kb_mp_str_eq(&key, "token_refresh_ts")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->token_refresh_ts = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->token_refresh_ts = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "token_lifetime_ts")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->token_lifetime_ts = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->token_lifetime_ts = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_org_info(const kb_org_info *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_organization_ids) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    if (v->has_organization_ids) {
        kb_mp_write_cstr(&w, "organizationIds");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->organization_ids.data, v->organization_ids.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_org_info_response(const uint8_t *body, size_t len, kb_org_info_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "organizations")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->organizations_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->organizations.data = body + start;
            out->organizations.len = kb_mp_reader_pos(&reader) - start;
            out->has_organizations = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_photo_upload(const kb_photo_upload *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_type) { count++; }
    if (v->has_profile) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    if (v->has_type) {
        kb_mp_write_cstr(&w, "type");
        kb_mp_write_i64(&w, (int64_t)v->type);
    }
    kb_mp_write_cstr(&w, "count");
    kb_mp_write_i64(&w, (int64_t)v->count);
    if (v->has_profile) {
        kb_mp_write_cstr(&w, "profile");
        kb_mp_write_bool(&w, v->profile);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_photo_upload_response(const uint8_t *body, size_t len, kb_photo_upload_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "url")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->url.data = (const char *)value.data;
            out->url.len = value.len;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_photo_url_refresh(const kb_photo_url_refresh *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_photo_url_refresh_response(const uint8_t *body, size_t len, kb_photo_url_refresh_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "media")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->media_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->media.data = body + start;
            out->media.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_preset_avatars(const kb_preset_avatars *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_preset_avatars_response(const uint8_t *body, size_t len, kb_preset_avatars_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "presetAvatars")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->preset_avatars_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->preset_avatars.data = body + start;
            out->preset_avatars.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_profile_delete(const kb_profile_delete *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_type) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "delete");
    kb_mp_write_bool(&w, v->delete_);
    if (v->has_type) {
        kb_mp_write_cstr(&w, "type");
        kb_mp_write_i64(&w, (int64_t)v->type);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_profile_delete_response(const uint8_t *body, size_t len, kb_profile_delete_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "timestamp")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->timestamp = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->timestamp = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_profile_delete_time(const kb_profile_delete_time *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_profile_delete_time_response(const uint8_t *body, size_t len, kb_profile_delete_time_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "timestamp")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->timestamp = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->timestamp = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_reactions_settings_get_by_chat_id(const kb_reactions_settings_get_by_chat_id *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_reactions_settings_get_by_chat_id_response(const uint8_t *body, size_t len, kb_reactions_settings_get_by_chat_id_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "chatReactionsSettings")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->chat_reactions_settings_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->chat_reactions_settings.data = body + start;
            out->chat_reactions_settings.len = kb_mp_reader_pos(&reader) - start;
            out->has_chat_reactions_settings = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_send_vote(const kb_send_vote *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 4;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "chatId");
    kb_mp_write_i64(&w, (int64_t)v->chat_id);
    kb_mp_write_cstr(&w, "pollId");
    kb_mp_write_i64(&w, (int64_t)v->poll_id);
    kb_mp_write_cstr(&w, "messageId");
    kb_mp_write_i64(&w, (int64_t)v->message_id);
    kb_mp_write_cstr(&w, "answersIds");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->answers_ids.data, v->answers_ids.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_send_vote_response(const uint8_t *body, size_t len, kb_send_vote_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "state")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->state.data = body + start;
            out->state.len = kb_mp_reader_pos(&reader) - start;
            out->has_state = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_stories_delete(const kb_stories_delete *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_stories_delete_response(const uint8_t *body, size_t len, kb_stories_delete_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "storiesPreview")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->stories_preview.data = body + start;
            out->stories_preview.len = kb_mp_reader_pos(&reader) - start;
            out->has_stories_preview = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_stories_edit(const kb_stories_edit *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_stories_edit_response(const uint8_t *body, size_t len, kb_stories_edit_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "story")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->story.data = body + start;
            out->story.len = kb_mp_reader_pos(&reader) - start;
            out->has_story = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_stories_get_by_owner(const kb_stories_get_by_owner *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_stories_get_by_owner_response(const uint8_t *body, size_t len, kb_stories_get_by_owner_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "peerStories")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->peer_stories_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->peer_stories.data = body + start;
            out->peer_stories.len = kb_mp_reader_pos(&reader) - start;
            out->has_peer_stories = 1;
        } else if (kb_mp_str_eq(&key, "storiesPreviews")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->stories_previews_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->stories_previews.data = body + start;
            out->stories_previews.len = kb_mp_reader_pos(&reader) - start;
            out->has_stories_previews = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_stories_get_by_story_id(const kb_stories_get_by_story_id *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_stories_get_by_story_id_response(const uint8_t *body, size_t len, kb_stories_get_by_story_id_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "stories")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->stories_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->stories.data = body + start;
            out->stories.len = kb_mp_reader_pos(&reader) - start;
            out->has_stories = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_stories_get_stats(const kb_stories_get_stats *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_stories_get_stats_response(const uint8_t *body, size_t len, kb_stories_get_stats_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "storyStats")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->story_stats_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->story_stats.data = body + start;
            out->story_stats.len = kb_mp_reader_pos(&reader) - start;
            out->has_story_stats = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_stories_list(const kb_stories_list *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_stories_list_response(const uint8_t *body, size_t len, kb_stories_list_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "storiesPreviews")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->stories_previews_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->stories_previews.data = body + start;
            out->stories_previews.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "cursor")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->cursor.data = (const char *)value.data;
            out->cursor.len = value.len;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_stories_mark(const kb_stories_mark *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_stories_mark_response(const uint8_t *body, size_t len, kb_stories_mark_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "success")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->success = value.boolean;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_stories_react(const kb_stories_react *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_stories_react_response(const uint8_t *body, size_t len, kb_stories_react_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "success")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->success = value.boolean;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_stories_send(const kb_stories_send *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_stories_send_response(const uint8_t *body, size_t len, kb_stories_send_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "stories")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->stories_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->stories.data = body + start;
            out->stories.len = kb_mp_reader_pos(&reader) - start;
            out->has_stories = 1;
        } else if (kb_mp_str_eq(&key, "storiesPreview")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->stories_preview.data = body + start;
            out->stories_preview.len = kb_mp_reader_pos(&reader) - start;
            out->has_stories_preview = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_transcription_result(const kb_transcription_result *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_transcription_result_response(const uint8_t *body, size_t len, kb_transcription_result_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "chatId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->chat_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->chat_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "messageId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->message_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->message_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "mediaId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->media_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->media_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "transcription")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->transcription.data = (const char *)value.data;
            out->transcription.len = value.len;
            out->has_transcription = 1;
        } else if (kb_mp_str_eq(&key, "transcriptionStatus")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->transcription_status.data = body + start;
            out->transcription_status.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_video_chat_delete_history(const kb_video_chat_delete_history *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_history_ids) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    if (v->has_history_ids) {
        kb_mp_write_cstr(&w, "historyIds");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->history_ids.data, v->history_ids.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_video_chat_delete_history_response(const uint8_t *body, size_t len, kb_video_chat_delete_history_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_video_chat_join_by_link(const kb_video_chat_join_by_link *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_video_chat_join_by_link_response(const uint8_t *body, size_t len, kb_video_chat_join_by_link_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "conversationId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->conversation_id.data = (const char *)value.data;
            out->conversation_id.len = value.len;
        } else if (kb_mp_str_eq(&key, "internalParams")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->internal_params.data = (const char *)value.data;
            out->internal_params.len = value.len;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_video_chat_start(const kb_video_chat_start *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_video_chat_start_response(const uint8_t *body, size_t len, kb_video_chat_start_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "type")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->type.data = (const char *)value.data;
            out->type.len = value.len;
            out->has_type = 1;
        } else if (kb_mp_str_eq(&key, "callerId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->caller_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->caller_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_caller_id = 1;
        } else if (kb_mp_str_eq(&key, "callName")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->call_name.data = (const char *)value.data;
            out->call_name.len = value.len;
            out->has_call_name = 1;
        } else if (kb_mp_str_eq(&key, "chatId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->chat_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->chat_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_chat_id = 1;
        } else if (kb_mp_str_eq(&key, "joinLink")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->join_link.data = (const char *)value.data;
            out->join_link.len = value.len;
            out->has_join_link = 1;
        } else if (kb_mp_str_eq(&key, "conversationId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->conversation_id.data = (const char *)value.data;
            out->conversation_id.len = value.len;
            out->has_conversation_id = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_video_chat_start_active(const kb_video_chat_start_active *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_video_chat_start_active_response(const uint8_t *body, size_t len, kb_video_chat_start_active_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "conversationId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->conversation_id.data = (const char *)value.data;
            out->conversation_id.len = value.len;
            out->has_conversation_id = 1;
        } else if (kb_mp_str_eq(&key, "internalCallerParams")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->internal_caller_params.data = (const char *)value.data;
            out->internal_caller_params.len = value.len;
            out->has_internal_caller_params = 1;
        } else if (kb_mp_str_eq(&key, "rejectedParticipants")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->rejected_participants_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->rejected_participants.data = body + start;
            out->rejected_participants.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_video_upload(const kb_video_upload *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 3;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "type");
    kb_mp_write_i64(&w, (int64_t)v->type);
    kb_mp_write_cstr(&w, "count");
    kb_mp_write_i64(&w, (int64_t)v->count);
    kb_mp_write_cstr(&w, "uploaderType");
    kb_mp_write_i64(&w, (int64_t)v->uploader_type);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_video_upload_response(const uint8_t *body, size_t len, kb_video_upload_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "info")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->info_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->info.data = body + start;
            out->info.len = kb_mp_reader_pos(&reader) - start;
            out->has_info = 1;
        } else if (kb_mp_str_eq(&key, "uploaderType")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->uploader_type = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->uploader_type = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_uploader_type = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_voters_list_by_answer(const kb_voters_list_by_answer *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 4;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_marker) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "chatId");
    kb_mp_write_i64(&w, (int64_t)v->chat_id);
    kb_mp_write_cstr(&w, "pollId");
    kb_mp_write_i64(&w, (int64_t)v->poll_id);
    kb_mp_write_cstr(&w, "messageId");
    kb_mp_write_i64(&w, (int64_t)v->message_id);
    kb_mp_write_cstr(&w, "answerId");
    kb_mp_write_i64(&w, (int64_t)v->answer_id);
    if (v->has_marker) {
        kb_mp_write_cstr(&w, "marker");
        kb_mp_write_i64(&w, (int64_t)v->marker);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_voters_list_by_answer_response(const uint8_t *body, size_t len, kb_voters_list_by_answer_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_web_app_init_data(const kb_web_app_init_data *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_web_app_init_data_response(const uint8_t *body, size_t len, kb_web_app_init_data_response *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "url")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->url.data = (const char *)value.data;
            out->url.len = value.len;
        } else if (kb_mp_str_eq(&key, "query_id")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->query_id.data = (const char *)value.data;
            out->query_id.len = value.len;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_admin_participant(const kb_admin_participant *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_permissions) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "id");
    kb_mp_write_i64(&w, (int64_t)v->id);
    if (v->has_permissions) {
        kb_mp_write_cstr(&w, "permissions");
        kb_mp_write_i64(&w, (int64_t)v->permissions);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_admin_participant(const uint8_t *body, size_t len, kb_admin_participant *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "id")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "permissions")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->permissions = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->permissions = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_permissions = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_attach(const kb_attach *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_event) { count++; }
    if (v->has_user_id) { count++; }
    if (v->has_user_ids) { count++; }
    if (v->has_title) { count++; }
    if (v->has_show_history) { count++; }
    if (v->has_chat_type) { count++; }
    if (v->has_start_payload) { count++; }
    if (v->has_crop) { count++; }
    if (v->has_photo_token) { count++; }
    if (v->has_base_url) { count++; }
    if (v->has_photo_id) { count++; }
    if (v->has_width) { count++; }
    if (v->has_height) { count++; }
    if (v->has_url) { count++; }
    if (v->has_preview_data) { count++; }
    if (v->has_video_id) { count++; }
    if (v->has_video_type) { count++; }
    if (v->has_token) { count++; }
    if (v->has_duration) { count++; }
    if (v->has_thumbnail) { count++; }
    if (v->has_thumbhash) { count++; }
    if (v->has_wave) { count++; }
    if (v->has_audio_id) { count++; }
    if (v->has_file_id) { count++; }
    if (v->has_sticker_id) { count++; }
    if (v->has_share_id) { count++; }
    if (v->has_description) { count++; }
    if (v->has_host) { count++; }
    if (v->has_image) { count++; }
    if (v->has_content_level) { count++; }
    if (v->has_conversation_id) { count++; }
    if (v->has_call_type) { count++; }
    if (v->has_hangup_type) { count++; }
    if (v->has_join_link) { count++; }
    if (v->has_vcf_body) { count++; }
    if (v->has_contact_id) { count++; }
    if (v->has_latitude) { count++; }
    if (v->has_longitude) { count++; }
    if (v->has_live_period) { count++; }
    if (v->has_zoom) { count++; }
    if (v->has_alt) { count++; }
    if (v->has_epu) { count++; }
    if (v->has_hdn) { count++; }
    if (v->has_spd) { count++; }
    if (v->has_keyboard) { count++; }
    if (v->has_callback_id) { count++; }
    if (v->has_buttons) { count++; }
    if (v->has_poll_id) { count++; }
    if (v->has_answers) { count++; }
    if (v->has_settings) { count++; }
    if (v->has_owner) { count++; }
    if (v->has_story_id) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "_type");
    kb_mp_write_str(&w, v->_type.data, v->_type.len);
    if (v->has_event) {
        kb_mp_write_cstr(&w, "event");
        kb_mp_write_str(&w, v->event.data, v->event.len);
    }
    if (v->has_user_id) {
        kb_mp_write_cstr(&w, "userId");
        kb_mp_write_i64(&w, (int64_t)v->user_id);
    }
    if (v->has_user_ids) {
        kb_mp_write_cstr(&w, "userIds");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->user_ids.data, v->user_ids.len);
    }
    if (v->has_title) {
        kb_mp_write_cstr(&w, "title");
        kb_mp_write_str(&w, v->title.data, v->title.len);
    }
    if (v->has_show_history) {
        kb_mp_write_cstr(&w, "showHistory");
        kb_mp_write_bool(&w, v->show_history);
    }
    if (v->has_chat_type) {
        kb_mp_write_cstr(&w, "chatType");
        kb_mp_write_str(&w, v->chat_type.data, v->chat_type.len);
    }
    if (v->has_start_payload) {
        kb_mp_write_cstr(&w, "startPayload");
        kb_mp_write_str(&w, v->start_payload.data, v->start_payload.len);
    }
    if (v->has_crop) {
        kb_mp_write_cstr(&w, "crop");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->crop.data, v->crop.len);
    }
    if (v->has_photo_token) {
        kb_mp_write_cstr(&w, "photoToken");
        kb_mp_write_str(&w, v->photo_token.data, v->photo_token.len);
    }
    if (v->has_base_url) {
        kb_mp_write_cstr(&w, "baseUrl");
        kb_mp_write_str(&w, v->base_url.data, v->base_url.len);
    }
    if (v->has_photo_id) {
        kb_mp_write_cstr(&w, "photoId");
        kb_mp_write_i64(&w, (int64_t)v->photo_id);
    }
    if (v->has_width) {
        kb_mp_write_cstr(&w, "width");
        kb_mp_write_i64(&w, (int64_t)v->width);
    }
    if (v->has_height) {
        kb_mp_write_cstr(&w, "height");
        kb_mp_write_i64(&w, (int64_t)v->height);
    }
    if (v->has_url) {
        kb_mp_write_cstr(&w, "url");
        kb_mp_write_str(&w, v->url.data, v->url.len);
    }
    if (v->has_preview_data) {
        kb_mp_write_cstr(&w, "previewData");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->preview_data.data, v->preview_data.len);
    }
    if (v->has_video_id) {
        kb_mp_write_cstr(&w, "videoId");
        kb_mp_write_i64(&w, (int64_t)v->video_id);
    }
    if (v->has_video_type) {
        kb_mp_write_cstr(&w, "videoType");
        kb_mp_write_i64(&w, (int64_t)v->video_type);
    }
    if (v->has_token) {
        kb_mp_write_cstr(&w, "token");
        kb_mp_write_str(&w, v->token.data, v->token.len);
    }
    if (v->has_duration) {
        kb_mp_write_cstr(&w, "duration");
        kb_mp_write_i64(&w, (int64_t)v->duration);
    }
    if (v->has_thumbnail) {
        kb_mp_write_cstr(&w, "thumbnail");
        kb_mp_write_str(&w, v->thumbnail.data, v->thumbnail.len);
    }
    if (v->has_thumbhash) {
        kb_mp_write_cstr(&w, "thumbhash");
        kb_mp_write_str(&w, v->thumbhash.data, v->thumbhash.len);
    }
    if (v->has_wave) {
        kb_mp_write_cstr(&w, "wave");
        kb_mp_write_str(&w, v->wave.data, v->wave.len);
    }
    if (v->has_audio_id) {
        kb_mp_write_cstr(&w, "audioId");
        kb_mp_write_i64(&w, (int64_t)v->audio_id);
    }
    if (v->has_file_id) {
        kb_mp_write_cstr(&w, "fileId");
        kb_mp_write_i64(&w, (int64_t)v->file_id);
    }
    if (v->has_sticker_id) {
        kb_mp_write_cstr(&w, "stickerId");
        kb_mp_write_i64(&w, (int64_t)v->sticker_id);
    }
    if (v->has_share_id) {
        kb_mp_write_cstr(&w, "shareId");
        kb_mp_write_i64(&w, (int64_t)v->share_id);
    }
    if (v->has_description) {
        kb_mp_write_cstr(&w, "description");
        kb_mp_write_str(&w, v->description.data, v->description.len);
    }
    if (v->has_host) {
        kb_mp_write_cstr(&w, "host");
        kb_mp_write_str(&w, v->host.data, v->host.len);
    }
    if (v->has_image) {
        kb_mp_write_cstr(&w, "image");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->image.data, v->image.len);
    }
    if (v->has_content_level) {
        kb_mp_write_cstr(&w, "contentLevel");
        kb_mp_write_bool(&w, v->content_level);
    }
    if (v->has_conversation_id) {
        kb_mp_write_cstr(&w, "conversationId");
        kb_mp_write_str(&w, v->conversation_id.data, v->conversation_id.len);
    }
    if (v->has_call_type) {
        kb_mp_write_cstr(&w, "callType");
        kb_mp_write_str(&w, v->call_type.data, v->call_type.len);
    }
    if (v->has_hangup_type) {
        kb_mp_write_cstr(&w, "hangupType");
        kb_mp_write_str(&w, v->hangup_type.data, v->hangup_type.len);
    }
    if (v->has_join_link) {
        kb_mp_write_cstr(&w, "joinLink");
        kb_mp_write_str(&w, v->join_link.data, v->join_link.len);
    }
    if (v->has_vcf_body) {
        kb_mp_write_cstr(&w, "vcfBody");
        kb_mp_write_str(&w, v->vcf_body.data, v->vcf_body.len);
    }
    if (v->has_contact_id) {
        kb_mp_write_cstr(&w, "contactId");
        kb_mp_write_i64(&w, (int64_t)v->contact_id);
    }
    if (v->has_latitude) {
        kb_mp_write_cstr(&w, "latitude");
        kb_mp_write_f64(&w, v->latitude);
    }
    if (v->has_longitude) {
        kb_mp_write_cstr(&w, "longitude");
        kb_mp_write_f64(&w, v->longitude);
    }
    if (v->has_live_period) {
        kb_mp_write_cstr(&w, "livePeriod");
        kb_mp_write_i64(&w, (int64_t)v->live_period);
    }
    if (v->has_zoom) {
        kb_mp_write_cstr(&w, "zoom");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->zoom.data, v->zoom.len);
    }
    if (v->has_alt) {
        kb_mp_write_cstr(&w, "alt");
        kb_mp_write_f64(&w, v->alt);
    }
    if (v->has_epu) {
        kb_mp_write_cstr(&w, "epu");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->epu.data, v->epu.len);
    }
    if (v->has_hdn) {
        kb_mp_write_cstr(&w, "hdn");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->hdn.data, v->hdn.len);
    }
    if (v->has_spd) {
        kb_mp_write_cstr(&w, "spd");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->spd.data, v->spd.len);
    }
    if (v->has_keyboard) {
        kb_mp_write_cstr(&w, "keyboard");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->keyboard.data, v->keyboard.len);
    }
    if (v->has_callback_id) {
        kb_mp_write_cstr(&w, "callbackId");
        kb_mp_write_str(&w, v->callback_id.data, v->callback_id.len);
    }
    if (v->has_buttons) {
        kb_mp_write_cstr(&w, "buttons");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->buttons.data, v->buttons.len);
    }
    if (v->has_poll_id) {
        kb_mp_write_cstr(&w, "pollId");
        kb_mp_write_i64(&w, (int64_t)v->poll_id);
    }
    if (v->has_answers) {
        kb_mp_write_cstr(&w, "answers");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->answers.data, v->answers.len);
    }
    if (v->has_settings) {
        kb_mp_write_cstr(&w, "settings");
        kb_mp_write_i64(&w, (int64_t)v->settings);
    }
    if (v->has_owner) {
        kb_mp_write_cstr(&w, "owner");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->owner.data, v->owner.len);
    }
    if (v->has_story_id) {
        kb_mp_write_cstr(&w, "storyId");
        kb_mp_write_i64(&w, (int64_t)v->story_id);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_attach(const uint8_t *body, size_t len, kb_attach *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "_type")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->_type.data = (const char *)value.data;
            out->_type.len = value.len;
        } else if (kb_mp_str_eq(&key, "event")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->event.data = (const char *)value.data;
            out->event.len = value.len;
            out->has_event = 1;
        } else if (kb_mp_str_eq(&key, "userId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->user_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->user_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_user_id = 1;
        } else if (kb_mp_str_eq(&key, "userIds")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->user_ids_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->user_ids.data = body + start;
            out->user_ids.len = kb_mp_reader_pos(&reader) - start;
            out->has_user_ids = 1;
        } else if (kb_mp_str_eq(&key, "title")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->title.data = (const char *)value.data;
            out->title.len = value.len;
            out->has_title = 1;
        } else if (kb_mp_str_eq(&key, "showHistory")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->show_history = value.boolean;
            out->has_show_history = 1;
        } else if (kb_mp_str_eq(&key, "chatType")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->chat_type.data = (const char *)value.data;
            out->chat_type.len = value.len;
            out->has_chat_type = 1;
        } else if (kb_mp_str_eq(&key, "startPayload")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->start_payload.data = (const char *)value.data;
            out->start_payload.len = value.len;
            out->has_start_payload = 1;
        } else if (kb_mp_str_eq(&key, "crop")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->crop.data = body + start;
            out->crop.len = kb_mp_reader_pos(&reader) - start;
            out->has_crop = 1;
        } else if (kb_mp_str_eq(&key, "photoToken")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->photo_token.data = (const char *)value.data;
            out->photo_token.len = value.len;
            out->has_photo_token = 1;
        } else if (kb_mp_str_eq(&key, "baseUrl")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->base_url.data = (const char *)value.data;
            out->base_url.len = value.len;
            out->has_base_url = 1;
        } else if (kb_mp_str_eq(&key, "photoId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->photo_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->photo_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_photo_id = 1;
        } else if (kb_mp_str_eq(&key, "width")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->width = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->width = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_width = 1;
        } else if (kb_mp_str_eq(&key, "height")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->height = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->height = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_height = 1;
        } else if (kb_mp_str_eq(&key, "url")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->url.data = (const char *)value.data;
            out->url.len = value.len;
            out->has_url = 1;
        } else if (kb_mp_str_eq(&key, "previewData")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->preview_data.data = body + start;
            out->preview_data.len = kb_mp_reader_pos(&reader) - start;
            out->has_preview_data = 1;
        } else if (kb_mp_str_eq(&key, "videoId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->video_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->video_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_video_id = 1;
        } else if (kb_mp_str_eq(&key, "videoType")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->video_type = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->video_type = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_video_type = 1;
        } else if (kb_mp_str_eq(&key, "token")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->token.data = (const char *)value.data;
            out->token.len = value.len;
            out->has_token = 1;
        } else if (kb_mp_str_eq(&key, "duration")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->duration = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->duration = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_duration = 1;
        } else if (kb_mp_str_eq(&key, "thumbnail")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->thumbnail.data = (const char *)value.data;
            out->thumbnail.len = value.len;
            out->has_thumbnail = 1;
        } else if (kb_mp_str_eq(&key, "thumbhash")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->thumbhash.data = (const char *)value.data;
            out->thumbhash.len = value.len;
            out->has_thumbhash = 1;
        } else if (kb_mp_str_eq(&key, "wave")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->wave.data = (const char *)value.data;
            out->wave.len = value.len;
            out->has_wave = 1;
        } else if (kb_mp_str_eq(&key, "audioId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->audio_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->audio_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_audio_id = 1;
        } else if (kb_mp_str_eq(&key, "fileId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->file_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->file_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_file_id = 1;
        } else if (kb_mp_str_eq(&key, "stickerId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->sticker_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->sticker_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_sticker_id = 1;
        } else if (kb_mp_str_eq(&key, "shareId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->share_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->share_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_share_id = 1;
        } else if (kb_mp_str_eq(&key, "description")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->description.data = (const char *)value.data;
            out->description.len = value.len;
            out->has_description = 1;
        } else if (kb_mp_str_eq(&key, "host")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->host.data = (const char *)value.data;
            out->host.len = value.len;
            out->has_host = 1;
        } else if (kb_mp_str_eq(&key, "image")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->image.data = body + start;
            out->image.len = kb_mp_reader_pos(&reader) - start;
            out->has_image = 1;
        } else if (kb_mp_str_eq(&key, "contentLevel")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->content_level = value.boolean;
            out->has_content_level = 1;
        } else if (kb_mp_str_eq(&key, "conversationId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->conversation_id.data = (const char *)value.data;
            out->conversation_id.len = value.len;
            out->has_conversation_id = 1;
        } else if (kb_mp_str_eq(&key, "callType")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->call_type.data = (const char *)value.data;
            out->call_type.len = value.len;
            out->has_call_type = 1;
        } else if (kb_mp_str_eq(&key, "hangupType")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->hangup_type.data = (const char *)value.data;
            out->hangup_type.len = value.len;
            out->has_hangup_type = 1;
        } else if (kb_mp_str_eq(&key, "joinLink")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->join_link.data = (const char *)value.data;
            out->join_link.len = value.len;
            out->has_join_link = 1;
        } else if (kb_mp_str_eq(&key, "vcfBody")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->vcf_body.data = (const char *)value.data;
            out->vcf_body.len = value.len;
            out->has_vcf_body = 1;
        } else if (kb_mp_str_eq(&key, "contactId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->contact_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->contact_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_contact_id = 1;
        } else if (kb_mp_str_eq(&key, "latitude")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_FLOAT) { return KB_ERR_INVALID; }
            out->latitude = value.f;
            out->has_latitude = 1;
        } else if (kb_mp_str_eq(&key, "longitude")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_FLOAT) { return KB_ERR_INVALID; }
            out->longitude = value.f;
            out->has_longitude = 1;
        } else if (kb_mp_str_eq(&key, "livePeriod")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->live_period = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->live_period = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_live_period = 1;
        } else if (kb_mp_str_eq(&key, "zoom")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->zoom.data = body + start;
            out->zoom.len = kb_mp_reader_pos(&reader) - start;
            out->has_zoom = 1;
        } else if (kb_mp_str_eq(&key, "alt")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_FLOAT) { return KB_ERR_INVALID; }
            out->alt = value.f;
            out->has_alt = 1;
        } else if (kb_mp_str_eq(&key, "epu")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->epu.data = body + start;
            out->epu.len = kb_mp_reader_pos(&reader) - start;
            out->has_epu = 1;
        } else if (kb_mp_str_eq(&key, "hdn")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->hdn.data = body + start;
            out->hdn.len = kb_mp_reader_pos(&reader) - start;
            out->has_hdn = 1;
        } else if (kb_mp_str_eq(&key, "spd")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->spd.data = body + start;
            out->spd.len = kb_mp_reader_pos(&reader) - start;
            out->has_spd = 1;
        } else if (kb_mp_str_eq(&key, "keyboard")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->keyboard.data = body + start;
            out->keyboard.len = kb_mp_reader_pos(&reader) - start;
            out->has_keyboard = 1;
        } else if (kb_mp_str_eq(&key, "callbackId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->callback_id.data = (const char *)value.data;
            out->callback_id.len = value.len;
            out->has_callback_id = 1;
        } else if (kb_mp_str_eq(&key, "buttons")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->buttons_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->buttons.data = body + start;
            out->buttons.len = kb_mp_reader_pos(&reader) - start;
            out->has_buttons = 1;
        } else if (kb_mp_str_eq(&key, "pollId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->poll_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->poll_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_poll_id = 1;
        } else if (kb_mp_str_eq(&key, "answers")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->answers_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->answers.data = body + start;
            out->answers.len = kb_mp_reader_pos(&reader) - start;
            out->has_answers = 1;
        } else if (kb_mp_str_eq(&key, "settings")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->settings = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->settings = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_settings = 1;
        } else if (kb_mp_str_eq(&key, "owner")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->owner.data = body + start;
            out->owner.len = kb_mp_reader_pos(&reader) - start;
            out->has_owner = 1;
        } else if (kb_mp_str_eq(&key, "storyId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->story_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->story_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_story_id = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_button(const kb_button *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_url) { count++; }
    if (v->has_intent) { count++; }
    if (v->has_payload) { count++; }
    if (v->has_is_quick) { count++; }
    if (v->has_contact_id) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "type");
    kb_mp_write_str(&w, v->type.data, v->type.len);
    kb_mp_write_cstr(&w, "text");
    kb_mp_write_str(&w, v->text.data, v->text.len);
    if (v->has_url) {
        kb_mp_write_cstr(&w, "url");
        kb_mp_write_str(&w, v->url.data, v->url.len);
    }
    if (v->has_intent) {
        kb_mp_write_cstr(&w, "intent");
        kb_mp_write_str(&w, v->intent.data, v->intent.len);
    }
    if (v->has_payload) {
        kb_mp_write_cstr(&w, "payload");
        kb_mp_write_str(&w, v->payload.data, v->payload.len);
    }
    if (v->has_is_quick) {
        kb_mp_write_cstr(&w, "isQuick");
        kb_mp_write_bool(&w, v->is_quick);
    }
    if (v->has_contact_id) {
        kb_mp_write_cstr(&w, "contactId");
        kb_mp_write_i64(&w, (int64_t)v->contact_id);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_button(const uint8_t *body, size_t len, kb_button *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "type")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->type.data = (const char *)value.data;
            out->type.len = value.len;
        } else if (kb_mp_str_eq(&key, "text")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->text.data = (const char *)value.data;
            out->text.len = value.len;
        } else if (kb_mp_str_eq(&key, "url")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->url.data = (const char *)value.data;
            out->url.len = value.len;
            out->has_url = 1;
        } else if (kb_mp_str_eq(&key, "intent")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->intent.data = (const char *)value.data;
            out->intent.len = value.len;
            out->has_intent = 1;
        } else if (kb_mp_str_eq(&key, "payload")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->payload.data = (const char *)value.data;
            out->payload.len = value.len;
            out->has_payload = 1;
        } else if (kb_mp_str_eq(&key, "isQuick")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->is_quick = value.boolean;
            out->has_is_quick = 1;
        } else if (kb_mp_str_eq(&key, "contactId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->contact_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->contact_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_contact_id = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_chat(const kb_chat *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 12;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_options) { count++; }
    if (v->has_title) { count++; }
    if (v->has_access) { count++; }
    if (v->has_participants_count) { count++; }
    if (v->has_messages_count) { count++; }
    if (v->has_reactions) { count++; }
    if (v->has_link) { count++; }
    if (v->has_admins) { count++; }
    if (v->has_admin_participants) { count++; }
    if (v->has_invited_by) { count++; }
    if (v->has_video_conversation) { count++; }
    if (v->has_new_messages) { count++; }
    if (v->has_cid) { count++; }
    if (v->has_restrictions) { count++; }
    if (v->has_prev_message_id) { count++; }
    if (v->has_has_bots) { count++; }
    if (v->has_description) { count++; }
    if (v->has_base_icon_url) { count++; }
    if (v->has_base_raw_icon_url) { count++; }
    if (v->has_participant_settings) { count++; }
    if (v->has_subject) { count++; }
    if (v->has_pinned_message) { count++; }
    if (v->has_pinned_message_id) { count++; }
    if (v->has_group_chat_info) { count++; }
    if (v->has_hide_pinned_message) { count++; }
    if (v->has_is_suspended) { count++; }
    if (v->has_marked_as_unread) { count++; }
    if (v->has_unread_pin) { count++; }
    if (v->has_unread_reply) { count++; }
    if (v->has_blocked_participants_count) { count++; }
    if (v->has_comments_blacklist_count) { count++; }
    if (v->has_pending_join_requests_count) { count++; }
    if (v->has_join_request_time) { count++; }
    if (v->has_last_mention_message_id) { count++; }
    if (v->has_last_reacted_message_id) { count++; }
    if (v->has_last_reaction) { count++; }
    if (v->has_live_location_message_ids) { count++; }
    if (v->has_live_stream_update_time) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "id");
    kb_mp_write_i64(&w, (int64_t)v->id);
    kb_mp_write_cstr(&w, "type");
    kb_mp_write_str(&w, v->type.data, v->type.len);
    kb_mp_write_cstr(&w, "status");
    kb_mp_write_str(&w, v->status.data, v->status.len);
    kb_mp_write_cstr(&w, "owner");
    kb_mp_write_i64(&w, (int64_t)v->owner);
    kb_mp_write_cstr(&w, "participants");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->participants.data, v->participants.len);
    kb_mp_write_cstr(&w, "lastMessage");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->last_message.data, v->last_message.len);
    kb_mp_write_cstr(&w, "lastEventTime");
    kb_mp_write_i64(&w, (int64_t)v->last_event_time);
    kb_mp_write_cstr(&w, "lastDelayedUpdateTime");
    kb_mp_write_i64(&w, (int64_t)v->last_delayed_update_time);
    kb_mp_write_cstr(&w, "lastFireDelayedErrorTime");
    kb_mp_write_i64(&w, (int64_t)v->last_fire_delayed_error_time);
    kb_mp_write_cstr(&w, "created");
    kb_mp_write_i64(&w, (int64_t)v->created);
    kb_mp_write_cstr(&w, "joinTime");
    kb_mp_write_i64(&w, (int64_t)v->join_time);
    kb_mp_write_cstr(&w, "modified");
    kb_mp_write_i64(&w, (int64_t)v->modified);
    if (v->has_options) {
        kb_mp_write_cstr(&w, "options");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->options.data, v->options.len);
    }
    if (v->has_title) {
        kb_mp_write_cstr(&w, "title");
        kb_mp_write_str(&w, v->title.data, v->title.len);
    }
    if (v->has_access) {
        kb_mp_write_cstr(&w, "access");
        kb_mp_write_str(&w, v->access.data, v->access.len);
    }
    if (v->has_participants_count) {
        kb_mp_write_cstr(&w, "participantsCount");
        kb_mp_write_i64(&w, (int64_t)v->participants_count);
    }
    if (v->has_messages_count) {
        kb_mp_write_cstr(&w, "messagesCount");
        kb_mp_write_i64(&w, (int64_t)v->messages_count);
    }
    if (v->has_reactions) {
        kb_mp_write_cstr(&w, "reactions");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->reactions.data, v->reactions.len);
    }
    if (v->has_link) {
        kb_mp_write_cstr(&w, "link");
        kb_mp_write_str(&w, v->link.data, v->link.len);
    }
    if (v->has_admins) {
        kb_mp_write_cstr(&w, "admins");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->admins.data, v->admins.len);
    }
    if (v->has_admin_participants) {
        kb_mp_write_cstr(&w, "adminParticipants");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->admin_participants.data, v->admin_participants.len);
    }
    if (v->has_invited_by) {
        kb_mp_write_cstr(&w, "invitedBy");
        kb_mp_write_i64(&w, (int64_t)v->invited_by);
    }
    if (v->has_video_conversation) {
        kb_mp_write_cstr(&w, "videoConversation");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->video_conversation.data, v->video_conversation.len);
    }
    if (v->has_new_messages) {
        kb_mp_write_cstr(&w, "newMessages");
        kb_mp_write_i64(&w, (int64_t)v->new_messages);
    }
    if (v->has_cid) {
        kb_mp_write_cstr(&w, "cid");
        kb_mp_write_i64(&w, (int64_t)v->cid);
    }
    if (v->has_restrictions) {
        kb_mp_write_cstr(&w, "restrictions");
        kb_mp_write_i64(&w, (int64_t)v->restrictions);
    }
    if (v->has_prev_message_id) {
        kb_mp_write_cstr(&w, "prevMessageId");
        kb_mp_write_i64(&w, (int64_t)v->prev_message_id);
    }
    if (v->has_has_bots) {
        kb_mp_write_cstr(&w, "hasBots");
        kb_mp_write_bool(&w, v->has_bots);
    }
    if (v->has_description) {
        kb_mp_write_cstr(&w, "description");
        kb_mp_write_str(&w, v->description.data, v->description.len);
    }
    if (v->has_base_icon_url) {
        kb_mp_write_cstr(&w, "baseIconUrl");
        kb_mp_write_str(&w, v->base_icon_url.data, v->base_icon_url.len);
    }
    if (v->has_base_raw_icon_url) {
        kb_mp_write_cstr(&w, "baseRawIconUrl");
        kb_mp_write_str(&w, v->base_raw_icon_url.data, v->base_raw_icon_url.len);
    }
    if (v->has_participant_settings) {
        kb_mp_write_cstr(&w, "participantSettings");
        kb_mp_write_i64(&w, (int64_t)v->participant_settings);
    }
    if (v->has_subject) {
        kb_mp_write_cstr(&w, "subject");
        kb_mp_write_str(&w, v->subject.data, v->subject.len);
    }
    if (v->has_pinned_message) {
        kb_mp_write_cstr(&w, "pinnedMessage");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->pinned_message.data, v->pinned_message.len);
    }
    if (v->has_pinned_message_id) {
        kb_mp_write_cstr(&w, "pinnedMessageId");
        kb_mp_write_i64(&w, (int64_t)v->pinned_message_id);
    }
    if (v->has_group_chat_info) {
        kb_mp_write_cstr(&w, "groupChatInfo");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->group_chat_info.data, v->group_chat_info.len);
    }
    if (v->has_hide_pinned_message) {
        kb_mp_write_cstr(&w, "hidePinnedMessage");
        kb_mp_write_bool(&w, v->hide_pinned_message);
    }
    if (v->has_is_suspended) {
        kb_mp_write_cstr(&w, "isSuspended");
        kb_mp_write_bool(&w, v->is_suspended);
    }
    if (v->has_marked_as_unread) {
        kb_mp_write_cstr(&w, "markedAsUnread");
        kb_mp_write_bool(&w, v->marked_as_unread);
    }
    if (v->has_unread_pin) {
        kb_mp_write_cstr(&w, "unreadPin");
        kb_mp_write_bool(&w, v->unread_pin);
    }
    if (v->has_unread_reply) {
        kb_mp_write_cstr(&w, "unreadReply");
        kb_mp_write_bool(&w, v->unread_reply);
    }
    if (v->has_blocked_participants_count) {
        kb_mp_write_cstr(&w, "blockedParticipantsCount");
        kb_mp_write_i64(&w, (int64_t)v->blocked_participants_count);
    }
    if (v->has_comments_blacklist_count) {
        kb_mp_write_cstr(&w, "commentsBlacklistCount");
        kb_mp_write_i64(&w, (int64_t)v->comments_blacklist_count);
    }
    if (v->has_pending_join_requests_count) {
        kb_mp_write_cstr(&w, "pendingJoinRequestsCount");
        kb_mp_write_i64(&w, (int64_t)v->pending_join_requests_count);
    }
    if (v->has_join_request_time) {
        kb_mp_write_cstr(&w, "joinRequestTime");
        kb_mp_write_i64(&w, (int64_t)v->join_request_time);
    }
    if (v->has_last_mention_message_id) {
        kb_mp_write_cstr(&w, "lastMentionMessageId");
        kb_mp_write_i64(&w, (int64_t)v->last_mention_message_id);
    }
    if (v->has_last_reacted_message_id) {
        kb_mp_write_cstr(&w, "lastReactedMessageId");
        kb_mp_write_i64(&w, (int64_t)v->last_reacted_message_id);
    }
    if (v->has_last_reaction) {
        kb_mp_write_cstr(&w, "lastReaction");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->last_reaction.data, v->last_reaction.len);
    }
    if (v->has_live_location_message_ids) {
        kb_mp_write_cstr(&w, "liveLocationMessageIds");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->live_location_message_ids.data, v->live_location_message_ids.len);
    }
    if (v->has_live_stream_update_time) {
        kb_mp_write_cstr(&w, "liveStreamUpdateTime");
        kb_mp_write_i64(&w, (int64_t)v->live_stream_update_time);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_chat(const uint8_t *body, size_t len, kb_chat *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "id")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "type")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->type.data = (const char *)value.data;
            out->type.len = value.len;
        } else if (kb_mp_str_eq(&key, "status")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->status.data = (const char *)value.data;
            out->status.len = value.len;
        } else if (kb_mp_str_eq(&key, "owner")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->owner = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->owner = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "participants")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->participants_len = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->participants.data = body + start;
            out->participants.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "lastMessage")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->last_message.data = body + start;
            out->last_message.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "lastEventTime")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->last_event_time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->last_event_time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "lastDelayedUpdateTime")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->last_delayed_update_time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->last_delayed_update_time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "lastFireDelayedErrorTime")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->last_fire_delayed_error_time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->last_fire_delayed_error_time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "created")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->created = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->created = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "joinTime")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->join_time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->join_time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "modified")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->modified = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->modified = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "options")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->options_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->options.data = body + start;
            out->options.len = kb_mp_reader_pos(&reader) - start;
            out->has_options = 1;
        } else if (kb_mp_str_eq(&key, "title")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->title.data = (const char *)value.data;
            out->title.len = value.len;
            out->has_title = 1;
        } else if (kb_mp_str_eq(&key, "access")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->access.data = (const char *)value.data;
            out->access.len = value.len;
            out->has_access = 1;
        } else if (kb_mp_str_eq(&key, "participantsCount")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->participants_count = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->participants_count = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_participants_count = 1;
        } else if (kb_mp_str_eq(&key, "messagesCount")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->messages_count = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->messages_count = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_messages_count = 1;
        } else if (kb_mp_str_eq(&key, "reactions")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->reactions.data = body + start;
            out->reactions.len = kb_mp_reader_pos(&reader) - start;
            out->has_reactions = 1;
        } else if (kb_mp_str_eq(&key, "link")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->link.data = (const char *)value.data;
            out->link.len = value.len;
            out->has_link = 1;
        } else if (kb_mp_str_eq(&key, "admins")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->admins_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->admins.data = body + start;
            out->admins.len = kb_mp_reader_pos(&reader) - start;
            out->has_admins = 1;
        } else if (kb_mp_str_eq(&key, "adminParticipants")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->admin_participants_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->admin_participants.data = body + start;
            out->admin_participants.len = kb_mp_reader_pos(&reader) - start;
            out->has_admin_participants = 1;
        } else if (kb_mp_str_eq(&key, "invitedBy")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->invited_by = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->invited_by = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_invited_by = 1;
        } else if (kb_mp_str_eq(&key, "videoConversation")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->video_conversation.data = body + start;
            out->video_conversation.len = kb_mp_reader_pos(&reader) - start;
            out->has_video_conversation = 1;
        } else if (kb_mp_str_eq(&key, "newMessages")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->new_messages = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->new_messages = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_new_messages = 1;
        } else if (kb_mp_str_eq(&key, "cid")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->cid = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->cid = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_cid = 1;
        } else if (kb_mp_str_eq(&key, "restrictions")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->restrictions = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->restrictions = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_restrictions = 1;
        } else if (kb_mp_str_eq(&key, "prevMessageId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->prev_message_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->prev_message_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_prev_message_id = 1;
        } else if (kb_mp_str_eq(&key, "hasBots")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->has_bots = value.boolean;
            out->has_has_bots = 1;
        } else if (kb_mp_str_eq(&key, "description")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->description.data = (const char *)value.data;
            out->description.len = value.len;
            out->has_description = 1;
        } else if (kb_mp_str_eq(&key, "baseIconUrl")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->base_icon_url.data = (const char *)value.data;
            out->base_icon_url.len = value.len;
            out->has_base_icon_url = 1;
        } else if (kb_mp_str_eq(&key, "baseRawIconUrl")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->base_raw_icon_url.data = (const char *)value.data;
            out->base_raw_icon_url.len = value.len;
            out->has_base_raw_icon_url = 1;
        } else if (kb_mp_str_eq(&key, "participantSettings")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->participant_settings = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->participant_settings = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_participant_settings = 1;
        } else if (kb_mp_str_eq(&key, "subject")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->subject.data = (const char *)value.data;
            out->subject.len = value.len;
            out->has_subject = 1;
        } else if (kb_mp_str_eq(&key, "pinnedMessage")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->pinned_message.data = body + start;
            out->pinned_message.len = kb_mp_reader_pos(&reader) - start;
            out->has_pinned_message = 1;
        } else if (kb_mp_str_eq(&key, "pinnedMessageId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->pinned_message_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->pinned_message_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_pinned_message_id = 1;
        } else if (kb_mp_str_eq(&key, "groupChatInfo")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->group_chat_info.data = body + start;
            out->group_chat_info.len = kb_mp_reader_pos(&reader) - start;
            out->has_group_chat_info = 1;
        } else if (kb_mp_str_eq(&key, "hidePinnedMessage")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->hide_pinned_message = value.boolean;
            out->has_hide_pinned_message = 1;
        } else if (kb_mp_str_eq(&key, "isSuspended")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->is_suspended = value.boolean;
            out->has_is_suspended = 1;
        } else if (kb_mp_str_eq(&key, "markedAsUnread")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->marked_as_unread = value.boolean;
            out->has_marked_as_unread = 1;
        } else if (kb_mp_str_eq(&key, "unreadPin")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->unread_pin = value.boolean;
            out->has_unread_pin = 1;
        } else if (kb_mp_str_eq(&key, "unreadReply")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->unread_reply = value.boolean;
            out->has_unread_reply = 1;
        } else if (kb_mp_str_eq(&key, "blockedParticipantsCount")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->blocked_participants_count = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->blocked_participants_count = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_blocked_participants_count = 1;
        } else if (kb_mp_str_eq(&key, "commentsBlacklistCount")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->comments_blacklist_count = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->comments_blacklist_count = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_comments_blacklist_count = 1;
        } else if (kb_mp_str_eq(&key, "pendingJoinRequestsCount")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->pending_join_requests_count = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->pending_join_requests_count = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_pending_join_requests_count = 1;
        } else if (kb_mp_str_eq(&key, "joinRequestTime")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->join_request_time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->join_request_time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_join_request_time = 1;
        } else if (kb_mp_str_eq(&key, "lastMentionMessageId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->last_mention_message_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->last_mention_message_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_last_mention_message_id = 1;
        } else if (kb_mp_str_eq(&key, "lastReactedMessageId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->last_reacted_message_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->last_reacted_message_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_last_reacted_message_id = 1;
        } else if (kb_mp_str_eq(&key, "lastReaction")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->last_reaction.data = body + start;
            out->last_reaction.len = kb_mp_reader_pos(&reader) - start;
            out->has_last_reaction = 1;
        } else if (kb_mp_str_eq(&key, "liveLocationMessageIds")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->live_location_message_ids_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->live_location_message_ids.data = body + start;
            out->live_location_message_ids.len = kb_mp_reader_pos(&reader) - start;
            out->has_live_location_message_ids = 1;
        } else if (kb_mp_str_eq(&key, "liveStreamUpdateTime")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->live_stream_update_time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->live_stream_update_time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_live_stream_update_time = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_chat_reactions(const kb_chat_reactions *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "isActive");
    kb_mp_write_bool(&w, v->is_active);
    kb_mp_write_cstr(&w, "updateTime");
    kb_mp_write_i64(&w, (int64_t)v->update_time);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_chat_reactions(const uint8_t *body, size_t len, kb_chat_reactions *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "isActive")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->is_active = value.boolean;
        } else if (kb_mp_str_eq(&key, "updateTime")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->update_time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->update_time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_contact(const kb_contact *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 11;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_phone) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "id");
    kb_mp_write_i64(&w, (int64_t)v->id);
    kb_mp_write_cstr(&w, "updateTime");
    kb_mp_write_i64(&w, (int64_t)v->update_time);
    kb_mp_write_cstr(&w, "registrationTime");
    kb_mp_write_i64(&w, (int64_t)v->registration_time);
    kb_mp_write_cstr(&w, "baseUrl");
    kb_mp_write_str(&w, v->base_url.data, v->base_url.len);
    kb_mp_write_cstr(&w, "baseRawUrl");
    kb_mp_write_str(&w, v->base_raw_url.data, v->base_raw_url.len);
    kb_mp_write_cstr(&w, "photoId");
    kb_mp_write_i64(&w, (int64_t)v->photo_id);
    if (v->has_phone) {
        kb_mp_write_cstr(&w, "phone");
        kb_mp_write_i64(&w, (int64_t)v->phone);
    }
    kb_mp_write_cstr(&w, "names");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->names.data, v->names.len);
    kb_mp_write_cstr(&w, "options");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->options.data, v->options.len);
    kb_mp_write_cstr(&w, "description");
    kb_mp_write_str(&w, v->description.data, v->description.len);
    kb_mp_write_cstr(&w, "accountStatus");
    kb_mp_write_i64(&w, (int64_t)v->account_status);
    kb_mp_write_cstr(&w, "country");
    kb_mp_write_str(&w, v->country.data, v->country.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_contact(const uint8_t *body, size_t len, kb_contact *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "id")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "updateTime")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->update_time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->update_time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "registrationTime")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->registration_time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->registration_time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "baseUrl")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->base_url.data = (const char *)value.data;
            out->base_url.len = value.len;
        } else if (kb_mp_str_eq(&key, "baseRawUrl")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->base_raw_url.data = (const char *)value.data;
            out->base_raw_url.len = value.len;
        } else if (kb_mp_str_eq(&key, "photoId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->photo_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->photo_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "phone")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->phone = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->phone = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_phone = 1;
        } else if (kb_mp_str_eq(&key, "names")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->names_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->names.data = body + start;
            out->names.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "options")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->options_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->options.data = body + start;
            out->options.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "description")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->description.data = (const char *)value.data;
            out->description.len = value.len;
        } else if (kb_mp_str_eq(&key, "accountStatus")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->account_status = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->account_status = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "country")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->country.data = (const char *)value.data;
            out->country.len = value.len;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_delayed_attributes(const kb_delayed_attributes *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_time_to_fire) { count++; }
    if (v->has_notify_sender) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    if (v->has_time_to_fire) {
        kb_mp_write_cstr(&w, "timeToFire");
        kb_mp_write_i64(&w, (int64_t)v->time_to_fire);
    }
    if (v->has_notify_sender) {
        kb_mp_write_cstr(&w, "notifySender");
        kb_mp_write_bool(&w, v->notify_sender);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_delayed_attributes(const uint8_t *body, size_t len, kb_delayed_attributes *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "timeToFire")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->time_to_fire = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->time_to_fire = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_time_to_fire = 1;
        } else if (kb_mp_str_eq(&key, "notifySender")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BOOL) { return KB_ERR_INVALID; }
            out->notify_sender = value.boolean;
            out->has_notify_sender = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_element(const kb_element *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 3;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "type");
    kb_mp_write_str(&w, v->type.data, v->type.len);
    kb_mp_write_cstr(&w, "from");
    kb_mp_write_i64(&w, (int64_t)v->from);
    kb_mp_write_cstr(&w, "length");
    kb_mp_write_i64(&w, (int64_t)v->length);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_element(const uint8_t *body, size_t len, kb_element *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "type")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->type.data = (const char *)value.data;
            out->type.len = value.len;
        } else if (kb_mp_str_eq(&key, "from")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->from = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->from = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "length")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->length = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->length = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_error_envelope(const kb_error_envelope *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 4;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "error");
    kb_mp_write_str(&w, v->error.data, v->error.len);
    kb_mp_write_cstr(&w, "message");
    kb_mp_write_str(&w, v->message.data, v->message.len);
    kb_mp_write_cstr(&w, "localizedMessage");
    kb_mp_write_str(&w, v->localized_message.data, v->localized_message.len);
    kb_mp_write_cstr(&w, "title");
    kb_mp_write_str(&w, v->title.data, v->title.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_error_envelope(const uint8_t *body, size_t len, kb_error_envelope *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "error")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->error.data = (const char *)value.data;
            out->error.len = value.len;
        } else if (kb_mp_str_eq(&key, "message")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->message.data = (const char *)value.data;
            out->message.len = value.len;
        } else if (kb_mp_str_eq(&key, "localizedMessage")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->localized_message.data = (const char *)value.data;
            out->localized_message.len = value.len;
        } else if (kb_mp_str_eq(&key, "title")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->title.data = (const char *)value.data;
            out->title.len = value.len;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_keyboard(const kb_keyboard *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "buttons");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->buttons.data, v->buttons.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_keyboard(const uint8_t *body, size_t len, kb_keyboard *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "buttons")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->buttons_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->buttons.data = body + start;
            out->buttons.len = kb_mp_reader_pos(&reader) - start;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_login_exp(const kb_login_exp *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "chatsCountGroups");
    kb_mp_write_bin(&w, v->chats_count_groups.data, v->chats_count_groups.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_login_exp(const uint8_t *body, size_t len, kb_login_exp *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "chatsCountGroups")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_BIN) { return KB_ERR_INVALID; }
            out->chats_count_groups.data = value.data;
            out->chats_count_groups.len = value.len;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_message(const kb_message *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 6;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_cid) { count++; }
    if (v->has_elements) { count++; }
    if (v->has_options) { count++; }
    if (v->has_update_time) { count++; }
    if (v->has_status) { count++; }
    if (v->has_reaction_info) { count++; }
    if (v->has_link) { count++; }
    if (v->has_delayed_attributes) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "id");
    kb_mp_write_i64(&w, (int64_t)v->id);
    kb_mp_write_cstr(&w, "time");
    kb_mp_write_i64(&w, (int64_t)v->time);
    kb_mp_write_cstr(&w, "type");
    kb_mp_write_str(&w, v->type.data, v->type.len);
    kb_mp_write_cstr(&w, "sender");
    kb_mp_write_i64(&w, (int64_t)v->sender);
    kb_mp_write_cstr(&w, "text");
    kb_mp_write_str(&w, v->text.data, v->text.len);
    if (v->has_cid) {
        kb_mp_write_cstr(&w, "cid");
        kb_mp_write_i64(&w, (int64_t)v->cid);
    }
    kb_mp_write_cstr(&w, "attaches");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->attaches.data, v->attaches.len);
    if (v->has_elements) {
        kb_mp_write_cstr(&w, "elements");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->elements.data, v->elements.len);
    }
    if (v->has_options) {
        kb_mp_write_cstr(&w, "options");
        kb_mp_write_i64(&w, (int64_t)v->options);
    }
    if (v->has_update_time) {
        kb_mp_write_cstr(&w, "updateTime");
        kb_mp_write_i64(&w, (int64_t)v->update_time);
    }
    if (v->has_status) {
        kb_mp_write_cstr(&w, "status");
        kb_mp_write_str(&w, v->status.data, v->status.len);
    }
    if (v->has_reaction_info) {
        kb_mp_write_cstr(&w, "reactionInfo");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->reaction_info.data, v->reaction_info.len);
    }
    if (v->has_link) {
        kb_mp_write_cstr(&w, "link");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->link.data, v->link.len);
    }
    if (v->has_delayed_attributes) {
        kb_mp_write_cstr(&w, "delayedAttributes");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->delayed_attributes.data, v->delayed_attributes.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_message(const uint8_t *body, size_t len, kb_message *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "id")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "time")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "type")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->type.data = (const char *)value.data;
            out->type.len = value.len;
        } else if (kb_mp_str_eq(&key, "sender")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->sender = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->sender = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "text")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->text.data = (const char *)value.data;
            out->text.len = value.len;
        } else if (kb_mp_str_eq(&key, "cid")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->cid = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->cid = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_cid = 1;
        } else if (kb_mp_str_eq(&key, "attaches")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->attaches_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->attaches.data = body + start;
            out->attaches.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "elements")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->elements_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->elements.data = body + start;
            out->elements.len = kb_mp_reader_pos(&reader) - start;
            out->has_elements = 1;
        } else if (kb_mp_str_eq(&key, "options")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->options = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->options = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_options = 1;
        } else if (kb_mp_str_eq(&key, "updateTime")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->update_time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->update_time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_update_time = 1;
        } else if (kb_mp_str_eq(&key, "status")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->status.data = (const char *)value.data;
            out->status.len = value.len;
            out->has_status = 1;
        } else if (kb_mp_str_eq(&key, "reactionInfo")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->reaction_info.data = body + start;
            out->reaction_info.len = kb_mp_reader_pos(&reader) - start;
            out->has_reaction_info = 1;
        } else if (kb_mp_str_eq(&key, "link")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->link.data = body + start;
            out->link.len = kb_mp_reader_pos(&reader) - start;
            out->has_link = 1;
        } else if (kb_mp_str_eq(&key, "delayedAttributes")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->delayed_attributes.data = body + start;
            out->delayed_attributes.len = kb_mp_reader_pos(&reader) - start;
            out->has_delayed_attributes = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_message_element(const kb_message_element *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_from) { count++; }
    if (v->has_attributes) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "type");
    kb_mp_write_str(&w, v->type.data, v->type.len);
    kb_mp_write_cstr(&w, "length");
    kb_mp_write_i64(&w, (int64_t)v->length);
    if (v->has_from) {
        kb_mp_write_cstr(&w, "from");
        kb_mp_write_i64(&w, (int64_t)v->from);
    }
    if (v->has_attributes) {
        kb_mp_write_cstr(&w, "attributes");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->attributes.data, v->attributes.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_message_element(const uint8_t *body, size_t len, kb_message_element *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "type")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->type.data = (const char *)value.data;
            out->type.len = value.len;
        } else if (kb_mp_str_eq(&key, "length")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->length = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->length = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "from")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->from = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->from = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_from = 1;
        } else if (kb_mp_str_eq(&key, "attributes")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->attributes.data = body + start;
            out->attributes.len = kb_mp_reader_pos(&reader) - start;
            out->has_attributes = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_message_link(const kb_message_link *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_message) { count++; }
    if (v->has_chat_id) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "type");
    kb_mp_write_str(&w, v->type.data, v->type.len);
    if (v->has_message) {
        kb_mp_write_cstr(&w, "message");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->message.data, v->message.len);
    }
    if (v->has_chat_id) {
        kb_mp_write_cstr(&w, "chatId");
        kb_mp_write_i64(&w, (int64_t)v->chat_id);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_message_link(const uint8_t *body, size_t len, kb_message_link *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "type")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->type.data = (const char *)value.data;
            out->type.len = value.len;
        } else if (kb_mp_str_eq(&key, "message")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->message.data = body + start;
            out->message.len = kb_mp_reader_pos(&reader) - start;
            out->has_message = 1;
        } else if (kb_mp_str_eq(&key, "chatId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->chat_id = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->chat_id = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_chat_id = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_name(const kb_name *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 4;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "name");
    kb_mp_write_str(&w, v->name.data, v->name.len);
    kb_mp_write_cstr(&w, "firstName");
    kb_mp_write_str(&w, v->first_name.data, v->first_name.len);
    kb_mp_write_cstr(&w, "lastName");
    kb_mp_write_str(&w, v->last_name.data, v->last_name.len);
    kb_mp_write_cstr(&w, "type");
    kb_mp_write_str(&w, v->type.data, v->type.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_name(const uint8_t *body, size_t len, kb_name *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "name")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->name.data = (const char *)value.data;
            out->name.len = value.len;
        } else if (kb_mp_str_eq(&key, "firstName")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->first_name.data = (const char *)value.data;
            out->first_name.len = value.len;
        } else if (kb_mp_str_eq(&key, "lastName")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->last_name.data = (const char *)value.data;
            out->last_name.len = value.len;
        } else if (kb_mp_str_eq(&key, "type")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->type.data = (const char *)value.data;
            out->type.len = value.len;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_outgoing_message(const kb_outgoing_message *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 4;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_link) { count++; }
    if (v->has_delayed_attributes) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "text");
    kb_mp_write_str(&w, v->text.data, v->text.len);
    kb_mp_write_cstr(&w, "cid");
    kb_mp_write_i64(&w, (int64_t)v->cid);
    kb_mp_write_cstr(&w, "elements");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->elements.data, v->elements.len);
    kb_mp_write_cstr(&w, "attaches");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->attaches.data, v->attaches.len);
    if (v->has_link) {
        kb_mp_write_cstr(&w, "link");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->link.data, v->link.len);
    }
    if (v->has_delayed_attributes) {
        kb_mp_write_cstr(&w, "delayedAttributes");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->delayed_attributes.data, v->delayed_attributes.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_outgoing_message(const uint8_t *body, size_t len, kb_outgoing_message *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "text")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->text.data = (const char *)value.data;
            out->text.len = value.len;
        } else if (kb_mp_str_eq(&key, "cid")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->cid = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->cid = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else if (kb_mp_str_eq(&key, "elements")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->elements_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->elements.data = body + start;
            out->elements.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "attaches")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->attaches_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->attaches.data = body + start;
            out->attaches.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "link")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->link.data = body + start;
            out->link.len = kb_mp_reader_pos(&reader) - start;
            out->has_link = 1;
        } else if (kb_mp_str_eq(&key, "delayedAttributes")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->delayed_attributes.data = body + start;
            out->delayed_attributes.len = kb_mp_reader_pos(&reader) - start;
            out->has_delayed_attributes = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_password_challenge(const kb_password_challenge *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 1;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_hint) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "trackId");
    kb_mp_write_str(&w, v->track_id.data, v->track_id.len);
    if (v->has_hint) {
        kb_mp_write_cstr(&w, "hint");
        kb_mp_write_str(&w, v->hint.data, v->hint.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_password_challenge(const uint8_t *body, size_t len, kb_password_challenge *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "trackId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->track_id.data = (const char *)value.data;
            out->track_id.len = value.len;
        } else if (kb_mp_str_eq(&key, "hint")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->hint.data = (const char *)value.data;
            out->hint.len = value.len;
            out->has_hint = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_preview_data(const kb_preview_data *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_preview_data(const uint8_t *body, size_t len, kb_preview_data *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_profile(const kb_profile *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_contact) { count++; }
    if (v->has_profile_options) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    if (v->has_contact) {
        kb_mp_write_cstr(&w, "contact");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->contact.data, v->contact.len);
    }
    if (v->has_profile_options) {
        kb_mp_write_cstr(&w, "profileOptions");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->profile_options.data, v->profile_options.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_profile(const uint8_t *body, size_t len, kb_profile *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "contact")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->contact.data = body + start;
            out->contact.len = kb_mp_reader_pos(&reader) - start;
            out->has_contact = 1;
        } else if (kb_mp_str_eq(&key, "profileOptions")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->profile_options.data = body + start;
            out->profile_options.len = kb_mp_reader_pos(&reader) - start;
            out->has_profile_options = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_reaction_counter(const kb_reaction_counter *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 2;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "reaction");
    kb_mp_write_str(&w, v->reaction.data, v->reaction.len);
    kb_mp_write_cstr(&w, "count");
    kb_mp_write_i64(&w, (int64_t)v->count);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_reaction_counter(const uint8_t *body, size_t len, kb_reaction_counter *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "reaction")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->reaction.data = (const char *)value.data;
            out->reaction.len = value.len;
        } else if (kb_mp_str_eq(&key, "count")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->count = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->count = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_reaction_info(const kb_reaction_info *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_counters) { count++; }
    if (v->has_total_count) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    if (v->has_counters) {
        kb_mp_write_cstr(&w, "counters");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->counters.data, v->counters.len);
    }
    if (v->has_total_count) {
        kb_mp_write_cstr(&w, "totalCount");
        kb_mp_write_i64(&w, (int64_t)v->total_count);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_reaction_info(const uint8_t *body, size_t len, kb_reaction_info *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "counters")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->counters_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->counters.data = body + start;
            out->counters.len = kb_mp_reader_pos(&reader) - start;
            out->has_counters = 1;
        } else if (kb_mp_str_eq(&key, "totalCount")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->total_count = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->total_count = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_total_count = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_sent_message(const kb_sent_message *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_id) { count++; }
    if (v->has_cid) { count++; }
    if (v->has_sender) { count++; }
    if (v->has_text) { count++; }
    if (v->has_time) { count++; }
    if (v->has_status) { count++; }
    if (v->has_elements) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    if (v->has_id) {
        kb_mp_write_cstr(&w, "id");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->id.data, v->id.len);
    }
    if (v->has_cid) {
        kb_mp_write_cstr(&w, "cid");
        kb_mp_write_i64(&w, (int64_t)v->cid);
    }
    if (v->has_sender) {
        kb_mp_write_cstr(&w, "sender");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->sender.data, v->sender.len);
    }
    if (v->has_text) {
        kb_mp_write_cstr(&w, "text");
        kb_mp_write_str(&w, v->text.data, v->text.len);
    }
    if (v->has_time) {
        kb_mp_write_cstr(&w, "time");
        kb_mp_write_i64(&w, (int64_t)v->time);
    }
    if (v->has_status) {
        kb_mp_write_cstr(&w, "status");
        kb_mp_write_str(&w, v->status.data, v->status.len);
    }
    if (v->has_elements) {
        kb_mp_write_cstr(&w, "elements");
        /* готовый msgpack вставляется как есть */
        kb_mp_write_raw(&w, v->elements.data, v->elements.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_sent_message(const uint8_t *body, size_t len, kb_sent_message *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "id")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->id.data = body + start;
            out->id.len = kb_mp_reader_pos(&reader) - start;
            out->has_id = 1;
        } else if (kb_mp_str_eq(&key, "cid")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->cid = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->cid = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_cid = 1;
        } else if (kb_mp_str_eq(&key, "sender")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->sender.data = body + start;
            out->sender.len = kb_mp_reader_pos(&reader) - start;
            out->has_sender = 1;
        } else if (kb_mp_str_eq(&key, "text")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->text.data = (const char *)value.data;
            out->text.len = value.len;
            out->has_text = 1;
        } else if (kb_mp_str_eq(&key, "time")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type == KB_MP_INT) {
                out->time = (int64_t)value.i;
            } else if (value.type == KB_MP_UINT) {
                out->time = (int64_t)value.u;
            } else { return KB_ERR_INVALID; }
            out->has_time = 1;
        } else if (kb_mp_str_eq(&key, "status")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->status.data = (const char *)value.data;
            out->status.len = value.len;
            out->has_status = 1;
        } else if (kb_mp_str_eq(&key, "elements")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->elements_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->elements.data = body + start;
            out->elements.len = kb_mp_reader_pos(&reader) - start;
            out->has_elements = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_token_attr(const kb_token_attr *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 0;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }
    if (v->has_token) { count++; }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    if (v->has_token) {
        kb_mp_write_cstr(&w, "token");
        kb_mp_write_str(&w, v->token.data, v->token.len);
    }

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_token_attr(const uint8_t *body, size_t len, kb_token_attr *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "token")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->token.data = (const char *)value.data;
            out->token.len = value.len;
            out->has_token = 1;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}

kb_status kb_encode_video_conversation(const kb_video_conversation *v, uint8_t *out, size_t cap,
                       size_t *out_len)
{
    kb_mp_writer w;
    size_t count = 4;

    if (v == NULL || out == NULL) {
        return KB_ERR_INVALID;
    }

    kb_mp_writer_init(&w, out, cap);
    kb_mp_write_map(&w, count);
    kb_mp_write_cstr(&w, "conversationId");
    kb_mp_write_str(&w, v->conversation_id.data, v->conversation_id.len);
    kb_mp_write_cstr(&w, "joinLink");
    kb_mp_write_str(&w, v->join_link.data, v->join_link.len);
    kb_mp_write_cstr(&w, "previewParticipantIds");
    /* готовый msgpack вставляется как есть */
    kb_mp_write_raw(&w, v->preview_participant_ids.data, v->preview_participant_ids.len);
    kb_mp_write_cstr(&w, "callType");
    kb_mp_write_str(&w, v->call_type.data, v->call_type.len);

    if (kb_mp_writer_status(&w) != KB_OK) {
        return kb_mp_writer_status(&w);
    }
    if (out_len != NULL) {
        *out_len = kb_mp_writer_len(&w);
    }
    return KB_OK;
}

kb_status kb_decode_video_conversation(const uint8_t *body, size_t len, kb_video_conversation *out)
{
    kb_mp_reader reader;
    kb_mp_value head;
    kb_mp_value key;
    size_t i;
    kb_status st;

    if (out == NULL || (body == NULL && len != 0)) {
        return KB_ERR_INVALID;
    }
    memset(out, 0, sizeof *out);

    st = kb_mp_reader_init(&reader, body, len);
    if (st != KB_OK) { return st; }
    st = kb_mp_read(&reader, &head);
    if (st != KB_OK) { return st; }
    if (head.type != KB_MP_MAP) { return KB_ERR_INVALID; }

    for (i = 0; i < head.len; i++) {
        size_t before = kb_mp_reader_pos(&reader);

        st = kb_mp_read(&reader, &key);
        if (st != KB_OK) { return st; }
        if (key.type != KB_MP_STR) {
            /* Ключ не строка: пропускаем пару целиком. */
            reader.pos = before;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            continue;
        }

        if (kb_mp_str_eq(&key, "conversationId")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->conversation_id.data = (const char *)value.data;
            out->conversation_id.len = value.len;
        } else if (kb_mp_str_eq(&key, "joinLink")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->join_link.data = (const char *)value.data;
            out->join_link.len = value.len;
        } else if (kb_mp_str_eq(&key, "previewParticipantIds")) {
            size_t start = kb_mp_reader_pos(&reader);
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            out->preview_participant_ids_count = value.len;
            reader.pos = start;
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
            out->preview_participant_ids.data = body + start;
            out->preview_participant_ids.len = kb_mp_reader_pos(&reader) - start;
        } else if (kb_mp_str_eq(&key, "callType")) {
            kb_mp_value value;
            st = kb_mp_read(&reader, &value);
            if (st != KB_OK) { return st; }
            if (value.type != KB_MP_STR) { return KB_ERR_INVALID; }
            out->call_type.data = (const char *)value.data;
            out->call_type.len = value.len;
        } else {
            /* Полей, которых нет в схеме, сервер вправе
               прислать сколько угодно: молча пропускаем. */
            st = kb_mp_skip(&reader);
            if (st != KB_OK) { return st; }
        }
    }
    return KB_OK;
}
