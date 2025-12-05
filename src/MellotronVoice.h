#pragma once
#include "daisy_seed.h"
#include "daisysp.h"
#include "fatfs.h"
#include <map>
#include <string>
#include <vector>

using namespace daisy;
using namespace daisysp;

// Configuración del Sampler
#define MAX_VOICES 12 // Increased to 12 for better polyphony

// Estructura para mapear notas a memoria
struct SampleInfo {
  float *start_ptr;
  size_t length;
};

// Clase para manejar una sola voz (nota)
class Voice {
public:
  enum State { INACTIVE, PLAYING, RELEASING };

  Voice()
      : state_(INACTIVE), sample_ptr_(nullptr), sample_len_(0), play_head_(0),
        speed_(1.0f), note_(-1), amp_(0.0f), release_coef_(0.0f) {}

  void Init(float *start_ptr, size_t length, int note) {
    sample_ptr_ = start_ptr;
    sample_len_ = length;
    note_ = note;
    play_head_ = 0;
    state_ = INACTIVE;
    amp_ = 0.0f;
  }

  void Trigger(float speed, float velocity) {
    if (sample_ptr_ != nullptr && sample_len_ > 0) {
      play_head_ = 0;
      speed_ = speed;
      state_ = PLAYING;
      // Simple attack (instant for now, or fast ramp could be added)
      amp_ = velocity;
    }
  }

  void Stop(bool immediate) {
    if (immediate) {
      state_ = INACTIVE;
      amp_ = 0.0f;
    } else if (state_ == PLAYING) {
      state_ = RELEASING;
      // Calculate release decrement for ~0.1s at 48kHz
      // 0.1s = 4800 samples.
      // We want to go from current Amp to 0 in 4800 steps.
      // Simplification: Just multiply by a coefficient < 1.0
      release_coef_ = 0.999f; // Tweak for release time
    }
  }

  float Process() {
    if (state_ == INACTIVE || sample_ptr_ == nullptr)
      return 0.0f;

    float index = play_head_;
    int i = (int)index;
    float frac = index - i;

    if (i >= sample_len_ - 1) {
      state_ = INACTIVE;
      return 0.0f;
    }

    float a = sample_ptr_[i];
    float b = sample_ptr_[i + 1];
    float sample = a + frac * (b - a);

    play_head_ += speed_;

    // Envelope Logic
    if (state_ == RELEASING) {
      amp_ *= 0.95f; // Fast exponential decay for release (~0.1s)
      if (amp_ < 0.001f) {
        state_ = INACTIVE;
        amp_ = 0.0f;
      }
    }

    return sample * amp_;
  }

  bool IsActive() const { return state_ != INACTIVE; }
  int GetNote() const { return note_; }

private:
  State state_;
  float *sample_ptr_;
  size_t sample_len_;
  float play_head_;
  float speed_;
  int note_;
  float amp_;
  float release_coef_;
};

// Clase principal del motor Mellotron
class MellotronEngine {
public:
  MellotronEngine() : loading_(false), ram_base_(nullptr), ram_offset_(0) {}
  ~MellotronEngine() {}

  void Init(DaisySeed *seed, float *ram_start, size_t ram_size) {
    hw_ = seed;
    ram_base_ = ram_start;
    ram_total_size_ = ram_size;
    ram_offset_ = 0;

    for (int i = 0; i < MAX_VOICES; i++) {
      voices_[i].Stop(true);
    }

    // Limpiar mapa
    for (int i = 0; i < 128; i++) {
      sample_map_[i].start_ptr = nullptr;
      sample_map_[i].length = 0;
    }
  }

