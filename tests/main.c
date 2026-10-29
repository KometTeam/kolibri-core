#include <stdio.h>

#include "kb_test.h"

int main(void)
{
    printf("kolibri-core: тесты\n\n");

    printf("LZ4 block\n");
    kb_tests_lz4();
    printf("\nКодек\n");
    kb_tests_codec();
    printf("\nРеассемблер\n");
    kb_tests_framing();
    printf("\nMessagePack\n");
    kb_tests_msgpack();
    printf("\nМедиа\n");
    kb_tests_media();
    printf("\nJSON\n");
    kb_tests_json();
    printf("\nЗвонки\n");
    kb_tests_calls();
    printf("\nWebSocket\n");
    kb_tests_ws();
    printf("\nМост msgpack↔JSON\n");
    kb_tests_mpjson();
    printf("\nТап на трафик\n");
    kb_tests_tap();
    printf("\nОперации из декомпила\n");
    kb_tests_decompiled();
    printf("\nСтруктуры из схемы\n");
    kb_tests_ops();
    printf("\nОпкоды\n");
    kb_tests_opcodes();
    printf("\nАвторизация\n");
    kb_tests_auth();
    printf("\nПрокси\n");
    kb_tests_proxy();
    printf("\nМашина сессии\n");
    kb_tests_session();
    printf("\nВектора эталонной реализации\n");
    kb_tests_vectors();
    printf("\nБитый вход\n");
    kb_tests_fuzz();

    printf("\n%ld проверок, %ld провалов\n", kb_test_checks, kb_test_failures);
    return kb_test_failures == 0 ? 0 : 1;
}
