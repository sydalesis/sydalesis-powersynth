#!/bin/bash
# Sydalesis installer for macOS. Double-click it (first time: right-click > Open, see README-macOS.txt).
# Installs for your user account only; no administrator password needed.
cd "$(dirname "$0")"
set -e
PLUG="$HOME/Library/Audio/Plug-Ins"
DOCS="$HOME/Documents/Sydalesis"
mkdir -p "$PLUG/VST3" "$PLUG/CLAP" "$PLUG/Components" "$DOCS"

echo "Removing the download quarantine (Gatekeeper) from this folder..."
xattr -dr com.apple.quarantine . 2>/dev/null || true

echo "Installing VST3..."
rm -rf "$PLUG/VST3/Sydalesis.vst3"; cp -R Sydalesis.vst3 "$PLUG/VST3/"
echo "Installing CLAP..."
rm -rf "$PLUG/CLAP/Sydalesis.clap"; cp -R Sydalesis.clap "$PLUG/CLAP/"
if [ -d Sydalesis.component ]; then
  echo "Installing Audio Unit..."
  rm -rf "$PLUG/Components/Sydalesis.component"; cp -R Sydalesis.component "$PLUG/Components/"
fi
for d in Wavetables Samples Tunings; do
  echo "Installing $d to $DOCS/$d..."
  mkdir -p "$DOCS/$d"; cp -R "$d/" "$DOCS/$d/"
done
xattr -dr com.apple.quarantine "$PLUG/VST3/Sydalesis.vst3" "$PLUG/CLAP/Sydalesis.clap" "$PLUG/Components/Sydalesis.component" 2>/dev/null || true
# make Logic / GarageBand rescan Audio Units
killall -9 AudioComponentRegistrar 2>/dev/null || true

echo
echo "Done. Rescan plug-ins in your DAW: Sydalesis (vendor Sydalesis) appears as an instrument."
echo "You can close this window."
