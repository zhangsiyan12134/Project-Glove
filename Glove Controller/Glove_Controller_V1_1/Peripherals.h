#ifndef PERIPHERALS_H
#define PERIPHERALS_H
#include "Config.h"

/* Function: GetBattery
 * --------------------
 * Convert battery voltage into 0~255 reading
 *
 * returns: Digital battery power level
 */
// int GetBattery(void) {
//   int value;
//   value = (analogRead(A7) * 6.6 / 4096);
//   return map(value, 3.2, 4.2, 0, 255);
// }

/* Function: LEDWrite
 * --------------------
 * lit RGB LED with Green and Red.
 *
 * @R: Red
 * @G: Green
 */
void LEDWrite(int R, int G) {
  analogWrite(rgbRed, R);
  analogWrite(rgbGreen, G);
  return;
}

/* Function: FadeIn
 * --------------------
 * Lit cycle for RGB LED throught mixture of red, green and blue
 *
 * @pin: signal connection pin
 */
void FadeIn(int Pin) {
  for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
    analogWrite(Pin, fadeValue);    // sets the value (range from 0 to 255):
    delay(30);    // wait for 30 milliseconds to see the dimming effect
  }
  return;
}

/* Function: FadeOut
 * --------------------
 * Off cycle for RGB LED throught mixture of red, green and blue
 *
 * @pin: signal connection pin
 */
void FadeOut(int Pin) {
  for (int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5) {
    analogWrite(Pin, fadeValue);    // sets the value (range from 0 to 255):
    delay(30);    // wait for 30 milliseconds to see the dimming effect
  }
  return;
}

#endif
