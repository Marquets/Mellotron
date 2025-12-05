#include "daisy_seed.h"
#include "daisysp.h"
#include "dev/oled_ssd1306.h"
#include "hid/encoder.h"
#include "hid/switch.h"
#include <cmath>
#include <string>
#include <vector>

// Usamos los namespaces estándar de Daisy
using namespace daisy;
using namespace daisysp;

// --- Hardware Definitions ---
DaisySeed hw;
MidiUsbHandler midi; // Usamos MIDI USB por defecto
OledDisplay<SSD1306_128x64> displayA;
OledDisplay<SSD1306_128x64> displayB;
SdmmcHandler sd;
Encoder enc_A;
Encoder enc_B;
Switch octave_down;
Switch octave_up;

// --- Memory ---
// 64MB SDRAM total. Reservamos todo para samples.
// 16 Millones de floats = 64MB
float DSY_SDRAM_BSS main_ram[16000000];

// --- Controls ---
// Definimos los pines para los potenciómetros
AdcChannelConfig adc_cfg[4];
enum AdcChannel { ADC_MIX = 0, ADC_VOL, ADC_TONE, ADC_PITCH, ADC_LAST };

#include "MellotronVoice.h"

// --- Audio Globals ---
MellotronEngine engineA;
MellotronEngine engineB;
Svf tone_filter;

// Lista de instrumentos disponibles (Dynamic)
std::vector<std::string> instrument_list;

int current_idx_A = 0;
int current_idx_B = 0;
int octave_offset = 0; // -2 to +2

void ScanInstruments() {
  instrument_list.clear();
  DIR dir;
  FILINFO fno;
  FRESULT res = f_opendir(&dir, "/");
  if (res == FR_OK) {
    while (true) {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0)
        break;
      if (fno.fattrib & AM_DIR) {
        // Ignore hidden folders (starting with .)
        if (fno.fname[0] != '.') {
          instrument_list.push_back(fno.fname);
        }
      }
    }
    f_closedir(&dir);
  }
  // Fallback if empty
  if (instrument_list.empty()) {
    instrument_list.push_back("No Instruments");
  }
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out,
                   size_t size) {
  // Leemos los potenciómetros (0.0 a 1.0)
  float mix_knob = hw.adc.GetFloat(ADC_MIX);
  float vol_knob = hw.adc.GetFloat(ADC_VOL);
  float tone_knob = hw.adc.GetFloat(ADC_TONE);
  float pitch_knob = hw.adc.GetFloat(ADC_PITCH);

  // Mapear Tono a Frecuencia (200Hz a 12kHz)
  float cutoff = fmap(tone_knob, 200.0f, 12000.0f, Mapping::LOG);
  tone_filter.SetFreq(cutoff);

  // Calculamos la velocidad de reproducción basada en el Pitch
  // Center (0.5) = 1.0x. Range approx 0.35x to 2.8x
  // Formula: 2 ^ ((val - 0.5) * 3.33)
  float playback_speed = powf(2.0f, (pitch_knob - 0.5f) * 3.33f);

  for (size_t i = 0; i < size; i++) {
    // Generamos señal de ambos motores
    float sigA = engineA.Process();
    float sigB = engineB.Process();

    // --- LÓGICA DE CROSSFADE (MIX) ---
    float gainA = 1.0f - mix_knob;
    float gainB = mix_knob;

    // Mezclamos las señales
    float mixed_signal = (sigA * gainA) + (sigB * gainB);

    // --- TONE CONTROL (LPF) ---
    tone_filter.Process(mixed_signal);
    mixed_signal = tone_filter.Low();

    // Aplicamos volumen maestro
    mixed_signal *= vol_knob;

    out[0][i] = mixed_signal; // Left
    out[1][i] = mixed_signal; // Right
  }
}

