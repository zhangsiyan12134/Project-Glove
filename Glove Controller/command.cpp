#include "command.hh"
#include "pheripheral.hh"

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
    }
  }
  /*
    to filter the noise, the following lines are averaging how many reading is higher
    than the thresholds in given numbers of continuous samples to be considered as a valid
    click. (in this case 85% reading has to be higher than thresholds in 10 continuous
    samples)
  */
  return ((data / 10 >= 0.85) ? 1 : 0);
}

int cmove(int x, int y) {
  //send move data of mouse curser to bluetooth module according to the given x,y value.
  //send new value only when it's changed 
  if((x != 0) or (y != 0)){
    ble.print(F("AT+BleHidMouseMove="));
    ble.print(x);
    ble.print(",");
    ble.println(y);
  }
  return 0;
}

int fingerRead(int finger, int offset) {
  //read one finger and give back a reading inbetween 0~4095
  int reading = analogRead(finger) - offset;
  return reading; //map(reading,0,4095,0,512);
}