  void LoadInstrument(const char *folder_name) {
    loading_ = true;
    ram_offset_ = 0; // Sobrescribimos la memoria asignada a este motor

    // Detener todas las voces
    for (int i = 0; i < MAX_VOICES; i++)
      voices_[i].Stop(true);

    // Limpiar mapa actual
    for (int i = 0; i < 128; i++) {
      sample_map_[i].start_ptr = nullptr;
      sample_map_[i].length = 0;
    }

    DIR dir;
    FILINFO fno;
    char path[64];
    sprintf(path, "/%s", folder_name); // Asumimos raíz

    FRESULT res = f_opendir(&dir, path);
    if (res == FR_OK) {
      while (true) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0)
          break;
        if (fno.fattrib & AM_DIR)
          continue;

        // Chequear extensión .wav
        std::string fname = fno.fname;
        if (fname.length() > 4) {
          std::string ext = fname.substr(fname.length() - 4);
          if (ext == ".wav" || ext == ".WAV") {
            // Extraer nota del nombre (ej: "C#3.wav" -> 49)
            int note = ParseNoteFromFilename(fname);
            if (note >= 0 && note < 128) {
              char full_path[128];
              sprintf(full_path, "%s/%s", path, fno.fname);
              LoadWavFile(full_path, note);
            }
          }
        }
      }
      f_closedir(&dir);
    }
    loading_ = false;
  }

  void NoteOn(int note, float velocity, float pitch_speed) {
    if (loading_)
      return;
    if (note < 0 || note > 127)
      return;

    // Si no tenemos sample para esta nota, ignorar
    if (sample_map_[note].start_ptr == nullptr)
      return;

    // Robar voz o buscar libre
    int voice_idx = -1;

    // 1. Buscar voz inactiva
    for (int i = 0; i < MAX_VOICES; i++) {
      if (!voices_[i].IsActive()) {
        voice_idx = i;
        break;
      }
    }

    // 2. Si no, buscar la que esté en Release
    if (voice_idx == -1) {
      for (int i = 0; i < MAX_VOICES; i++) {
        // TODO: Check state directly if possible, or assume active means
        // playing/releasing For now, simple stealing of first voice if all full
        // (Round Robinish)
      }
      // Simple stealing: just take the oldest or first one for now
      voice_idx = 0;
    }

    voices_[voice_idx].Init(sample_map_[note].start_ptr,
                            sample_map_[note].length, note);
    voices_[voice_idx].Trigger(pitch_speed, velocity);
  }

  void NoteOff(int note) {
    for (int i = 0; i < MAX_VOICES; i++) {
      if (voices_[i].IsActive() && voices_[i].GetNote() == note) {
        voices_[i].Stop(false); // Trigger release
      }
    }
  }

  float Process() {
    if (loading_)
      return 0.0f;
    float mix = 0.0f;
    for (int i = 0; i < MAX_VOICES; i++) {
      mix += voices_[i].Process();
    }
    return mix;
  }

