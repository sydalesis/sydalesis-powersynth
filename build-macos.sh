#!/bin/bash
# Sydalesis — macOS build (universal: Apple Silicon + Intel). Run on a Mac with the Xcode command line
# tools (xcode-select --install), or let GitHub Actions run it (.github/workflows/build.yml).
# Output: dist/Sydalesis-<version>-macOS.zip  (VST3, CLAP, AU + wavetables, samples, tunings, installer)
set -euo pipefail
cd "$(dirname "$0")"
ROOT="$PWD"
VERSION="5.1.6"
DPF_COMMIT=4238e1c7f0351bbe488d79f0899c540543ac7583   # the DPF the patch was written against

# 1. DPF framework, pinned, with the Sydalesis patch (VST3 note expression, CLAP restore bracket / MIDI learn)
if [ ! -d dpf ]; then
  git clone --recursive https://github.com/DISTRHO/DPF.git dpf
fi
( cd dpf && git checkout -q "$DPF_COMMIT" && git submodule update --init --recursive -q && git checkout -q -- . && git apply "$ROOT/source/dpf-sydalesis.patch" )

# 2. universal build
export MACOSX_DEPLOYMENT_TARGET=10.15
ARCH="-arch x86_64 -arch arm64 -mmacosx-version-min=10.15"
# flags go in the environment (DPF appends its own to them; on the make command line they would replace them)
( cd source && rm -rf build bin && rm -rf "$ROOT/dpf/build"
  CFLAGS="$ARCH" CXXFLAGS="$ARCH" LDFLAGS="$ARCH" make -j"$(sysctl -n hw.ncpu)" DPF_PATH="$ROOT/dpf" HAVE_OPENGL=true )

# 3. ad-hoc signature (Apple Silicon refuses to load unsigned native code; this is not notarisation)
for b in source/bin/Sydalesis.vst3 source/bin/Sydalesis.clap source/bin/Sydalesis.component; do
  [ -e "$b" ] && codesign --force --deep --sign - "$b"
done
lipo -info source/bin/Sydalesis.vst3/Contents/MacOS/Sydalesis

# 4. package
PKG="dist/Sydalesis-$VERSION-macOS"
rm -rf "$PKG" && mkdir -p "$PKG"
cp -R source/bin/Sydalesis.vst3 source/bin/Sydalesis.clap "$PKG/"
[ -e source/bin/Sydalesis.component ] && cp -R source/bin/Sydalesis.component "$PKG/"
cp -R Wavetables Samples Tunings "$PKG/"
cp LICENSE.txt CHANGES-5.1.6.txt macos/README-macOS.txt "$PKG/"
cp macos/Install\ Sydalesis.command "$PKG/"
tar --exclude=build --exclude=bin -cf - source | tar -xf - -C "$PKG"
chmod +x "$PKG/Install Sydalesis.command"
( cd dist && rm -f "Sydalesis-$VERSION-macOS.zip" && ditto -c -k --keepParent "Sydalesis-$VERSION-macOS" "Sydalesis-$VERSION-macOS.zip" )
echo "built dist/Sydalesis-$VERSION-macOS.zip"
