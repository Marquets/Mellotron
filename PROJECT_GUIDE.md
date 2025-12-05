# Mellotron Micro Clone - Project Guide

This document outlines the hardware requirements, wiring connections, and a step-by-step plan to build your DIY Mellotron using the Daisy Seed platform.

**Last Updated**: December 2025 - Based on Web Emulator v2.0

---

## 1. Bill of Materials (BOM)

| Component | Quantity | Description | Notes |
| :--- | :---: | :--- | :--- |
| **Daisy Seed** | 1 | Microcontroller & Audio Codec | The brain of the project. |
| **OLED Display** | 2 | SSD1306 128x64 I2C | **Crucial:** You need one configured to address `0x3C` and the other to `0x3D`. Often done via a solder jumper on the back. |
| **Potentiometers** | 4 | Linear Taper (B10K or B100K) | For Mix, Volume, Tone, and Pitch. |
| **Rotary Encoders** | 2 | EC11 Type | For Sound Selection (A and B). |
| **SD Card Reader** | 1 | Micro SD Card Breakout Board | Must support SDMMC mode (4-bit). Look for modules with DAT0-DAT3 pins exposed. |
| **Micro SD Card** | 1 | 16GB or 32GB (Class 10) | For storing WAV samples. |
| **Breadboard** | 1 | Large size recommended | For prototyping. |
| **Jumper Wires** | 1 set | Male-Male & Male-Female | For connections. |
| **Audio Jacks** | 2 | 1/4" or 3.5mm Mono Jacks | For Audio Output L/R (Optional, can use Daisy's built-in jack for testing). |
| **MIDI Input** | 1 | MIDI DIN Jack + Optocoupler Circuit | Optional: For external MIDI keyboard control. |
| **Enclosure** | 1 | Metal (e.g., Hammond 1590XX) or Wood | Metal is durable but hard to cut for displays. Thin wood (3mm plywood) or acrylic is easier for rectangular cutouts. |

### Enclosure Alternatives & Tips
*   **Metal**: Very durable and professional looking. **Tip**: For rectangular OLED cutouts, drill a hole in each corner and use a nibbler tool or a jeweler's saw. It is labor-intensive without CNC.
*   **Thin Wood (3mm Plywood/MDF)**: Excellent alternative. Easy to cut with a coping saw or utility knife. Can be laser cut cheaply. Sand and stain for a premium look.
*   **Acrylic/Plexiglass**: Modern look, easy to laser cut, but can crack if drilled aggressively.
*   **3D Printing**: If you have access to a printer, this is the easiest way to get perfect cutouts.

---

## 2. Wiring & Connections

### Power
*   **Daisy Seed**: Powered via USB-C.
*   **3.3V Out (Pin 38)**: Powers Potentiometers and OLEDs.
*   **AGND (Pin 36)**: Ground for Potentiometers.
*   **DGND (Pin 37)**: Ground for OLEDs and Encoders.

### Potentiometers (Analog Inputs)
Connect the **Left Leg** to Ground (AGND), **Right Leg** to 3.3V, and **Middle Leg (Wiper)** to the Daisy Seed pins below:

| Function | Daisy Pin | Variable Name | Range |
| :--- | :---: | :--- | :--- |
| **Mix (Crossfade)** | **15** | `ADC_MIX` | 0-100% (A to B) |
| **Master Volume** | **16** | `ADC_VOL` | 0-100% |
| **Tone (Filter)** | **17** | `ADC_TONE` | 200Hz - 12kHz (log) |
| **Pitch (Speed)** | **18** | `ADC_PITCH` | 0.35x - 2.8x (±2 octaves) |

### Rotary Encoders (Digital Inputs)
Connect the **Common (C)** pin to Ground (DGND). Connect **A** and **B** pins as follows:

| Function | Pin A | Pin B | Notes |
| :--- | :---: | :---: | :--- |
| **Select Sound A** | **20** | **21** | Click button unused. |
| **Select Sound B** | **22** | **23** | Click button unused. |

### Octave Buttons (Digital Inputs)
Connect momentary push buttons between the pin and Ground.

| Function | Daisy Pin | Notes |
| :--- | :---: | :--- |
| **Octave Down** | **24** | Internal Pull-up required (Z key in emulator) |
| **Octave Up** | **25** | Internal Pull-up required (X key in emulator) |

### OLED Displays (I2C)
Both screens share the same I2C bus lines.

| Pin | Connection |
| :--- | :--- |
| **SCL** | Daisy Pin **11** |
| **SDA** | Daisy Pin **12** |
| **VCC** | 3.3V |
| **GND** | DGND |

*   **Screen 1 (Left - Display A)**: Address `0x3C` (Default). Shows Sound A name and mix percentage.
*   **Screen 2 (Right - Display B)**: Address `0x3D` (Modify hardware jumper on back of OLED module). Shows Sound B name and mix percentage.

