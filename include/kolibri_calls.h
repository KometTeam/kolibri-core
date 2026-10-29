/*
 * Звонки: разбор vcp, событий ws2 и sans-IO сигнализация.
 *
 * Основной сокет только заводит звонок (опкоды 78/166 отдают vcp/endpoint,
 * пуш 137 сообщает о входящем). Всё живое идёт по отдельному WebSocket ws2, а
 * медиа WebRTC остаётся в хосте. Здесь — только разбор и сборка JSON; сокет,
 * TLS и кадры WebSocket ведёт вызывающий (для кадров есть kolibri_ws.h).
 */

#ifndef KOLIBRI_CALLS_H
#define KOLIBRI_CALLS_H

#include <stddef.h>
#include <stdint.h>

#include "kolibri.h"
#include "kolibri_json.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* ─── vcp: параметры соединения ─────────────────────────────────────────── */

/*
 * Разобранный vcp. Держит корневой JSON-объект, указывающий внутрь scratch,
 * который передали в kb_vcp_decode: scratch обязан жить, пока жив kb_vcp.
 */
typedef struct {
    kb_json_value root;
} kb_vcp;

/*
 * Разобрать vcp вида "<rawLen>:<base64(LZ4-block(JSON))>". JSON распакуется в
 * начало scratch и там останется. scratch_cap должно вмещать распакованный
 * JSON плюс промежуточные base64-байты. KB_ERR_TRUNCATED — не хватило scratch.
 */
kb_status kb_vcp_decode(const char *vcp, size_t len, uint8_t *scratch,
                        size_t scratch_cap, kb_vcp *out);

/* Строковые поля во внешний буфер (с '\0'). KB_ERR_NOT_FOUND — поля нет. */
kb_status kb_vcp_token(const kb_vcp *vcp, char *out, size_t cap);
kb_status kb_vcp_ws_endpoint(const kb_vcp *vcp, char *out, size_t cap);
kb_status kb_vcp_stun(const kb_vcp *vcp, char *out, size_t cap);
kb_status kb_vcp_turn_user(const kb_vcp *vcp, char *out, size_t cap);
kb_status kb_vcp_turn_password(const kb_vcp *vcp, char *out, size_t cap);

int kb_vcp_is_video(const kb_vcp *vcp);
/* expires_at (unix-секунды). KB_ERR_NOT_FOUND, если поля нет. */
kb_status kb_vcp_expires_at(const kb_vcp *vcp, int64_t *out);
/* «Истекает в пределах 5 c» считается истёкшим. now_secs — unix-время. */
int kb_vcp_is_expired(const kb_vcp *vcp, int64_t now_secs);
/* id пользователя звонков — часть после последнего ':' в turn_user; 0, если нет. */
int64_t kb_vcp_user_id(const kb_vcp *vcp);

/* Итерация turn-серверов (поле trne — список через запятую). */
typedef struct {
    const char *cur;
    const char *end;
} kb_vcp_turn_iter;
void kb_vcp_turn_begin(const kb_vcp *vcp, kb_vcp_turn_iter *iter);
kb_status kb_vcp_turn_next(kb_vcp_turn_iter *iter, char *out, size_t cap);

/* Итерация ws_ips (поле wsip — массив строк). */
typedef struct {
    kb_json_value it;
    int ok;
} kb_vcp_ip_iter;
void kb_vcp_ip_begin(const kb_vcp *vcp, kb_vcp_ip_iter *iter);
kb_status kb_vcp_ip_next(kb_vcp_ip_iter *iter, char *out, size_t cap);

/* Параметры ws2, которые не приходят с сервера. Заполни kb_ws2_client_default. */
typedef struct {
    const char *capabilities;
    const char *device;
    const char *platform;
    const char *client_type;
    const char *app_version;
    const char *os_version;
} kb_ws2_client;

void kb_ws2_client_default(kb_ws2_client *client);

/*
 * URL подключения ws2 для входящего звонка: заменяет query у ws_endpoint на
 * набор параметров (userId, conversationId, token, версия и поля клиента).
 */
kb_status kb_vcp_ws2_url(const kb_vcp *vcp, const char *conversation_id,
                         const kb_ws2_client *client, char *out, size_t cap);

/*
 * URL ws2 для исходящего звонка: endpoint уже несёт token и id в query,
 * докидываем параметры клиента (при совпадении ключа — перезапись).
 */
kb_status kb_ws2_url_from_endpoint(const char *endpoint,
                                   const kb_ws2_client *client, char *out,
                                   size_t cap);

/* ─── События ws2 ───────────────────────────────────────────────────────── */

