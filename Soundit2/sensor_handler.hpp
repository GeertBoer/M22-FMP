#pragma once

#include "soundbox_ui.hpp"
#include "accelerometer.hpp"
#include "tof_sensor.hpp"
#include "threshold_rule.hpp"
#include "fx_rule.hpp"


class SensorHandler {
private:
  Accelerometer* acc;
  ToFSensor* tof_sensor;
  std::vector<string> sensors_and_values;
  char sensor_and_value_buffer[60];

  ThresholdRule* threshold_rule;
  int initial_threshold = 0.0;
  ORIENTATIONS initial_orientation = Z_UP;
  bool initial_condition_bool = true;

  int fx_map_values[4] = {};
  FXRule* fx_rule = NULL;

public:
  SensorHandler();
  void display_all_sensor_values(SoundboxUI* UI, std::string title, int& position, bool selected);

  void display_threshold_mapper(SoundboxUI* UI, THRESHOLD_TRIGGER_TYPES trigger_type, std::string trigger_name, int position);
  void set_threshold(THRESHOLD_TRIGGER_TYPES trigger_type, int position);
  bool check_threshold();
  bool threshold_is_active();
  void set_initial_threshold(THRESHOLD_TRIGGER_TYPES trigger_type);
  void flip_condition_bool();

  void change_fx_value(int position, ROTARY_STATES rotary, FX effect);
  void display_fx_mapper(SoundboxUI* UI, int& position, FX_SENSOR_TYPES sensor_type, bool selected);

  void store_fx_rule(FX_SENSOR_TYPES sensor_type, FX effect);

  void set_current_to_logical(FX_SENSOR_TYPES fx_sensor, FX effect, int position);

  bool check_fx(FX& effect, int& fx_value);
};

void SensorHandler::set_current_to_logical(FX_SENSOR_TYPES fx_sensor, FX effect, int position) {
  if (position < 0) {
    return;
  }
  if (position < 2) {
    switch (fx_sensor) {
      case FX_S_DISTANCE:
        fx_map_values[position] = tof_sensor->read_value();
        break;
      case FX_S_X:
        fx_map_values[position] = acc->x();
        break;
      case FX_S_Y:
        fx_map_values[position] = acc->y();
        break;
      case FX_S_Z:
        fx_map_values[position] = acc->z();
        break;
    }
  }
  if (position == 2) {
    if (effect == (LPF || HPF)) {
      fx_map_values[position] = 10;
    }
    if (effect == VOLUME) {
      fx_map_values[position] = 0;
    }
  }
  if (position == 3) {
    if (effect == (LPF || HPF)) {
      fx_map_values[position] = 10000;
    }
    if (effect == VOLUME) {
      fx_map_values[position] = 100;
    }
  }
}

void SensorHandler::store_fx_rule(FX_SENSOR_TYPES sensor_type, FX effect) {
  if (fx_rule != NULL) {
    delete fx_rule;
  }
  fx_rule = new FXRule(sensor_type, effect, fx_map_values[0], fx_map_values[1], fx_map_values[2], fx_map_values[3]);
}

SensorHandler::SensorHandler() {
  tof_sensor = new ToFSensor();
  acc = new Accelerometer();
  threshold_rule = new ThresholdRule(THRESH_X, -150.0, false, false);
}

void SensorHandler::change_fx_value(int position, ROTARY_STATES rotary, FX effect) {
  int multiplier = 1;

  switch (effect) {
    case LPF:
      if (fx_map_values[position] > 25) {
        multiplier = 5;
      }
      if (fx_map_values[position] > 50) {
        multiplier = 10;
      }
      if (fx_map_values[position] > 100) {
        multiplier = 100;
      }
      if (fx_map_values[position] > 1000) {
        multiplier = 1000;
      }
      break;
    case HPF:
      if (fx_map_values[position] > 25) {
        multiplier = 5;
      }
      if (fx_map_values[position] > 50) {
        multiplier = 10;
      }
      if (fx_map_values[position] > 100) {
        multiplier = 100;
      }
      if (fx_map_values[position] > 1000) {
        multiplier = 1000;
      }
      break;
    case VOLUME:
      multiplier = 5;
      break;
  }

  switch (rotary) {
    case STILL:
      break;
    case TURNED_CW:
      fx_map_values[position] += multiplier;
      break;
    case TURNED_CCW:
      fx_map_values[position] -= multiplier;
      break;
    default:
      break;
  }
}

void SensorHandler::display_fx_mapper(SoundboxUI* UI, int& position, FX_SENSOR_TYPES sensor_type, bool selected) {
  std::string title;
  switch (sensor_type) {
    case FX_S_DISTANCE:
      title = "distance: " + to_string(tof_sensor->read_value());
      break;
    case FX_S_X:
      title = "X axis: " + to_string(acc->x());
      break;
    case FX_S_Y:
      title = "Y axis: " + to_string(acc->y());
      break;
    case FX_S_Z:
      title = "Z axis: " + to_string(acc->z());
      break;
    default:
      return;
      break;
  }
  std::vector<string> display_value;
  display_value.push_back("Sensor min: " + to_string(fx_map_values[0]));
  display_value.push_back("Sensor max: " + to_string(fx_map_values[1]));
  display_value.push_back("Fx min: " + to_string(fx_map_values[2]));
  display_value.push_back("Fx max: " + to_string(fx_map_values[3]));
  display_value.push_back("Save settings");

  UI->draw_list_selector(display_value, title, position, selected);
}

