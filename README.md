# Acid Ladder VST

A monophonic acid-style synthesizer implemented in C++ using JUCE.
Features a nonlinear 4-pole ladder filter, accent, slide, and decay-only envelope
inspired by classic acid synthesis.

## Features
- Saw / Square oscillator
- Nonlinear ladder low-pass filter (tanh saturation)
- Accent and slide (portamento)
- Decay-only envelope (303-style)
- VST3 plugin, tested in FL Studio

## DSP Overview
Signal path:
Oscillator → Ladder Filter → Saturation → Output

## Build
- JUCE 7+
- VST3
- CMake or ProJucer
- Optional VST2: set `ACID_LADDER_ENABLE_VST2=ON` and provide the VST2 SDK path via `ACID_LADDER_VST2_SDK_PATH` (or `VST2_SDK_PATH` env var).

## Disclaimer
This is an original implementation for educational and research purposes.
It is not an emulation of any specific commercial product.