typedef enum {
    KB_CALL_CONNECTION = 0,   /* топология, участники, ICE */
    KB_CALL_TRANSMITTED,      /* SDP offer/answer либо ICE-кандидат */
    KB_CALL_HUNGUP,
    KB_CALL_CLOSED,
    KB_CALL_TOPOLOGY_CHANGED,
    KB_CALL_ERROR,
    KB_CALL_OTHER
} kb_call_event_type;

typedef enum {
    KB_TD_SDP = 0,
    KB_TD_CANDIDATE
} kb_transmitted_type;

/*
 * Разобранное уведомление ws2. root указывает внутрь переданного JSON-буфера,
 * который обязан жить, пока читаются поля.
 */
typedef struct {
    kb_call_event_type type;
    kb_json_value root;
} kb_call_event;

kb_status kb_call_event_parse(const char *json, size_t len, kb_call_event *out);

/* Для KB_CALL_CONNECTION и KB_CALL_TOPOLOGY_CHANGED. */
kb_status kb_call_topology(const kb_call_event *ev, char *out, size_t cap);
int kb_call_is_sfu(const kb_call_event *ev);

/* Итерация участников (id). */
typedef struct {
    kb_json_value it;
    int ok;
} kb_call_participant_iter;
void kb_call_participants_begin(const kb_call_event *ev,
                                kb_call_participant_iter *iter);
int kb_call_participants_next(kb_call_participant_iter *iter, int64_t *out);
/* Первый участник, отличный от my_user_id; 0 и KB_ERR_NOT_FOUND, если нет. */
kb_status kb_call_peer_of(const kb_call_event *ev, int64_t my_user_id,
                          int64_t *out);

/* Для KB_CALL_TRANSMITTED: что внутри — SDP или кандидат. */
kb_status kb_call_transmitted_type(const kb_call_event *ev,
                                   kb_transmitted_type *out);
kb_status kb_call_sdp_type(const kb_call_event *ev, char *out, size_t cap);
kb_status kb_call_sdp(const kb_call_event *ev, char *out, size_t cap);
kb_status kb_call_candidate(const kb_call_event *ev, char *out, size_t cap);
kb_status kb_call_candidate_mid(const kb_call_event *ev, char *out, size_t cap);
kb_status kb_call_candidate_mline(const kb_call_event *ev, int64_t *out);

/* Текст ошибки для KB_CALL_ERROR. */
kb_status kb_call_error_text(const kb_call_event *ev, char *out, size_t cap);

/* ─── Сигнализация ws2 (sans-IO) ────────────────────────────────────────── */

/*
 * Конверт: запрос {"command":...,...,"sequence":N}, ответ
 * {"sequence":N,"response":"<cmd>","type":"response"}, уведомление с полем
 * "notification", keepalive — текстовый кадр "ping" в ответ на "pong".
 *
 * Состояние здесь — только счётчик seq. Сокет и сопоставление ответов ведёт
 * вызывающий: seq из билдера кладёшь в свою карту ожиданий.
 */
typedef struct {
    int64_t seq;
} kb_ws2;

void kb_ws2_init(kb_ws2 *ws2);

/* Классификация входящего кадра. */
typedef enum {
    KB_WS2_PING = 0,          /* ответь текстом "pong" */
    KB_WS2_RESPONSE,          /* ответ на команду; seq заполнен */
    KB_WS2_ERROR,             /* ошибка на команду; seq заполнен */
    KB_WS2_NOTIFICATION,      /* серверное уведомление */
    KB_WS2_IGNORE             /* нераспознанный кадр */
} kb_ws2_frame;

/* seq заполняется для RESPONSE/ERROR (иначе -1). */
kb_ws2_frame kb_ws2_classify(const char *text, size_t len, int64_t *seq);

/*
 * Билдеры команд. Пишут JSON во внешний буфер (с '\0'), возвращают
 * использованный seq в *seq. Строки внутри экранируются.
 */
kb_status kb_ws2_transmit_sdp(kb_ws2 *ws2, int64_t participant_id,
                              const char *sdp_type, const char *sdp, char *out,
                              size_t cap, size_t *out_len, int64_t *seq);
kb_status kb_ws2_transmit_candidate(kb_ws2 *ws2, int64_t participant_id,
                                    const char *candidate, const char *sdp_mid,
                                    int64_t sdp_mline_index, char *out,
                                    size_t cap, size_t *out_len, int64_t *seq);
kb_status kb_ws2_accept_call(kb_ws2 *ws2, char *out, size_t cap,
                             size_t *out_len, int64_t *seq);
kb_status kb_ws2_hangup(kb_ws2 *ws2, const char *reason, char *out, size_t cap,
                        size_t *out_len, int64_t *seq);
kb_status kb_ws2_change_media(kb_ws2 *ws2, int audio, int video, int screen,
                              char *out, size_t cap, size_t *out_len,
                              int64_t *seq);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_CALLS_H */