### Micro SD Card (SDMMC Mode)
For fast sample loading, use the dedicated SDMMC pins.

| Pin SD Breakout | Pin Daisy Seed |
| :--- | :--- |
| **DAT2** | Pin 4 |
| **DAT3 (CD)** | Pin 5 |
| **CMD** | Pin 7 |
| **CLK / SCK** | Pin 6 |
| **DAT0** | Pin 2 |
| **DAT1** | Pin 3 |
| **VCC / 3V3** | 3.3V (Pin 38) |
| **GND** | DGND (Pin 37) |

### MIDI Input (Optional)
For hardware MIDI input, use a standard MIDI optocoupler circuit:

| MIDI Pin | Connection |
| :--- | :--- |
| **MIDI RX** | Daisy Pin **14** (USART1_RX) |
| **5V** | Via optocoupler (6N138 or H11L1) |
| **GND** | DGND |

---

## 3. Available Instruments

The system supports **21 instruments** loaded from the SD card. Each instrument folder contains WAV samples named by note (e.g., `C3.wav`, `C#3.wav`, etc.):

1. **8 Choir**
2. **Bassoon**
3. **Cello**
4. **Combined Choir**
5. **GC3 Brass**
6. **M300A**
7. **M300B**
8. **Mixed Brass B**
9. **Mixed Brass B 2**
10. **MkII Church Organ**
11. **MkII Combined Brass**
12. **MkII Flute** *(Default Sound A)*
13. **MkII Violins** *(Default Sound B)*
14. **Orchestra**
15. **String Section**
16. **Tenor & Alto Sax**
17. **Tenor Sax**
18. **Trombone**
19. **Trombone & Trumpet**
20. **Trumpet**
21. **Woodwind 2**

### Sample Format
- **Format**: 16-bit WAV
- **Sample Rate**: 44.1kHz or 48kHz
- **Range**: F2 to F5 (35 keys, chromatic)
- **Naming**: `NoteName.wav` (e.g., `C3.wav`, `C#3.wav`, `D3.wav`)

---

## 4. Audio Processing Features

### Implemented in Web Emulator (Ready for Hardware Port)

#### 4.1 Dual Engine Architecture
- **Engine A** and **Engine B** run simultaneously
- Each engine can load a different instrument
- Crossfade between engines using the **Mix** knob

#### 4.2 Pitch Control
- **Formula**: `playbackRate = 2^((knobValue - 50) / 33.3)`
- **Range**: ±2 octaves (0.35x to 2.8x speed)
- **Center**: 50 = 1.0x (original pitch)

#### 4.3 Tone Filter
- **Type**: Biquad Lowpass Filter
- **Formula**: `cutoff = 200 * (12000/200)^knobValue`
- **Range**: 200Hz to 12kHz (logarithmic)
- **Full Open**: 12kHz (100% knob position)

#### 4.4 Octave Switching
- **Range**: C1 to C6 (6 octaves)
- **Default**: C3
- **Controls**: Z (down) / X (up) in emulator, buttons in hardware
- **Display**: Shows current octave on screen

#### 4.5 MIDI Support
- **Protocol**: Standard MIDI Note On/Off
- **Channels**: All channels (1-16)
- **Octave Offset**: Applied to incoming MIDI notes
- **Velocity**: Ignored (fixed velocity for Mellotron-style behavior)

---

## 5. Implementation Plan

### Phase 1: Hardware Assembly
1.  **Prepare the Daisy Seed**: Solder headers if necessary and place it on the breadboard.
2.  **Connect Potentiometers**: Wire the **4 pots** to the specified pins (Mix, Volume, Tone, Pitch).
3.  **Connect Encoders**: Wire both rotary encoders for sound selection.
4.  **Connect Octave Buttons**: Wire two momentary buttons with internal pull-ups.
5.  **Connect OLEDs**: Wire both screens to pins 11 & 12. **Ensure one screen is set to 0x3D**.
6.  **Connect SD Card**: Wire SD card breakout using SDMMC mode for fast access.
7.  **Optional - MIDI Input**: Add MIDI optocoupler circuit if using external MIDI keyboard.
8.  **Power Check**: Plug in USB and ensure the Daisy Seed power LED turns on.

