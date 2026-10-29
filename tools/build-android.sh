#!/bin/sh
# Сборка libkolibri.so под Android.
#
#   NDK=~/ndk-old/android-ndk-r13b API=9  tools/build-android.sh   # Android 2.3
#   NDK=~/Library/Android/sdk/ndk/27.0.12077973 API=21 tools/build-android.sh
#
# Для самого древнего Android (2.3 Gingerbread, API 9) нужен СТАРЫЙ NDK
# (r13b — последний с ABI armeabi/ARMv5 и платформой android-9). Современные
# NDK (r27+) умеют только API 21+ и не имеют armeabi.
#
# Ядро — freestanding C89, из ОС зависит только транспорт (POSIX-сокеты,
# getaddrinfo, select); всё это есть в Bionic с API 9. TLS/zstd — подключаемые
# бэкенды, в этот .so не входят (собери и слинкуй их отдельно при надобности).

set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
: "${NDK:?задай NDK=/путь/к/android-ndk}"
: "${API:=9}"
OUT="${OUT:-$ROOT/build/android}"

HOST=darwin-x86_64
[ -d "$NDK/toolchains/llvm/prebuilt/$HOST" ] || HOST=linux-x86_64
CLANG="$NDK/toolchains/llvm/prebuilt/$HOST/bin/clang"
GCC="$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/$HOST"
STRIP="$GCC/bin/arm-linux-androideabi-strip"
SYSROOT="$NDK/platforms/android-$API/arch-arm"

# Только протокол + транспорт; опциональные бэкенды исключаем.
SRCS=$(ls "$ROOT"/src/*.c | grep -vE 'tls_openssl|tls_mbedtls|zstd')

build() {   # $1 abi   $2 target   $3 доп.флаги
    abi=$1; target=$2; extra=$3
    out="$OUT/$abi/libkolibri.so"
    mkdir -p "$OUT/$abi"
    # shellcheck disable=SC2086
    "$CLANG" --target="$target" -gcc-toolchain "$GCC" --sysroot="$SYSROOT" \
        $extra -fPIC -I"$ROOT/include" -std=c99 -O2 \
        -shared -o "$out" $SRCS
    [ -x "$STRIP" ] && "$STRIP" "$out" || true
    printf "  %-14s %5d КБ  %s\n" "$abi" "$(( $(wc -c < "$out") / 1024 ))" "$out"
}

echo "kolibri-core -> Android API $API (NDK $(cat "$NDK/source.properties" 2>/dev/null | sed -n 's/Pkg.Revision = //p'))"
build armeabi     armv5te-none-linux-androideabi "-march=armv5te -mtune=xscale -msoft-float"
build armeabi-v7a armv7-none-linux-androideabi    "-march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp"
echo "готово: $OUT"
