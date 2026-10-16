#! /usr/bin/env bash
#
# Compiles Skia (aseprite-m124 branch) for Android (arm64 and x64).
#
# Usage:
#   ./build-skia.sh [SKIA_DIR] [NDK_DIR]
#
# Requirements: git, python3, ninja (pip install ninja), Android NDK.
#

set -e

SKIA_DIR=${1:-$HOME/deps/skia}
NDK_DIR=${2:-$ANDROID_NDK_HOME}
SKIA_TAG=$(cat "$(dirname "${BASH_SOURCE[0]}")/../laf/misc/skia-tag.txt" | xargs)

if [ -z "$NDK_DIR" ] ; then
    echo "Specify the Android NDK directory as the second argument or in ANDROID_NDK_HOME"
    exit 1
fi

if [ ! -d "$SKIA_DIR" ] ; then
    git clone --depth 1 --branch "$SKIA_TAG" https://github.com/aseprite/skia.git "$SKIA_DIR"
fi

cd "$SKIA_DIR"
python3 tools/git-sync-deps || python3 tools/git-sync-deps

for cpu in arm64 x64 ; do
    bin/gn gen out/Android-$cpu --args="
      target_os=\"android\"
      target_cpu=\"$cpu\"
      ndk=\"$NDK_DIR\"
      ndk_api=26
      is_debug=false
      is_official_build=true
      skia_use_gl=true
      skia_use_vulkan=false
      skia_use_system_expat=false
      skia_use_system_icu=false
      skia_use_system_libjpeg_turbo=false
      skia_use_system_libpng=false
      skia_use_system_libwebp=false
      skia_use_system_zlib=false
      skia_use_freetype=true
      skia_use_harfbuzz=true
      skia_use_system_freetype2=false
      skia_use_system_harfbuzz=false
      skia_enable_pdf=false
      skia_use_dng_sdk=false
      extra_cflags_cc=[\"-frtti\"]
      extra_ldflags=[\"-Wl,-z,max-page-size=16384\"]"
    ninja -C out/Android-$cpu skia modules
done
