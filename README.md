# Mellotron Web Emulator

A web-based Mellotron sampler emulator with dual sound engines, built with the Web Audio API.

## 🎹 Features

- **Dual Sound Engines**: Mix two Mellotron instruments simultaneously
- **21 Classic Sounds**: Flutes, Strings, Choirs, Brass, and more
- **Real-time Controls**:
  - Crossfade Mix between A/B sounds
  - Pitch control (~3 octaves range)
  - Tone filter (200Hz - 12kHz)
  - Master Volume
  - Reverb effect
- **Multiple Input Methods**:
  - Virtual on-screen keyboard
  - Computer keyboard (Ableton-style mapping)
  - MIDI keyboard support (Web MIDI API)
- **Octave Switching**: Z/X keys to shift octave range

## 🚀 Quick Start

### Local Development

1. Clone this repository
2. Run the local server:
   ```bash
   chmod +x start_emulator.sh
   ./start_emulator.sh
   ```
3. Open `http://localhost:8080/web_emulator.html` in your browser

### Online Demo

Visit: `https://YOUR_USERNAME.github.io/mellotron-web-emulator/web_emulator.html`

## 🎮 Controls

### Computer Keyboard
- **White Keys**: A, S, D, F, G, H, J, K
- **Black Keys**: W, E, T, Y, U
- **Octave Down**: Z
- **Octave Up**: X

### MIDI
Connect any MIDI keyboard via USB. The emulator will auto-detect it.

## 📁 Project Structure

```
mellotron_micro_clone/
├── web_emulator.html       # Main emulator (standalone file)
├── samples/                # WAV sample library
│   ├── MkII Flute/
│   ├── MkII Violins/
│   └── ... (21 instruments)
├── src/                    # C++ code for hardware version (Daisy Seed)
└── start_emulator.sh       # Local HTTP server script
```

## 🔊 Sample Format

- **Format**: WAV, 16-bit PCM
- **Sample Rate**: 44.1kHz or 48kHz
- **Channels**: Mono or Stereo (auto-converted)
- **Naming**: Note name + Octave (e.g., `C3.wav`, `F#4.wav`)

## 🛠️ Hardware Version

This project also includes firmware for a physical Mellotron module using:
- **Platform**: Electrosmith Daisy Seed
- **Features**: Dual OLED displays, rotary encoders, MIDI I/O
- See `PROJECT_GUIDE.md` for hardware build instructions

## 📝 License

MIT License - Feel free to use and modify!

## 🙏 Credits

- Original Mellotron samples from various public domain sources
- Built with Web Audio API
- Inspired by the classic Mellotron M400

---

Made with ❤️ for vintage synth enthusiasts
