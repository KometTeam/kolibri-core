/*
 * @generated tools/gen_opcodes.py — не редактировать руками.
 *
 * Источник: schema/opcodes.toml
 * Перегенерация: python3 tools/gen_opcodes.py
 */


#ifndef KOLIBRI_OPCODES_H
#define KOLIBRI_OPCODES_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif


/* ── Session ── */
#define KB_OP_PING                             1
#define KB_OP_DEBUG                            2
#define KB_OP_RECONNECT                        3
#define KB_OP_LOG                              5
#define KB_OP_SESSION_INIT                     6
#define KB_OP_CONTACTS_GET                     8

/* ── Profile ── */
#define KB_OP_PROFILE                          16

/* ── Auth ── */
#define KB_OP_AUTH_REQUEST                     17
#define KB_OP_AUTH                             18
#define KB_OP_LOGIN                            19
#define KB_OP_LOGOUT                           20
#define KB_OP_SYNC                             21
#define KB_OP_CONFIG                           22
#define KB_OP_AUTH_CONFIRM                     23

/* ── Auth 2FA ── */
#define KB_OP_AUTH_LOGIN_RESTORE_PASSWORD      101
#define KB_OP_AUTH_2FA_DETAILS                 104
#define KB_OP_EXTERNAL_CALLBACK                105
#define KB_OP_AUTH_VALIDATE_PASSWORD           107
#define KB_OP_AUTH_VALIDATE_HINT               108
#define KB_OP_AUTH_VERIFY_EMAIL                109
#define KB_OP_AUTH_CHECK_EMAIL                 110
#define KB_OP_AUTH_SET_2FA                     111
#define KB_OP_AUTH_CREATE_TRACK                112
#define KB_OP_AUTH_CHECK_PASSWORD              113
#define KB_OP_AUTH_LOGIN_CHECK_PASSWORD        115
#define KB_OP_AUTH_LOGIN_PROFILE_DELETE        116

/* ── Assets ── */
#define KB_OP_PRESET_AVATARS                   25
#define KB_OP_ASSETS_GET                       26
#define KB_OP_ASSETS_UPDATE                    27
#define KB_OP_ASSETS_GET_BY_IDS                28
#define KB_OP_ASSETS_ADD                       29
#define KB_OP_ASSETS_REMOVE                    259
#define KB_OP_ASSETS_MOVE                      260
#define KB_OP_ASSETS_LIST_MODIFY               261

/* ── Contacts ── */
#define KB_OP_CONTACT_INFO                     32
#define KB_OP_CONTACT_ADD                      33
#define KB_OP_CONTACT_UPDATE                   34
#define KB_OP_CONTACT_PRESENCE                 35
#define KB_OP_CONTACT_LIST                     36
#define KB_OP_CONTACT_SEARCH                   37
#define KB_OP_CONTACT_MUTUAL                   38
#define KB_OP_CONTACT_PHOTOS                   39
#define KB_OP_CONTACT_SORT                     40
#define KB_OP_CONTACT_VERIFY                   42
#define KB_OP_REMOVE_CONTACT_PHOTO             43
#define KB_OP_CONTACT_INFO_BY_PHONE            46

/* ── Chats ── */
#define KB_OP_CHAT_INFO                        48
#define KB_OP_CHAT_HISTORY                     49
#define KB_OP_CHAT_MARK                        50
#define KB_OP_CHAT_MEDIA                       51
#define KB_OP_CHAT_DELETE                      52
#define KB_OP_CHATS_LIST                       53
#define KB_OP_CHAT_CLEAR                       54
#define KB_OP_CHAT_UPDATE                      55
#define KB_OP_CHAT_CHECK_LINK                  56
#define KB_OP_CHAT_JOIN                        57
#define KB_OP_CHAT_LEAVE                       58
#define KB_OP_CHAT_MEMBERS                     59
#define KB_OP_PUBLIC_SEARCH                    60
#define KB_OP_CHAT_PERSONAL_CONFIG             61
#define KB_OP_CHAT_CREATE                      63

