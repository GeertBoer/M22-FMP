#pragma once

enum ROTARY_STATES {
  STILL,
  TURNED_CW,
  TURNED_CCW,
};

enum FX {
  LPF,
  HPF,
  VOLUME,
};

enum MAIN_STATES {
  PLAYBACK,
  RECORDER,
  ADJUST_VOLUME,
};

enum RECORDER_STATES {
  BEFORE_RECORDING,
  DURING_RECORDING,
  AFTER_RECORDING
};