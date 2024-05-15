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
AudioPlaySdResmp playSdWav;  //xy=158,218
AudioInputI2S i2s2;          //xy=301,462
AudioFilterBiquad biquad1;   //xy=351,333
AudioFilterBiquad biquad0;   //xy=367,232
AudioRecordQueue queue1;     //xy=500,462
AudioAnalyzePeak peak1;      //xy=502,519
AudioMixer4 mixer0;          //xy=520,219
AudioMixer4 mixer1;          //xy=525,347
AudioOutputI2S i2s1;         //xy=745,218
AudioConnection patchCord1(playSdWav, 0, mixer0, 0);
AudioConnection patchCord2(playSdWav, 0, biquad0, 0);
AudioConnection patchCord3(playSdWav, 1, mixer1, 0);
AudioConnection patchCord4(playSdWav, 1, biquad1, 0);
AudioConnection patchCord5(i2s2, 0, queue1, 0);
AudioConnection patchCord6(i2s2, 0, peak1, 0);
AudioConnection patchCord7(biquad1, 0, mixer1, 1);
AudioConnection patchCord8(biquad0, 0, mixer0, 1);
AudioConnection patchCord9(mixer0, 0, i2s1, 0);
AudioConnection patchCord10(mixer1, 0, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1;  //xy=403,92
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
  byte byte1, byte2, byte3, byte4;
  // end write wav

  int mic_gain = 80;



public:
  AudioSystem(int master_volume);
  void set_volume(int volume);
  int get_volume();
  bool play_wav(int samplenr);
  bool is_playing();
  void stop_playing();

  void start_recording(int samplenr);
  unsigned long continue_recording();
  void stop_recording();
  const char* get_last_recording_name();

  void mic_gain_plus();
  void mic_gain_min();
  float get_mic_peak();

  void set_lpf(int cutoff);
  void set_hpf(int cutoff);

  void set_speed(float speed);

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

  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      S_PL("Unable to access the SD card");
      delay(500);
    }
  }

  AudioMemory(8);
}

float AudioSystem::mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int AudioSystem::get_volume() {
  return mapf(master_volume_f, 0, 0.3, 0.0, 100.0);
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
      byte buffer[512];
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
      frec.write((byte*)queue1.readBuffer(), 256);
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
  // S_P("Setting cutoff L: ");
  // S_PL(cutoff);
  biquad0.setLowpass(0, (float)cutoff, 1.5f);
  biquad1.setLowpass(0, (float)cutoff, 1.5f);

  mixer0.gain(0, 0.0);
  mixer1.gain(0, 0.0);
  mixer0.gain(1, master_volume_f);
  mixer1.gain(1, master_volume_f);
}

void AudioSystem::set_hpf(int cutoff) {
  // S_P("Setting cutoff H: ");
  // S_PL(cutoff);
  biquad0.setHighpass(0, (float)cutoff, 1.5f);
  biquad1.setHighpass(0, (float)cutoff, 1.5f);

  mixer0.gain(0, 0.0);
  mixer1.gain(0, 0.0);
  mixer0.gain(1, master_volume_f);
  mixer1.gain(1, master_volume_f);
}