private:
  DaisySeed *hw_;
  Voice voices_[MAX_VOICES];
  SampleInfo sample_map_[128];

  float *ram_base_;
  size_t ram_total_size_;
  size_t ram_offset_;
  bool loading_;

  int ParseNoteFromFilename(std::string fname) {
    // Remove extension
    size_t lastindex = fname.find_last_of(".");
    std::string raw = fname.substr(0, lastindex);

    // Map note names to indices
    // C, C#, D, D#, E, F, F#, G, G#, A, A#, B
    const char *notes[] = {"C",  "C#", "D",  "D#", "E",  "F",
                           "F#", "G",  "G#", "A",  "A#", "B"};

    // Find note part and octave part
    // Regex would be nice, but manual parsing is safer on embedded
    // Format: [Note][Octave] e.g. "C#3", "A2"

    std::string note_str;
    int octave = -1;

    // Find where the digit starts
    size_t digit_pos = std::string::npos;
    for (size_t i = 0; i < raw.length(); i++) {
      if (isdigit(raw[i])) {
        digit_pos = i;
        break;
      }
    }

    if (digit_pos == std::string::npos)
      return -1;

    note_str = raw.substr(0, digit_pos);
    // Handle flats if necessary (e.g. Bb -> A#).
    // Assuming standard naming from the web emulator which uses sharps.

    try {
      octave = std::stoi(raw.substr(digit_pos));
    } catch (...) {
      return -1;
    }

    int note_idx = -1;
    for (int i = 0; i < 12; i++) {
      if (note_str == notes[i]) {
        note_idx = i;
        break;
      }
    }

    if (note_idx == -1)
      return -1;

    // MIDI Note = (Octave + 1) * 12 + NoteIdx
    return (octave + 1) * 12 + note_idx;
  }

  void LoadWavFile(const char *filename, int note) {
    FIL file;
    if (f_open(&file, filename, FA_READ) != FR_OK)
      return;

    // Read RIFF header
    UINT bytes_read;
    uint8_t header[12];
    f_read(&file, header, 12, &bytes_read);

    // Check RIFF and WAVE
    if (memcmp(header, "RIFF", 4) != 0 || memcmp(header + 8, "WAVE", 4) != 0) {
      f_close(&file);
      return;
    }

    // Search for "fmt " and "data" chunks
    uint32_t chunk_size;
    uint8_t chunk_id[4];
    uint16_t channels = 1;
    uint32_t data_offset = 0;
    uint32_t data_size = 0;

    while (true) {
      f_read(&file, chunk_id, 4, &bytes_read);
      if (bytes_read < 4)
        break;
      f_read(&file, &chunk_size, 4, &bytes_read);
      if (bytes_read < 4)
        break;

      if (memcmp(chunk_id, "fmt ", 4) == 0) {
        uint8_t fmt_data[16];
        f_read(&file, fmt_data, 16, &bytes_read);
        channels = *(uint16_t *)(fmt_data + 2);
        // Skip remaining fmt bytes if any
        if (chunk_size > 16)
          f_lseek(&file, f_tell(&file) + chunk_size - 16);
      } else if (memcmp(chunk_id, "data", 4) == 0) {
        data_size = chunk_size;
        data_offset = f_tell(&file);
        break; // Found data, stop searching
      } else {
        // Skip unknown chunk
        f_lseek(&file, f_tell(&file) + chunk_size);
      }
    }

    if (data_size == 0) {
      f_close(&file);
      return;
    }

    // Calculate samples to read
    // Assuming 16-bit depth.
    uint32_t num_samples_total = data_size / 2; // Total samples (L+R if stereo)
    uint32_t num_frames =
        (channels == 2) ? num_samples_total / 2 : num_samples_total;

    if (ram_offset_ + num_frames > ram_total_size_) {
      f_close(&file);
      return;
    }

    float *dest = &ram_base_[ram_offset_];
    sample_map_[note].start_ptr = dest;
    sample_map_[note].length = num_frames;

    // Read Data
    const int CHUNK_SIZE = 512; // Buffer size in bytes
    int16_t temp_buf[CHUNK_SIZE];

    uint32_t frames_loaded = 0;

    f_lseek(&file, data_offset);

    while (frames_loaded < num_frames) {
      // We want to read enough bytes to get a chunk of frames
      // If Mono: 1 frame = 2 bytes. If Stereo: 1 frame = 4 bytes.
      UINT bytes_to_read = sizeof(temp_buf);
      UINT bytes_remaining = (num_frames - frames_loaded) * 2 * channels;
      if (bytes_to_read > bytes_remaining)
        bytes_to_read = bytes_remaining;

      f_read(&file, temp_buf, bytes_to_read, &bytes_read);
      if (bytes_read == 0)
        break;

      int samples_read = bytes_read / 2;
      int frames_in_chunk = samples_read / channels;

      for (int i = 0; i < frames_in_chunk; i++) {
        float s;
        if (channels == 2) {
          // Mix L+R to Mono
          int16_t l = temp_buf[i * 2];
          int16_t r = temp_buf[i * 2 + 1];
          s = (l + r) / 65536.0f; // Average and normalize
        } else {
          s = temp_buf[i] / 32768.0f;
        }
        dest[frames_loaded + i] = s;
      }
      frames_loaded += frames_in_chunk;
    }

    ram_offset_ += frames_loaded;
    f_close(&file);
  }
};
