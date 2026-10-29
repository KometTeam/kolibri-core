/*
 * @generated tools/gen_schema.py -- не редактировать руками.
 *
 * Источник: файлы schema, расширение toml
 * Перегенерация: python3 tools/gen_schema.py
 */


#include <string.h>

#include "kolibri_api.h"

kb_status kb_send_auth_request(kb_session *s, const kb_auth_request *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_request(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_AUTH_REQUEST, s->bufs.scratch, n, seq);
}

kb_status kb_call_auth_request(kb_client *c, const kb_auth_request *v, kb_auth_request_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_request(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_AUTH_REQUEST, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_auth_request_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_auth_verify(kb_session *s, const kb_auth_verify *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_verify(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_AUTH_VERIFY, s->bufs.scratch, n, seq);
}

kb_status kb_call_auth_verify(kb_client *c, const kb_auth_verify *v, kb_auth_verify_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_verify(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_AUTH_VERIFY, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_auth_verify_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_login(kb_session *s, const kb_login *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_login(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_LOGIN, s->bufs.scratch, n, seq);
}

kb_status kb_call_login(kb_client *c, const kb_login *v, kb_login_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_login(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_LOGIN, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_login_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_logout(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_logout v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_logout(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_LOGOUT, s->bufs.scratch, n, seq);
}

kb_status kb_call_logout(kb_client *c, kb_logout_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_logout v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_logout(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_LOGOUT, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_logout_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_config(kb_session *s, const kb_config *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_config(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CONFIG, s->bufs.scratch, n, seq);
}

kb_status kb_call_config(kb_client *c, const kb_config *v, kb_config_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_config(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CONFIG, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_config_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_preset_avatars(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_preset_avatars v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_preset_avatars(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_PRESET_AVATARS, s->bufs.scratch, n, seq);
}

kb_status kb_call_preset_avatars(kb_client *c, kb_preset_avatars_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_preset_avatars v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_preset_avatars(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_PRESET_AVATARS, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_preset_avatars_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_contact_info(kb_session *s, const kb_contact_info *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_contact_info(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CONTACT_INFO, s->bufs.scratch, n, seq);
}

kb_status kb_call_contact_info(kb_client *c, const kb_contact_info *v, kb_contact_info_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_contact_info(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CONTACT_INFO, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_contact_info_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_contact_presence(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_contact_presence v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_contact_presence(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CONTACT_PRESENCE, s->bufs.scratch, n, seq);
}

kb_status kb_call_contact_presence(kb_client *c, kb_contact_presence_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_contact_presence v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_contact_presence(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CONTACT_PRESENCE, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_contact_presence_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_contact_info_by_phone(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_contact_info_by_phone v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_contact_info_by_phone(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CONTACT_INFO_BY_PHONE, s->bufs.scratch, n, seq);
}

kb_status kb_call_contact_info_by_phone(kb_client *c, kb_contact_info_by_phone_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_contact_info_by_phone v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_contact_info_by_phone(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CONTACT_INFO_BY_PHONE, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_contact_info_by_phone_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_chat_info(kb_session *s, const kb_chat_info *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_chat_info(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CHAT_INFO, s->bufs.scratch, n, seq);
}

kb_status kb_call_chat_info(kb_client *c, const kb_chat_info *v, kb_chat_info_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_chat_info(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CHAT_INFO, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_chat_info_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_chat_history(kb_session *s, const kb_chat_history *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_chat_history(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CHAT_HISTORY, s->bufs.scratch, n, seq);
}

kb_status kb_call_chat_history(kb_client *c, const kb_chat_history *v, kb_chat_history_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_chat_history(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CHAT_HISTORY, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_chat_history_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_chat_mark(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_chat_mark v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_chat_mark(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CHAT_MARK, s->bufs.scratch, n, seq);
}

kb_status kb_call_chat_mark(kb_client *c, kb_chat_mark_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_chat_mark v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_chat_mark(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CHAT_MARK, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_chat_mark_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_chat_media(kb_session *s, const kb_chat_media *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_chat_media(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CHAT_MEDIA, s->bufs.scratch, n, seq);
}

kb_status kb_call_chat_media(kb_client *c, const kb_chat_media *v, kb_chat_media_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_chat_media(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CHAT_MEDIA, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_chat_media_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_chat_join(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_chat_join v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_chat_join(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CHAT_JOIN, s->bufs.scratch, n, seq);
}

kb_status kb_call_chat_join(kb_client *c, kb_chat_join_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_chat_join v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_chat_join(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CHAT_JOIN, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_chat_join_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_chat_members(kb_session *s, const kb_chat_members *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_chat_members(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CHAT_MEMBERS, s->bufs.scratch, n, seq);
}

kb_status kb_call_chat_members(kb_client *c, const kb_chat_members *v, kb_chat_members_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_chat_members(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CHAT_MEMBERS, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_chat_members_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_chat_personal_config(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_chat_personal_config v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_chat_personal_config(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CHAT_PERSONAL_CONFIG, s->bufs.scratch, n, seq);
}

kb_status kb_call_chat_personal_config(kb_client *c, kb_chat_personal_config_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_chat_personal_config v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_chat_personal_config(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CHAT_PERSONAL_CONFIG, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_chat_personal_config_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_chat_livestream_info(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_chat_livestream_info v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_chat_livestream_info(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CHAT_LIVESTREAM_INFO, s->bufs.scratch, n, seq);
}

kb_status kb_call_chat_livestream_info(kb_client *c, kb_chat_livestream_info_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_chat_livestream_info v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_chat_livestream_info(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CHAT_LIVESTREAM_INFO, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_chat_livestream_info_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_msg_send(kb_session *s, const kb_msg_send *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_msg_send(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_MSG_SEND, s->bufs.scratch, n, seq);
}

kb_status kb_call_msg_send(kb_client *c, const kb_msg_send *v, kb_msg_send_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_msg_send(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_MSG_SEND, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_msg_send_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_msg_typing(kb_session *s, const kb_msg_typing *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_msg_typing(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_MSG_TYPING, s->bufs.scratch, n, seq);
}

kb_status kb_call_msg_typing(kb_client *c, const kb_msg_typing *v, kb_msg_typing_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_msg_typing(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_MSG_TYPING, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_msg_typing_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_msg_get(kb_session *s, const kb_msg_get *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_msg_get(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_MSG_GET, s->bufs.scratch, n, seq);
}

kb_status kb_call_msg_get(kb_client *c, const kb_msg_get *v, kb_msg_get_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_msg_get(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_MSG_GET, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_msg_get_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_video_chat_start(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_video_chat_start v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_video_chat_start(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_VIDEO_CHAT_START, s->bufs.scratch, n, seq);
}

kb_status kb_call_video_chat_start(kb_client *c, kb_video_chat_start_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_video_chat_start v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_video_chat_start(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_VIDEO_CHAT_START, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_video_chat_start_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_video_chat_start_active(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_video_chat_start_active v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_video_chat_start_active(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_VIDEO_CHAT_START_ACTIVE, s->bufs.scratch, n, seq);
}

kb_status kb_call_video_chat_start_active(kb_client *c, kb_video_chat_start_active_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_video_chat_start_active v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_video_chat_start_active(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_VIDEO_CHAT_START_ACTIVE, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_video_chat_start_active_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_photo_upload(kb_session *s, const kb_photo_upload *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_photo_upload(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_PHOTO_UPLOAD, s->bufs.scratch, n, seq);
}

kb_status kb_call_photo_upload(kb_client *c, const kb_photo_upload *v, kb_photo_upload_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_photo_upload(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_PHOTO_UPLOAD, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_photo_upload_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_video_upload(kb_session *s, const kb_video_upload *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_video_upload(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_VIDEO_UPLOAD, s->bufs.scratch, n, seq);
}

kb_status kb_call_video_upload(kb_client *c, const kb_video_upload *v, kb_video_upload_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_video_upload(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_VIDEO_UPLOAD, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_video_upload_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_file_download(kb_session *s, const kb_file_download *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_file_download(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_FILE_DOWNLOAD, s->bufs.scratch, n, seq);
}

kb_status kb_call_file_download(kb_client *c, const kb_file_download *v, kb_file_download_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_file_download(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_FILE_DOWNLOAD, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_file_download_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_get_comments_updates(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_get_comments_updates v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_get_comments_updates(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_GET_COMMENTS_UPDATES, s->bufs.scratch, n, seq);
}

kb_status kb_call_get_comments_updates(kb_client *c, kb_get_comments_updates_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_get_comments_updates v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_get_comments_updates(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_GET_COMMENTS_UPDATES, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_get_comments_updates_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_auth_login_restore_password(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_auth_login_restore_password v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_auth_login_restore_password(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_AUTH_LOGIN_RESTORE_PASSWORD, s->bufs.scratch, n, seq);
}

kb_status kb_call_auth_login_restore_password(kb_client *c, kb_auth_login_restore_password_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_auth_login_restore_password v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_auth_login_restore_password(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_AUTH_LOGIN_RESTORE_PASSWORD, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_auth_login_restore_password_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_auth2fa_details(kb_session *s, const kb_auth2fa_details *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth2fa_details(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_AUTH2FA_DETAILS, s->bufs.scratch, n, seq);
}

kb_status kb_call_auth2fa_details(kb_client *c, const kb_auth2fa_details *v, kb_auth2fa_details_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth2fa_details(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_AUTH2FA_DETAILS, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_auth2fa_details_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_external_callback(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_external_callback v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_external_callback(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_EXTERNAL_CALLBACK, s->bufs.scratch, n, seq);
}

kb_status kb_call_external_callback(kb_client *c, kb_external_callback_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_external_callback v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_external_callback(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_EXTERNAL_CALLBACK, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_external_callback_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_auth_verify_email(kb_session *s, const kb_auth_verify_email *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_verify_email(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_AUTH_VERIFY_EMAIL, s->bufs.scratch, n, seq);
}

kb_status kb_call_auth_verify_email(kb_client *c, const kb_auth_verify_email *v, kb_auth_verify_email_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_verify_email(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_AUTH_VERIFY_EMAIL, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_auth_verify_email_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_auth_check_email(kb_session *s, const kb_auth_check_email *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_check_email(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_AUTH_CHECK_EMAIL, s->bufs.scratch, n, seq);
}

kb_status kb_call_auth_check_email(kb_client *c, const kb_auth_check_email *v, kb_auth_check_email_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_check_email(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_AUTH_CHECK_EMAIL, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_auth_check_email_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_auth_set2fa(kb_session *s, const kb_auth_set2fa *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_set2fa(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_AUTH_SET2FA, s->bufs.scratch, n, seq);
}

kb_status kb_call_auth_set2fa(kb_client *c, const kb_auth_set2fa *v, kb_auth_set2fa_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_set2fa(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_AUTH_SET2FA, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_auth_set2fa_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_auth_create_track(kb_session *s, const kb_auth_create_track *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_create_track(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_AUTH_CREATE_TRACK, s->bufs.scratch, n, seq);
}

kb_status kb_call_auth_create_track(kb_client *c, const kb_auth_create_track *v, kb_auth_create_track_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_create_track(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_AUTH_CREATE_TRACK, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_auth_create_track_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_auth_check_password(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_auth_check_password v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_auth_check_password(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_AUTH_CHECK_PASSWORD, s->bufs.scratch, n, seq);
}

kb_status kb_call_auth_check_password(kb_client *c, kb_auth_check_password_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_auth_check_password v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_auth_check_password(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_AUTH_CHECK_PASSWORD, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_auth_check_password_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_auth_login_check_password(kb_session *s, const kb_auth_login_check_password *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_login_check_password(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_AUTH_LOGIN_CHECK_PASSWORD, s->bufs.scratch, n, seq);
}

kb_status kb_call_auth_login_check_password(kb_client *c, const kb_auth_login_check_password *v, kb_auth_login_check_password_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_auth_login_check_password(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_AUTH_LOGIN_CHECK_PASSWORD, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_auth_login_check_password_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_notif_msg_delete(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_notif_msg_delete v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_notif_msg_delete(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_NOTIF_MSG_DELETE, s->bufs.scratch, n, seq);
}

kb_status kb_call_notif_msg_delete(kb_client *c, kb_notif_msg_delete_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_notif_msg_delete v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_notif_msg_delete(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_NOTIF_MSG_DELETE, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_notif_msg_delete_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_bot_info(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_bot_info v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_bot_info(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_BOT_INFO, s->bufs.scratch, n, seq);
}

kb_status kb_call_bot_info(kb_client *c, kb_bot_info_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_bot_info v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_bot_info(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_BOT_INFO, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_bot_info_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_notif_msg_delayed(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_notif_msg_delayed v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_notif_msg_delayed(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_NOTIF_MSG_DELAYED, s->bufs.scratch, n, seq);
}

kb_status kb_call_notif_msg_delayed(kb_client *c, kb_notif_msg_delayed_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_notif_msg_delayed v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_notif_msg_delayed(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_NOTIF_MSG_DELAYED, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_notif_msg_delayed_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_ok_token(kb_session *s, const kb_ok_token *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_ok_token(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_OK_TOKEN, s->bufs.scratch, n, seq);
}

kb_status kb_call_ok_token(kb_client *c, const kb_ok_token *v, kb_ok_token_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_ok_token(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_OK_TOKEN, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_ok_token_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_notif_profile(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_notif_profile v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_notif_profile(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_NOTIF_PROFILE, s->bufs.scratch, n, seq);
}

kb_status kb_call_notif_profile(kb_client *c, kb_notif_profile_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_notif_profile v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_notif_profile(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_NOTIF_PROFILE, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_notif_profile_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_web_app_init_data(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_web_app_init_data v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_web_app_init_data(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_WEB_APP_INIT_DATA, s->bufs.scratch, n, seq);
}

kb_status kb_call_web_app_init_data(kb_client *c, kb_web_app_init_data_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_web_app_init_data v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_web_app_init_data(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_WEB_APP_INIT_DATA, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_web_app_init_data_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_complain(kb_session *s, const kb_complain *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_complain(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_COMPLAIN, s->bufs.scratch, n, seq);
}

kb_status kb_call_complain(kb_client *c, const kb_complain *v, kb_complain_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_complain(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_COMPLAIN, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_complain_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_complain_reasons_get(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_complain_reasons_get v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_complain_reasons_get(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_COMPLAIN_REASONS_GET, s->bufs.scratch, n, seq);
}

kb_status kb_call_complain_reasons_get(kb_client *c, kb_complain_reasons_get_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_complain_reasons_get v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_complain_reasons_get(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_COMPLAIN_REASONS_GET, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_complain_reasons_get_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_call_history(kb_session *s, const kb_call_history *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_call_history(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CALL_HISTORY, s->bufs.scratch, n, seq);
}

kb_status kb_call_call_history(kb_client *c, const kb_call_history *v, kb_call_history_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_call_history(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CALL_HISTORY, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_call_history_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_video_chat_delete_history(kb_session *s, const kb_video_chat_delete_history *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_video_chat_delete_history(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_VIDEO_CHAT_DELETE_HISTORY, s->bufs.scratch, n, seq);
}

kb_status kb_call_video_chat_delete_history(kb_client *c, const kb_video_chat_delete_history *v, kb_video_chat_delete_history_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_video_chat_delete_history(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_VIDEO_CHAT_DELETE_HISTORY, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_video_chat_delete_history_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_notif_call_history(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_notif_call_history v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_notif_call_history(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_NOTIF_CALL_HISTORY, s->bufs.scratch, n, seq);
}

kb_status kb_call_notif_call_history(kb_client *c, kb_notif_call_history_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_notif_call_history v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_notif_call_history(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_NOTIF_CALL_HISTORY, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_notif_call_history_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_video_chat_join_by_link(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_video_chat_join_by_link v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_video_chat_join_by_link(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_VIDEO_CHAT_JOIN_BY_LINK, s->bufs.scratch, n, seq);
}

kb_status kb_call_video_chat_join_by_link(kb_client *c, kb_video_chat_join_by_link_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_video_chat_join_by_link v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_video_chat_join_by_link(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_VIDEO_CHAT_JOIN_BY_LINK, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_video_chat_join_by_link_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_msg_get_reactions(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_msg_get_reactions v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_msg_get_reactions(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_MSG_GET_REACTIONS, s->bufs.scratch, n, seq);
}

kb_status kb_call_msg_get_reactions(kb_client *c, kb_msg_get_reactions_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_msg_get_reactions v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_msg_get_reactions(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_MSG_GET_REACTIONS, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_msg_get_reactions_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_profile_delete(kb_session *s, const kb_profile_delete *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_profile_delete(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_PROFILE_DELETE, s->bufs.scratch, n, seq);
}

kb_status kb_call_profile_delete(kb_client *c, const kb_profile_delete *v, kb_profile_delete_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_profile_delete(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_PROFILE_DELETE, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_profile_delete_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_profile_delete_time(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_profile_delete_time v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_profile_delete_time(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_PROFILE_DELETE_TIME, s->bufs.scratch, n, seq);
}

kb_status kb_call_profile_delete_time(kb_client *c, kb_profile_delete_time_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_profile_delete_time v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_profile_delete_time(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_PROFILE_DELETE_TIME, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_profile_delete_time_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_audio_transcription(kb_session *s, const kb_audio_transcription *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_audio_transcription(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_AUDIO_TRANSCRIPTION, s->bufs.scratch, n, seq);
}

kb_status kb_call_audio_transcription(kb_client *c, const kb_audio_transcription *v, kb_audio_transcription_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_audio_transcription(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_AUDIO_TRANSCRIPTION, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_audio_transcription_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_photo_url_refresh(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_photo_url_refresh v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_photo_url_refresh(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_PHOTO_URL_REFRESH, s->bufs.scratch, n, seq);
}

kb_status kb_call_photo_url_refresh(kb_client *c, kb_photo_url_refresh_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_photo_url_refresh v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_photo_url_refresh(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_PHOTO_URL_REFRESH, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_photo_url_refresh_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_stories_list(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_stories_list v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_list(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_STORIES_LIST, s->bufs.scratch, n, seq);
}

kb_status kb_call_stories_list(kb_client *c, kb_stories_list_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_stories_list v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_list(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_STORIES_LIST, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_stories_list_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_stories_get_by_owner(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_stories_get_by_owner v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_get_by_owner(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_STORIES_GET_BY_OWNER, s->bufs.scratch, n, seq);
}

kb_status kb_call_stories_get_by_owner(kb_client *c, kb_stories_get_by_owner_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_stories_get_by_owner v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_get_by_owner(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_STORIES_GET_BY_OWNER, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_stories_get_by_owner_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_stories_get_stats(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_stories_get_stats v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_get_stats(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_STORIES_GET_STATS, s->bufs.scratch, n, seq);
}

kb_status kb_call_stories_get_stats(kb_client *c, kb_stories_get_stats_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_stories_get_stats v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_get_stats(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_STORIES_GET_STATS, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_stories_get_stats_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_stories_react(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_stories_react v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_react(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_STORIES_REACT, s->bufs.scratch, n, seq);
}

kb_status kb_call_stories_react(kb_client *c, kb_stories_react_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_stories_react v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_react(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_STORIES_REACT, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_stories_react_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_stories_mark(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_stories_mark v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_mark(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_STORIES_MARK, s->bufs.scratch, n, seq);
}

kb_status kb_call_stories_mark(kb_client *c, kb_stories_mark_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_stories_mark v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_mark(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_STORIES_MARK, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_stories_mark_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_stories_send(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_stories_send v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_send(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_STORIES_SEND, s->bufs.scratch, n, seq);
}

kb_status kb_call_stories_send(kb_client *c, kb_stories_send_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_stories_send v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_send(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_STORIES_SEND, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_stories_send_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_notif_stories_update(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_notif_stories_update v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_notif_stories_update(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_NOTIF_STORIES_UPDATE, s->bufs.scratch, n, seq);
}

kb_status kb_call_notif_stories_update(kb_client *c, kb_notif_stories_update_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_notif_stories_update v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_notif_stories_update(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_NOTIF_STORIES_UPDATE, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_notif_stories_update_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_stories_edit(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_stories_edit v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_edit(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_STORIES_EDIT, s->bufs.scratch, n, seq);
}

kb_status kb_call_stories_edit(kb_client *c, kb_stories_edit_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_stories_edit v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_edit(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_STORIES_EDIT, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_stories_edit_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_stories_delete(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_stories_delete v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_delete(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_STORIES_DELETE, s->bufs.scratch, n, seq);
}

kb_status kb_call_stories_delete(kb_client *c, kb_stories_delete_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_stories_delete v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_delete(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_STORIES_DELETE, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_stories_delete_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_stories_get_by_story_id(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_stories_get_by_story_id v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_get_by_story_id(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_STORIES_GET_BY_STORY_ID, s->bufs.scratch, n, seq);
}

kb_status kb_call_stories_get_by_story_id(kb_client *c, kb_stories_get_by_story_id_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_stories_get_by_story_id v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_stories_get_by_story_id(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_STORIES_GET_BY_STORY_ID, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_stories_get_by_story_id_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_org_info(kb_session *s, const kb_org_info *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_org_info(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_ORG_INFO, s->bufs.scratch, n, seq);
}

kb_status kb_call_org_info(kb_client *c, const kb_org_info *v, kb_org_info_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_org_info(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_ORG_INFO, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_org_info_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_chat_reactions_settings_set(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_chat_reactions_settings_set v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_chat_reactions_settings_set(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CHAT_REACTIONS_SETTINGS_SET, s->bufs.scratch, n, seq);
}

kb_status kb_call_chat_reactions_settings_set(kb_client *c, kb_chat_reactions_settings_set_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_chat_reactions_settings_set v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_chat_reactions_settings_set(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CHAT_REACTIONS_SETTINGS_SET, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_chat_reactions_settings_set_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_reactions_settings_get_by_chat_id(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_reactions_settings_get_by_chat_id v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_reactions_settings_get_by_chat_id(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_REACTIONS_SETTINGS_GET_BY_CHAT_ID, s->bufs.scratch, n, seq);
}

kb_status kb_call_reactions_settings_get_by_chat_id(kb_client *c, kb_reactions_settings_get_by_chat_id_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_reactions_settings_get_by_chat_id v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_reactions_settings_get_by_chat_id(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_REACTIONS_SETTINGS_GET_BY_CHAT_ID, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_reactions_settings_get_by_chat_id_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_folders_get(kb_session *s, const kb_folders_get *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_folders_get(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_FOLDERS_GET, s->bufs.scratch, n, seq);
}

kb_status kb_call_folders_get(kb_client *c, const kb_folders_get *v, kb_folders_get_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_folders_get(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_FOLDERS_GET, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_folders_get_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_folders_get_by_id(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_folders_get_by_id v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_folders_get_by_id(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_FOLDERS_GET_BY_ID, s->bufs.scratch, n, seq);
}

kb_status kb_call_folders_get_by_id(kb_client *c, kb_folders_get_by_id_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_folders_get_by_id v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_folders_get_by_id(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_FOLDERS_GET_BY_ID, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_folders_get_by_id_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_folders_update(kb_session *s, const kb_folders_update *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_folders_update(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_FOLDERS_UPDATE, s->bufs.scratch, n, seq);
}

kb_status kb_call_folders_update(kb_client *c, const kb_folders_update *v, kb_folders_update_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_folders_update(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_FOLDERS_UPDATE, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_folders_update_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_folders_reorder(kb_session *s, const kb_folders_reorder *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_folders_reorder(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_FOLDERS_REORDER, s->bufs.scratch, n, seq);
}

kb_status kb_call_folders_reorder(kb_client *c, const kb_folders_reorder *v, kb_folders_reorder_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_folders_reorder(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_FOLDERS_REORDER, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_folders_reorder_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_folders_delete(kb_session *s, const kb_folders_delete *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_folders_delete(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_FOLDERS_DELETE, s->bufs.scratch, n, seq);
}

kb_status kb_call_folders_delete(kb_client *c, const kb_folders_delete *v, kb_folders_delete_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_folders_delete(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_FOLDERS_DELETE, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_folders_delete_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_notif_folders(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_notif_folders v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_notif_folders(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_NOTIF_FOLDERS, s->bufs.scratch, n, seq);
}

kb_status kb_call_notif_folders(kb_client *c, kb_notif_folders_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_notif_folders v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_notif_folders(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_NOTIF_FOLDERS, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_notif_folders_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_transcription_result(kb_session *s, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    kb_transcription_result v;
    if (s == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_transcription_result(&v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_TRANSCRIPTION_RESULT, s->bufs.scratch, n, seq);
}

kb_status kb_call_transcription_result(kb_client *c, kb_transcription_result_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    kb_transcription_result v;
    if (c == NULL) {
        return KB_ERR_INVALID;
    }
    memset(&v, 0, sizeof v);
    st = kb_encode_transcription_result(&v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_TRANSCRIPTION_RESULT, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_transcription_result_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_chat_suggest(kb_session *s, const kb_chat_suggest *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_chat_suggest(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_CHAT_SUGGEST, s->bufs.scratch, n, seq);
}

kb_status kb_call_chat_suggest(kb_client *c, const kb_chat_suggest *v, kb_chat_suggest_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_chat_suggest(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_CHAT_SUGGEST, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_chat_suggest_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_banners_get(kb_session *s, const kb_banners_get *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_banners_get(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_BANNERS_GET, s->bufs.scratch, n, seq);
}

kb_status kb_call_banners_get(kb_client *c, const kb_banners_get *v, kb_banners_get_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_banners_get(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_BANNERS_GET, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_banners_get_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_send_vote(kb_session *s, const kb_send_vote *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_send_vote(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_SEND_VOTE, s->bufs.scratch, n, seq);
}

kb_status kb_call_send_vote(kb_client *c, const kb_send_vote *v, kb_send_vote_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_send_vote(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_SEND_VOTE, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_send_vote_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_voters_list_by_answer(kb_session *s, const kb_voters_list_by_answer *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_voters_list_by_answer(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_VOTERS_LIST_BY_ANSWER, s->bufs.scratch, n, seq);
}

kb_status kb_call_voters_list_by_answer(kb_client *c, const kb_voters_list_by_answer *v, kb_voters_list_by_answer_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_voters_list_by_answer(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_VOTERS_LIST_BY_ANSWER, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_voters_list_by_answer_response(r, rlen, resp) : KB_OK;
}

kb_status kb_send_get_poll_updates(kb_session *s, const kb_get_poll_updates *v, uint16_t *seq)
{
    size_t n = 0;
    kb_status st;
    if (s == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_get_poll_updates(v, s->bufs.scratch, s->bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    return kb_session_request(s, KB_OPCODE_GET_POLL_UPDATES, s->bufs.scratch, n, seq);
}

kb_status kb_call_get_poll_updates(kb_client *c, const kb_get_poll_updates *v, kb_get_poll_updates_response *resp)
{
    size_t n = 0, rlen = 0;
    const uint8_t *r = NULL;
    kb_status st;
    if (c == NULL || v == NULL) {
        return KB_ERR_INVALID;
    }
    st = kb_encode_get_poll_updates(v, c->session.bufs.scratch, c->session.bufs.scratch_capacity, &n);
    if (st != KB_OK) {
        return st;
    }
    st = kb_client_call(c, KB_OPCODE_GET_POLL_UPDATES, c->session.bufs.scratch, n, &r, &rlen);
    if (st != KB_OK) {
        return st;
    }
    return resp != NULL ? kb_decode_get_poll_updates_response(r, rlen, resp) : KB_OK;
}
