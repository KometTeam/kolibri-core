/* Микрокаркас для тестов: без зависимостей, C89. */

#ifndef KB_TEST_H
#define KB_TEST_H

#include <stddef.h>

extern long kb_test_checks;
extern long kb_test_failures;

void kb_test_begin(const char *name);
int kb_test_check(int ok, const char *file, int line, const char *expr);
int kb_test_eq_long(long got, long want, const char *file, int line,
                    const char *what);
int kb_test_eq_mem(const void *got, size_t got_len, const void *want,
                   size_t want_len, const char *file, int line,
                   const char *what);

#define KB_TEST(name) kb_test_begin(name)
#define CHECK(expr) kb_test_check((expr) ? 1 : 0, __FILE__, __LINE__, #expr)
#define CHECK_EQ(got, want) \
    kb_test_eq_long((long)(got), (long)(want), __FILE__, __LINE__, #got)
#define CHECK_MEM(got, got_len, want, want_len) \
    kb_test_eq_mem((got), (got_len), (want), (want_len), __FILE__, __LINE__, #got)

/* Детерминированный xorshift64: те же данные на любой платформе. */
typedef struct {
    unsigned long lo;
    unsigned long hi;
} kb_rng;

void kb_rng_seed(kb_rng *rng, unsigned long seed);
unsigned char kb_rng_byte(kb_rng *rng);
unsigned long kb_rng_below(kb_rng *rng, unsigned long bound);

/* Формы полезной нагрузки для round-trip: разные профили сжимаемости. */
#define KB_SHAPE_COUNT 6
size_t kb_gen_payload(int shape, size_t len, unsigned char *out, kb_rng *rng);
const char *kb_shape_name(int shape);

void kb_tests_lz4(void);
void kb_tests_codec(void);
void kb_tests_framing(void);
void kb_tests_vectors(void);
void kb_tests_fuzz(void);
void kb_tests_msgpack(void);
void kb_tests_session(void);
void kb_tests_proxy(void);
void kb_tests_auth(void);
void kb_tests_opcodes(void);
void kb_tests_ops(void);
void kb_tests_media(void);
void kb_tests_json(void);
void kb_tests_calls(void);
void kb_tests_ws(void);
void kb_tests_mpjson(void);
void kb_tests_tap(void);
void kb_tests_decompiled(void);

#endif /* KB_TEST_H */
