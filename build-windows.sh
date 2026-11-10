#!/bin/bash
# Sydalesis — Windows x64 build, cross-compiled on Linux with MinGW-w64 (how every release so far was made).
#   sudo apt install g++-mingw-w64-x86-64-posix make git zip
# Output: dist/Sydalesis-<version>-win64.zip
set -euo pipefail
cd "$(dirname "$0")"
ROOT="$PWD"
VERSION="5.1.7"
DPF_COMMIT=4238e1c7f0351bbe488d79f0899c540543ac7583
if [ ! -d dpf ]; then git clone --recursive https://github.com/DISTRHO/DPF.git dpf; fi
( cd dpf && git checkout -q "$DPF_COMMIT" && git submodule update --init --recursive -q && git checkout -q -- . && git apply "$ROOT/source/dpf-sydalesis.patch" )
( cd source && rm -rf build bin && make -j"$(nproc)" DPF_PATH="$ROOT/dpf" CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ WINDOWS=true HAVE_OPENGL=true )
PKG="dist/Sydalesis-$VERSION-win64"
rm -rf "$PKG" && mkdir -p "$PKG"
cp -r source/bin/Sydalesis.vst3 source/bin/Sydalesis.clap Wavetables Samples Tunings LICENSE.txt CHANGES-5.1.7.txt install.bat "$PKG/"
[ -d Screenshots ] && cp -r Screenshots "$PKG/"
tar --exclude=build --exclude=bin -cf - source | tar -xf - -C "$PKG"
( cd dist && rm -f "Sydalesis-$VERSION-win64.zip" && zip -qr "Sydalesis-$VERSION-win64.zip" "Sydalesis-$VERSION-win64" )
echo "built dist/Sydalesis-$VERSION-win64.zip"
