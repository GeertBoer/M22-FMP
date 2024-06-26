#include <string>
#include <type_traits>
#include <sys/_intsup.h>
#pragma once

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <TeensyVariablePlayback.h>




// GUItool: begin automatically generated code
AudioPlaySdResmp playSdWav;  //xy=78,173
AudioInputI2S i2s2;          //xy=257,675
AudioFilterBiquad biquad0;   //xy=309,204
AudioFilterBiquad biquad1;   //xy=314,402
AudioRecordQueue queue1;     //xy=456,675
AudioAnalyzePeak peak1;      //xy=458,732
AudioMixer4 mixer1;          //xy=497,389
AudioEffectDelay delay2;     //xy=501,522
AudioEffectDelay delay1;     //xy=545,245
AudioMixer4 mixer0;          //xy=550,124
AudioOutputI2S i2s1;         //xy=779,271
AudioConnection patchCord1(playSdWav, 0, mixer0, 0);
AudioConnection patchCord2(playSdWav, 0, biquad0, 0);
AudioConnection patchCord3(playSdWav, 1, mixer1, 0);
AudioConnection patchCord4(playSdWav, 1, biquad1, 0);
AudioConnection patchCord5(i2s2, 0, queue1, 0);
AudioConnection patchCord6(i2s2, 0, peak1, 0);
AudioConnection patchCord7(biquad0, 0, mixer0, 1);
AudioConnection patchCord8(biquad1, 0, mixer1, 1);
AudioConnection patchCord9(mixer1, 0, i2s1, 1);
AudioConnection patchCord10(mixer1, delay2);
AudioConnection patchCord11(delay2, 0, mixer1, 2);
AudioConnection patchCord12(delay1, 0, mixer0, 2);
AudioConnection patchCord13(mixer0, 0, i2s1, 0);
AudioConnection patchCord14(mixer0, delay1);
AudioControlSGTL5000 sgtl5000_1;  //xy=443,23
// GUItool: end automatically generated code





class AudioSystem {
private:
  float master_volume_f = 0.0;
  void write_wav_header();
  File frec;
  unsigned long millis_started = 0;

  std::string recording_filename = "";
  bool is_recording = false;

  // write wav
  unsigned long ChunkSize = 0L;
  unsigned long Subchunk1Size = 16;
  unsigned int AudioFormat = 1;
  unsigned int numChannels = 1;
  unsigned long sampleRate = 44100;
  unsigned int bitsPerSample = 16;
  unsigned long byteRate = sampleRate * numChannels * (bitsPerSample / 8);  // samplerate x channels x (bitspersample / 8)
  unsigned int blockAlign = numChannels * bitsPerSample / 8;
  unsigned long Subchunk2Size = 0L;
  unsigned long recByteSaved = 0L;
  unsigned long NumSamples = 0L;
  uint8_t byte1, byte2, byte3, byte4;
  // end write wav

  int mic_gain = 50;



public:
  AudioSystem(int master_volume);

  void handle_effect(EFFECTS effect, int sensor_value);

  void set_volume(int volume);
  int get_volume();
  bool play_wav(int samplenr);
  bool is_playing();
  void stop_playing();

  void start_recording(int samplenr);
  unsigned long continue_recording();
  void stop_recording();
  const char* get_last_recording_name();

  void delete_sample(int samplenr);

  void mic_gain_plus();
  void mic_gain_min();
  int get_mic_gain();
  float get_mic_peak();

  void set_lpf(int cutoff);
  void set_hpf(int cutoff);
  void set_bpf(int cutoff);
  void disable_filter();

  void set_delay(int level);

  void set_speed(float speed);

  void reset_fx();

  float mapf(float x, float in_min, float in_max, float out_min, float out_max);
};

#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14



AudioSystem::AudioSystem(int master_volume) {
  // configure audio lib
  recording_filename = "";
  is_recording = false;

  set_volume(master_volume);

  sgtl5000_1.enable();
  sgtl5000_1.muteHeadphone();

  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);

  sgtl5000_1.micGain(mic_gain);  //0-63

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);

  playSdWav.enableInterpolation(true);
  playSdWav.setPlaybackRate(1.0);

  delay1.delay(0, 400);
  delay2.delay(0, 400);
  mixer0.gain(2, 0.0);
  mixer1.gain(2, 0.0);

  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      S_PL("Unable to access the SD card");
      delay(500);
    }
  }

  AudioMemory(512);
}

