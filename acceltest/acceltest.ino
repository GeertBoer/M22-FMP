#include "accelerometer.hpp"


Accelerometer* acc;

void setup() {
  // put your setup code here, to run once:
  Wire.begin();

  Serial.begin(115200);
  while (!Serial) {}  // Comment this when not on PC USB

  delay(1000);
  acc = new Accelerometer();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(acc->x());
  delay(100);
}
