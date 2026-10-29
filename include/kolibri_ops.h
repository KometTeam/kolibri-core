/*
 * @generated tools/gen_schema.py -- не редактировать руками.
 *
 * Источник: файлы schema, расширение toml
 * Перегенерация: python3 tools/gen_schema.py
 */


#ifndef KOLIBRI_OPS_H
#define KOLIBRI_OPS_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * Строка msgpack: без завершающего нуля, указывает внутрь
 * исходного буфера и живёт ровно столько же.
 */
typedef struct {
    const char *data;
    size_t len;
} kb_str;

/* Бинарное поле — так же, без копирования. */
typedef struct {
    const uint8_t *data;
    size_t len;
} kb_bin;

/*
 * Кусок msgpack как есть: массив, map или вложенная структура.
 * Разбирается курсором из kolibri_msgpack.h, когда понадобится.
 */
typedef struct {
    const uint8_t *data;
    size_t len;
} kb_raw;

/*
 * Извлечено из декомпила 26.28. В клиенте опкод назван TRANSCRIBE_MEDIA. Request thg(long,long,long) super(x0c.G3) c=12 confirmed (chatId included). Response cmh.a singleton builds bmh(String,bnh). Both keys confirmed.
 *
 * Опкод 202 (AUDIO_TRANSCRIPTION).
 */
typedef struct {
    /* mediaId */
    int64_t media_id;
    /* messageId */
    int64_t message_id;
    /* chatId */
    int64_t chat_id;
} kb_audio_transcription;

/*
 * Ответ на AudioTranscription.
 */
typedef struct {
    /* transcription */
    kb_str transcription;
    /* transcriptionStatus */
    int32_t transcription_status;
} kb_audio_transcription_response;

#define KB_OPCODE_AUDIO_TRANSCRIPTION 202

