/* Кадры WebSocket: сборка с маской, разбор, дробный вход. */

#include <string.h>

#include "kb_test.h"
#include "kolibri.h"
#include "kolibri_ws.h"

void kb_tests_ws(void)
{
    KB_TEST("сборка текстового кадра и обратный разбор");
    {
        uint8_t frame[128];
        size_t flen = 0;
        kb_ws_frame parsed;
        size_t consumed = 0;
        const char *msg = "hello ws2";
        uint8_t unmasked[64];
        size_t i;

        CHECK_EQ(kb_ws_build_text(msg, strlen(msg), 0xDEADBEEF, frame,
                                  sizeof frame, &flen),
                 KB_OK);
        CHECK_EQ((frame[0] & 0x80), 0x80);       /* FIN */
        CHECK_EQ((frame[0] & 0x0F), KB_WS_TEXT);
        CHECK_EQ((frame[1] & 0x80), 0x80);       /* MASK у клиента */

        /* Снимем маску вручную и сверим с исходным текстом. */
        {
            uint8_t mask[4];
            size_t hdr = 2;
            mask[0] = frame[hdr]; mask[1] = frame[hdr + 1];
            mask[2] = frame[hdr + 2]; mask[3] = frame[hdr + 3];
            hdr += 4;
            for (i = 0; i < strlen(msg); i++) {
                unmasked[i] = (uint8_t)(frame[hdr + i] ^ mask[i & 3]);
            }
            CHECK_MEM(unmasked, strlen(msg), msg, strlen(msg));
        }

        /* Серверный (немаскированный) кадр разбираем. */
        {
            uint8_t srv[64];
            srv[0] = 0x80 | KB_WS_TEXT;
            srv[1] = (uint8_t)strlen(msg); /* без маски */
            memcpy(srv + 2, msg, strlen(msg));
            CHECK_EQ(kb_ws_parse(srv, 2 + strlen(msg), &parsed, &consumed),
                     KB_OK);
            CHECK_EQ(parsed.opcode, KB_WS_TEXT);
            CHECK_EQ(parsed.fin, 1);
            CHECK_MEM(parsed.payload, parsed.payload_len, msg, strlen(msg));
            CHECK_EQ(consumed, 2 + strlen(msg));
        }
    }

    KB_TEST("расширенные длины 126 и 127");
    {
        uint8_t big[70000];
        uint8_t frame[70100];
        size_t flen = 0;
        kb_ws_frame parsed;
        memset(big, 'x', sizeof big);

        /* 200 байт → 16-битная длина. */
        CHECK_EQ(kb_ws_build(KB_WS_BINARY, big, 200, 1, frame, sizeof frame,
                             &flen),
                 KB_OK);
        CHECK_EQ((frame[1] & 0x7F), 126);
        /* 70000 байт → 64-битная длина, разбор находит всю нагрузку. */
        {
            /* Немаскированный серверный кадр той же длины. */
            uint8_t *srv = frame;
            size_t pos;
            srv[0] = 0x80 | KB_WS_BINARY;
            srv[1] = 127;
            {
                int s;
                for (s = 0; s < 8; s++) {
                    srv[2 + s] = (uint8_t)(((uint64_t)70000 >> (56 - s * 8)) &
                                           0xFF);
                }
            }
            pos = 10;
            memcpy(srv + pos, big, 70000);
            CHECK_EQ(kb_ws_parse(srv, pos + 70000, &parsed, NULL), KB_OK);
            CHECK_EQ(parsed.payload_len, (size_t)70000);
        }
    }

    KB_TEST("частичный кадр — TRUNCATED, не ошибка");
    {
        uint8_t srv[16];
        kb_ws_frame parsed;
        srv[0] = 0x80 | KB_WS_TEXT;
        srv[1] = 10;                 /* заявлено 10 байт нагрузки */
        srv[2] = 'a';
        CHECK_EQ(kb_ws_parse(srv, 3, &parsed, NULL), KB_ERR_TRUNCATED);
        CHECK_EQ(kb_ws_parse(srv, 1, &parsed, NULL), KB_ERR_TRUNCATED);
    }

    KB_TEST("замаскированный серверный кадр отвергается");
    {
        uint8_t srv[8];
        kb_ws_frame parsed;
        srv[0] = 0x80 | KB_WS_TEXT;
        srv[1] = 0x80 | 1;           /* MASK выставлен — недопустимо от сервера */
        srv[2] = srv[3] = srv[4] = srv[5] = 0;
        srv[6] = 0;
        CHECK_EQ(kb_ws_parse(srv, 7, &parsed, NULL), KB_ERR_INVALID);
    }

    KB_TEST("ping/pong и close");
    {
        uint8_t frame[16];
        size_t flen = 0;
        kb_ws_frame parsed;
        uint8_t srv_ping[2];

        CHECK_EQ(kb_ws_build(KB_WS_PONG, NULL, 0, 0x01020304, frame,
                             sizeof frame, &flen),
                 KB_OK);
        CHECK_EQ((frame[0] & 0x0F), KB_WS_PONG);
        CHECK_EQ(flen, (size_t)6);   /* 2 заголовок + 4 маска, нет данных */

        srv_ping[0] = 0x80 | KB_WS_PING;
        srv_ping[1] = 0;
        CHECK_EQ(kb_ws_parse(srv_ping, 2, &parsed, NULL), KB_OK);
        CHECK_EQ(parsed.opcode, KB_WS_PING);
        CHECK_EQ(parsed.payload_len, (size_t)0);
    }

    KB_TEST("RSV-биты и мал буфер");
    {
        uint8_t srv[4];
        uint8_t frame[4];
        kb_ws_frame parsed;
        size_t flen = 0;
        srv[0] = 0x80 | 0x40 | KB_WS_TEXT;  /* RSV1 выставлен */
        srv[1] = 0;
        CHECK_EQ(kb_ws_parse(srv, 2, &parsed, NULL), KB_ERR_INVALID);

        CHECK_EQ(kb_ws_build_text("toolong", 7, 0, frame, sizeof frame, &flen),
                 KB_ERR_TOO_LARGE);
    }
}
