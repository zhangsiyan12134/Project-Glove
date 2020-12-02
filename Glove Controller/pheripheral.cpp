#include "pheripheral.hh"

/*******************************************************
 * For battery info and LED control
********************************************************/

int getBattery(void) {
  //Convert battery voltage into 0~255 reading
  int value;
  value = (analogRead(A7) * 6.6 / 4096);
  return map(value, 3.2, 4.2, 0, 255);
}

int LEDWrite(int R, int G) {
  //lit RGB LED with Green and Red.
  analogWrite(rgbRed, R);
  analogWrite(rgbGreen, G);
}

int FadeIn(int Pin) {
  //Lit cycle for RGB LED throught mixture of red, green and blue
  for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
    // sets the value (range from 0 to 255):
    analogWrite(Pin, fadeValue);
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }
  return 0;
}

int FadeOut(int Pin) {
  //Off cycle for RGB LED throught mixture of red, green and blue
  for (int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5) {
    // sets the value (range from 0 to 255):
    analogWrite(Pin, fadeValue);
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }
  return 0;
}