kb_status kb_encode_audio_transcription(const kb_audio_transcription *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_audio_transcription_response(const uint8_t *body, size_t len,
                       kb_audio_transcription_response *out);

/*
 * Извлечено из декомпила 26.28. Request builder g9b(String) sets only trackId (conditionally). Response 'ogcVar' file null -> unreadable, empty response.
 *
 * Опкод 104 (AUTH_2FA_DETAILS).
 */
typedef struct {
    /* trackId */
    kb_str track_id;
    int has_track_id;
} kb_auth2fa_details;

/*
 * Ответ на Auth2faDetails.
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_auth2fa_details_response;

#define KB_OPCODE_AUTH2FA_DETAILS 104

kb_status kb_encode_auth2fa_details(const kb_auth2fa_details *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_auth2fa_details_response(const uint8_t *body, size_t len,
                       kb_auth2fa_details_response *out);

/*
 * Извлечено из декомпила 26.28. Request builder ub0 (not a factory), both fields always set. Response tyf.d=new tyf(1), case 1 decompiled; only keys 'trackId','email'.
 *
 * Опкод 110 (AUTH_CHECK_EMAIL).
 */
typedef struct {
    /* trackId */
    kb_str track_id;
    /* verifyCode */
    kb_str verify_code;
} kb_auth_check_email;

/*
 * Ответ на AuthCheckEmail.
 */
typedef struct {
    /* trackId */
    kb_str track_id;
    int has_track_id;
    /* email */
    kb_str email;
    int has_email;
} kb_auth_check_email_response;

#define KB_OPCODE_AUTH_CHECK_EMAIL 110

kb_status kb_encode_auth_check_email(const kb_auth_check_email *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_auth_check_email_response(const uint8_t *body, size_t len,
                       kb_auth_check_email_response *out);

/*
 * Извлечено из декомпила 26.28. No request builder -> empty map. Response gl3.b=new gl3(1), case 1 decompiled; only key 'trackId'.
 *
 * Опкод 113 (AUTH_CHECK_PASSWORD).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_auth_check_password;

/*
 * Ответ на AuthCheckPassword.
 */
typedef struct {
    /* trackId */
    kb_str track_id;
    int has_track_id;
} kb_auth_check_password_response;

#define KB_OPCODE_AUTH_CHECK_PASSWORD 113

kb_status kb_encode_auth_check_password(const kb_auth_check_password *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_auth_check_password_response(const uint8_t *body, size_t len,
                       kb_auth_check_password_response *out);

/*
 * Извлечено из декомпила 26.28. Request builder no-arg sets only 'type' (Java short). Response mx4.c=new mx4(1), case 1 decompiled; only key 'trackId'.
 *
 * Опкод 112 (AUTH_CREATE_TRACK).
 */
typedef struct {
    /* type */
    int32_t type;
} kb_auth_create_track;

/*
 * Ответ на AuthCreateTrack.
 */
typedef struct {
    /* trackId */
    kb_str track_id;
    int has_track_id;
} kb_auth_create_track_response;

#define KB_OPCODE_AUTH_CREATE_TRACK 112

kb_status kb_encode_auth_create_track(const kb_auth_create_track *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_auth_create_track_response(const uint8_t *body, size_t len,
                       kb_auth_create_track_response *out);

/*
 * Облачный пароль (2FA) после `passwordChallenge`. В ответе — тот же `tokenAttrs.LOGIN`.
 *
 * Опкод 115 (AUTH_LOGIN_CHECK_PASSWORD).
 */
typedef struct {
    /* trackId */
    kb_str track_id;
    /* password */
    kb_str password;
} kb_auth_login_check_password;

/*
 * Ответ на AuthLoginCheckPassword.
 */
typedef struct {
    /* tokenAttrs */
    kb_raw token_attrs;
    size_t token_attrs_count;
    int has_token_attrs;
    /* profile */
    kb_raw profile;
    int has_profile;
    /* error */
    kb_raw error;
    int has_error;
} kb_auth_login_check_password_response;

#define KB_OPCODE_AUTH_LOGIN_CHECK_PASSWORD 115

kb_status kb_encode_auth_login_check_password(const kb_auth_login_check_password *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_auth_login_check_password_response(const uint8_t *body, size_t len,
                       kb_auth_login_check_password_response *out);

/*
 * Извлечено из декомпила 26.28. No request builder -> empty map. Verified bya.c=new bya(1); case 1 decompiled, only key is 'tokenAttrs'.
 *
 * Опкод 101 (AUTH_LOGIN_RESTORE_PASSWORD).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_auth_login_restore_password;

/*
 * Ответ на AuthLoginRestorePassword.
 */
typedef struct {
    /* tokenAttrs */
    kb_raw token_attrs;
    size_t token_attrs_count;
    int has_token_attrs;
} kb_auth_login_restore_password_response;

#define KB_OPCODE_AUTH_LOGIN_RESTORE_PASSWORD 101

kb_status kb_encode_auth_login_restore_password(const kb_auth_login_restore_password *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_auth_login_restore_password_response(const uint8_t *body, size_t len,
                       kb_auth_login_restore_password_response *out);

/*
 * Старт телефонной авторизации — сервер отправляет SMS с кодом.
 *
 * `type` — `START_AUTH` для первого запроса, `RESEND` для повторного.
 * `mode` — 96-байтный anti-spoof fingerprint из
 * `kolibri_net::auth::chat_cache_fingerprint`; дайджесты должны быть от **той же**
 * версии приложения, что и `app_version`/`build_number` в handshake.
 *
 * Опкод 17 (AUTH_REQUEST).
 */
typedef struct {
    /* phone */
    kb_str phone;
    /* type */
    kb_str type;
    /* language */
    kb_str language;
    int has_language;
    /* mode */
    kb_bin mode;
    int has_mode;
} kb_auth_request;

/*
 * Ответ на AuthRequest.
 */
typedef struct {
    /* token */
    kb_str token;
} kb_auth_request_response;

#define KB_OPCODE_AUTH_REQUEST 17

kb_status kb_encode_auth_request(const kb_auth_request *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_auth_request_response(const uint8_t *body, size_t len,
                       kb_auth_request_response *out);

/*
 * Извлечено из декомпила 26.28. Request builder verified; corrected expectedCapabilities from [any] to [i8] (list is built from Byte values). Response cd0.a=new cd0() (no parse method) -> unreadable.
 *
 * Опкод 111 (AUTH_SET_2FA).
 */
typedef struct {
    /* trackId */
    kb_str track_id;
    /* password */
    kb_str password;
    int has_password;
    /* hint */
    kb_str hint;
    int has_hint;
    /* remove2fa */
    int remove2fa;
    int has_remove2fa;
    /* expectedCapabilities */
    kb_raw expected_capabilities;
    size_t expected_capabilities_count;
} kb_auth_set2fa;

/*
 * Ответ на AuthSet2fa.
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_auth_set2fa_response;

#define KB_OPCODE_AUTH_SET2FA 111

kb_status kb_encode_auth_set2fa(const kb_auth_set2fa *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_auth_set2fa_response(const uint8_t *body, size_t len,
                       kb_auth_set2fa_response *out);

/*
 * Проверка кода из SMS.
 *
 * Три исхода: в `tokenAttrs.LOGIN` лежит login-токен (вход), в
 * `tokenAttrs.REGISTER` — токен регистрации (номер новый), либо пришёл
 * `passwordChallenge` — включена 2FA, нужен `AuthLoginCheckPassword`.
 *
 * Опкод 18 (AUTH).
 */
typedef struct {
    /* token */
    kb_str token;
    /* verifyCode */
    kb_str verify_code;
    /* authTokenType */
    kb_str auth_token_type;
} kb_auth_verify;

/*
 * Ответ на AuthVerify.
 */
typedef struct {
    /* tokenAttrs */
    kb_raw token_attrs;
    size_t token_attrs_count;
    int has_token_attrs;
    /* profile */
    kb_raw profile;
    int has_profile;
    /* passwordChallenge */
    kb_raw password_challenge;
    int has_password_challenge;
    /* presetAvatars */
    kb_raw preset_avatars;
    int has_preset_avatars;
} kb_auth_verify_response;

#define KB_OPCODE_AUTH_VERIFY 18

kb_status kb_encode_auth_verify(const kb_auth_verify *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_auth_verify_response(const uint8_t *body, size_t len,
                       kb_auth_verify_response *out);

/*
 * Извлечено из декомпила 26.28. Request builder verified. Response cya.c=new cya(1); cya.h(w2a) is dump-skipped (unit count 1700) -> response unreadable.
 *
 * Опкод 109 (AUTH_VERIFY_EMAIL).
 */
typedef struct {
    /* trackId */
    kb_str track_id;
    /* email */
    kb_str email;
    int has_email;
} kb_auth_verify_email;

/*
 * Ответ на AuthVerifyEmail.
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_auth_verify_email_response;

#define KB_OPCODE_AUTH_VERIFY_EMAIL 109

kb_status kb_encode_auth_verify_email(const kb_auth_verify_email *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_auth_verify_email_response(const uint8_t *body, size_t len,
                       kb_auth_verify_email_response *out);

/*
 * Извлечено из декомпила 26.28. Builder xl0 confirmed. Response zl0.b (N=0) dispatch not decompiled -> empty.
 *
 * Опкод 302 (BANNERS_GET).
 */
typedef struct {
    /* bannersSync */
    int64_t banners_sync;
} kb_banners_get;

/*
 * Ответ на BannersGet.
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_banners_get_response;

#define KB_OPCODE_BANNERS_GET 302

kb_status kb_encode_banners_get(const kb_banners_get *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_banners_get_response(const uint8_t *body, size_t len,
                       kb_banners_get_response *out);

/*
 * Извлечено из декомпила 26.28. Verified via gya.h smali (gya.c=new gya(1); packed-switch 0x1 -> a=1 = pswitch_7, inlined). All three keys confirmed.
 *
 * Опкод 145 (BOT_INFO).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_bot_info;

/*
 * Ответ на BotInfo.
 */
typedef struct {
    /* contact */
    kb_raw contact;
    /* startMessage */
    kb_raw start_message;
    /* commands */
    kb_raw commands;
    size_t commands_count;
} kb_bot_info_response;

#define KB_OPCODE_BOT_INFO 145

kb_status kb_encode_bot_info(const kb_bot_info *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_bot_info_response(const uint8_t *body, size_t len,
                       kb_bot_info_response *out);

/*
 * Извлечено из декомпила 26.28. Request g9b(long) c=24 confirmed. Response zl0.h smali a=2 = pswitch_4 building lf1(List,long,boolean). All confirmed.
 *
 * Опкод 163 (CALL_HISTORY).
 */
typedef struct {
    /* callHistorySync */
    int64_t call_history_sync;
} kb_call_history;

/*
 * Ответ на CallHistory.
 */
typedef struct {
    /* reset */
    int reset;
    /* callHistoryItems */
    kb_raw call_history_items;
    size_t call_history_items_count;
    /* callHistorySync */
    int64_t call_history_sync;
} kb_call_history_response;

#define KB_OPCODE_CALL_HISTORY 163

kb_status kb_encode_call_history(const kb_call_history *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_call_history_response(const uint8_t *body, size_t len,
                       kb_call_history_response *out);

/*
 * Извлечено из декомпила 26.28. mq2 ctor c=3 super(x0c.m1)=opcode 49 (x0c.java confirms (short)49). All 12 fields confirmed by literal setters. No response parser.
 *
 * Опкод 49 (CHAT_HISTORY).
 */
typedef struct {
    /* chatId */
    int64_t chat_id;
    /* postId */
    int64_t post_id;
    int has_post_id;
    /* from */
    int64_t from;
    /* forward */
    int32_t forward;
    /* forwardTime */
    int64_t forward_time;
    /* backward */
    int32_t backward;
    /* backwardTime */
    int64_t backward_time;
    /* getChat */
    int get_chat;
    /* getMessages */
    int get_messages;
    /* chatAccessToken */
    kb_str chat_access_token;
    int has_chat_access_token;
    /* itemType */
    kb_str item_type;
    /* interactive */
    int interactive;
} kb_chat_history;

/*
 * Ответ на ChatHistory.
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_chat_history_response;

#define KB_OPCODE_CHAT_HISTORY 49

kb_status kb_encode_chat_history(const kb_chat_history *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_chat_history_response(const uint8_t *body, size_t len,
                       kb_chat_history_response *out);

/*
 * Извлечено из декомпила 26.28. Builder ctor confirmed (worklist had it null). Only chatIds set. No response parser.
 *
 * Опкод 48 (CHAT_INFO).
 */
typedef struct {
    /* chatIds */
    kb_raw chat_ids;
    size_t chat_ids_count;
} kb_chat_info;

/*
 * Ответ на ChatInfo.
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_chat_info_response;

#define KB_OPCODE_CHAT_INFO 48

kb_status kb_encode_chat_info(const kb_chat_info *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_chat_info_response(const uint8_t *body, size_t len,
                       kb_chat_info_response *out);

/*
 * Извлечено из декомпила 26.28. Verified: mx4 case 2 reads only 'chat'. No request builder.
 *
 * Опкод 57 (CHAT_JOIN).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_chat_join;

/*
 * Ответ на ChatJoin.
 */
typedef struct {
    /* chat */
    kb_raw chat;
    int has_chat;
} kb_chat_join_response;

#define KB_OPCODE_CHAT_JOIN 57

kb_status kb_encode_chat_join(const kb_chat_join *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_chat_join_response(const uint8_t *body, size_t len,
                       kb_chat_join_response *out);

/*
 * Извлечено из декомпила 26.28. Verified: gia case 2 reads only 'liveStreams'. No request builder.
 *
 * Опкод 62 (CHAT_LIVESTREAM_INFO).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_chat_livestream_info;

/*
 * Ответ на ChatLivestreamInfo.
 */
typedef struct {
    /* liveStreams */
    kb_raw live_streams;
    size_t live_streams_count;
} kb_chat_livestream_info_response;

#define KB_OPCODE_CHAT_LIVESTREAM_INFO 62

kb_status kb_encode_chat_livestream_info(const kb_chat_livestream_info *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_chat_livestream_info_response(const uint8_t *body, size_t len,
                       kb_chat_livestream_info_response *out);

/*
 * Извлечено из декомпила 26.28. No request_builder in worklist. Response field confirmed via smali. Unchanged.
 *
 * Опкод 50 (CHAT_MARK).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_chat_mark;

/*
 * Ответ на ChatMark.
 */
typedef struct {
    /* timestamp */
    int64_t timestamp;
} kb_chat_mark_response;

#define KB_OPCODE_CHAT_MARK 50

kb_status kb_encode_chat_mark(const kb_chat_mark *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_chat_mark_response(const uint8_t *body, size_t len,
                       kb_chat_mark_response *out);

/*
 * Извлечено из декомпила 26.28. mq2 ctor c=9 super(x0c.o1)=opcode 51 (x0c.java confirms (short)51). All fields confirmed. No response parser.
 *
 * Опкод 51 (CHAT_MEDIA).
 */
typedef struct {
    /* chatId */
    int64_t chat_id;
    /* messageId */
    int64_t message_id;
    /* attachTypes */
    kb_raw attach_types;
    size_t attach_types_count;
    int has_attach_types;
    /* forward */
    int32_t forward;
    int has_forward;
    /* backward */
    int32_t backward;
    int has_backward;
} kb_chat_media;

/*
 * Ответ на ChatMedia.
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_chat_media_response;

#define KB_OPCODE_CHAT_MEDIA 51

kb_status kb_encode_chat_media(const kb_chat_media *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_chat_media_response(const uint8_t *body, size_t len,
                       kb_chat_media_response *out);

/*
 * Извлечено из декомпила 26.28. Request verified against mq2 c=10 ctor; both response keys literally compared in bya case 2. Unchanged from draft.
 *
 * Опкод 59 (CHAT_MEMBERS).
 */
typedef struct {
    /* chatId */
    int64_t chat_id;
    /* type */
    kb_str type;
    int has_type;
    /* marker */
    int64_t marker;
    int has_marker;
    /* count */
    int32_t count;
    int has_count;
    /* query */
    kb_str query;
    int has_query;
} kb_chat_members;

/*
 * Ответ на ChatMembers.
 */
typedef struct {
    /* members */
    kb_raw members;
    size_t members_count;
    /* marker */
    int64_t marker;
} kb_chat_members_response;

#define KB_OPCODE_CHAT_MEMBERS 59

kb_status kb_encode_chat_members(const kb_chat_members *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_chat_members_response(const uint8_t *body, size_t len,
                       kb_chat_members_response *out);

/*
 * Извлечено из декомпила 26.28. cya.d=new cya(2). Dispatch cya.h(w2a) is 'Method not decompiled'; helpers m/n also not decompiled. Branch unreadable => empty.
 *
 * Опкод 61 (CHAT_PERSONAL_CONFIG).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_chat_personal_config;

/*
 * Ответ на ChatPersonalConfig.
 */
typedef struct {
    /* chat */
    kb_raw chat;
} kb_chat_personal_config_response;

#define KB_OPCODE_CHAT_PERSONAL_CONFIG 61

kb_status kb_encode_chat_personal_config(const kb_chat_personal_config *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_chat_personal_config_response(const uint8_t *body, size_t len,
                       kb_chat_personal_config_response *out);

/*
 * Извлечено из декомпила 26.28. Verified key 'chatReactionsSettings' and return new t53 in fya case-2 branch. Kept.
 *
 * Опкод 257 (CHAT_REACTIONS_SETTINGS_SET).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_chat_reactions_settings_set;

/*
 * Ответ на ChatReactionsSettingsSet.
 */
typedef struct {
    /* chatReactionsSettings */
    kb_raw chat_reactions_settings;
    int has_chat_reactions_settings;
} kb_chat_reactions_settings_set_response;

#define KB_OPCODE_CHAT_REACTIONS_SETTINGS_SET 257

kb_status kb_encode_chat_reactions_settings_set(const kb_chat_reactions_settings_set *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_chat_reactions_settings_set_response(const uint8_t *body, size_t len,
                       kb_chat_reactions_settings_set_response *out);

/*
 * Извлечено из декомпила 26.28. Builder xw(String,long[]) c=2 confirmed. Response gya.d (N=2) dispatch not decompiled -> empty.
 *
 * Опкод 300 (CHAT_SUGGEST).
 */
typedef struct {
    /* folderId */
    kb_str folder_id;
    int has_folder_id;
    /* userChatIds */
    kb_raw user_chat_ids;
    size_t user_chat_ids_count;
    int has_user_chat_ids;
} kb_chat_suggest;

/*
 * Ответ на ChatSuggest.
 */
typedef struct {
    /* chats */
    kb_raw chats;
    size_t chats_count;
} kb_chat_suggest_response;

#define KB_OPCODE_CHAT_SUGGEST 300

kb_status kb_encode_chat_suggest(const kb_chat_suggest *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_chat_suggest_response(const uint8_t *body, size_t len,
                       kb_chat_suggest_response *out);

/*
 * Извлечено из декомпила 26.28. Request mq2(cw3,byte,long[],Long,String,Long) c=20 confirmed. Response iya.h smali a=2 = pswitch_5 reads 'success'. All fields confirmed.
 *
 * Опкод 161 (COMPLAIN).
 */
typedef struct {
    /* typeId */
    int32_t type_id;
    /* reasonId */
    int32_t reason_id;
    /* ids */
    kb_raw ids;
    size_t ids_count;
    /* parentId */
    int64_t parent_id;
    int has_parent_id;
    /* postId */
    int64_t post_id;
    int has_post_id;
    /* details */
    kb_str details;
    int has_details;
} kb_complain;

/*
 * Ответ на Complain.
 */
typedef struct {
    /* success */
    int success;
} kb_complain_response;

#define KB_OPCODE_COMPLAIN 161

kb_status kb_encode_complain(const kb_complain *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_complain_response(const uint8_t *body, size_t len,
                       kb_complain_response *out);

/*
 * Извлечено из декомпила 26.28. Verified via dfk.h jadx case 2 (dfk.c=new dfk(2)). Both keys confirmed; builds bw3(long,List).
 *
 * Опкод 162 (COMPLAIN_REASONS_GET).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_complain_reasons_get;

/*
 * Ответ на ComplainReasonsGet.
 */
typedef struct {
    /* complainSync */
    int64_t complain_sync;
    /* complains */
    kb_raw complains;
    size_t complains_count;
} kb_complain_reasons_get_response;

#define KB_OPCODE_COMPLAIN_REASONS_GET 162

kb_status kb_encode_complain_reasons_get(const kb_complain_reasons_get *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_complain_reasons_get_response(const uint8_t *body, size_t len,
                       kb_complain_reasons_get_response *out);

/*
 * Извлечено из декомпила 26.28. Verified mq2.java factory constructor with this.c==21 (super(x0c.r)); all 5 request keys confirmed. Response gl3.d=new gl3(3); dispatch gl3.h case 3 -> gl3.i(w2a) reads keys 'hash' and 'user'. Confirmed.
 *
 * Опкод 22 (CONFIG).
 */
typedef struct {
    /* pushToken */
    kb_str push_token;
    int has_push_token;
    /* pushTokens */
    kb_raw push_tokens;
    size_t push_tokens_count;
    int has_push_tokens;
    /* pushOptions */
    int64_t push_options;
    int has_push_options;
    /* settings */
    kb_raw settings;
    size_t settings_count;
    int has_settings;
    /* reset */
    int reset;
    int has_reset;
} kb_config;

/*
 * Ответ на Config.
 */
typedef struct {
    /* hash */
    kb_str hash;
    int has_hash;
    /* user */
    kb_raw user;
    int has_user;
} kb_config_response;

#define KB_OPCODE_CONFIG 22

kb_status kb_encode_config(const kb_config *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_config_response(const uint8_t *body, size_t len,
                       kb_config_response *out);

/*
 * Извлечено из декомпила 26.28. Both request fields and the response field confirmed by literal setters/key comparisons. No changes.
 *
 * Опкод 32 (CONTACT_INFO).
 */
typedef struct {
    /* contactIds */
    kb_raw contact_ids;
    size_t contact_ids_count;
    /* chat_id */
    int64_t chat_id;
    int has_chat_id;
} kb_contact_info;

/*
 * Ответ на ContactInfo.
 */
typedef struct {
    /* contacts */
    kb_raw contacts;
    size_t contacts_count;
    int has_contacts;
} kb_contact_info_response;

#define KB_OPCODE_CONTACT_INFO 32

kb_status kb_encode_contact_info(const kb_contact_info *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_contact_info_response(const uint8_t *body, size_t len,
                       kb_contact_info_response *out);

/*
 * Извлечено из декомпила 26.28. No request_builder in worklist. Response field confirmed. Unchanged.
 *
 * Опкод 46 (CONTACT_INFO_BY_PHONE).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_contact_info_by_phone;

/*
 * Ответ на ContactInfoByPhone.
 */
typedef struct {
    /* contact */
    kb_raw contact;
    int has_contact;
} kb_contact_info_by_phone_response;

#define KB_OPCODE_CONTACT_INFO_BY_PHONE 46

kb_status kb_encode_contact_info_by_phone(const kb_contact_info_by_phone *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_contact_info_by_phone_response(const uint8_t *body, size_t len,
                       kb_contact_info_by_phone_response *out);

/*
 * Извлечено из декомпила 26.28. Request builder xw.java empty ctor xw(){super(x0c.e1);c=4;} sets no fields -> empty request. Response keys confirmed in bya g().
 *
 * Опкод 35 (CONTACT_PRESENCE).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_contact_presence;

/*
 * Ответ на ContactPresence.
 */
typedef struct {
    /* presence */
    kb_raw presence;
    int has_presence;
    /* time */
    int64_t time;
} kb_contact_presence_response;

#define KB_OPCODE_CONTACT_PRESENCE 35

kb_status kb_encode_contact_presence(const kb_contact_presence *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_contact_presence_response(const uint8_t *body, size_t len,
                       kb_contact_presence_response *out);

/*
 * Извлечено из декомпила 26.28. No request builder -> empty map. Verified bya.f=new bya(4) -> i(w2a) decompiled; only keys 'botId','startParam'.
 *
 * Опкод 105 (EXTERNAL_CALLBACK).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_external_callback;

/*
 * Ответ на ExternalCallback.
 */
typedef struct {
    /* botId */
    int64_t bot_id;
    int has_bot_id;
    /* startParam */
    kb_str start_param;
    int has_start_param;
} kb_external_callback_response;

#define KB_OPCODE_EXTERNAL_CALLBACK 105

kb_status kb_encode_external_callback(const kb_external_callback *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_external_callback_response(const uint8_t *body, size_t len,
                       kb_external_callback_response *out);

/*
 * Извлечено из декомпила 26.28. Request verified against mq2 c=28 ctor (fileId, chatId, messageId all i64). RESPONSE DROPPED vs draft: cya.f=new cya(4) but dispatch cya.h(w2a) is 'Method not decompiled', so N=4 branch unreadable. Draft's url/unsafe came from helper l() (returns ld6) which cannot be tied to N=4 by any decompiled code => empty per prefer-dropping-over-guessing.
 *
 * Опкод 88 (FILE_DOWNLOAD).
 */
typedef struct {
    /* fileId */
    int64_t file_id;
    /* chatId */
    int64_t chat_id;
    /* messageId */
    int64_t message_id;
} kb_file_download;

/*
 * Ответ на FileDownload.
 */
typedef struct {
    /* url */
    kb_str url;
    /* unsafe */
    int unsafe;
    int has_unsafe;
} kb_file_download_response;

#define KB_OPCODE_FILE_DOWNLOAD 88

kb_status kb_encode_file_download(const kb_file_download *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_file_download_response(const uint8_t *body, size_t len,
                       kb_file_download_response *out);

/*
 * Извлечено из декомпила 26.28. Builder wr6 and response fya case4 both confirmed on-path.
 *
 * Опкод 276 (FOLDERS_DELETE).
 */
typedef struct {
    /* folderIds */
    kb_raw folder_ids;
    size_t folder_ids_count;
} kb_folders_delete;

/*
 * Ответ на FoldersDelete.
 */
typedef struct {
    /* folderSync */
    int64_t folder_sync;
    int has_folder_sync;
} kb_folders_delete_response;

#define KB_OPCODE_FOLDERS_DELETE 276

kb_status kb_encode_folders_delete(const kb_folders_delete *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_folders_delete_response(const uint8_t *body, size_t len,
                       kb_folders_delete_response *out);

/*
 * Извлечено из декомпила 26.28. Verified request: zr6 sets folderSync via f(long,String) => i64. Response parser iya.g => new iya(4); iya.h(w2a) is 'Method not decompiled', N=4 branch unreadable; response empty.
 *
 * Опкод 272 (FOLDERS_GET).
 */
typedef struct {
    /* folderSync */
    int64_t folder_sync;
} kb_folders_get;

/*
 * Ответ на FoldersGet.
 */
typedef struct {
    /* folderSync */
    int64_t folder_sync;
    /* folders */
    kb_raw folders;
    size_t folders_count;
    /* foldersOrder */
    kb_raw folders_order;
    size_t folders_order_count;
    /* allFilterExcludeFolders */
    kb_raw all_filter_exclude_folders;
    size_t all_filter_exclude_folders_count;
} kb_folders_get_response;

#define KB_OPCODE_FOLDERS_GET 272

kb_status kb_encode_folders_get(const kb_folders_get *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_folders_get_response(const uint8_t *body, size_t len,
                       kb_folders_get_response *out);

/*
 * Извлечено из декомпила 26.28. No request builder -> empty request. DROPPED 'folders': gya.h(w2a) dispatch is 'Method not decompiled' (UnsupportedOperationException), so the N=4 branch cannot be confirmed to call helper m() which reads 'folders'. Attribution was inferred, not proven -> response emptied.
 *
 * Опкод 273 (FOLDERS_GET_BY_ID).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_folders_get_by_id;

/*
 * Ответ на FoldersGetById.
 */
typedef struct {
    /* folders */
    kb_raw folders;
    size_t folders_count;
} kb_folders_get_by_id_response;

#define KB_OPCODE_FOLDERS_GET_BY_ID 273

kb_status kb_encode_folders_get_by_id(const kb_folders_get_by_id *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_folders_get_by_id_response(const uint8_t *body, size_t len,
                       kb_folders_get_by_id_response *out);

/*
 * Извлечено из декомпила 26.28. Builder dt6 and response dfk case4 both confirmed on-path.
 *
 * Опкод 275 (FOLDERS_REORDER).
 */
typedef struct {
    /* foldersOrder */
    kb_raw folders_order;
    size_t folders_order_count;
} kb_folders_reorder;

/*
 * Ответ на FoldersReorder.
 */
typedef struct {
    /* folderSync */
    int64_t folder_sync;
    int has_folder_sync;
} kb_folders_reorder_response;

#define KB_OPCODE_FOLDERS_REORDER 275

kb_status kb_encode_folders_reorder(const kb_folders_reorder *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_folders_reorder_response(const uint8_t *body, size_t len,
                       kb_folders_reorder_response *out);

/*
 * Извлечено из декомпила 26.28. Builder kt6 verified (all 6 keys confirmed, types match). Response parser zl0.g (N=5) dispatch not decompiled; no fields confirmable -> empty.
 *
 * Опкод 274 (FOLDERS_UPDATE).
 */
typedef struct {
    /* id */
    kb_str id;
    /* title */
    kb_str title;
    /* include */
    kb_raw include;
    size_t include_count;
    int has_include;
    /* favorites */
    kb_raw favorites;
    size_t favorites_count;
    int has_favorites;
    /* filters */
    kb_raw filters;
    size_t filters_count;
    int has_filters;
    /* options */
    kb_raw options;
    size_t options_count;
    int has_options;
} kb_folders_update;

/*
 * Ответ на FoldersUpdate.
 */
typedef struct {
    /* folder */
    kb_raw folder;
    /* folderSync */
    int64_t folder_sync;
    /* foldersOrder */
    kb_raw folders_order;
    size_t folders_order_count;
} kb_folders_update_response;

#define KB_OPCODE_FOLDERS_UPDATE 274

kb_status kb_encode_folders_update(const kb_folders_update *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_folders_update_response(const uint8_t *body, size_t len,
                       kb_folders_update_response *out);

/*
 * Извлечено из декомпила 26.28. No request builder -> empty map. Verified ogc.g=new ogc(5). CAVEAT: ogc.h(w2a) dispatch is dump-skipped so l()->N=5 mapping is unconfirmed; kept the field because l() is the only readable parser and its key matches this opcode's semantics.
 *
 * Опкод 91 (GET_COMMENTS_UPDATES).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_get_comments_updates;

/*
 * Ответ на GetCommentsUpdates.
 */
typedef struct {
    /* commentsInfoUpdates */
    kb_raw comments_info_updates;
    size_t comments_info_updates_count;
    int has_comments_info_updates;
} kb_get_comments_updates_response;

#define KB_OPCODE_GET_COMMENTS_UPDATES 91

kb_status kb_encode_get_comments_updates(const kb_get_comments_updates *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_get_comments_updates_response(const uint8_t *body, size_t len,
                       kb_get_comments_updates_response *out);

/*
 * Извлечено из декомпила 26.28. Builder gla and response dfk case5->i() both confirmed on-path.
 *
 * Опкод 306 (GET_POLL_UPDATES).
 */
typedef struct {
    /* chatId */
    int64_t chat_id;
    /* polls */
    kb_raw polls;
    size_t polls_count;
} kb_get_poll_updates;

/*
 * Ответ на GetPollUpdates.
 */
typedef struct {
    /* polls */
    kb_raw polls;
    size_t polls_count;
    int has_polls;
} kb_get_poll_updates_response;

#define KB_OPCODE_GET_POLL_UPDATES 306

kb_status kb_encode_get_poll_updates(const kb_get_poll_updates *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_get_poll_updates_response(const uint8_t *body, size_t len,
                       kb_get_poll_updates_response *out);

/*
 * Вход по login-токену: авторизует уже установленное соединение и отдаёт
 * чаты, контакты и конфиг.
 *
 * `presenceSync`/`chatsSync` = -1 означает «отдай всё с нуля»; при инкрементальной
 * синхронизации туда идут метки времени из прошлого ответа.
 *
 * Опкод 19 (LOGIN).
 */
typedef struct {
    /* token */
    kb_str token;
    /* interactive */
    int interactive;
    /* exp */
    kb_raw exp;
    int has_exp;
    /* chatCacheFingerprint */
    kb_bin chat_cache_fingerprint;
    int has_chat_cache_fingerprint;
    /* presenceSync */
    int64_t presence_sync;
    int has_presence_sync;
    /* chatsSync */
    int64_t chats_sync;
    int has_chats_sync;
    /* contactsSync */
    int64_t contacts_sync;
    int has_contacts_sync;
    /* callsSync */
    int64_t calls_sync;
    int has_calls_sync;
    /* lastLogin */
    int64_t last_login;
    int has_last_login;
    /* bannersSync */
    int64_t banners_sync;
    int has_banners_sync;
    /* configHash */
    kb_str config_hash;
    int has_config_hash;
} kb_login;

/*
 * Ответ на Login.
 */
typedef struct {
    /* token */
    kb_str token;
    int has_token;
    /* profile */
    kb_raw profile;
    int has_profile;
    /* chats */
    kb_raw chats;
    size_t chats_count;
    int has_chats;
    /* time */
    int64_t time;
    int has_time;
    /* config */
    kb_raw config;
    int has_config;
    /* contacts */
    kb_raw contacts;
    int has_contacts;
    /* presence */
    kb_raw presence;
    int has_presence;
} kb_login_response;

#define KB_OPCODE_LOGIN 19

kb_status kb_encode_login(const kb_login *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_login_response(const uint8_t *body, size_t len,
                       kb_login_response *out);

/*
 * Разлогинить соединение. Payload — пустая map, как в референсном клиенте.
 *
 * Опкод 20 (LOGOUT).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_logout;

/*
 * Ответ на Logout.
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_logout_response;

#define KB_OPCODE_LOGOUT 20

kb_status kb_encode_logout(const kb_logout *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_logout_response(const uint8_t *body, size_t len,
                       kb_logout_response *out);

/*
 * Извлечено из декомпила 26.28. Request verified (d77 c=8). RESPONSE DROPPED vs draft: iya.h=new iya(5) but dispatch iya.h(w2a) is 'Method not decompiled', so the N=5 branch is unreadable. Draft's chatId/messages came from private helper d() (returns cla) which cannot be tied to N=5 by any decompiled code => empty per prefer-dropping-over-guessing.
 *
 * Опкод 71 (MSG_GET).
 */
typedef struct {
    /* chatId */
    int64_t chat_id;
    /* messageIds */
    kb_raw message_ids;
    size_t message_ids_count;
} kb_msg_get;

/*
 * Ответ на MsgGet.
 */
typedef struct {
    /* chatId */
    int64_t chat_id;
    /* messages */
    kb_raw messages;
    size_t messages_count;
} kb_msg_get_response;

#define KB_OPCODE_MSG_GET 71

kb_status kb_encode_msg_get(const kb_msg_get *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_msg_get_response(const uint8_t *body, size_t len,
                       kb_msg_get_response *out);

/*
 * Извлечено из декомпила 26.28. Verified via zl0.n smali (zl0.h=new zl0(6); a=6 = pswitch_2 -> n(w2a)). Single map key confirmed.
 *
 * Опкод 180 (MSG_GET_REACTIONS).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_msg_get_reactions;

/*
 * Ответ на MsgGetReactions.
 */
typedef struct {
    /* messagesReactions */
    kb_raw messages_reactions;
    size_t messages_reactions_count;
} kb_msg_get_reactions_response;

#define KB_OPCODE_MSG_GET_REACTIONS 180

kb_status kb_encode_msg_get_reactions(const kb_msg_get_reactions *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_msg_get_reactions_response(const uint8_t *body, size_t len,
                       kb_msg_get_reactions_response *out);

/*
 * Отправить сообщение в чат.
 *
 * `message.cid` — клиентский идентификатор, по нему приходит подтверждение;
 * референсный клиент кладёт туда отрицательный timestamp в миллисекундах.
 *
 * Опкод 64 (MSG_SEND).
 */
typedef struct {
    /* chatId */
    int64_t chat_id;
    /* message */
    kb_raw message;
    /* notify */
    int notify;
    int has_notify;
} kb_msg_send;

/*
 * Ответ на MsgSend.
 */
typedef struct {
    /* chatId */
    int64_t chat_id;
    int has_chat_id;
    /* message */
    kb_raw message;
    int has_message;
    /* unread */
    int64_t unread;
    int has_unread;
    /* mark */
    int64_t mark;
    int has_mark;
} kb_msg_send_response;

#define KB_OPCODE_MSG_SEND 64

kb_status kb_encode_msg_send(const kb_msg_send *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_msg_send_response(const uint8_t *body, size_t len,
                       kb_msg_send_response *out);

/*
 * Индикатор набора текста.
 *
 * Опкод 65 (MSG_TYPING).
 */
typedef struct {
    /* chatId */
    int64_t chat_id;
    /* type */
    kb_str type;
} kb_msg_typing;

/*
 * Ответ на MsgTyping.
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_msg_typing_response;

#define KB_OPCODE_MSG_TYPING 65

kb_status kb_encode_msg_typing(const kb_msg_typing *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_msg_typing_response(const uint8_t *body, size_t len,
                       kb_msg_typing_response *out);

/*
 * Извлечено из декомпила 26.28. Verified via bya.k smali (bya.h=new bya(6); switch case 6 -> k). Builds sza(JJ,rza,List,[J). All five keys confirmed.
 *
 * Опкод 165 (NOTIF_CALL_HISTORY).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_notif_call_history;

/*
 * Ответ на NotifCallHistory.
 */
typedef struct {
    /* historyIds */
    kb_raw history_ids;
    size_t history_ids_count;
    /* callHistoryItems */
    kb_raw call_history_items;
    size_t call_history_items_count;
    /* prevCallHistorySync */
    int64_t prev_call_history_sync;
    /* callHistorySync */
    int64_t call_history_sync;
    /* action */
    kb_str action;
} kb_notif_call_history_response;

#define KB_OPCODE_NOTIF_CALL_HISTORY 165

kb_status kb_encode_notif_call_history(const kb_notif_call_history *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_notif_call_history_response(const uint8_t *body, size_t len,
                       kb_notif_call_history_response *out);

/*
 * Извлечено из декомпила 26.28. No request builder (notification). Response parser cya.h (N=6) dispatch not decompiled; no fields confirmable.
 *
 * Опкод 277 (NOTIF_FOLDERS).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_notif_folders;

/*
 * Ответ на NotifFolders.
 */
typedef struct {
    /* folderSync */
    int64_t folder_sync;
    /* folders */
    kb_raw folders;
    size_t folders_count;
    /* foldersOrder */
    kb_raw folders_order;
    size_t folders_order_count;
    /* allFilterExcludeFolders */
    kb_raw all_filter_exclude_folders;
    size_t all_filter_exclude_folders_count;
} kb_notif_folders_response;

#define KB_OPCODE_NOTIF_FOLDERS 277

kb_status kb_encode_notif_folders(const kb_notif_folders *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_notif_folders_response(const uint8_t *body, size_t len,
                       kb_notif_folders_response *out);

/*
 * Извлечено из декомпила 26.28. Verified via gya.n smali (a=6 = pswitch_3 -> n(w2a)). Builds p0b. All six keys confirmed with correct types.
 *
 * Опкод 154 (NOTIF_MSG_DELAYED).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_notif_msg_delayed;

/*
 * Ответ на NotifMsgDelayed.
 */
typedef struct {
    /* message */
    kb_raw message;
    int has_message;
    /* userId */
    int64_t user_id;
    /* updateTypeId */
    int32_t update_type_id;
    /* lastDelayedUpdateTime */
    int64_t last_delayed_update_time;
    int has_last_delayed_update_time;
    /* chatId */
    int64_t chat_id;
    /* messageIds */
    kb_raw message_ids;
    size_t message_ids_count;
} kb_notif_msg_delayed_response;

#define KB_OPCODE_NOTIF_MSG_DELAYED 154

kb_status kb_encode_notif_msg_delayed(const kb_notif_msg_delayed *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_notif_msg_delayed_response(const uint8_t *body, size_t len,
                       kb_notif_msg_delayed_response *out);

/*
 * Извлечено из декомпила 26.28. Verified via iya.h smali (iya.i=new iya(6); packed-switch 0x1 -> a=6 = pswitch_2). Builds t0b(nm2,[J,Z). All three keys confirmed; nothing dropped.
 *
 * Опкод 142 (NOTIF_MSG_DELETE).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_notif_msg_delete;

/*
 * Ответ на NotifMsgDelete.
 */
typedef struct {
    /* chat */
    kb_raw chat;
    /* ttl */
    int ttl;
    /* messageIds */
    kb_raw message_ids;
    size_t message_ids_count;
} kb_notif_msg_delete_response;

#define KB_OPCODE_NOTIF_MSG_DELETE 142

kb_status kb_encode_notif_msg_delete(const kb_notif_msg_delete *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_notif_msg_delete_response(const uint8_t *body, size_t len,
                       kb_notif_msg_delete_response *out);

/*
 * Извлечено из декомпила 26.28. Verified via dfk.j jadx (dfk.g=new dfk(6); switch case 6 -> j). Single key confirmed.
 *
 * Опкод 159 (NOTIF_PROFILE).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_notif_profile;

/*
 * Ответ на NotifProfile.
 */
typedef struct {
    /* profile */
    kb_raw profile;
} kb_notif_profile_response;

#define KB_OPCODE_NOTIF_PROFILE 159

kb_status kb_encode_notif_profile(const kb_notif_profile *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_notif_profile_response(const uint8_t *body, size_t len,
                       kb_notif_profile_response *out);

/*
 * Извлечено из декомпила 26.28. Verified: parser zl0.i => new zl0(7); zl0.h(w2a) is 'Method not decompiled'. Branch unreadable. Response empty (confirmed).
 *
 * Опкод 216 (NOTIF_STORIES_UPDATE).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_notif_stories_update;

/*
 * Ответ на NotifStoriesUpdate.
 */
typedef struct {
    /* storiesPreview */
    kb_raw stories_preview;
    int has_stories_preview;
} kb_notif_stories_update_response;

#define KB_OPCODE_NOTIF_STORIES_UPDATE 216

kb_status kb_encode_notif_stories_update(const kb_notif_stories_update *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_notif_stories_update_response(const uint8_t *body, size_t len,
                       kb_notif_stories_update_response *out);

/*
 * Извлечено из декомпила 26.28. Request g9b(String,long) c=0 confirmed. Response zg6.h smali: packed-switch only maps a=1; a=7 falls to default branch reading token/token_refresh_ts/token_lifetime_ts into h9b. 4th long is client nanoTime, not a wire field.
 *
 * Опкод 158 (OK_TOKEN).
 */
typedef struct {
    /* value */
    kb_str value;
    /* userId */
    int64_t user_id;
} kb_ok_token;

/*
 * Ответ на OkToken.
 */
typedef struct {
    /* token */
    kb_str token;
    /* token_refresh_ts */
    int64_t token_refresh_ts;
    /* token_lifetime_ts */
    int64_t token_lifetime_ts;
} kb_ok_token_response;

#define KB_OPCODE_OK_TOKEN 158

kb_status kb_encode_ok_token(const kb_ok_token *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_ok_token_response(const uint8_t *body, size_t len,
                       kb_ok_token_response *out);

/*
 * Извлечено из декомпила 26.28. Verified request key in z2c (rpa confirmed to wrap long[] => [i64]) and response key 'organizations' in tyf case-7 branch. Kept.
 *
 * Опкод 256 (ORG_INFO).
 */
typedef struct {
    /* organizationIds */
    kb_raw organization_ids;
    size_t organization_ids_count;
    int has_organization_ids;
} kb_org_info;

/*
 * Ответ на OrgInfo.
 */
typedef struct {
    /* organizations */
    kb_raw organizations;
    size_t organizations_count;
    int has_organizations;
} kb_org_info_response;

#define KB_OPCODE_ORG_INFO 256

kb_status kb_encode_org_info(const kb_org_info *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_org_info_response(const uint8_t *body, size_t len,
                       kb_org_info_response *out);

/*
 * Извлечено из декомпила 26.28. Request verified against g9b c=1 ctor. Response parser ogc.b=new ogc(0); dispatch ogc.h(w2a) not decompiled => empty.
 *
 * Опкод 80 (PHOTO_UPLOAD).
 */
typedef struct {
    /* type */
    int32_t type;
    int has_type;
    /* count */
    int32_t count;
    /* profile */
    int profile;
    int has_profile;
} kb_photo_upload;

/*
 * Ответ на PhotoUpload.
 */
typedef struct {
    /* url */
    kb_str url;
} kb_photo_upload_response;

#define KB_OPCODE_PHOTO_UPLOAD 80

kb_status kb_encode_photo_upload(const kb_photo_upload *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_photo_upload_response(const uint8_t *body, size_t len,
                       kb_photo_upload_response *out);

/*
 * Извлечено из декомпила 26.28. Verified via gl3.j jadx (gl3.h=new gl3(7); case 7 -> j). Single key confirmed.
 *
 * Опкод 203 (PHOTO_URL_REFRESH).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_photo_url_refresh;

/*
 * Ответ на PhotoUrlRefresh.
 */
typedef struct {
    /* media */
    kb_raw media;
    size_t media_count;
} kb_photo_url_refresh_response;

#define KB_OPCODE_PHOTO_URL_REFRESH 203

kb_status kb_encode_photo_url_refresh(const kb_photo_url_refresh *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_photo_url_refresh_response(const uint8_t *body, size_t len,
                       kb_photo_url_refresh_response *out);

/*
 * Извлечено из декомпила 26.28. No request builder (empty). Response bya.i=new bya(7); dispatch bya.h case 7 -> bya.l(w2a) reads 'presetAvatars'. Confirmed.
 *
 * Опкод 25 (PRESET_AVATARS).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_preset_avatars;

/*
 * Ответ на PresetAvatars.
 */
typedef struct {
    /* presetAvatars */
    kb_raw preset_avatars;
    size_t preset_avatars_count;
} kb_preset_avatars_response;

#define KB_OPCODE_PRESET_AVATARS 25

kb_status kb_encode_preset_avatars(const kb_preset_avatars *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_preset_avatars_response(const uint8_t *body, size_t len,
                       kb_preset_avatars_response *out);

/*
 * Извлечено из декомпила 26.28. Request d77(boolean,int) c=23 confirmed. Response fya.l jadx (fya.i=new fya(7); case 7 -> l). Confirmed.
 *
 * Опкод 199 (PROFILE_DELETE).
 */
typedef struct {
    /* delete */
    int delete_;
    /* type */
    int32_t type;
    int has_type;
} kb_profile_delete;

/*
 * Ответ на ProfileDelete.
 */
typedef struct {
    /* timestamp */
    int64_t timestamp;
} kb_profile_delete_response;

#define KB_OPCODE_PROFILE_DELETE 199

kb_status kb_encode_profile_delete(const kb_profile_delete *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_profile_delete_response(const uint8_t *body, size_t len,
                       kb_profile_delete_response *out);

/*
 * Извлечено из декомпила 26.28. Request d77() c=24 sets no fields => empty request. Response gya.o smali (gya.i=new gya(7); a=7 = pswitch_2 -> o(w2a)). Single key confirmed.
 *
 * Опкод 200 (PROFILE_DELETE_TIME).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_profile_delete_time;

/*
 * Ответ на ProfileDeleteTime.
 */
typedef struct {
    /* timestamp */
    int64_t timestamp;
} kb_profile_delete_time_response;

#define KB_OPCODE_PROFILE_DELETE_TIME 200

kb_status kb_encode_profile_delete_time(const kb_profile_delete_time *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_profile_delete_time_response(const uint8_t *body, size_t len,
                       kb_profile_delete_time_response *out);

/*
 * Извлечено из декомпила 26.28. Verified key 'chatReactionsSettings' and return new or2 in gl3 case-2 branch. Kept.
 *
 * Опкод 258 (REACTIONS_SETTINGS_GET_BY_CHAT_ID).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_reactions_settings_get_by_chat_id;

/*
 * Ответ на ReactionsSettingsGetByChatId.
 */
typedef struct {
    /* chatReactionsSettings */
    kb_raw chat_reactions_settings;
    size_t chat_reactions_settings_count;
    int has_chat_reactions_settings;
} kb_reactions_settings_get_by_chat_id_response;

#define KB_OPCODE_REACTIONS_SETTINGS_GET_BY_CHAT_ID 258

kb_status kb_encode_reactions_settings_get_by_chat_id(const kb_reactions_settings_get_by_chat_id *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_reactions_settings_get_by_chat_id_response(const uint8_t *body, size_t len,
                       kb_reactions_settings_get_by_chat_id_response *out);

/*
 * Извлечено из декомпила 26.28. Builder luc and response gia case7->m() both confirmed on-path.
 *
 * Опкод 304 (SEND_VOTE).
 */
typedef struct {
    /* chatId */
    int64_t chat_id;
    /* pollId */
    int64_t poll_id;
    /* messageId */
    int64_t message_id;
    /* answersIds */
    kb_raw answers_ids;
    size_t answers_ids_count;
} kb_send_vote;

/*
 * Ответ на SendVote.
 */
typedef struct {
    /* state */
    kb_raw state;
    int has_state;
} kb_send_vote_response;

#define KB_OPCODE_SEND_VOTE 304

kb_status kb_encode_send_vote(const kb_send_vote *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_send_vote_response(const uint8_t *body, size_t len,
                       kb_send_vote_response *out);

/*
 * Извлечено из декомпила 26.28. Verified key 'storiesPreview' and return new phg(storiesPreview) in tyf N=8 default branch. Kept.
 *
 * Опкод 218 (STORIES_DELETE).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_stories_delete;

/*
 * Ответ на StoriesDelete.
 */
typedef struct {
    /* storiesPreview */
    kb_raw stories_preview;
    int has_stories_preview;
} kb_stories_delete_response;

#define KB_OPCODE_STORIES_DELETE 218

kb_status kb_encode_stories_delete(const kb_stories_delete *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_stories_delete_response(const uint8_t *body, size_t len,
                       kb_stories_delete_response *out);

/*
 * Извлечено из декомпила 26.28. Verified key 'story' and return new uhg(story) in gl3 N=8 default branch. Kept.
 *
 * Опкод 217 (STORIES_EDIT).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_stories_edit;

/*
 * Ответ на StoriesEdit.
 */
typedef struct {
    /* story */
    kb_raw story;
    int has_story;
} kb_stories_edit_response;

#define KB_OPCODE_STORIES_EDIT 217

kb_status kb_encode_stories_edit(const kb_stories_edit *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_stories_edit_response(const uint8_t *body, size_t len,
                       kb_stories_edit_response *out);

/*
 * Извлечено из декомпила 26.28. В клиенте опкод назван STORIES_GET_BY_OWNER_ID. Verified both keys and the return new whg(peerStories, storiesPreviews) in the mx4 N=8 default branch. Kept as-is.
 *
 * Опкод 210 (STORIES_GET_BY_OWNER).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_stories_get_by_owner;

/*
 * Ответ на StoriesGetByOwner.
 */
typedef struct {
    /* peerStories */
    kb_raw peer_stories;
    size_t peer_stories_count;
    int has_peer_stories;
    /* storiesPreviews */
    kb_raw stories_previews;
    size_t stories_previews_count;
    int has_stories_previews;
} kb_stories_get_by_owner_response;

#define KB_OPCODE_STORIES_GET_BY_OWNER 210

kb_status kb_encode_stories_get_by_owner(const kb_stories_get_by_owner *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_stories_get_by_owner_response(const uint8_t *body, size_t len,
                       kb_stories_get_by_owner_response *out);

/*
 * Извлечено из декомпила 26.28. Verified key 'stories' and return new xhg(stories) in gia N=8 default branch. Kept.
 *
 * Опкод 220 (STORIES_GET_BY_STORY_ID).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_stories_get_by_story_id;

/*
 * Ответ на StoriesGetByStoryId.
 */
typedef struct {
    /* stories */
    kb_raw stories;
    size_t stories_count;
    int has_stories;
} kb_stories_get_by_story_id_response;

#define KB_OPCODE_STORIES_GET_BY_STORY_ID 220

kb_status kb_encode_stories_get_by_story_id(const kb_stories_get_by_story_id *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_stories_get_by_story_id_response(const uint8_t *body, size_t len,
                       kb_stories_get_by_story_id_response *out);

/*
 * Извлечено из декомпила 26.28. Verified key 'storyStats' and return new zhg(storyStats) in bya N=8 default branch. Kept.
 *
 * Опкод 211 (STORIES_GET_STATS).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_stories_get_stats;

/*
 * Ответ на StoriesGetStats.
 */
typedef struct {
    /* storyStats */
    kb_raw story_stats;
    size_t story_stats_count;
    int has_story_stats;
} kb_stories_get_stats_response;

#define KB_OPCODE_STORIES_GET_STATS 211

kb_status kb_encode_stories_get_stats(const kb_stories_get_stats *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_stories_get_stats_response(const uint8_t *body, size_t len,
                       kb_stories_get_stats_response *out);

/*
 * Извлечено из декомпила 26.28. Verified via fya.m jadx (fya.j=new fya(8); case 8 -> m) building lig(zpa,String). Both keys confirmed.
 *
 * Опкод 208 (STORIES_LIST).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_stories_list;

/*
 * Ответ на StoriesList.
 */
typedef struct {
    /* storiesPreviews */
    kb_raw stories_previews;
    size_t stories_previews_count;
    /* cursor */
    kb_str cursor;
} kb_stories_list_response;

#define KB_OPCODE_STORIES_LIST 208

kb_status kb_encode_stories_list(const kb_stories_list *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_stories_list_response(const uint8_t *body, size_t len,
                       kb_stories_list_response *out);

/*
 * Извлечено из декомпила 26.28. Verified: parser gya.j => new gya(8); gya.h(w2a) is 'Method not decompiled'. Branch unreadable. Response empty (confirmed).
 *
 * Опкод 214 (STORIES_MARK).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_stories_mark;

/*
 * Ответ на StoriesMark.
 */
typedef struct {
    /* success */
    int success;
} kb_stories_mark_response;

#define KB_OPCODE_STORIES_MARK 214

kb_status kb_encode_stories_mark(const kb_stories_mark *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_stories_mark_response(const uint8_t *body, size_t len,
                       kb_stories_mark_response *out);

/*
 * Извлечено из декомпила 26.28. Verified: parser iya.k => new iya(8); iya.h(w2a) is 'Method not decompiled'. Branch unreadable. Response empty (confirmed).
 *
 * Опкод 213 (STORIES_REACT).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_stories_react;

/*
 * Ответ на StoriesReact.
 */
typedef struct {
    /* success */
    int success;
} kb_stories_react_response;

#define KB_OPCODE_STORIES_REACT 213

kb_status kb_encode_stories_react(const kb_stories_react *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_stories_react_response(const uint8_t *body, size_t len,
                       kb_stories_react_response *out);

/*
 * Извлечено из декомпила 26.28. Verified both keys and return new ojg(stories, storiesPreview) in dfk N=8 default branch. Kept.
 *
 * Опкод 215 (STORIES_SEND).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_stories_send;

/*
 * Ответ на StoriesSend.
 */
typedef struct {
    /* stories */
    kb_raw stories;
    size_t stories_count;
    int has_stories;
    /* storiesPreview */
    kb_raw stories_preview;
    int has_stories_preview;
} kb_stories_send_response;

#define KB_OPCODE_STORIES_SEND 215

kb_status kb_encode_stories_send(const kb_stories_send *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_stories_send_response(const uint8_t *body, size_t len,
                       kb_stories_send_response *out);

/*
 * Извлечено из декомпила 26.28. В клиенте опкод назван NOTIF_TRANSCRIPTION. No request builder (notification). Response parser g1b/vl3.h not decompiled. Unrecoverable.
 *
 * Опкод 293 (TRANSCRIPTION_RESULT).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_transcription_result;

/*
 * Ответ на TranscriptionResult.
 */
typedef struct {
    /* chatId */
    int64_t chat_id;
    /* messageId */
    int64_t message_id;
    /* mediaId */
    int64_t media_id;
    /* transcription */
    kb_str transcription;
    int has_transcription;
    /* transcriptionStatus */
    kb_raw transcription_status;
} kb_transcription_result_response;

#define KB_OPCODE_TRANSCRIPTION_RESULT 293

kb_status kb_encode_transcription_result(const kb_transcription_result *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_transcription_result_response(const uint8_t *body, size_t len,
                       kb_transcription_result_response *out);

/*
 * Извлечено из декомпила 26.28. В клиенте опкод назван CALL_HISTORY_CLEAR. Request g9b(long[]) c=23 confirmed. Response dfk.h jadx case 1 returns kf1.c singleton (empty, no parsed fields).
 *
 * Опкод 164 (VIDEO_CHAT_DELETE_HISTORY).
 */
typedef struct {
    /* historyIds */
    kb_raw history_ids;
    size_t history_ids_count;
    int has_history_ids;
} kb_video_chat_delete_history;

/*
 * Ответ на VideoChatDeleteHistory.
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_video_chat_delete_history_response;

#define KB_OPCODE_VIDEO_CHAT_DELETE_HISTORY 164

kb_status kb_encode_video_chat_delete_history(const kb_video_chat_delete_history *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_video_chat_delete_history_response(const uint8_t *body, size_t len,
                       kb_video_chat_delete_history_response *out);

/*
 * Извлечено из декомпила 26.28. В клиенте опкод назван VIDEO_CHAT_JOIN. Verified via fya.h jadx (fya.k=new fya(9); a=9 not an explicit case -> default branch shared with cases 3/5) building pci(conversationId,internalParams).
 *
 * Опкод 166 (VIDEO_CHAT_JOIN_BY_LINK).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_video_chat_join_by_link;

/*
 * Ответ на VideoChatJoinByLink.
 */
typedef struct {
    /* conversationId */
    kb_str conversation_id;
    /* internalParams */
    kb_str internal_params;
} kb_video_chat_join_by_link_response;

#define KB_OPCODE_VIDEO_CHAT_JOIN_BY_LINK 166

kb_status kb_encode_video_chat_join_by_link(const kb_video_chat_join_by_link *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_video_chat_join_by_link_response(const uint8_t *body, size_t len,
                       kb_video_chat_join_by_link_response *out);

/*
 * Извлечено из декомпила 26.28. ogc.d=new ogc(2); dispatch ogc.h(w2a) 'Method not decompiled', helper k not decompiled. Branch unreadable => empty. No request builder.
 *
 * Опкод 76 (VIDEO_CHAT_START).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_video_chat_start;

/*
 * Ответ на VideoChatStart.
 */
typedef struct {
    /* type */
    kb_str type;
    int has_type;
    /* callerId */
    int64_t caller_id;
    int has_caller_id;
    /* callName */
    kb_str call_name;
    int has_call_name;
    /* chatId */
    int64_t chat_id;
    int has_chat_id;
    /* joinLink */
    kb_str join_link;
    int has_join_link;
    /* conversationId */
    kb_str conversation_id;
    int has_conversation_id;
} kb_video_chat_start_response;

#define KB_OPCODE_VIDEO_CHAT_START 76

kb_status kb_encode_video_chat_start(const kb_video_chat_start *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_video_chat_start_response(const uint8_t *body, size_t len,
                       kb_video_chat_start_response *out);

/*
 * Извлечено из декомпила 26.28. gya.k=new gya(9); dispatch gya.h(w2a) 'Method not decompiled', helper n not decompiled. Readable helpers parse unrelated keys, cannot attribute N=9 => empty. No request builder.
 *
 * Опкод 78 (VIDEO_CHAT_START_ACTIVE).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_video_chat_start_active;

/*
 * Ответ на VideoChatStartActive.
 */
typedef struct {
    /* conversationId */
    kb_str conversation_id;
    int has_conversation_id;
    /* internalCallerParams */
    kb_str internal_caller_params;
    int has_internal_caller_params;
    /* rejectedParticipants */
    kb_raw rejected_participants;
    size_t rejected_participants_count;
} kb_video_chat_start_active_response;

#define KB_OPCODE_VIDEO_CHAT_START_ACTIVE 78

kb_status kb_encode_video_chat_start_active(const kb_video_chat_start_active *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_video_chat_start_active_response(const uint8_t *body, size_t len,
                       kb_video_chat_start_active_response *out);

/*
 * Извлечено из декомпила 26.28. Request verified (thg c=17). Response parser iya.l=new iya(9); dispatch iya.h(w2a) not decompiled => empty.
 *
 * Опкод 82 (VIDEO_UPLOAD).
 */
typedef struct {
    /* type */
    int32_t type;
    /* count */
    int32_t count;
    /* uploaderType */
    int32_t uploader_type;
} kb_video_upload;

/*
 * Ответ на VideoUpload.
 */
typedef struct {
    /* info */
    kb_raw info;
    size_t info_count;
    int has_info;
    /* uploaderType */
    int64_t uploader_type;
    int has_uploader_type;
} kb_video_upload_response;

#define KB_OPCODE_VIDEO_UPLOAD 82

kb_status kb_encode_video_upload(const kb_video_upload *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_video_upload_response(const uint8_t *body, size_t len,
                       kb_video_upload_response *out);

/*
 * Извлечено из декомпила 26.28. Builder suc confirmed (all 5 keys). Response qma.g (N=7) parse methods not decompiled -> empty.
 *
 * Опкод 305 (VOTERS_LIST_BY_ANSWER).
 */
typedef struct {
    /* chatId */
    int64_t chat_id;
    /* pollId */
    int64_t poll_id;
    /* messageId */
    int64_t message_id;
    /* answerId */
    int32_t answer_id;
    /* marker */
    int64_t marker;
    int has_marker;
} kb_voters_list_by_answer;

/*
 * Ответ на VotersListByAnswer.
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_voters_list_by_answer_response;

#define KB_OPCODE_VOTERS_LIST_BY_ANSWER 305

kb_status kb_encode_voters_list_by_answer(const kb_voters_list_by_answer *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_voters_list_by_answer_response(const uint8_t *body, size_t len,
                       kb_voters_list_by_answer_response *out);

/*
 * Извлечено из декомпила 26.28. Verified via zl0.h smali (zl0.l=new zl0(10); packed-switch 0x0 covers 0..7, a=10 falls to default = pswitch_0) building n2j(url,query_id).
 *
 * Опкод 160 (WEB_APP_INIT_DATA).
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_web_app_init_data;

/*
 * Ответ на WebAppInitData.
 */
typedef struct {
    /* url */
    kb_str url;
    /* query_id */
    kb_str query_id;
} kb_web_app_init_data_response;

#define KB_OPCODE_WEB_APP_INIT_DATA 160

kb_status kb_encode_web_app_init_data(const kb_web_app_init_data *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_web_app_init_data_response(const uint8_t *body, size_t len,
                       kb_web_app_init_data_response *out);

/*
 * Значение map chats[].adminParticipants (ключ — id участника). Из Ð¾ÑÐ²ÐµÑÐ° LOGIN, напр. {'10000001':{'id':10000001,'permissions':4095}}. adminParticipants присутствует у 44 из 50 чатов; в записях id есть всегда (44/44), permissions — у 42/44.
 */
typedef struct {
    /* id */
    int64_t id;
    /* permissions */
    int64_t permissions;
    int has_permissions;
} kb_admin_participant;

kb_status kb_encode_admin_participant(const kb_admin_participant *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_admin_participant(const uint8_t *body, size_t len,
                       kb_admin_participant *out);

/*
 * Discriminated union by _type (defpackage/p40 enum). Base class d30; the JSON reader d30.b(w2a) is 'not decompiled', so received field names come from the live capture (channel history response etc.) an
 */
typedef struct {
    /* _type */
    kb_str _type;
    /* event */
    kb_str event;
    int has_event;
    /* userId */
    int64_t user_id;
    int has_user_id;
    /* userIds */
    kb_raw user_ids;
    size_t user_ids_count;
    int has_user_ids;
    /* title */
    kb_str title;
    int has_title;
    /* showHistory */
    int show_history;
    int has_show_history;
    /* chatType */
    kb_str chat_type;
    int has_chat_type;
    /* startPayload */
    kb_str start_payload;
    int has_start_payload;
    /* crop */
    kb_raw crop;
    int has_crop;
    /* photoToken */
    kb_str photo_token;
    int has_photo_token;
    /* baseUrl */
    kb_str base_url;
    int has_base_url;
    /* photoId */
    int64_t photo_id;
    int has_photo_id;
    /* width */
    int64_t width;
    int has_width;
    /* height */
    int64_t height;
    int has_height;
    /* url */
    kb_str url;
    int has_url;
    /* previewData */
    kb_raw preview_data;
    int has_preview_data;
    /* videoId */
    int64_t video_id;
    int has_video_id;
    /* videoType */
    int64_t video_type;
    int has_video_type;
    /* token */
    kb_str token;
    int has_token;
    /* duration */
    int64_t duration;
    int has_duration;
    /* thumbnail */
    kb_str thumbnail;
    int has_thumbnail;
    /* thumbhash */
    kb_str thumbhash;
    int has_thumbhash;
    /* wave */
    kb_str wave;
    int has_wave;
    /* audioId */
    int64_t audio_id;
    int has_audio_id;
    /* fileId */
    int64_t file_id;
    int has_file_id;
    /* stickerId */
    int64_t sticker_id;
    int has_sticker_id;
    /* shareId */
    int64_t share_id;
    int has_share_id;
    /* description */
    kb_str description;
    int has_description;
    /* host */
    kb_str host;
    int has_host;
    /* image */
    kb_raw image;
    int has_image;
    /* contentLevel */
    int content_level;
    int has_content_level;
    /* conversationId */
    kb_str conversation_id;
    int has_conversation_id;
    /* callType */
    kb_str call_type;
    int has_call_type;
    /* hangupType */
    kb_str hangup_type;
    int has_hangup_type;
    /* joinLink */
    kb_str join_link;
    int has_join_link;
    /* vcfBody */
    kb_str vcf_body;
    int has_vcf_body;
    /* contactId */
    int64_t contact_id;
    int has_contact_id;
    /* latitude */
    double latitude;
    int has_latitude;
    /* longitude */
    double longitude;
    int has_longitude;
    /* livePeriod */
    int64_t live_period;
    int has_live_period;
    /* zoom */
    kb_raw zoom;
    int has_zoom;
    /* alt */
    double alt;
    int has_alt;
    /* epu */
    kb_raw epu;
    int has_epu;
    /* hdn */
    kb_raw hdn;
    int has_hdn;
    /* spd */
    kb_raw spd;
    int has_spd;
    /* keyboard */
    kb_raw keyboard;
    int has_keyboard;
    /* callbackId */
    kb_str callback_id;
    int has_callback_id;
    /* buttons */
    kb_raw buttons;
    size_t buttons_count;
    int has_buttons;
    /* pollId */
    int64_t poll_id;
    int has_poll_id;
    /* answers */
    kb_raw answers;
    size_t answers_count;
    int has_answers;
    /* settings */
    int64_t settings;
    int has_settings;
    /* owner */
    kb_raw owner;
    int has_owner;
    /* storyId */
    int64_t story_id;
    int has_story_id;
} kb_attach;

kb_status kb_encode_attach(const kb_attach *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_attach(const uint8_t *body, size_t len,
                       kb_attach *out);

/*
 * Inline keyboard button. defpackage/i21; serialized in b18.a; in capture only type/text/url observed (LINK buttons).
 */
typedef struct {
    /* type */
    kb_str type;
    /* text */
    kb_str text;
    /* url */
    kb_str url;
    int has_url;
    /* intent */
    kb_str intent;
    int has_intent;
    /* payload */
    kb_str payload;
    int has_payload;
    /* isQuick */
    int is_quick;
    int has_is_quick;
    /* contactId */
    int64_t contact_id;
    int has_contact_id;
} kb_button;

kb_status kb_encode_button(const kb_button *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_button(const uint8_t *body, size_t len,
                       kb_button *out);

/*
 * A chat/dialog/channel entry. Verified against chats[] (50 samples) in Ð¾ÑÐ²ÐµÑÐ° LOGIN and the decompiled parser nm2.java (case labels). Optionality from presence across the 50 login samples; parser-only fields (absent from capture, present as case labels in nm2.java) are optional. All counts, enum values and value types re-checked. Correction vs prior output: liveLocationMessageIds is a map {i64} in the parser, not [i64].
 */
typedef struct {
    /* id */
    int64_t id;
    /* type */
    kb_str type;
    /* status */
    kb_str status;
    /* owner */
    int64_t owner;
    /* participants */
    kb_raw participants;
    size_t participants_len;
    /* lastMessage */
    kb_raw last_message;
    /* lastEventTime */
    int64_t last_event_time;
    /* lastDelayedUpdateTime */
    int64_t last_delayed_update_time;
    /* lastFireDelayedErrorTime */
    int64_t last_fire_delayed_error_time;
    /* created */
    int64_t created;
    /* joinTime */
    int64_t join_time;
    /* modified */
    int64_t modified;
    /* options */
    kb_raw options;
    size_t options_count;
    int has_options;
    /* title */
    kb_str title;
    int has_title;
    /* access */
    kb_str access;
    int has_access;
    /* participantsCount */
    int64_t participants_count;
    int has_participants_count;
    /* messagesCount */
    int64_t messages_count;
    int has_messages_count;
    /* reactions */
    kb_raw reactions;
    int has_reactions;
    /* link */
    kb_str link;
    int has_link;
    /* admins */
    kb_raw admins;
    size_t admins_count;
    int has_admins;
    /* adminParticipants */
    kb_raw admin_participants;
    size_t admin_participants_count;
    int has_admin_participants;
    /* invitedBy */
    int64_t invited_by;
    int has_invited_by;
    /* videoConversation */
    kb_raw video_conversation;
    int has_video_conversation;
    /* newMessages */
    int64_t new_messages;
    int has_new_messages;
    /* cid */
    int64_t cid;
    int has_cid;
    /* restrictions */
    int64_t restrictions;
    int has_restrictions;
    /* prevMessageId */
    int64_t prev_message_id;
    int has_prev_message_id;
    /* hasBots */
    int has_bots;
    int has_has_bots;
    /* description */
    kb_str description;
    int has_description;
    /* baseIconUrl */
    kb_str base_icon_url;
    int has_base_icon_url;
    /* baseRawIconUrl */
    kb_str base_raw_icon_url;
    int has_base_raw_icon_url;
    /* participantSettings */
    int64_t participant_settings;
    int has_participant_settings;
    /* subject */
    kb_str subject;
    int has_subject;
    /* pinnedMessage */
    kb_raw pinned_message;
    int has_pinned_message;
    /* pinnedMessageId */
    int64_t pinned_message_id;
    int has_pinned_message_id;
    /* groupChatInfo */
    kb_raw group_chat_info;
    int has_group_chat_info;
    /* hidePinnedMessage */
    int hide_pinned_message;
    int has_hide_pinned_message;
    /* isSuspended */
    int is_suspended;
    int has_is_suspended;
    /* markedAsUnread */
    int marked_as_unread;
    int has_marked_as_unread;
    /* unreadPin */
    int unread_pin;
    int has_unread_pin;
    /* unreadReply */
    int unread_reply;
    int has_unread_reply;
    /* blockedParticipantsCount */
    int64_t blocked_participants_count;
    int has_blocked_participants_count;
    /* commentsBlacklistCount */
    int64_t comments_blacklist_count;
    int has_comments_blacklist_count;
    /* pendingJoinRequestsCount */
    int64_t pending_join_requests_count;
    int has_pending_join_requests_count;
    /* joinRequestTime */
    int64_t join_request_time;
    int has_join_request_time;
    /* lastMentionMessageId */
    int64_t last_mention_message_id;
    int has_last_mention_message_id;
    /* lastReactedMessageId */
    int64_t last_reacted_message_id;
    int has_last_reacted_message_id;
    /* lastReaction */
    kb_raw last_reaction;
    int has_last_reaction;
    /* liveLocationMessageIds */
    kb_raw live_location_message_ids;
    size_t live_location_message_ids_count;
    int has_live_location_message_ids;
    /* liveStreamUpdateTime */
    int64_t live_stream_update_time;
    int has_live_stream_update_time;
} kb_chat;

kb_status kb_encode_chat(const kb_chat *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_chat(const uint8_t *body, size_t len,
                       kb_chat *out);

/*
 * chats[].reactions из Ð¾ÑÐ²ÐµÑÐ° LOGIN, напр. {'isActive':false,'updateTime':1700000000000}. Присутствует у 46 из 50 чатов; оба поля есть во всех 46.
 */
typedef struct {
    /* isActive */
    int is_active;
    /* updateTime */
    int64_t update_time;
} kb_chat_reactions;

kb_status kb_encode_chat_reactions(const kb_chat_reactions *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_chat_reactions(const uint8_t *body, size_t len,
                       kb_chat_reactions *out);

/*
 * User/contact profile. Verified: Ð¾ÑÐ²ÐµÑÐ° CONTACT_INFO contacts[0] and Ð¾ÑÐ²ÐµÑÐ° LOGIN profile.contact. All keys present in both samples except phone.
 */
typedef struct {
    /* id */
    int64_t id;
    /* updateTime */
    int64_t update_time;
    /* registrationTime */
    int64_t registration_time;
    /* baseUrl */
    kb_str base_url;
    /* baseRawUrl */
    kb_str base_raw_url;
    /* photoId */
    int64_t photo_id;
    /* phone */
    int64_t phone;
    int has_phone;
    /* names */
    kb_raw names;
    size_t names_count;
    /* options */
    kb_raw options;
    size_t options_count;
    /* description */
    kb_str description;
    /* accountStatus */
    int64_t account_status;
    /* country */
    kb_str country;
} kb_contact;

kb_status kb_encode_contact(const kb_contact *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_contact(const uint8_t *body, size_t len,
                       kb_contact *out);

/*
 * Модель i55 (ov9.q). НЕ в захвате. Ключ поля и подполя подтверждены декомпилированным i55.c() (i55.java:24) и x3c.java:52.
 */
typedef struct {
    /* timeToFire */
    int64_t time_to_fire;
    int has_time_to_fire;
    /* notifySender */
    int notify_sender;
    int has_notify_sender;
} kb_delayed_attributes;

kb_status kb_encode_delayed_attributes(const kb_delayed_attributes *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_delayed_attributes(const uint8_t *body, size_t len,
                       kb_delayed_attributes *out);

/*
 * Кусок форматирования в тексте сообщения.
 * `from` и `length` считаются в **UTF-16 code units**, а не в символах:
 * эмодзи вне BMP занимает 2 единицы. См. `markup::utf16_len`.
 */
typedef struct {
    /* type */
    kb_str type;
    /* from */
    int64_t from;
    /* length */
    int64_t length;
} kb_element;

kb_status kb_encode_element(const kb_element *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_element(const uint8_t *body, size_t len,
                       kb_element *out);

/*
 * Весь Ð¾ÑÐ²ÐµÑ-Ð¾ÑÐ¸Ð±ÐºÐ° — конверт ошибки сервера MAX. Все 4 поля присутствуют.
 */
typedef struct {
    /* error */
    kb_str error;
    /* message */
    kb_str message;
    /* localizedMessage */
    kb_str localized_message;
    /* title */
    kb_str title;
} kb_error_envelope;

kb_status kb_encode_error_envelope(const kb_error_envelope *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_error_envelope(const uint8_t *body, size_t len,
                       kb_error_envelope *out);

/*
 * INLINE_KEYBOARD wrapper (received in capture under key 'keyboard'). defpackage/zg8 (field a = List<List<i21>>).
 */
typedef struct {
    /* buttons */
    kb_raw buttons;
    size_t buttons_count;
} kb_keyboard;

kb_status kb_encode_keyboard(const kb_keyboard *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_keyboard(const uint8_t *body, size_t len,
                       kb_keyboard *out);

/*
 * Блок `exp` в LOGIN. Байты — фиксированные, взяты из референсного клиента.
 */
typedef struct {
    /* chatsCountGroups */
    kb_bin chats_count_groups;
} kb_login_exp;

kb_status kb_encode_login_exp(const kb_login_exp *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_login_exp(const uint8_t *body, size_t len,
                       kb_login_exp *out);

/*
 * Сообщение TamTam/MAX. Модель ov9. JSON-парсер pma.L(w2a) подтверждённо 'Method not decompiled' (pma.java:175), поэтому ключи восстановлены из захвата и исходящего сериализатора x3c. После адверсариал-
 */
typedef struct {
    /* id */
    int64_t id;
    /* time */
    int64_t time;
    /* type */
    kb_str type;
    /* sender */
    int64_t sender;
    /* text */
    kb_str text;
    /* cid */
    int64_t cid;
    int has_cid;
    /* attaches */
    kb_raw attaches;
    size_t attaches_count;
    /* elements */
    kb_raw elements;
    size_t elements_count;
    int has_elements;
    /* options */
    int64_t options;
    int has_options;
    /* updateTime */
    int64_t update_time;
    int has_update_time;
    /* status */
    kb_str status;
    int has_status;
    /* reactionInfo */
    kb_raw reaction_info;
    int has_reaction_info;
    /* link */
    kb_raw link;
    int has_link;
    /* delayedAttributes */
    kb_raw delayed_attributes;
    int has_delayed_attributes;
} kb_message;

kb_status kb_encode_message(const kb_message *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_message(const uint8_t *body, size_t len,
                       kb_message *out);

/*
 * An inline formatting/entity range inside Message.elements[] (LOGIN lastMessage). Describes a styled or linked span over the message text. Statistics from the 3 element instances present across the 50 LOGIN lastMessage samples.
 */
typedef struct {
    /* type */
    kb_str type;
    /* length */
    int64_t length;
    /* from */
    int64_t from;
    int has_from;
    /* attributes */
    kb_raw attributes;
    int has_attributes;
} kb_message_element;

kb_status kb_encode_message_element(const kb_message_element *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_message_element(const uint8_t *body, size_t len,
                       kb_message_element *out);

/*
 * reply/forward на сообщение — ВХОДЯЩАЯ форма (ov9.i). Подтверждена захватом 49_history_channel (5 шт.). Замечание: исходящий сериализатор z3c для отправки reply/forward использует ИНЫЕ ключи (type, cha
 */
typedef struct {
    /* type */
    kb_str type;
    /* message */
    kb_raw message;
    int has_message;
    /* chatId */
    int64_t chat_id;
    int has_chat_id;
} kb_message_link;

kb_status kb_encode_message_link(const kb_message_link *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_message_link(const uint8_t *body, size_t len,
                       kb_message_link *out);

/*
 * Element of Contact.names. Verified in both captures.
 */
typedef struct {
    /* name */
    kb_str name;
    /* firstName */
    kb_str first_name;
    /* lastName */
    kb_str last_name;
    /* type */
    kb_str type;
} kb_name;

kb_status kb_encode_name(const kb_name *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_name(const uint8_t *body, size_t len,
                       kb_name *out);

/*
 * Тело исходящего сообщения — поле `message` в MSG_SEND.
 */
typedef struct {
    /* text */
    kb_str text;
    /* cid */
    int64_t cid;
    /* elements */
    kb_raw elements;
    size_t elements_count;
    /* attaches */
    kb_raw attaches;
    size_t attaches_count;
    /* link */
    kb_raw link;
    int has_link;
    /* delayedAttributes */
    kb_raw delayed_attributes;
    int has_delayed_attributes;
} kb_outgoing_message;

kb_status kb_encode_outgoing_message(const kb_outgoing_message *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_outgoing_message(const uint8_t *body, size_t len,
                       kb_outgoing_message *out);

/*
 * Сервер требует облачный пароль (2FA) вместо выдачи токена.
 */
typedef struct {
    /* trackId */
    kb_str track_id;
    /* hint */
    kb_str hint;
    int has_hint;
} kb_password_challenge;

kb_status kb_encode_password_challenge(const kb_password_challenge *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_password_challenge(const uint8_t *body, size_t len,
                       kb_password_challenge *out);

/*
 * Inline blurred preview blob for PHOTO/VIDEO. In capture: object with a single key holding base64 WEBP bytes.
 */
typedef struct {
    int reserved; /* операция без полей */
} kb_preview_data;

kb_status kb_encode_preview_data(const kb_preview_data *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_preview_data(const uint8_t *body, size_t len,
                       kb_preview_data *out);

typedef struct {
    /* contact */
    kb_raw contact;
    int has_contact;
    /* profileOptions */
    kb_raw profile_options;
    int has_profile_options;
} kb_profile;

kb_status kb_encode_profile(const kb_profile *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_profile(const uint8_t *body, size_t len,
                       kb_profile *out);

/*
 * Элемент counters в reactionInfo (из захвата 49_history_channel).
 */
typedef struct {
    /* reaction */
    kb_str reaction;
    /* count */
    int64_t count;
} kb_reaction_counter;

kb_status kb_encode_reaction_counter(const kb_reaction_counter *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_reaction_counter(const uint8_t *body, size_t len,
                       kb_reaction_counter *out);

/*
 * Модель t1a. Парсер pfk.b подтверждённо 'Method not decompiled' (pfk.java:93). Структура — из захвата (49_history_channel) и полей t1a. yourReaction УДАЛЕН: нет в захвате, ключ не подтверждён (парсер н
 */
typedef struct {
    /* counters */
    kb_raw counters;
    size_t counters_count;
    int has_counters;
    /* totalCount */
    int64_t total_count;
    int has_total_count;
} kb_reaction_info;

kb_status kb_encode_reaction_info(const kb_reaction_info *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_reaction_info(const uint8_t *body, size_t len,
                       kb_reaction_info *out);

/*
 * Сообщение, как его вернул сервер.
 */
typedef struct {
    /* id */
    kb_raw id;
    int has_id;
    /* cid */
    int64_t cid;
    int has_cid;
    /* sender */
    kb_raw sender;
    int has_sender;
    /* text */
    kb_str text;
    int has_text;
    /* time */
    int64_t time;
    int has_time;
    /* status */
    kb_str status;
    int has_status;
    /* elements */
    kb_raw elements;
    size_t elements_count;
    int has_elements;
} kb_sent_message;

kb_status kb_encode_sent_message(const kb_sent_message *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_sent_message(const uint8_t *body, size_t len,
                       kb_sent_message *out);

/*
 * Запись в `tokenAttrs`: tokenAttrs.LOGIN.token / tokenAttrs.REGISTER.token.
 */
typedef struct {
    /* token */
    kb_str token;
    int has_token;
} kb_token_attr;

kb_status kb_encode_token_attr(const kb_token_attr *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_token_attr(const uint8_t *body, size_t len,
                       kb_token_attr *out);

/*
 * chats[].videoConversation in LOGIN response. The videoConversation field itself is present on 41/50 chats (optional on Chat), but when present all four fields appear on 41/41 -> required within this type.
 */
typedef struct {
    /* conversationId */
    kb_str conversation_id;
    /* joinLink */
    kb_str join_link;
    /* previewParticipantIds */
    kb_raw preview_participant_ids;
    size_t preview_participant_ids_count;
    /* callType */
    kb_str call_type;
} kb_video_conversation;

kb_status kb_encode_video_conversation(const kb_video_conversation *v, uint8_t *out,
                       size_t cap, size_t *out_len);
kb_status kb_decode_video_conversation(const uint8_t *body, size_t len,
                       kb_video_conversation *out);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_OPS_H */
