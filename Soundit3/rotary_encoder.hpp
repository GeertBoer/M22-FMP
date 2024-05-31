#pragma once
#include <Encoder.h>
#include "enums.hpp"



class RotaryEncoder {
private:
  Encoder* encoder;

  bool encoder_turned = false;
  ROTARY_STATES encoder_state = STILL;

  long encoder_tracker = 0;
  long encoder_position_new = 0;
  long encoder_position_old = 0;

  int steps_per_click = 0;

public:
  RotaryEncoder(int pin_a, int pin_b, int steps_per_click);
  ~RotaryEncoder();
  void update();
  ROTARY_STATES get_state();
};

RotaryEncoder::RotaryEncoder(int pin_a, int pin_b, int steps_per_click) {
  encoder = new Encoder(pin_a, pin_b);
  this->steps_per_click = steps_per_click;
  encoder_turned = false;
  encoder_state = STILL;
}

RotaryEncoder::~RotaryEncoder() {
  delete this->encoder;
}

void RotaryEncoder::update() {
  encoder_turned = false;
  encoder_state = STILL;

  encoder_position_new = encoder->read();
  if (encoder_position_new != encoder_position_old) {
    encoder_tracker += (encoder_position_old - encoder_position_new);
    encoder_position_old = encoder_position_new;
  }

  if (encoder_tracker >= steps_per_click) {
    encoder_tracker = 0;
    encoder_state = TURNED_CW;
  } else if (encoder_tracker <= -steps_per_click) {
    encoder_tracker = 0;
    encoder_state = TURNED_CCW;
  }
}

ROTARY_STATES RotaryEncoder::get_state() {
  return encoder_state;
}