int main(void) {
  // 1. Inicializar Hardware
  hw.Init();
  hw.SetAudioBlockSize(4); // Baja latencia
  float sample_rate = hw.AudioSampleRate();

  // Inicializar Filtro de Tono
  tone_filter.Init(sample_rate);
  tone_filter.SetRes(0.1f); // Resonancia baja para tono suave
  tone_filter.SetDrive(0.0f);

  // 2. Configurar ADC (Potenciómetros)
  // Pines 15-18 para controles de audio
  adc_cfg[ADC_MIX].InitSingle(hw.GetPin(15));
  adc_cfg[ADC_VOL].InitSingle(hw.GetPin(16));
  adc_cfg[ADC_TONE].InitSingle(hw.GetPin(17));
  adc_cfg[ADC_PITCH].InitSingle(hw.GetPin(18));

  hw.adc.Init(adc_cfg, ADC_LAST);
  hw.adc.Start();

  // Inicializar Encoders
  enc_A.Init(hw.GetPin(19), hw.GetPin(21), hw.GetPin(25));
  enc_B.Init(hw.GetPin(20), hw.GetPin(22), hw.GetPin(26));

  // Inicializar Botones de Octava
  octave_down.Init(hw.GetPin(23), 1000, Switch::Type::TYPE_MOMENTARY,
                   Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_UP);
  octave_up.Init(hw.GetPin(24), 1000, Switch::Type::TYPE_MOMENTARY,
                 Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_UP);

  // 3. Inicializar Pantallas (I2C)
  OledDisplay<SSD1306_128x64>::Config disp_cfg;
  disp_cfg.driver_config.transport_config.i2c_address = 0x3C;
  displayA.Init(disp_cfg);
  disp_cfg.driver_config.transport_config.i2c_address = 0x3D;
  displayB.Init(disp_cfg);

  // 4. Inicializar SD Card
  SdmmcHandler::Config sd_cfg;
  sd_cfg.Defaults();
  sd.Init(sd_cfg);
  f_mount(&sd.GetFatFS(), "/", 1);

  // Scan for instruments
  ScanInstruments();

  // Set default indices if available
  if (instrument_list.size() > 1)
    current_idx_B = 1;

  // 5. Inicializar Motores Mellotron
  size_t split_size = 8000000;
  engineA.Init(&hw, &main_ram[0], split_size);
  engineB.Init(&hw, &main_ram[split_size], split_size);

  // Cargar sonidos iniciales
  if (!instrument_list.empty()) {
    engineA.LoadInstrument(instrument_list[current_idx_A].c_str());
    engineB.LoadInstrument(instrument_list[current_idx_B].c_str());
  }

  // 6. Arrancar Audio
  hw.StartAudio(AudioCallback);

  // Bucle Principal (UI y MIDI)
  while (1) {
    // --- Leer Encoders ---
    enc_A.Debounce();
    enc_B.Debounce();
    int inc_A = enc_A.Increment();
    int inc_B = enc_B.Increment();

    if (inc_A != 0 && !instrument_list.empty()) {
      current_idx_A += inc_A;
      if (current_idx_A < 0)
        current_idx_A = instrument_list.size() - 1;
      if (current_idx_A >= (int)instrument_list.size())
        current_idx_A = 0;
      engineA.LoadInstrument(instrument_list[current_idx_A].c_str());
    }

    if (inc_B != 0 && !instrument_list.empty()) {
      current_idx_B += inc_B;
      if (current_idx_B < 0)
        current_idx_B = instrument_list.size() - 1;
      if (current_idx_B >= (int)instrument_list.size())
        current_idx_B = 0;
      engineB.LoadInstrument(instrument_list[current_idx_B].c_str());
    }

    // --- Leer Botones Octava ---
    octave_down.Debounce();
    octave_up.Debounce();

    if (octave_down.RisingEdge()) {
      if (octave_offset > -2)
        octave_offset--;
    }
    if (octave_up.RisingEdge()) {
      if (octave_offset < 2)
        octave_offset++;
    }

    // --- Calcular Porcentajes de Mix ---
    float mix_val = hw.adc.GetFloat(ADC_MIX);
    int mix_pct_B = (int)(mix_val * 100.0f);
    int mix_pct_A = 100 - mix_pct_B;

    char str_buf[32];
    const int center_offset = 10;
    const int title_w = 7 * 7;
    const int title_x = ((128 - title_w) / 2) + center_offset;
    const int mix_label_x = 10;
    const int bar_x = 35;
    const int bar_w = 50;
    const int pct_x = 90;

    // --- Actualizar Pantalla A ---
    displayA.Fill(false);
    displayA.SetCursor(title_x, 5);
    displayA.WriteString("Sound A", Font_7x10, true);

    if (!instrument_list.empty()) {
      int inst_w_A = instrument_list[current_idx_A].length() * 11;
      int inst_x_A = ((128 - inst_w_A) / 2) + center_offset;
      if (inst_x_A < 0)
        inst_x_A = 0;
      displayA.SetCursor(inst_x_A, 25);
      displayA.WriteString(instrument_list[current_idx_A].c_str(), Font_11x18,
                           true);
    }

    displayA.SetCursor(mix_label_x, 52);
    displayA.WriteString("Mix", Font_7x10, true);
    int fill_w_A = (int)(bar_w * (mix_pct_A / 100.0f));
    displayA.DrawRect(bar_x, 54, bar_x + bar_w, 60, true, false);
    if (fill_w_A > 0)
      displayA.DrawRect(bar_x, 54, bar_x + fill_w_A, 60, true, true);
    sprintf(str_buf, "%d%%", mix_pct_A);
    displayA.SetCursor(pct_x, 52);
    displayA.WriteString(str_buf, Font_7x10, true);

    // Octave Indicator A
    sprintf(str_buf, "Oct:%+d", octave_offset);
    displayA.SetCursor(0, 0);
    displayA.WriteString(str_buf, Font_6x8, true);

    displayA.Update();

    // --- Actualizar Pantalla B ---
    displayB.Fill(false);
    displayB.SetCursor(title_x, 5);
    displayB.WriteString("Sound B", Font_7x10, true);

    if (!instrument_list.empty()) {
      int inst_w_B = instrument_list[current_idx_B].length() * 11;
      int inst_x_B = ((128 - inst_w_B) / 2) + center_offset;
      if (inst_x_B < 0)
        inst_x_B = 0;
      displayB.SetCursor(inst_x_B, 25);
      displayB.WriteString(instrument_list[current_idx_B].c_str(), Font_11x18,
                           true);
    }

    displayB.SetCursor(mix_label_x, 52);
    displayB.WriteString("Mix", Font_7x10, true);
    int fill_w_B = (int)(bar_w * (mix_pct_B / 100.0f));
    displayB.DrawRect(bar_x, 54, bar_x + bar_w, 60, true, false);
    if (fill_w_B > 0)
      displayB.DrawRect(bar_x, 54, bar_x + fill_w_B, 60, true, true);
    sprintf(str_buf, "%d%%", mix_pct_B);
    displayB.SetCursor(pct_x, 52);
    displayB.WriteString(str_buf, Font_7x10, true);

    displayB.Update();

    // Procesar MIDI
    midi.Listen();
    while (midi.HasEvents()) {
      auto msg = midi.PopEvent();
      if (msg.type == NoteOn) {
        NoteOnEvent p = msg.AsNoteOn();
        if (p.velocity > 0) {
          float pitch_val = hw.adc.GetFloat(ADC_PITCH);
          float speed = powf(2.0f, (pitch_val - 0.5f) * 3.33f);
          int note = p.note + octave_offset * 12;
          engineA.NoteOn(note, p.velocity / 127.0f, speed);
          engineB.NoteOn(note, p.velocity / 127.0f, speed);
        } else {
          // Velocity 0 = Note Off
          int note = p.note + octave_offset * 12;
          engineA.NoteOff(note);
          engineB.NoteOff(note);
        }
      } else if (msg.type == NoteOff) {
        NoteOffEvent p = msg.AsNoteOff();
        int note = p.note + octave_offset * 12;
        engineA.NoteOff(note);
        engineB.NoteOff(note);
      }
    }

    System::Delay(30);
  }
}
