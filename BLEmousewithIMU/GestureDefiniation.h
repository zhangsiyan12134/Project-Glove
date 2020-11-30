#ifndef GESTUREDEFINIATION_H // include guard
#define GESTUREDEFINIATION_H

#include "FlexSensor.h"

int LeftClick(void) {
  /*
    Read fingers and compare to the thresholds, adjust thresholds and fingers with debug flag enabled.
    Reading from finger is in between 0 to 4095.
    All five fingers are avaliable as input(thumb, index, middle, ring and pinky).
  */
  int data = 0;
  for ( int i = 0; i <= 10; i++) {
    if (fingerRead(middle, 0) > 3000 && fingerRead(ring, 0) > 3000) {
      data += 1;
    } else {
      data += 0;
    }
  }
  /*
    to filter the noise, the following lines are averaging how many reading is higher
    than the thresholds in given numbers of continuous samples to be considered as a valid
    click. (in this case 85% reading has to be higher than thresholds in 10 continuous
    samples)
  */
  if (data / 10 >= 0.85) {
    return 1;
  } else {
    return 0;
  }
}

#endif
