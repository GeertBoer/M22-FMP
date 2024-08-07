#pragma once

#include <string>


#include <ADXL345_WE.h>
#include <Wire.h>

#include "enums.hpp"

class Accelerometer {
private:
  ADXL345_WE* acc;
  xyzFloat raw;

public:
  Accelerometer();
  int x();
  int y();
  int z();
  std::string get_orientation_string();
  ORIENTATIONS get_orientation_or();
};

Accelerometer::Accelerometer() {
  acc = new ADXL345_WE();
  // // start accelerometer
  if (!acc->init()) {
    Serial.println("ADXL345 not connected!");
  } else {
    Serial.println("ADXL345 connected!");
  }
  acc->setCorrFactors(-266.0, 285.0, -268.0, 278.0, -291.0, 214.0);  // idk stond in de lib... ??
  acc->measureAngleOffsets();
  acc->setDataRate(ADXL345_DATA_RATE_100);
  acc->setRange(ADXL345_RANGE_2G);
}

int Accelerometer::x() {
  raw = acc->getRawValues();
  return (int)raw.x;
}

int Accelerometer::y() {
  raw = acc->getRawValues();
  return (int)raw.y;
}

int Accelerometer::z() {
  raw = acc->getRawValues();
  return (int)raw.z;
}

std::string Accelerometer::get_orientation_string() {
  return acc->getOrientationAsString().c_str();
}

ORIENTATIONS Accelerometer::get_orientation_or(){
  std::string out = get_orientation_string();
  if (out.compare("x up") == 0) {
    return X_UP;
  }
  if (out.compare("x down") == 0) {
    return X_DOWN;
  }
  if (out.compare("y up") == 0) {
    return Y_UP;
  }
  if (out.compare("y down") == 0) {
    return Y_DOWN;
  }
  if (out.compare("z up") == 0) {
    return Z_UP;
  }
  if (out.compare("z down") == 0) {
    return Z_DOWN;
  }

  while (1) {
    Serial.println("Terminal error at ORIENTATIONS Accelerometer::get_orientation_or():: orientation was ");
    Serial.println(get_orientation_string().c_str());
  }
}




