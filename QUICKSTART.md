# LumenDSP quick start

Get from download to playing in under two minutes.

## 1. Install / open the app

### macOS (standalone)

1. Build or download `LumenDSP.app`.
2. Open it (right-click → **Open** the first time if Gatekeeper warns).
3. When asked, allow **microphone / input** access so your guitar interface can be used.

### VST3

1. Copy `LumenDSP.vst3` into:
   - macOS: `~/Library/Audio/Plug-Ins/VST3/`
   - Windows: `C:\Program Files\Common Files\VST3\`
   - Linux: `~/.vst3/`
2. Rescan plugins in your DAW and load **LumenDSP** on a mono/stereo guitar track.

## 2. Set audio input and output (standalone)

1. Click **Audio** in the top-right of the LumenDSP window.
2. Choose:
   - **Output**: your headphones, monitors, or interface outputs
   - **Input**: the interface input your guitar (or wireless pack) is plugged into
   - **Sample rate**: 48 kHz is a solid default
   - **Buffer size**: 128–256 samples for low latency; raise if you hear crackles
3. Close the settings dialog.

**Tip:** Guitar should hit the interface around −18 to −12 dBFS on loud strums. Use the interface gain, not only the LumenDSP **Input** knob.

## 3. Play the factory fusion tones

On first launch, LumenDSP installs factory models, cab IRs, and presets automatically. The default preset is:

**01 Glass Clean** — glassy Twin-style clean with a bright, tight cab IR.

Use the **Presets** dropdown to try:

| Preset | Character |
|--------|-----------|
| **01 Glass Clean** | Clear, chiming clean — pick attack and headroom (default) |
| **02 Airy Clean** | Slightly more open / airy clean platform |
| **03 Smooth Lead** | Compressed, sustaining lead-friendly clean-crunch |
| **04 Full Rig Clean** | Amp+cab capture (cab IR bypassed — already in the model) |
| **05 Light Crunch** | Tighter low end, light edge for rhythm / fusion grit |

Use the bottom section navigator to move through the rack:

```
INPUT → AMP → EQ → CAB
```

Global **Input / Gate / Output** always stay in the top chrome (Neural-style constant I/O).

## 4. Dial in your guitar

| Control | What to do |
|---------|------------|
| **Input** | Match your pick attack; start at 0 dB |
| **Gate** | Raise threshold if idle noise bothers you |
| **Bass / Mid / Treble** | Small moves (±2–3 dB); fusion tones like controlled lows and present mids |
| **Output** | Set monitoring level last so you do not clip your interface |
| **Gate / EQ / Cab** toggles | Bypass a block without losing the rest of the chain |

## 5. Load your own NAM or IR (optional)

- Drop a `.nam` file onto **NAM Model**, or click **Browse**
- Drop a `.wav` IR onto **Cab IR**, or click **Browse**
- Save with a name in the preset bar → **Save**

Factory files live on your machine under the LumenDSP app-data folder:

- macOS: `~/Library/LumenDSP/`
- Windows: `%APPDATA%\\LumenDSP\\`
- Linux: `~/.config/LumenDSP/` (or your platform’s user app-data path)

Folders:

- `FactoryContent/Models/` — NAM captures  
- `FactoryContent/IRs/` — cab impulses  
- `Presets/` — `.lumenpreset` files  

## 6. Latency and feel checklist

1. Direct monitoring off in the interface (or blend carefully) so you hear the modeled amp, not a dry double.
2. ASIO (Windows) / Core Audio (macOS) selected — not a generic “system default” if you have a real interface.
3. If the model feels “spongy,” lower buffer size; if it crackles, raise buffer size or freeze other heavy plugins.

## Factory tone notes

Factory presets target **modern fusion** territory: glassy cleans, clear pick attack, smooth lead sustain, tight low end. They are starting points inspired by that aesthetic (players like Mateus Asato / Jack Gardiner), not official artist packs.

See `Resources/FactoryContent/ATTRIBUTION.md` for capture credits and licenses.

Want more models? Browse free community captures at [TONE3000](https://www.tone3000.com/) and load any `.nam` into LumenDSP.

## Troubleshooting

| Issue | Fix |
|-------|-----|
| No sound | Check **Audio** input/output; confirm guitar input channel; raise **Output** carefully |
| Presets empty | Quit and reopen once so factory install can finish; check Application Support folder |
| Model not loading | Ensure the file ends in `.nam`; try a factory preset to verify the engine |
| Crackles | Increase buffer size; close other audio apps |
| Too dark / too bright | Toggle **Cab**, try another factory IR, or nudge **Treble** |
| VST3 not listed | Confirm install path and force a plugin rescan |

## Build from source (developers)

```bash
git clone https://github.com/Kripu77/LumenDSP.git
cd LumenDSP
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
open build/LumenDSP_artefacts/Release/Standalone/LumenDSP.app   # macOS
```

Full project details: [README.md](README.md).