/* ── Messages ── */
#define KB_OP_MSG_SEND                         64
#define KB_OP_MSG_TYPING                       65
#define KB_OP_MSG_DELETE                       66
#define KB_OP_MSG_EDIT                         67
#define KB_OP_CHAT_SEARCH                      68
#define KB_OP_MSG_SHARE_PREVIEW                70
#define KB_OP_MSG_GET                          71
#define KB_OP_MSG_SEARCH_TOUCH                 72
#define KB_OP_MSG_SEARCH                       73
#define KB_OP_MSG_GET_STAT                     74
#define KB_OP_CHAT_SUBSCRIBE                   75
#define KB_OP_MSG_DELETE_RANGE                 92

/* ── Reactions ── */
#define KB_OP_MSG_REACTION                     178
#define KB_OP_MSG_CANCEL_REACTION              179
#define KB_OP_MSG_GET_REACTIONS                180
#define KB_OP_MSG_GET_DETAILED_REACTIONS       181
#define KB_OP_CHAT_REACTIONS_SETTINGS_SET      257
#define KB_OP_REACTIONS_SETTINGS_GET_BY_CHAT_ID 258

/* ── Calls & Video ── */
#define KB_OP_VIDEO_CHAT_START                 76
#define KB_OP_CHAT_MEMBERS_UPDATE              77
#define KB_OP_VIDEO_CHAT_START_ACTIVE          78
#define KB_OP_VIDEO_CHAT_HISTORY               79
#define KB_OP_VIDEO_CHAT_DELETE_HISTORY        164
#define KB_OP_VIDEO_CHAT_CREATE_JOIN_LINK      84
#define KB_OP_VIDEO_CHAT_JOIN_BY_LINK          166
#define KB_OP_VIDEO_CHAT_MEMBERS               195
#define KB_OP_GET_INBOUND_CALLS                103

/* ── Media & Files ── */
#define KB_OP_PHOTO_UPLOAD                     80
#define KB_OP_STICKER_UPLOAD                   81
#define KB_OP_VIDEO_UPLOAD                     82
#define KB_OP_VIDEO_PLAY                       83
#define KB_OP_CHAT_PIN_SET_VISIBILITY          86
#define KB_OP_FILE_UPLOAD                      87
#define KB_OP_FILE_DOWNLOAD                    88
#define KB_OP_LINK_INFO                        89
#define KB_OP_AUDIO_PLAY                       301

/* ── Sessions ── */
#define KB_OP_SESSIONS_INFO                    96
#define KB_OP_SESSIONS_CLOSE                   97
#define KB_OP_PHONE_BIND_REQUEST               98
#define KB_OP_PHONE_BIND_CONFIRM               99

/* ── Bots ── */
#define KB_OP_CHAT_COMPLAIN                    117
#define KB_OP_MSG_SEND_CALLBACK                118
#define KB_OP_SUSPEND_BOT                      119
#define KB_OP_CHAT_BOT_COMMANDS                144
#define KB_OP_BOT_INFO                         145

/* ── Location ── */
#define KB_OP_LOCATION_STOP                    124

/* ── Mentions ── */
#define KB_OP_GET_LAST_MENTIONS                127

/* ── Stickers (creation) ── */
#define KB_OP_STICKER_CREATE                   193
#define KB_OP_STICKER_SUGGEST                  194

/* ── Notifications (server push) ── */
#define KB_OP_NOTIF_MESSAGE                    128
#define KB_OP_NOTIF_TYPING                     129
#define KB_OP_NOTIF_MARK                       130
#define KB_OP_NOTIF_CONTACT                    131
#define KB_OP_NOTIF_PRESENCE                   132
#define KB_OP_NOTIF_CONFIG                     134
#define KB_OP_NOTIF_CHAT                       135
#define KB_OP_NOTIF_ATTACH                     136
#define KB_OP_NOTIF_CALL_START                 137
#define KB_OP_NOTIF_CONTACT_SORT               139
#define KB_OP_NOTIF_MSG_DELETE_RANGE           140
#define KB_OP_NOTIF_MSG_DELETE                 142
#define KB_OP_NOTIF_CALLBACK_ANSWER            143
#define KB_OP_NOTIF_LOCATION                   147
#define KB_OP_NOTIF_LOCATION_REQUEST           148
#define KB_OP_NOTIF_ASSETS_UPDATE              150
#define KB_OP_NOTIF_DRAFT                      152
#define KB_OP_NOTIF_DRAFT_DISCARD              153
#define KB_OP_NOTIF_MSG_DELAYED                154
#define KB_OP_NOTIF_MSG_REACTIONS_CHANGED      155
#define KB_OP_NOTIF_MSG_YOU_REACTED            156
#define KB_OP_NOTIF_PROFILE                    159
#define KB_OP_NOTIF_BANNERS                    292
#define KB_OP_NOTIF_FOLDERS                    277