### Phase 2: Software Environment
1.  **Install Toolchain**: Follow the [DaisyWiki](https://github.com/electro-smith/DaisyWiki/wiki) to install the toolchain for your OS (VS Code + Cortex-Debug recommended).
2.  **Clone Project**: Ensure this project folder is set up correctly with `libDaisy` and `DaisySP`.
3.  **Compile**: Run `make` or build via VS Code to ensure `main.cpp` compiles without errors.
4.  **Test Web Emulator**: Run `./start_emulator.sh` to test the web emulator and understand the expected behavior.

### Phase 3: SD Card Setup
1.  **Format SD Card**: FAT32 format.
2.  **Create Folders**: Create one folder per instrument (see instrument list above).
3.  **Add Samples**: Place WAV files in each folder following the naming convention:
    ```
    /Samples/MkII Flute/C3.wav
    /Samples/MkII Flute/C#3.wav
    /Samples/MkII Flute/D3.wav
    ...
    /Samples/MkII Violins/C3.wav
    /Samples/MkII Violins/C#3.wav
    ...
    ```
4.  **Verify Structure**: See `SD_CARD_STRUCTURE.md` for detailed folder layout.

### Phase 4: Code Implementation

#### 4.4.1 Core Features (Priority 1)
- [x] Dual OLED display rendering
- [x] Encoder reading for instrument selection
- [x] Potentiometer reading (Mix, Volume, Tone, Pitch)
- [ ] SD card sample loading
- [ ] Dual playback engine with crossfade
- [ ] Tone filter implementation (SVF lowpass)
- [ ] Pitch control via playback rate

#### 4.4.2 Enhanced Features (Priority 2)
- [ ] Octave buttons with debouncing
- [ ] OLED display of current octave
- [ ] Sample caching/streaming for memory efficiency

#### 4.4.3 Advanced Features (Priority 3)
- [ ] MIDI input via USART
- [ ] MIDI octave offset
- [ ] Lazy loading of instruments (load on demand)
- [ ] Loading progress indicator on OLED
- [ ] Polyphony management (voice stealing)

### Phase 5: Testing & Refinement
1.  **Flash Code**: Upload the firmware to the Daisy Seed.
2.  **Verify UI**: Check if both screens light up and show the UI. Test if the "Select" encoders change the instrument names.
3.  **Test Octave Buttons**: Verify octave switching works and displays correctly.
4.  **Verify Audio**: Connect headphones/speakers. Test each control:
    - Mix knob crossfades between A and B
    - Pitch knob shifts playback speed (±2 octaves)
    - Tone knob filters high frequencies (200Hz-12kHz)
    - Volume knob controls master output
5.  **Test MIDI**: If MIDI input is connected, verify external keyboard control.
6.  **Tuning**: Fine-tune the control curves to match the web emulator behavior.

---

## 6. Control Mapping Reference

### Knob Value Formulas (from Web Emulator)

```cpp
// Mix (Linear 0-100%)
float mix = knobValue / 100.0f;
float gainA = (1.0f - mix) * 0.5f;
float gainB = mix * 0.5f;

// Pitch (Logarithmic, ±2 octaves)
float pitch = pow(2.0f, (knobValue - 50.0f) / 33.3f);

// Tone (Logarithmic, 200Hz - 12kHz)
float cutoff = 200.0f * pow(12000.0f / 200.0f, knobValue / 100.0f);

// Volume (Linear 0-100%)
float volume = knobValue / 100.0f;
```

### OLED Display Layout

**Display A (Left - 0x3C)**
```
┌────────────────────┐
│    Sound A         │  <- Title (centered)
│                    │
│   MkII Flute       │  <- Instrument name (centered)
│                    │
│ Mix [████████  ] 80%│  <- Mix bar + percentage
└────────────────────┘
```

**Display B (Right - 0x3D)**
```
┌────────────────────┐
│    Sound B         │  <- Title (centered)
│                    │
│  MkII Violins      │  <- Instrument name (centered)
│                    │
│ Mix [██        ] 20%│  <- Mix bar + percentage
└────────────────────┘
```

---

## 7. Next Steps for Hardware Product

### Immediate Tasks
1.  **Implement MIDI Input**: Add USART MIDI parsing in main loop
2.  **Optimize Sample Loading**: Implement streaming or chunked loading for large sample sets
3.  **Test with All 21 Instruments**: Verify SD card loading for complete instrument library

### Future Enhancements
- **Preset System**: Save/load favorite A/B combinations
- **Expression Pedal Input**: Add CV input for external control
- **Stereo Output**: Use both Daisy Seed audio outputs
- **USB MIDI**: Add USB MIDI class support
- **Enclosure Design**: Create 3D-printable case with panel cutouts

---

## 8. Resources

- **Web Emulator**: `web_emulator.html` - Full reference implementation
- **Sample Structure**: `SD_CARD_STRUCTURE.md` - Folder organization guide
- **Deployment**: `DEPLOY.md` - Web emulator deployment instructions
- **DaisyWiki**: https://github.com/electro-smith/DaisyWiki/wiki
- **DaisySP Docs**: https://electro-smith.github.io/DaisySP/index.html