float AudioSystem::mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int AudioSystem::get_volume() {
  return mapf(master_volume_f, 0, 0.3, 0.0, 100.0);
}

int AudioSystem::get_mic_gain() {
  return mic_gain;
}

void AudioSystem::set_volume(int volume) {
  if (volume < 0) {
    volume = 0;
  }
  if (volume > 100) {
    volume = 100;
  }
  master_volume_f = mapf((float)volume, 0, 100, 0.0, 0.3);
  mixer0.gain(0, master_volume_f);
  mixer0.gain(1, master_volume_f);
  mixer1.gain(0, master_volume_f);
  mixer1.gain(1, master_volume_f);
}

void AudioSystem::set_speed(float speed) {
  playSdWav.setPlaybackRate(speed);
}

bool AudioSystem::play_wav(int samplenr) {
  std::string playback_filename = "";
  playback_filename = "/Soundit/" + to_string(samplenr) + ".wav";
  return playSdWav.playWav(playback_filename.c_str());
}

void AudioSystem::stop_playing() {
  playSdWav.stop();
}

bool AudioSystem::is_playing() {
  return playSdWav.isPlaying();
}

void AudioSystem::reset_fx() {
  mixer0.gain(2, 0.0);
  mixer1.gain(2, 0.0);
  disable_filter();
  playSdWav.setPlaybackRate(1.0);
}

void AudioSystem::delete_sample(int samplenr) {
  S_P("deleting sample");

  std::string deleting_sample = "";
  deleting_sample = "/Soundit/" + to_string(samplenr) + ".wav";

  if (SD.exists(deleting_sample.c_str())) {
    S_P("deleting sample");
    S_PL(deleting_sample.c_str());
    SD.remove(deleting_sample.c_str());
  } else {
    S_P(deleting_sample.c_str());
    S_PL("does not exist");
  }
}

void AudioSystem::start_recording(int samplenr) {
  sgtl5000_1.muteLineout();

  S_PL("startRecording");

  recording_filename = "";
  recording_filename = "/Soundit/" + to_string(samplenr) + ".wav";

  while (SD.exists(recording_filename.c_str())) {
    recording_filename = to_string(samplenr) + ".wav";
  }
  S_PL(recording_filename.c_str());
  frec = SD.open(recording_filename.c_str(), FILE_WRITE);
  if (frec) {
    queue1.begin();
    recByteSaved = 0L;
    is_recording = true;
  }

  millis_started = millis();
}

unsigned long AudioSystem::continue_recording() {
  if (is_recording) {
    if (queue1.available() >= 2) {
      uint8_t buffer[512];
      memcpy(buffer, queue1.readBuffer(), 256);
      queue1.freeBuffer();
      memcpy(buffer + 256, queue1.readBuffer(), 256);
      queue1.freeBuffer();
      frec.write(buffer, 512);
      recByteSaved += 512;
    }

    return millis() - millis_started;
  }
  return 0;
}

