/* Звонки: vcp, ws2-URL, события, сигнализация. */

#include <stdio.h>
#include <string.h>

#include "kb_test.h"
#include "kolibri.h"
#include "kolibri_calls.h"
#include "kolibri_proxy.h"

/* Собрать vcp-строку "<rawLen>:<base64(lz4(json))>" из JSON — обратный путь к
   тому, что делает сервер, нашими же компрессором и base64. */
static size_t make_vcp(const char *json, char *out, size_t cap)
{
    uint8_t comp[4096];
    char b64[8192];
    size_t json_len = strlen(json);
    size_t comp_len = 0;
    size_t at = 0;
    size_t n;

    kb_lz4_compress((const uint8_t *)json, json_len, comp, sizeof comp,
                    &comp_len);
    n = kb_base64_encode(comp, comp_len, b64, sizeof b64);
    at = (size_t)snprintf(out, cap, "%lu:", (unsigned long)json_len);
    memcpy(out + at, b64, n);
    at += n;
    out[at] = '\0';
    return at;
}

void kb_tests_calls(void)
{
    static const char *VCP_JSON =
        "{\"tkn\":\"tok123\",\"wse\":\"wss://ws.example.com/ws2?x=1\","
        "\"wsip\":[\"1.2.3.4\",\"5.6.7.8\"],\"iv\":true,\"et\":1700000000,"
        "\"stne\":\"stun:stun.example.com:3478\","
        "\"trne\":\"turn:t1:3478, turn:t2:3478\","
        "\"trnu\":\"1700000000:USER:42\",\"trnp\":\"secret\"}";

    char vcp_str[8192];
    uint8_t scratch[8192];
    kb_vcp vcp;
    char buf[512];

    KB_TEST("vcp: декод и поля");
    {
        make_vcp(VCP_JSON, vcp_str, sizeof vcp_str);
        CHECK_EQ(kb_vcp_decode(vcp_str, strlen(vcp_str), scratch,
                               sizeof scratch, &vcp),
                 KB_OK);
        CHECK_EQ(kb_vcp_token(&vcp, buf, sizeof buf), KB_OK);
        CHECK_EQ(strcmp(buf, "tok123"), 0);
        CHECK_EQ(kb_vcp_ws_endpoint(&vcp, buf, sizeof buf), KB_OK);
        CHECK_EQ(strcmp(buf, "wss://ws.example.com/ws2?x=1"), 0);
        CHECK_EQ(kb_vcp_is_video(&vcp), 1);
        CHECK_EQ(kb_vcp_stun(&vcp, buf, sizeof buf), KB_OK);
        CHECK_EQ(strcmp(buf, "stun:stun.example.com:3478"), 0);
        CHECK_EQ(kb_vcp_turn_user(&vcp, buf, sizeof buf), KB_OK);
        CHECK_EQ(strcmp(buf, "1700000000:USER:42"), 0);
        CHECK_EQ(kb_vcp_user_id(&vcp), 42);
    }

    KB_TEST("vcp: expires и is_expired");
    {
        int64_t exp = 0;
        CHECK_EQ(kb_vcp_expires_at(&vcp, &exp), KB_OK);
        CHECK(exp == 1700000000LL);
        CHECK_EQ(kb_vcp_is_expired(&vcp, 1699999990LL), 0);
        /* в пределах 5 c до истечения — уже истёк */
        CHECK_EQ(kb_vcp_is_expired(&vcp, 1699999996LL), 1);
        CHECK_EQ(kb_vcp_is_expired(&vcp, 1700000000LL), 1);
    }

    KB_TEST("vcp: списки turn и ws_ips");
    {
        kb_vcp_turn_iter tit;
        kb_vcp_ip_iter iit;
        int n = 0;

        kb_vcp_turn_begin(&vcp, &tit);
        CHECK_EQ(kb_vcp_turn_next(&tit, buf, sizeof buf), KB_OK);
        CHECK_EQ(strcmp(buf, "turn:t1:3478"), 0);
        CHECK_EQ(kb_vcp_turn_next(&tit, buf, sizeof buf), KB_OK);
        CHECK_EQ(strcmp(buf, "turn:t2:3478"), 0);  /* пробел после запятой снят */
        CHECK_EQ(kb_vcp_turn_next(&tit, buf, sizeof buf), KB_ERR_NOT_FOUND);

        kb_vcp_ip_begin(&vcp, &iit);
        while (kb_vcp_ip_next(&iit, buf, sizeof buf) == KB_OK) {
            n++;
        }
        CHECK_EQ(n, 2);
    }

    KB_TEST("vcp: ws2-URL для входящего");
    {
        char url[1024];
        CHECK_EQ(kb_vcp_ws2_url(&vcp, "conv-77", NULL, url, sizeof url), KB_OK);
        CHECK(strstr(url, "wss://ws.example.com/ws2?") == url);
        CHECK(strstr(url, "userId=42") != NULL);
        CHECK(strstr(url, "conversationId=conv-77") != NULL);
        CHECK(strstr(url, "token=tok123") != NULL);
        CHECK(strstr(url, "entityType=USER") != NULL);
        CHECK(strstr(url, "platform=ANDROID") != NULL);
        CHECK(strstr(url, "clientType=ONE_ME") != NULL);
        /* старый query (?x=1) заменён целиком */
        CHECK(strstr(url, "x=1") == NULL);
    }

    KB_TEST("ws2-URL из endpoint: extra перекрывает");
    {
        char url[1024];
        CHECK_EQ(kb_ws2_url_from_endpoint(
                     "wss://h/ws2?token=abc&platform=IOS&conversationId=9",
                     NULL, url, sizeof url),
                 KB_OK);
        /* сохранились token и conversationId */
        CHECK(strstr(url, "token=abc") != NULL);
        CHECK(strstr(url, "conversationId=9") != NULL);
        /* platform перезаписан на ANDROID, старого IOS нет */
        CHECK(strstr(url, "platform=ANDROID") != NULL);
        CHECK(strstr(url, "IOS") == NULL);
        CHECK(strstr(url, "tgt=start") != NULL);
        CHECK(strstr(url, "version=5") != NULL);
    }

    KB_TEST("событие: connection");
    {
        static const char *n =
            "{\"type\":\"notification\",\"notification\":\"connection\","
            "\"conversation\":{\"topology\":\"SERVER\","
            "\"participants\":[{\"id\":42},{\"id\":99}]}}";
        kb_call_event ev;
        kb_call_participant_iter pit;
        int64_t id, peer = 0;
        int count = 0;

        CHECK_EQ(kb_call_event_parse(n, strlen(n), &ev), KB_OK);
        CHECK_EQ(ev.type, KB_CALL_CONNECTION);
        CHECK_EQ(kb_call_topology(&ev, buf, sizeof buf), KB_OK);
        CHECK_EQ(strcmp(buf, "SERVER"), 0);
        CHECK_EQ(kb_call_is_sfu(&ev), 1);
        kb_call_participants_begin(&ev, &pit);
        while (kb_call_participants_next(&pit, &id)) {
            count++;
        }
        CHECK_EQ(count, 2);
        CHECK_EQ(kb_call_peer_of(&ev, 42, &peer), KB_OK);
        CHECK(peer == 99);
    }

    KB_TEST("событие: transmitted-data SDP и кандидат");
    {
        static const char *sdp =
            "{\"notification\":\"transmitted-data\",\"data\":{\"sdp\":{"
            "\"type\":\"offer\",\"sdp\":\"v=0\\r\\no=- 1 2\"}}}";
        static const char *cand =
            "{\"notification\":\"transmitted-data\",\"data\":{\"candidate\":{"
            "\"candidate\":\"candidate:1 1 udp\",\"sdpMid\":\"0\","
            "\"sdpMLineIndex\":0}}}";
        kb_call_event ev;
        kb_transmitted_type td;
        int64_t mline = -1;

        CHECK_EQ(kb_call_event_parse(sdp, strlen(sdp), &ev), KB_OK);
        CHECK_EQ(ev.type, KB_CALL_TRANSMITTED);
        CHECK_EQ(kb_call_transmitted_type(&ev, &td), KB_OK);
        CHECK_EQ(td, KB_TD_SDP);
        CHECK_EQ(kb_call_sdp_type(&ev, buf, sizeof buf), KB_OK);
        CHECK_EQ(strcmp(buf, "offer"), 0);
        CHECK_EQ(kb_call_sdp(&ev, buf, sizeof buf), KB_OK);
        CHECK_EQ(strcmp(buf, "v=0\r\no=- 1 2"), 0);  /* escape раскрыт */

        CHECK_EQ(kb_call_event_parse(cand, strlen(cand), &ev), KB_OK);
        CHECK_EQ(kb_call_transmitted_type(&ev, &td), KB_OK);
        CHECK_EQ(td, KB_TD_CANDIDATE);
        CHECK_EQ(kb_call_candidate(&ev, buf, sizeof buf), KB_OK);
        CHECK_EQ(strncmp(buf, "candidate:", 10), 0);
        CHECK_EQ(kb_call_candidate_mid(&ev, buf, sizeof buf), KB_OK);
        CHECK_EQ(strcmp(buf, "0"), 0);
        CHECK_EQ(kb_call_candidate_mline(&ev, &mline), KB_OK);
        CHECK(mline == 0);
    }

    KB_TEST("событие: hungup, closed, error, прочее");
    {
        kb_call_event ev;
        CHECK_EQ(kb_call_event_parse("{\"notification\":\"hungup\"}",
                                     strlen("{\"notification\":\"hungup\"}"), &ev),
                 KB_OK);
        CHECK_EQ(ev.type, KB_CALL_HUNGUP);
        {
            static const char *c = "{\"notification\":\"closed-conversation\"}";
            CHECK_EQ(kb_call_event_parse(c, strlen(c), &ev), KB_OK);
        }
        CHECK_EQ(ev.type, KB_CALL_CLOSED);
        {
            static const char *e = "{\"type\":\"error\",\"error\":\"boom\"}";
            CHECK_EQ(kb_call_event_parse(e, strlen(e), &ev), KB_OK);
            CHECK_EQ(ev.type, KB_CALL_ERROR);
            CHECK_EQ(kb_call_error_text(&ev, buf, sizeof buf), KB_OK);
            CHECK_EQ(strcmp(buf, "boom"), 0);
        }
        {
            static const char *o = "{\"notification\":\"whatever\"}";
            CHECK_EQ(kb_call_event_parse(o, strlen(o), &ev), KB_OK);
        }
        CHECK_EQ(ev.type, KB_CALL_OTHER);
    }

    KB_TEST("сигнализация: конверт и seq");
    {
        kb_ws2 ws2;
        char cmd[1024];
        size_t len = 0;
        int64_t seq = -1;

        kb_ws2_init(&ws2);
        CHECK_EQ(kb_ws2_accept_call(&ws2, cmd, sizeof cmd, &len, &seq), KB_OK);
        CHECK(seq == 1);
        CHECK_EQ(strcmp(cmd, "{\"command\":\"accept-call\",\"sequence\":1}"), 0);

        CHECK_EQ(kb_ws2_hangup(&ws2, "bye", cmd, sizeof cmd, &len, &seq), KB_OK);
        CHECK(seq == 2);
        CHECK_EQ(strcmp(cmd,
                        "{\"command\":\"hangup\",\"reason\":\"bye\","
                        "\"sequence\":2}"),
                 0);
    }

    KB_TEST("сигнализация: transmit sdp экранирует");
    {
        kb_ws2 ws2;
        char cmd[1024];
        int64_t seq = -1;
        kb_ws2_init(&ws2);
        CHECK_EQ(kb_ws2_transmit_sdp(&ws2, 99, "offer", "v=0\r\na=\"x\"", cmd,
                                     sizeof cmd, NULL, &seq),
                 KB_OK);
        CHECK(strstr(cmd, "\"participantId\":99") != NULL);
        CHECK(strstr(cmd, "\"type\":\"offer\"") != NULL);
        /* перевод строки и кавычка экранированы */
        CHECK(strstr(cmd, "v=0\\r\\na=\\\"x\\\"") != NULL);
        CHECK(strstr(cmd, "\"capabilities\":\"1\"") != NULL);
        CHECK(strstr(cmd, "\"sequence\":1}") != NULL);
    }

    KB_TEST("сигнализация: change-media булевы");
    {
        kb_ws2 ws2;
        char cmd[1024];
        kb_ws2_init(&ws2);
        CHECK_EQ(kb_ws2_change_media(&ws2, 1, 0, 0, cmd, sizeof cmd, NULL, NULL),
                 KB_OK);
        CHECK(strstr(cmd, "\"isAudioEnabled\":true") != NULL);
        CHECK(strstr(cmd, "\"isVideoEnabled\":false") != NULL);
        CHECK(strstr(cmd, "\"isScreenSharingEnabled\":false") != NULL);
        CHECK(strstr(cmd, "\"isAnimojiEnabled\":false") != NULL);
    }

    KB_TEST("сигнализация: классификация входящих");
    {
        int64_t seq = -1;
        CHECK_EQ(kb_ws2_classify("ping", 4, &seq), KB_WS2_PING);
        {
            static const char *r =
                "{\"sequence\":7,\"response\":\"hangup\",\"type\":\"response\"}";
            CHECK_EQ(kb_ws2_classify(r, strlen(r), &seq), KB_WS2_RESPONSE);
            CHECK(seq == 7);
        }
        {
            static const char *e =
                "{\"sequence\":8,\"type\":\"error\",\"error\":\"nope\"}";
            CHECK_EQ(kb_ws2_classify(e, strlen(e), &seq), KB_WS2_ERROR);
            CHECK(seq == 8);
        }
        {
            static const char *n =
                "{\"type\":\"notification\",\"notification\":\"connection\"}";
            CHECK_EQ(kb_ws2_classify(n, strlen(n), &seq), KB_WS2_NOTIFICATION);
        }
        CHECK_EQ(kb_ws2_classify("{}", 2, &seq), KB_WS2_IGNORE);
        CHECK_EQ(kb_ws2_classify("garbage", 7, &seq), KB_WS2_IGNORE);
    }

    KB_TEST("vcp: битый вход");
    {
        kb_vcp v;
        CHECK_EQ(kb_vcp_decode("nocolon", 7, scratch, sizeof scratch, &v),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_vcp_decode(":abc", 4, scratch, sizeof scratch, &v),
                 KB_ERR_INVALID);
        CHECK_EQ(kb_vcp_decode("0:abc", 5, scratch, sizeof scratch, &v),
                 KB_ERR_TRUNCATED);
        /* rawLen больше scratch */
        CHECK_EQ(kb_vcp_decode("100:AAAA", 8, scratch, 50, &v),
                 KB_ERR_TRUNCATED);
    }
}
