# Acid Ladder VST

A monophonic acid‑style synthesizer plugin built in C++ with JUCE. It combines a classic saw↔square oscillator, a nonlinear 4‑pole ladder filter, and a compact modulation matrix with modern conveniences like unison, sub‑oscillator blend, and built‑in FX.

## Table of Contents
- [Features](#features)
- [Signal Flow](#signal-flow)
- [Parameters](#parameters)
- [Modulation Matrix](#modulation-matrix)
- [MIDI Behavior](#midi-behavior)
- [Build Requirements](#build-requirements)
- [Building](#building)
- [Project Structure](#project-structure)
- [Notes](#notes)

## Features
- Saw ↔ square morph oscillator with sub‑oscillator blend
- Nonlinear 4‑pole ladder low‑pass filter with resonance
- Accent and glide (portamento)
- Decay‑only envelope (303‑style) plus a separate modulation envelope
- Unison detune/spread
- Modulation matrix with 3 assignable slots
- Built‑in FX chain: drive, chorus, delay, reverb
- VST3 output (optional VST2 with SDK)

## Signal Flow
```
Oscillator (+Sub +Unison) → Ladder Filter → Saturation → FX (Drive → Chorus → Delay → Reverb) → Output
```

## Parameters
All parameters are exposed via the plugin’s GUI and the host automation system.

| Parameter | Range | Default | Description |
| --- | --- | --- | --- |
| Wave | 0.0 → 1.0 | 0.0 | Saw ↔ square morph (0 = saw, 1 = square). |
| Cutoff | 20 Hz → 18 kHz | 800 Hz | Filter cutoff frequency. |
| Resonance | 0.0 → 0.995 | 0.35 | Ladder filter resonance. |
| Env Mod | 0.0 → 1.0 | 0.75 | Filter envelope modulation depth. |
| Decay | 0.01 → 2.0 s | 0.18 s | Main amplitude envelope decay. |
| Accent | 0.0 → 1.0 | 0.75 | Accent amount applied on note on. |
| Glide (ms) | 0 → 500 ms | 80 ms | Portamento time. |
| Drive | 0.0 → 1.0 | 0.35 | Pre‑filter drive. |
| Saturation | 0.0 → 1.0 | 0.2 | Output saturation amount. |
| Sub Mix | 0.0 → 1.0 | 0.35 | Sub‑oscillator blend. |
| Unison | 0.0 → 1.0 | 0.25 | Unison amount (dual detune). |
| Unison Spread | 0.0 → 1.0 | 0.45 | Unison stereo spread. |
| Gain | 0.0 → 1.5 | 0.85 | Output gain. |
| LFO 1 Rate | 0.0 → 15 Hz | 2.2 Hz | LFO 1 frequency. |
| LFO 2 Rate | 0.0 → 15 Hz | 4.8 Hz | LFO 2 frequency. |
| Mod Env Decay | 0.0 → 2.5 s | 0.55 s | Modulation envelope decay. |
| Mod 1 Amount | -1.0 → 1.0 | 0.0 | Mod slot 1 depth. |
| Mod 2 Amount | -1.0 → 1.0 | 0.0 | Mod slot 2 depth. |
| Mod 3 Amount | -1.0 → 1.0 | 0.0 | Mod slot 3 depth. |
| FX Drive | 0.0 → 1.0 | 0.15 | FX drive amount. |
| Chorus | 0.0 → 1.0 | 0.18 | Chorus mix. |
| Delay | 0.0 → 1.0 | 0.2 | Delay mix. |
| Delay Time | 5 → 700 ms | 260 ms | Delay time. |
| Reverb | 0.0 → 1.0 | 0.25 | Reverb mix. |

## Modulation Matrix
The synth exposes three modulation slots with selectable sources and destinations.

**Sources**
- Off
- LFO 1
- LFO 2
- Mod Env
- Velocity
- Aftertouch

**Destinations**
- Off
- Cutoff
- Pitch
- Drive
- Gain
- Pan

Set the source/destination for each slot and dial in the amount (negative values invert the modulation).

## MIDI Behavior
- **Monophonic**: last‑note priority with legato glide.
- **Velocity** and **aftertouch** are available as modulation sources.
- **Channel pressure** and **poly aftertouch** are both supported.

## Build Requirements
- CMake 3.15+ (CMake presets require 3.19+)
- A C++17 compiler
- JUCE 7+ (included in `juce/JUCE`)
- Optional VST2 SDK (if building VST2)

## Building
### CMake (recommended)
```bash
cmake --preset linux-release
cmake --build --preset linux
```

Windows cross‑compile via MinGW (see `cmake/toolchains/mingw64.cmake`):
```bash
cmake --preset windows-release
cmake --build --preset windows
```

### Build helper
```bash
./build-all.sh
```

### Output location
By default, JUCE places artifacts in:
```
build-*/AcidLadderVST_artefacts/Release/VST3/
```

### Optional VST2
```bash
cmake -DACID_LADDER_ENABLE_VST2=ON \
      -DACID_LADDER_VST2_SDK_PATH=/path/to/VST2_SDK \
      -S . -B build-vst2
```

## Project Structure
- `src/plugin/PluginProcessor.*`: DSP core, parameter layout, audio/MIDI processing
- `src/plugin/PluginEditor.*`: GUI layout and component wiring
- `juce/JUCE`: JUCE framework
- `cmake/`: toolchains and build configuration

## Notes
- This is an original implementation for educational/research use; it is **not** a clone of any commercial product.
- The synth is designed for modern hosts that support VST3.
