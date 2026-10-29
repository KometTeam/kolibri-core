# kolibri-core — сборка статической и разделяемой библиотеки плюс тесты.
#
#   make            статическая + разделяемая библиотека
#   make test       собрать и прогнать тесты
#   make c89        проверить, что ядро компилируется как строгий C89
#   make vectors    перегенерировать таблицы векторов из tests/vectors.json
#   make clean

CC      ?= cc
AR      ?= ar
PYTHON  ?= python3
# Чем запускать собранные тесты. Пусто — напрямую; для кросс-сборки сюда
# подставляется qemu, и тогда прогон идёт на чужой архитектуре по-настоящему.
RUNNER  ?=

STD     ?= -std=c99
OPT     ?= -O2
WARN     = -Wall -Wextra -pedantic -Wshadow -Wstrict-prototypes
CPPFLAGS = -Iinclude
CFLAGS  += $(STD) $(OPT) $(WARN)

BUILD    = build
LIB      = $(BUILD)/libkolibri.a

SRCS     = src/kolibri_codec.c src/kolibri_framing.c src/kolibri_lz4.c \
           src/kolibri_status.c src/kolibri_msgpack.c \
           src/kolibri_session.c src/kolibri_proxy.c src/kolibri_sha256.c src/kolibri_mincifry_ca.c src/kolibri_auth.c src/kolibri_opcodes.c src/kolibri_ops.c src/kolibri_media.c src/kolibri_json.c \
       src/kolibri_calls.c src/kolibri_ws.c src/kolibri_mpjson.c \
           src/kolibri_transport.c src/kolibri_client.c src/kolibri_api.c
OBJS     = $(BUILD)/kolibri_codec.o $(BUILD)/kolibri_framing.o \
           $(BUILD)/kolibri_lz4.o $(BUILD)/kolibri_status.o \
           $(BUILD)/kolibri_msgpack.o $(BUILD)/kolibri_session.o \
           $(BUILD)/kolibri_proxy.o $(BUILD)/kolibri_transport.o \
           $(BUILD)/kolibri_sha256.o $(BUILD)/kolibri_auth.o \
           $(BUILD)/kolibri_opcodes.o $(BUILD)/kolibri_ops.o $(BUILD)/kolibri_media.o \
       $(BUILD)/kolibri_json.o $(BUILD)/kolibri_calls.o $(BUILD)/kolibri_ws.o $(BUILD)/kolibri_mpjson.o

TEST_SRCS = tests/main.c tests/kb_test.c tests/test_lz4.c tests/test_codec.c \
            tests/test_framing.c tests/test_vectors.c tests/test_fuzz.c \
            tests/test_msgpack.c tests/test_session.c tests/test_proxy.c \
            tests/test_auth.c tests/test_ops.c tests/test_media.c \
            tests/test_json.c tests/test_calls.c tests/test_ws.c \
            tests/test_mpjson.c tests/test_tap.c tests/test_decompiled.c \
            tests/vectors_data.c

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  SOEXT   = dylib
  SOFLAGS = -dynamiclib -install_name @rpath/libkolibri.dylib
else
  SOEXT   = so
  SOFLAGS = -shared
endif
SHARED = $(BUILD)/libkolibri.$(SOEXT)

# Цель по умолчанию задаётся явно: иначе ею становится первая в файле, и
# добавление цели выше all тихо ломает голый `make`.
.DEFAULT_GOAL := all

.PHONY: all test asan c89 shared python-test vectors vectors-check dump \
        client integration socket-checks zstd-check opcodes \
        opcodes-check schema schema-check clean

# Полный клиент на C: транспорт + сессия. Он же интеграционный тест.
# С TLS: make client OPENSSL=/usr/local/opt/openssl@3
client: $(BUILD)/client

ifdef OPENSSL
  TLS_CFLAGS = -DKB_WITH_OPENSSL -I$(OPENSSL)/include
  TLS_LDFLAGS = -L$(OPENSSL)/lib -lssl -lcrypto
  TLS_SRCS = src/kolibri_tls_openssl.c
endif

$(BUILD)/client: examples/c/client.c $(SRCS) $(TLS_SRCS) include/kolibri.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TLS_CFLAGS) -o $@ examples/c/client.c \
	    $(SRCS) $(TLS_SRCS) $(TLS_LDFLAGS)

# Клиент пересобирается всегда: наличие OPENSSL в зависимостях не отражается,
# и иначе после смены флага прогонялся бы старый бинарь.
# Адаптер к libzstd на настоящих данных. Нужен путь к библиотеке:
#   make zstd-check ZSTD=/usr/local
# Для сборки только на распаковку (так и надо для клиента) добавь
#   ZSTD_DECODE_ONLY=1 — тогда тест возьмёт компрессор из внешней утилиты zstd.
ifdef ZSTD
  ZSTD_CFLAGS = -DKB_WITH_ZSTD -I$(ZSTD)/include -I$(ZSTD)/lib \
                $(if $(ZSTD_DECODE_ONLY),-DKB_ZSTD_DECODE_ONLY,)
  ZSTD_LDFLAGS = $(if $(wildcard $(ZSTD)/lib/libzstd.a),$(ZSTD)/lib/libzstd.a,-L$(ZSTD)/lib -lzstd)
  ZSTD_SRCS = src/kolibri_zstd.c
