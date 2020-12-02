#ifndef COMMAND_H // include guard
#define COMMAND_H

#include "Config.h"

/* Function: MouseMove
 * --------------------
 * send move data of mouse curser to bluetooth module according to the given x,y value.
 *
 * @x: Movement on x-axis
 * @y: Movement on y-axis
 */
void MouseMove(int x, int y) {
  if((x != 0) or (y != 0)){
    ble.print(F("AT+BleHidMouseMove="));
    ble.print(x);
    ble.print(",");
    ble.println(y);
  }
}

/* Function: FingerRead
 * --------------------
 * read one finger and give back a reading in between 0~4095
 *
 * @finger: Designated finger to read
 * @Offset: Original state reading
 *
 * returns: finger movement readings
 *          map(reading,0,4095,0,512)
 */
int FingerRead(int finger, int offset) {
  int reading;
  reading = analogRead(finger) - offset;
  return reading;
}

/* Function: LeftClick
 * --------------------
 * Read fingers and compare to the thresholds, adjust thresholds and fingers with debug flag enabled.
 * Reading from finger is in between 0 to 4095.
 * All five fingers are avaliable as input(thumb, index, middle, ring and pinky).
 *
 * returns: True (Left Click)
 *          False (No Action)
 */
bool LeftClick(void) {
  int data = 0;
  for ( int i = 0; i <= 10; i++) {
    if (FingerRead(middle, 0) > 3000 && FingerRead(ring, 0) > 3000) {
      data += 1;
    }
  }
  /*Filter the noise by getting 85% reading has to be higher than thresholds in 10 continuous samples*/
  return ((data / 10 >= 0.85) ? 1 : 0);
}


#endif
