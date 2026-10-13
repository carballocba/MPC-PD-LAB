#!/usr/bin/env bash
# Runs INSIDE the toolchain container; separate libpd source/object trees per ABI.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
MODE=${1:-armv7}
DEPS=${DEPS_DIR:-$ROOT/work/deps}
case "$MODE" in
  armv7) CC=arm-linux-gnueabihf-gcc; FLAGS=(-march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=hard);;
  desktop) CC=gcc; FLAGS=();;
  *) echo 'usage: build.sh armv7|desktop' >&2; exit 2;;
esac
FLAGS+=(-D_FILE_OFFSET_BITS=64)
command -v "$CC" >/dev/null
python3 "$ROOT/scripts/check_pins.py" "$DEPS"
OUT=$ROOT/work/$MODE
mkdir -p "$OUT"
rm -rf "$OUT/package"
rm -f "$OUT/mpc-pd-delay-$MODE.tar.gz"
# Never run make clean or cross-compile in the existing desktop dependency cache.
rm -rf "$OUT/libpd"
mkdir -p "$OUT/libpd"
tar -C "$DEPS/libpd" --exclude=.git --exclude='*.o' --exclude='libs/*' -cf - . | tar -C "$OUT/libpd" -xf -
mkdir -p "$OUT/libpd/libs"
make -C "$OUT/libpd" -j"${JOBS:-2}" CC="$CC" UTIL=false EXTRA=false MULTI=false SETLOCALE=false \
  ADDITIONAL_CFLAGS="${FLAGS[*]} -fvisibility=hidden" libpd 2>&1 | tee "$OUT/libpd-build.log"
# Compile an archive from the same PIC objects, avoiding libpd .so lookup and symbol interposition.
find "$OUT/libpd" -name '*.o' -print | LC_ALL=C sort > "$OUT/objects.txt"
if [[ $MODE == armv7 ]]; then AR=arm-linux-gnueabihf-ar; else AR=ar; fi
mapfile -t OBJS < "$OUT/objects.txt"
"$AR" rcs "$OUT/libpd.a" "${OBJS[@]}"
python3 "$DEPS/mpc-vst-plugins/tools/gen_vst.py" "$ROOT/packaging/vst.json"
PKG="$OUT/package/MPC-PD-LAB - VST - MPC PD Delay"
mkdir -p "$PKG/data" "$PKG/licenses"
"$CC" -O2 -Wall -Wextra -Wno-unused-parameter -std=gnu11 -fPIC -fvisibility=hidden \
  "${FLAGS[@]}" -I"$ROOT/packaging/build" -I"$DEPS/mpc-vst-plugins/wrapper" \
  -I"$OUT/libpd/libpd_wrapper" -I"$OUT/libpd/pure-data/src" \
  "$ROOT/src/libpd_engine.c" "$DEPS/mpc-vst-plugins/wrapper/vst2_wrap.c" \
  "$OUT/libpd.a" -shared -Wl,--no-undefined,-Bsymbolic,--exclude-libs,ALL -lm -ldl -lpthread \
  -o "$PKG/mpc_pd_delay.so" 2>&1 | tee "$OUT/plugin-build.log"
cp "$ROOT/patches/libpd_host.pd" "$ROOT/patches/stereo_delay_smoke.pd" "$PKG/data/"
cp "$ROOT/packaging/build/pluginlist-entry.xml" "$OUT/package/"
cp "$ROOT/packaging/vst.json" "$PKG/"
cp "$ROOT/docs/ARMV7_BUILD.md" "$OUT/package/BUILD_AND_TESTING.md"
cp "$ROOT/docs/PHASE2_VALIDATION.md" "$OUT/package/RECORDED_VALIDATION.md"
cp "$OUT/libpd/LICENSE.txt" "$PKG/licenses/libpd.txt"
cp "$OUT/libpd/pure-data/LICENSE.txt" "$PKG/licenses/pure-data.txt"
cp "$DEPS/mpc-vst-plugins/LICENSE" "$PKG/licenses/wrapper.txt"
"$CC" -O2 -Wall -Wextra -Werror "${FLAGS[@]}" "$ROOT/tests/vst_host.c" -ldl -lm -o "$OUT/vst_host"
"$CC" -O2 -Wall -Wextra -Werror "${FLAGS[@]}" \
  -I"$DEPS/mpc-vst-plugins/wrapper" -I"$OUT/libpd/libpd_wrapper" -I"$OUT/libpd/pure-data/src" \
  "$ROOT/src/libpd_engine.c" "$ROOT/tests/audio_smoke.c" "$OUT/libpd.a" -lm -ldl -lpthread -o "$OUT/audio_smoke"
if [[ $MODE == armv7 ]]; then
  python3 "$ROOT/scripts/verify_elf.py" "$PKG/mpc_pd_delay.so" | tee "$OUT/elf-report.txt"
  RUN=(qemu-arm -L /usr/arm-linux-gnueabihf)
else RUN=(); fi
"${RUN[@]}" "$OUT/audio_smoke" engine "$ROOT/patches"
"${RUN[@]}" "$OUT/audio_smoke" hosted "$ROOT/patches"
"${RUN[@]}" "$OUT/vst_host" "$PKG/mpc_pd_delay.so" | tee "$OUT/host-report.txt"
# Missing either Pd resource must fail entry, rather than create a silent effect.
for PATCH in libpd_host.pd stereo_delay_smoke.pd; do
  mv "$PKG/data/$PATCH" "$OUT/$PATCH.saved"
  if ! "${RUN[@]}" "$OUT/vst_host" "$PKG/mpc_pd_delay.so" --expect-failure; then
    mv "$OUT/$PATCH.saved" "$PKG/data/$PATCH"
    exit 1
  fi
  mv "$OUT/$PATCH.saved" "$PKG/data/$PATCH"
done
cp /toolchain-packages.txt "$OUT/" 2>/dev/null || true
(cd "$OUT/package" && find . -type f ! -name SHA256SUMS -print0 | LC_ALL=C sort -z | xargs -0 sha256sum > SHA256SUMS)
tar --sort=name --mtime=@0 --owner=0 --group=0 --numeric-owner -C "$OUT/package" -czf "$OUT/mpc-pd-delay-$MODE.tar.gz" .
