#pragma once

enum ROTARY_STATES {
  STILL,
  TURNED_CW,
  TURNED_CCW,
};

enum EFFECTS {
  LPF,
  HPF,
  DELAYMIX,
  SPEEDUP,
  SLOWDOWN,
  REVERSE,
};

const int amount_of_effects = 6;
std::string EFFECTS_STRINGS[amount_of_effects] = {"Low-pass filter", "High-pass filter", "Delay mix", "Speed up", "Slow down", "Reverse"};
std::string AVAILABLE_AXES[4] = {"X+", "X-", "Y+", "Y-"};

enum MAIN_STATES {
  PLAYBACK,
  RECORDER,
};

enum RECORDER_STATES {
  ADJUSTING_MIC_GAIN,
  ADJUSTING_RECORDING_SETTINGS,
  BEFORE_RECORDING,
  MAKING_NEW_FILE_TO_RECORD,
  DURING_RECORDING,
  AFTER_RECORDING,
};

enum PLAYBACK_STATES {
  ADJUSTING_VOLUME,
  CHANGING_SAMPLE_SETTINGS,
  CHANGING_FX_SETTINGS,
  SELECTING_FILE,
  PLAYING,
};

enum SAMPLE_SETTINGS {
  GO_BACK,
  DELETE_SAMPLE,
};

enum ORIENTATIONS {
  X_UP,
  X_DOWN,
  Y_UP,
  Y_DOWN,
  Z_UP,
  Z_DOWN,
};

