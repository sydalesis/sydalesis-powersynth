SYDALESIS 5.1.7 FOR macOS
=========================

Universal build: runs natively on Apple Silicon (M1-M4) and Intel Macs, macOS 10.15 or later.
Formats: VST3, CLAP and Audio Unit (Logic Pro, GarageBand, MainStage).


INSTALL
-------
1. Unzip the download.
2. Right-click "Install Sydalesis.command" and choose Open.
   (A plain double-click is blocked the first time, see below.)
3. Click Open in the warning, and let the Terminal window finish.
4. Rescan plug-ins in your DAW.

The installer copies:
   Sydalesis.vst3       -> ~/Library/Audio/Plug-Ins/VST3
   Sydalesis.clap       -> ~/Library/Audio/Plug-Ins/CLAP
   Sydalesis.component  -> ~/Library/Audio/Plug-Ins/Components
   Wavetables, Samples, Tunings -> ~/Documents/Sydalesis
Presets you save, favourites and MIDI-learn maps also live in ~/Documents/Sydalesis.


WHY macOS WARNS YOU (GATEKEEPER)
--------------------------------
Sydalesis is free and isn't signed with a paid Apple Developer certificate, so macOS
marks the download as "from an unidentified developer". That's a one-time step per
version. The installer removes the mark from the plug-ins it installs.

If you install by hand instead (dragging the plug-ins into the folders above), and the
DAW won't load Sydalesis or macOS says it "is damaged" or "can't be opened", either:

  a) System Settings > Privacy & Security, scroll down, click "Open Anyway"
     next to the Sydalesis message, or

  b) open Terminal and run, for each format you installed:
       xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/Sydalesis.vst3
       xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/CLAP/Sydalesis.clap
       xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/Sydalesis.component

Then restart or rescan the DAW. You won't need to do it again for this version.

Logic Pro: if Sydalesis is missing after installing, open the Plug-in Manager
(Logic Pro > Settings > Plug-in Manager), select Sydalesis and click "Reset & Rescan Selection".
