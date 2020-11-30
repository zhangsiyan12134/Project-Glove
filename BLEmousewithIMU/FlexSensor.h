#ifndef FLEXSENSOR_H // include guard
#define FLEXSENSOR_H

#include "PinoutConfig.h"

int fingerRead(int finger, int offset) {
  //read one finger and give back a reading inbetween 0~4095
  int reading;
  reading = analogRead(finger) - offset;

  return reading; //map(reading,0,4095,0,512);
}

#endif
