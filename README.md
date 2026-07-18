# LumenDSP

**LumenDSP** is a free, open-source desktop amplifier modeler for guitarists. Load any Neural Amp Modeler (`.nam`) capture and play through it in real time, with a premium standalone experience inspired by Neural DSP Archetype plugins—focused on modern fusion tones in the spirit of Mateus Asato and Jack Gardiner.

## Tone target

LumenDSP is built around a clear fusion-oriented voice:

- Glassy clean-to-light-crunch character
- Strong pick-attack clarity
- Smooth, compressed sustain on leads
- Tight low end

The core tone always comes from **your** `.nam` file. LumenDSP does not replace the capture—it gives you a clean signal path, cab IR loading, and a restrained control set around it.

## Features

| Area | Capability |
|------|------------|
| Amp | Load and hot-swap `.nam` / A2 models via NeuralAmpModelerCore |
| Cab | Load and hot-swap impulse response `.wav` files |
| Dynamics | Input gain, output level, simple noise gate |
| Tone | Three-band EQ (post-amp, pre-cab) |
| Presets | Named presets storing model path, IR path, and all control values |
| Formats | Standalone desktop app and VST3 plugin |
| UI | Custom LookAndFeel, signal-flow layout, LED meters, micro-interactions |

### Signal flow

```
Input → Amp (NAM) → EQ → Cab (IR) → Output
```

## Status

| Milestone | State |
|-----------|-------|
| 1. Repository setup and README | Complete |
| 2. Audio engine core (NAM load + real-time process) | Complete |
| 3. JUCE standalone with audio device selection | Complete |
| 4. UI foundation (LookAndFeel + core controls) | Complete |
| 5. Model / IR hot-swap loading | Complete |
| 6. Preset system | Complete |
| 7. Polish (layout, signal flow, metering, micro-interactions) | Complete |

## Stack

- **JUCE** (8.x default) — application framework, audio I/O, VST3, UI  
  CMake uses the same `juce_add_plugin` workflow as JUCE 7. The default tag is **8.0.6** because JUCE 7.0.x fails to build `juceaide` against the **macOS 15 SDK** (`CGWindowListCreateImage` removed). Override with `-DLUMENDSP_JUCE_GIT_TAG=...` if you use an older SDK.
- **CMake** — cross-platform build and dependency fetch
- **[NeuralAmpModelerCore](https://github.com/sdatkinson/NeuralAmpModelerCore)** — real-time `.nam` / A2 inference (MIT)

## Build requirements

- CMake 3.24 or newer
- C++20 compiler (Xcode 14+, MSVC 2022, or recent GCC/Clang)
- Git (JUCE and NeuralAmpModelerCore are fetched at configure time)
- On macOS: Xcode command-line tools
- On Windows: Visual Studio 2022 with C++ desktop workload
- On Linux: standard audio/dev packages (`libasound2-dev`, `libfreetype6-dev`, `libx11-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libcurl4-openssl-dev`, etc.)

VST3 support is provided through JUCE; no separate Steinberg SDK install is required for building.

## Build instructions

```bash
git clone https://github.com/Kripu77/LumenDSP.git
cd LumenDSP

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

First configure downloads JUCE and NeuralAmpModelerCore (including Eigen). That step can take several minutes.

### Outputs

After a successful build (paths may vary by platform/generator):

| Format | Typical path |
|--------|----------------|
| Standalone (macOS) | `build/LumenDSP_artefacts/Release/Standalone/LumenDSP.app` |
| VST3 (macOS) | `build/LumenDSP_artefacts/Release/VST3/LumenDSP.vst3` |
| Standalone (Linux) | `build/LumenDSP_artefacts/Release/Standalone/LumenDSP` |
| VST3 (Linux) | `build/LumenDSP_artefacts/Release/VST3/LumenDSP.vst3` |
| Standalone (Windows) | `build/LumenDSP_artefacts/Release/Standalone/LumenDSP.exe` |
| VST3 (Windows) | `build/LumenDSP_artefacts/Release/VST3/LumenDSP.vst3` |

Install the VST3 by copying it into:

- macOS: `~/Library/Audio/Plug-Ins/VST3/`
- Windows: `C:\Program Files\Common Files\VST3\`
- Linux: `~/.vst3/`

### Optional configure flags

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DLUMENDSP_COPY_PLUGIN_AFTER_BUILD=ON \
  -DLUMENDSP_JUCE_GIT_TAG=8.0.6 \
  -DLUMENDSP_NAM_GIT_TAG=v0.5.4
```

## Usage

1. Launch the standalone app (or load the VST3 in your DAW).
2. **Standalone:** click **Audio** to open the device selector (input interface, sample rate, buffer size).
3. Load a `.nam` capture via **Browse** or drag-and-drop—tone is driven entirely by that model.
4. Optionally load a cab IR (`.wav`).
5. Dial **Input**, **Gate**, **Bass / Mid / Treble**, and **Output**.
6. Toggle **Gate**, **EQ**, and **Cab** as needed.
7. Save the combination as a named preset (stored under the user application data `LumenDSP/Presets` folder).

### Preset format

Presets are simple XML files (`.lumenpreset`) containing:

- Preset name
- Absolute path to the NAM model
- Absolute path to the IR (optional)
- Full parameter state (gains, toggles, etc.)

## Project layout

```
LumenDSP/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── Source/
│   ├── audio/           # NAM engine, IR convolver, gate, EQ, meters, pipeline
│   │   ├── AudioPipeline.*
│   │   ├── NamEngine.*
│   │   ├── IrConvolver.*
│   │   ├── NoiseGate.*
│   │   ├── ThreeBandEq.*
│   │   └── LevelMeter.*
│   ├── parameters/      # Parameter IDs, ranges, layout
│   ├── presets/         # Preset save / recall
│   ├── ui/              # LookAndFeel, knobs, meters, signal flow, slots
│   │   ├── DesignTokens.h
│   │   ├── LumenLookAndFeel.*
│   │   ├── KnobComponent.*
│   │   ├── LedMeterComponent.*
│   │   ├── SignalFlowStrip.*
│   │   ├── FileSlotComponent.*
│   │   └── PresetBarComponent.*
│   ├── PluginProcessor.*
│   └── PluginEditor.*
└── cmake/
```

## Architecture notes

- **Hot-swap:** NAM models and IRs load on a background thread, then swap in on the audio thread with minimal interruption.
- **Pipeline order:** input gain → noise gate → NAM → three-band EQ → cab IR → output level, with input/output peak meters.
- **UI tokens:** accent color, spacing (8px scale), typography, and meter timing live in `Source/ui/DesignTokens.h`.
- **Code style:** C++ sources avoid inline comments; behavior is expressed through naming and structure. Numeric values use named constants.

## License

LumenDSP original source code is released under the **MIT License**. See [LICENSE](LICENSE).

Third-party components retain their own licenses:

- [JUCE](https://github.com/juce-framework/JUCE) — see JUCE license terms for your use case
- [NeuralAmpModelerCore](https://github.com/sdatkinson/NeuralAmpModelerCore) — MIT
- Eigen (via NeuralAmpModelerCore) — MPL2

## Credits

- Neural Amp Modeler ecosystem by [Steven Atkinson](https://github.com/sdatkinson) and contributors
- Tone inspiration: modern fusion players such as Mateus Asato and Jack Gardiner
- Visual language: informed by the clarity and polish of Neural DSP Archetype-style interfaces (independent open-source project; not affiliated with Neural DSP)

## Contributing

Issues and pull requests are welcome. Prefer small, focused changes with clear conventional commit messages (`feat:`, `fix:`, `docs:`, `chore:`).
