#pragma once

enum ROTARY_STATES {
  STILL,
  TURNED_CW,
  TURNED_CCW,
};

enum EFFECTS {
  LPF,
  HPF,
  VOLUME,
  SPEED,
  BITCRUSH,
};

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

