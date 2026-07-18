# ATLAS — Boogie Labs

*Modern Analog. Timeless Inspiration.*

Version 0.1 — the first playable milestone of Atlas, matching your UI mockup:

- Saw/Square oscillator, 8-voice polyphonic
- ENVELOPE knob (one knob shapes attack/decay/release)
- CUTOFF + RESO low-pass filter
- DELAY with TIME / FB / MIX modes on one knob
- MONITOR waveform display
- Preset browser (save, prev/next) — presets live in `Documents/Boogie Labs/Atlas Presets`
- OUTPUT knob, Bypass, virtual keyboard (C2–C6)
- Dark chassis, machined-knob look, orange/cyan accents

Formats: VST3 (Mac + Windows), AU (Mac), plus a Standalone app for testing without a DAW.

## Project layout

```
Atlas/
├── CMakeLists.txt            # build recipe — downloads JUCE automatically
└── Source/
    ├── PluginProcessor.*     # engine: parameters, voices -> filter -> delay -> out
    ├── PluginEditor.*        # the UI from the mockup
    ├── SynthVoice.h          # generates sound for ONE note
    ├── SynthSound.h          # tells the synth which notes to accept
    ├── AtlasLookAndFeel.h    # colors + custom knob drawing
    └── PresetManager.h       # save/load presets as XML files
```

Learning order: `SynthVoice.h` → `PluginProcessor.cpp` → `PluginEditor.cpp`.

## Prerequisites

**macOS**
1. Xcode command line tools: `xcode-select --install`
2. CMake: `brew install cmake`

**Windows**
1. [Visual Studio 2022 Community](https://visualstudio.microsoft.com/) — pick the
   **"Desktop development with C++"** workload during install
2. [CMake](https://cmake.org/download/) — check "Add to PATH" during install

## Build

Open a terminal (macOS) or "x64 Native Tools Command Prompt for VS 2022" (Windows)
in the Atlas folder:

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

First configure downloads JUCE (a few minutes). The build auto-installs the plugin:

- **macOS VST3:** `~/Library/Audio/Plug-Ins/VST3/Atlas.vst3`
- **macOS AU:** `~/Library/Audio/Plug-Ins/Components/Atlas.component`
- **Windows VST3:** `C:\Program Files\Common Files\VST3\Atlas.vst3`
  (if that fails for permissions, copy it manually from `build\Atlas_artefacts\Release\VST3\`)

Rescan plugins in your DAW and look for **Boogie Labs > Atlas**. Or test instantly
with the Standalone app in `build/Atlas_artefacts/Release/Standalone/`.

## Build without installing anything (GitHub Actions)

Don't want to install Xcode/Visual Studio? Push this folder to a GitHub repo and
GitHub builds it for you — the included `.github/workflows/build.yml` compiles
Mac + Windows versions on every push:

1. Create a repo at github.com, then in this folder:
   `git init && git add . && git commit -m "Atlas v0.1" && git branch -M main`
   `git remote add origin https://github.com/YOURNAME/atlas.git && git push -u origin main`
2. On GitHub: **Actions** tab → latest run → download the **Atlas-macOS** /
   **Atlas-Windows** artifacts, and copy the .vst3 to your plugin folder.

## Roadmap to the full Atlas vision

Each version builds on this codebase — nothing gets thrown away:

- **v0.2 — Oscillator section:** Osc B with detune, sub oscillator, noise generator, mixer panel. (Extend `SynthVoice.h` — add more phase accumulators and a level for each.)
- **v0.3 — Filter section:** filter 2, drive, key tracking, filter envelope amount.
- **v0.4 — Modulation:** LFO 1/2, mod envelope, a simple mod matrix. This is where **Motion Engine™** starts: one macro knob routed to cutoff, delay mix, and width at once.
- **v0.5 — Effects rack:** chorus, phaser, reverb, compressor, limiter (`juce::dsp` has building blocks for all of these).
- **v0.6 — Sound library:** factory preset bank organized by category (Bass/Leads/Pads/Keys/Plucks), search, random preset.
- **v1.0 — Hardware-quality UI:** filmstrip or vector knob animations, glass monitor, OLED-style displays, GPU-accelerated rendering.

## Troubleshooting

- **"cmake: command not found"** — CMake isn't on your PATH; reinstall and tick the PATH option.
- **No sound in Standalone** — Options > Audio/MIDI Settings: pick an output device and enable a MIDI input (or click the on-screen keys).
- **Plugin missing in DAW** — make sure the .vst3 is in the system VST3 folder above, then force a rescan.