/* ── Transcription ── */
#define KB_OP_AUDIO_TRANSCRIPTION              202
#define KB_OP_TRANSCRIPTION_RESULT             293

/* ── Misc ── */
#define KB_OP_OK_TOKEN                         158
#define KB_OP_WEB_APP_INIT_DATA                160
#define KB_OP_COMPLAIN                         161
#define KB_OP_COMPLAIN_REASONS_GET             162
#define KB_OP_DRAFT_SAVE                       176
#define KB_OP_DRAFT_DISCARD                    177
#define KB_OP_CHAT_HIDE                        196
#define KB_OP_CHAT_SEARCH_COMMON_PARTICIPANTS  198
#define KB_OP_PROFILE_DELETE                   199
#define KB_OP_PROFILE_DELETE_TIME              200
#define KB_OP_AUTH_QR_APPROVE                  290
#define KB_OP_CHAT_SUGGEST                     300

/* ── Polls ── */
#define KB_OP_SEND_VOTE                        304
#define KB_OP_VOTERS_LIST_BY_ANSWER            305
#define KB_OP_GET_POLL_UPDATES                 306

/* ── Folders ── */
#define KB_OP_FOLDERS_GET                      272
#define KB_OP_FOLDERS_GET_BY_ID                273
#define KB_OP_FOLDERS_UPDATE                   274
#define KB_OP_FOLDERS_REORDER                  275
#define KB_OP_FOLDERS_DELETE                   276

/* ── Stories ── */
#define KB_OP_STORIES_LIST                     208
#define KB_OP_STORIES_LIST_BY_OWNER            209
#define KB_OP_STORIES_GET_BY_OWNER             210
#define KB_OP_STORIES_GET_STATS                211
#define KB_OP_STORIES_GET_DETAILED_STATS       212
#define KB_OP_STORIES_REACT                    213
#define KB_OP_STORIES_MARK                     214
#define KB_OP_STORIES_SEND                     215
#define KB_OP_NOTIF_STORIES_UPDATE             216
#define KB_OP_STORIES_EDIT                     217
#define KB_OP_STORIES_DELETE                   218
#define KB_OP_STORIES_GET_BY_STORY_ID          220

/* ── Добавлено из декомпила 26.28 ── */
#define KB_OP_CHAT_LIVESTREAM_INFO             62
#define KB_OP_GET_COMMENTS_UPDATES             91
#define KB_OP_MSG_DELETE_USER                  94
#define KB_OP_PHONE_WEBAPP_SHARE               106
#define KB_OP_CALL_HISTORY                     163
#define KB_OP_NOTIF_CALL_HISTORY               165
#define KB_OP_PHOTO_URL_REFRESH                203
#define KB_OP_ORG_INFO                         256
#define KB_OP_BANNERS_GET                      302
#define KB_OP_MSG_DELIVERY                     303
#define KB_OP_CHAT_CHECK_ESIA                  307

/* Сколько опкодов знает таблица. */
#define KB_OPCODE_COUNT 179

/*
 * Имя опкода или NULL, если он таблице неизвестен.
 * Строка статическая, освобождать не нужно.
 */
const char *kb_opcode_name(uint16_t opcode);

/* Номер по имени; 0, если имя неизвестно (нулевого опкода в протоколе нет). */
uint16_t kb_opcode_by_name(const char *name);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_OPCODES_H */
