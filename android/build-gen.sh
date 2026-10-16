#! /usr/bin/env bash
#
# Compiles the "gen" tool for the host machine (Linux/macOS), required
# to cross-compile Aseprite for Android.
#
# Usage: ./build-gen.sh [BUILD_DIR]

set -e
BUILD_DIR=${1:-$HOME/deps/build-gen-host}
SRC_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DLAF_BACKEND=none \
  -DENABLE_UPDATER=OFF -DENABLE_NEWS=OFF -DENABLE_WEBSOCKET=OFF \
  -DENABLE_CCACHE=OFF -DENABLE_SCRIPTING=OFF \
  -DLAF_WITH_EXAMPLES=OFF -DLAF_WITH_TESTS=OFF "$SRC_DIR"
ninja gen
echo "gen compiled in $BUILD_DIR/bin/gen"