void AudioSystem::stop_recording() {
  sgtl5000_1.unmuteLineout();
  S_PL("stopRecording");
  queue1.end();
  if (is_recording) {
    while (queue1.available() > 0) {
      frec.write((uint8_t*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
      recByteSaved += 256;
    }
    write_wav_header();
    frec.flush();
    frec.close();
  }
  S_P("saved as ");
  S_PL(recording_filename.c_str());
}

void AudioSystem::write_wav_header() {
  Subchunk2Size = recByteSaved;
  ChunkSize = Subchunk2Size + 36;
  frec.seek(0);
  frec.write("RIFF");
  byte1 = ChunkSize & 0xff;
  byte2 = (ChunkSize >> 8) & 0xff;
  byte3 = (ChunkSize >> 16) & 0xff;
  byte4 = (ChunkSize >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  frec.write("WAVE");
  frec.write("fmt ");
  byte1 = Subchunk1Size & 0xff;
  byte2 = (Subchunk1Size >> 8) & 0xff;
  byte3 = (Subchunk1Size >> 16) & 0xff;
  byte4 = (Subchunk1Size >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  byte1 = AudioFormat & 0xff;
  byte2 = (AudioFormat >> 8) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  byte1 = numChannels & 0xff;
  byte2 = (numChannels >> 8) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  byte1 = sampleRate & 0xff;
  byte2 = (sampleRate >> 8) & 0xff;
  byte3 = (sampleRate >> 16) & 0xff;
  byte4 = (sampleRate >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  byte1 = byteRate & 0xff;
  byte2 = (byteRate >> 8) & 0xff;
  byte3 = (byteRate >> 16) & 0xff;
  byte4 = (byteRate >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  byte1 = blockAlign & 0xff;
  byte2 = (blockAlign >> 8) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  byte1 = bitsPerSample & 0xff;
  byte2 = (bitsPerSample >> 8) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write("data");
  byte1 = Subchunk2Size & 0xff;
  byte2 = (Subchunk2Size >> 8) & 0xff;
  byte3 = (Subchunk2Size >> 16) & 0xff;
  byte4 = (Subchunk2Size >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  frec.close();
  S_PL("header written");
  S_P("Subchunk2: ");
  S_PL(Subchunk2Size);
}

const char* AudioSystem::get_last_recording_name() {
  return recording_filename.c_str();
}

void AudioSystem::mic_gain_plus() {
  mic_gain++;
  sgtl5000_1.micGain(mic_gain);
}

void AudioSystem::mic_gain_min() {
  mic_gain--;
  sgtl5000_1.micGain(mic_gain);
}

float AudioSystem::get_mic_peak() {
  return peak1.read();
}


void AudioSystem::set_lpf(int cutoff) {
  biquad0.setLowpass(0, (float)cutoff, 1.5f);
  biquad1.setLowpass(0, (float)cutoff, 1.5f);

  mixer0.gain(0, 0.0);
  mixer1.gain(0, 0.0);
  mixer0.gain(1, master_volume_f);
  mixer1.gain(1, master_volume_f);
}

void AudioSystem::set_hpf(int cutoff) {
  biquad0.setHighpass(0, (float)cutoff, 1.5f);
  biquad1.setHighpass(0, (float)cutoff, 1.5f);

  mixer0.gain(0, 0.0);
  mixer1.gain(0, 0.0);
  mixer0.gain(1, master_volume_f);
  mixer1.gain(1, master_volume_f);
}

void AudioSystem::set_bpf(int cutoff) {
  biquad0.setBandpass(0, (float)cutoff, 1.0f);
  biquad1.setBandpass(0, (float)cutoff, 1.0f);

  mixer0.gain(0, 0.0);
  mixer1.gain(0, 0.0);
  mixer0.gain(1, master_volume_f);
  mixer1.gain(1, master_volume_f);
}

void AudioSystem::disable_filter() {
  mixer0.gain(0, master_volume_f);
  mixer1.gain(0, master_volume_f);
  mixer0.gain(1, 0.0);
  mixer1.gain(1, 0.0);
}


void AudioSystem::set_delay(int level) {
  float level_f = mapf((float)level, 0, 200, 0.0, 0.6);
  mixer0.gain(2, level_f);
  mixer1.gain(2, level_f);
}

void AudioSystem::handle_effect(EFFECTS effect, int sensor_value) {
  if (sensor_value < 0) {
    S_PL(sensor_value);
    sensor_value = 0;
  }
  if (sensor_value > 250) {
    S_PL(sensor_value);
    sensor_value = 250;
  }

  switch (effect) {
    case LPF:
      {
        int fx_value = map(sensor_value, 0, 250, 5000, 350);
        if (fx_value < 350) {
          fx_value = 350;
        }
        set_lpf(fx_value);
      }
      break;
    case HPF:
      {
        int fx_value = map(sensor_value, 0, 250, 300, 4000);
        set_hpf(fx_value);
      }
      break;
    case DELAYMIX:
      {
        set_delay(sensor_value);
      }
      break;
    case SPEEDUP:
      {
        float speed = mapf((float)sensor_value, 0, 250, 1.0, 2.0);
        set_speed(speed);
      }
      break;
    case SLOWDOWN:
      {
        float speed = mapf((float)sensor_value, 0, 250, 1.0, 0.3);
        set_speed(speed);
      }
      break;
    case REVERSE:
      {
        float speed = 1.0;
        if (sensor_value < 50) {
          speed = mapf((float)sensor_value, 0, 100, 1.0, -0.5);
        }
        else {
          speed = mapf((float)sensor_value, 100, 250, -0.5, -2.0);
        }

        set_speed(speed);
      }
    default:
      {
        break;
      }
  }
}