void SensorHandler::set_initial_threshold(THRESHOLD_TRIGGER_TYPES trigger_type) {
  if (trigger_type == THRESH_TOF) {
    initial_threshold = tof_sensor->read_value();
  }
  if (trigger_type == THRESH_X) {
    initial_threshold = acc->x();
  }
  if (trigger_type == THRESH_Y) {
    initial_threshold = acc->y();
  }
  if (trigger_type == THRESH_Z) {
    initial_threshold = acc->z();
  }
  if (trigger_type == THRESH_ORIENTATION) {
    initial_orientation = acc->get_orientation_or();
  }
}

void SensorHandler::display_threshold_mapper(SoundboxUI* UI, THRESHOLD_TRIGGER_TYPES trigger_type, std::string trigger_name, int position) {
  if (trigger_type == THRESH_TOF) {
    UI->draw_threshold_mapper("Click to set threshold", trigger_name, tof_sensor->read_value(), initial_threshold + position, initial_condition_bool);
  }
  if (trigger_type == THRESH_X) {
    UI->draw_threshold_mapper("Click to set threshold", trigger_name, acc->x(), initial_threshold + position, initial_condition_bool);
  }
  if (trigger_type == THRESH_Y) {
    UI->draw_threshold_mapper("Click to set threshold", trigger_name, acc->y(), initial_threshold + position, initial_condition_bool);
  }
  if (trigger_type == THRESH_Z) {
    UI->draw_threshold_mapper("Click to set threshold", trigger_name, acc->z(), initial_threshold + position, initial_condition_bool);
  }
  if (trigger_type == THRESH_ORIENTATION) {
    UI->draw_threshold_mapper("Click to set threshold", trigger_name, acc->get_orientation_string(), initial_condition_bool);
  }
}

void SensorHandler::set_threshold(THRESHOLD_TRIGGER_TYPES trigger_type, int position) {
  if (trigger_type == THRESH_TOF) {
    delete threshold_rule;
    threshold_rule = new ThresholdRule(trigger_type, initial_threshold + position, initial_condition_bool, true);
  }
  if (trigger_type == THRESH_X) {
    delete threshold_rule;
    threshold_rule = new ThresholdRule(trigger_type, initial_threshold + position, initial_condition_bool, true);
  }
  if (trigger_type == THRESH_Y) {
    delete threshold_rule;
    threshold_rule = new ThresholdRule(trigger_type, initial_threshold + position, initial_condition_bool, true);
  }
  if (trigger_type == THRESH_Z) {
    delete threshold_rule;
    threshold_rule = new ThresholdRule(trigger_type, initial_threshold + position, initial_condition_bool, true);
  }
  if (trigger_type == THRESH_ORIENTATION) {
    delete threshold_rule;
    threshold_rule = new ThresholdRule(acc->get_orientation_or(), initial_condition_bool, true);
  }
}

void SensorHandler::display_all_sensor_values(SoundboxUI* UI, std::string title, int& position, bool selected) {
  sensors_and_values.clear();

  sensor_and_value_buffer[59] = '\n';
  sprintf(sensor_and_value_buffer, "%s: %i", "Distance in mm", tof_sensor->read_value());
  sensors_and_values.push_back(sensor_and_value_buffer);

  sensor_and_value_buffer[59] = '\n';
  sprintf(sensor_and_value_buffer, "%s: %i", "Angle x", acc->x());
  sensors_and_values.push_back(sensor_and_value_buffer);

  sensor_and_value_buffer[59] = '\n';
  sprintf(sensor_and_value_buffer, "%s: %i", "Angle y", acc->y());
  sensors_and_values.push_back(sensor_and_value_buffer);

  sensor_and_value_buffer[59] = '\n';
  sprintf(sensor_and_value_buffer, "%s: %i", "Angle z", acc->z());
  sensors_and_values.push_back(sensor_and_value_buffer);
  sensors_and_values.push_back("Orientation: " + acc->get_orientation_string());

  UI->draw_list_selector(sensors_and_values, title, position, selected);
}

bool SensorHandler::check_threshold() {
  return threshold_rule->check(tof_sensor, acc);
}

bool SensorHandler::threshold_is_active() {
  return threshold_rule->get_is_active();
}

void SensorHandler::flip_condition_bool() {
  initial_condition_bool = !initial_condition_bool;
}

bool SensorHandler::check_fx(FX& effect, int& fx_value) {
  if (fx_rule == NULL) {
    return false;
  }

  return fx_rule->check(tof_sensor, acc, effect, fx_value);
}


/*
if (trigger_type == THRESH_TOF) {

  }
  if (trigger_type == THRESH_X) {

  }
  if (trigger_type == THRESH_Y) {

  }
  if (trigger_type == THRESH_Z) {

  }
  if (trigger_type == THRESH_ORIENTATION) {

  }


*/