endif

zstd-check: $(BUILD)/zstd_check
	$(RUNNER) $(BUILD)/zstd_check

$(BUILD)/zstd_check: tests/integration/zstd_check.c $(SRCS) $(ZSTD_SRCS) include/kolibri.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ZSTD_CFLAGS) -o $@ \
	    tests/integration/zstd_check.c $(SRCS) $(ZSTD_SRCS) $(ZSTD_LDFLAGS)

# Проверки, которым нужен настоящий сокет: обрыв соединения и SIGPIPE.
socket-checks: $(BUILD)/socket_checks
	$(RUNNER) $(BUILD)/socket_checks

$(BUILD)/socket_checks: tests/integration/socket_checks.c $(SRCS) include/kolibri.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/integration/socket_checks.c $(SRCS)

integration: socket-checks | $(BUILD)
	@rm -f $(BUILD)/client
	@$(MAKE) --no-print-directory $(BUILD)/client OPENSSL=$(OPENSSL)
	$(PYTHON) tests/integration/run.py $(BUILD)/client $(if $(OPENSSL),--tls,)

all: $(LIB) $(SHARED)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: src/%.c include/kolibri.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	$(AR) rcs $@ $(OBJS)

shared: $(SHARED)

$(SHARED): $(SRCS) include/kolibri.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC $(SOFLAGS) -o $@ $(SRCS)

$(BUILD)/run_tests: $(SRCS) $(TEST_SRCS) include/kolibri.h | $(BUILD)
	$(CC) $(CPPFLAGS) -Itests $(CFLAGS) -o $@ $(SRCS) $(TEST_SRCS)

test: $(BUILD)/run_tests
	$(RUNNER) $(BUILD)/run_tests

# Тот же прогон под ASan и UBSan: ядро разбирает недоверенные байты, так что
# проверка границ — не роскошь.
asan: | $(BUILD)
	$(CC) $(CPPFLAGS) -Itests -std=c99 -O1 -g -fno-omit-frame-pointer \
	    -fsanitize=address,undefined -fno-sanitize-recover=all $(WARN) \
	    -o $(BUILD)/run_tests_asan $(SRCS) $(TEST_SRCS)
	$(RUNNER) $(BUILD)/run_tests_asan

# Строгий C89 — тот диалект, который принимают MSVC 2013 и clang эпохи iOS 6.
# Проверяем только ядро: тесты пользуются длинными строками и другими вольностями.
# Переносимое ядро обязано быть строгим C89. Транспорт исключён не из
# снисхождения к нему, а потому что системные заголовки в режиме
# -std=c89 -pedantic-errors прячут POSIX-объявления; его синтаксис всё равно
# C89, и это ловит обычная сборка с -pedantic.
CORE_SRCS = src/kolibri_codec.c src/kolibri_framing.c src/kolibri_lz4.c \
            src/kolibri_status.c src/kolibri_msgpack.c \
            src/kolibri_session.c src/kolibri_proxy.c src/kolibri_sha256.c src/kolibri_mincifry_ca.c \
            src/kolibri_auth.c src/kolibri_opcodes.c src/kolibri_ops.c \
            src/kolibri_media.c src/kolibri_json.c \
            src/kolibri_calls.c src/kolibri_ws.c src/kolibri_mpjson.c

c89: | $(BUILD)
	@rm -f $(BUILD)/c89.log
	@for f in $(CORE_SRCS); do \
	    $(CC) $(CPPFLAGS) -std=c89 -pedantic-errors $(WARN) -O1 -c $$f \
	        -o $(BUILD)/c89.o 2>> $(BUILD)/c89.log \
	        || echo "не собралось: $$f" >> $(BUILD)/c89.log; \
	done
	@rm -f $(BUILD)/c89.o
	@if [ -s $(BUILD)/c89.log ]; then cat $(BUILD)/c89.log; exit 1; \
	else echo "C89: чисто"; fi

# Дамп того, что производит C, — для проверки эталонной реализацией.
# См. «Дифференциальная проверка» в README.
dump: $(BUILD)/dump_c_vectors
	$(BUILD)/dump_c_vectors

$(BUILD)/dump_c_vectors: tools/dump_c_vectors.c $(SRCS) include/kolibri.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tools/dump_c_vectors.c $(SRCS)

# Обёртка на ctypes гоняет те же вектора, что и тесты на C.
python-test: $(SHARED)
	cd python && KOLIBRI_LIBRARY=../$(SHARED) $(PYTHON) -m unittest discover -s . -v

schema:
	$(PYTHON) tools/gen_schema.py
	$(PYTHON) tools/gen_python.py

schema-check:
	$(PYTHON) tools/gen_schema.py --check
	$(PYTHON) tools/gen_python.py --check

opcodes:
	$(PYTHON) tools/gen_opcodes.py

opcodes-check:
	$(PYTHON) tools/gen_opcodes.py --check

vectors:
	$(PYTHON) tools/gen_vectors.py

vectors-check:
	$(PYTHON) tools/gen_vectors.py --check

clean:
	rm -rf $(BUILD)
