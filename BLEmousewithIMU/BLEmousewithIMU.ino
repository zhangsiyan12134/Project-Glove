/*
  This is the program for Adafruit Feather M0 Bluefruit LE with Adafruit BNO085 IMU Module.
  In theory, it can be directly uses on Arduino M0 with Adafruit Bluefruit LE SPI Friend and Adafruit BNO085 IMU Module.
*/

//include libraries for microcontroller and BLE module
#include "BluefruitConfig.h"
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

//include libraries for IMU
#include <Wire.h>
#include <Adafruit_BNO08x.h>

//include other libraries
#include "PinoutConfig.h"
#include "peripherals.h"
#include "FlexSensor.h"
#include "GestureDefiniation.h"

//settings for the project
const int sensitivity = 700; //sensitivity of the movement
const int debug = 0; //Debug flag
const int stream = 1; //3D rotational data streaming

//initialize all global variables
int xMove, yMove, drift, b;
bool leftClick = 0, rightClick = 0;
float _x0, _y0, _z0;
float _r1, _x1, _y1, _z1, _x2, _y2;


// Set the delay between fresh samples
Adafruit_BNO08x  bno08x(BNO08X_RESET);
//declear variable for the sensor data
sh2_SensorValue_t sensorValue;

/*=========================================================================
  APPLICATION SETTINGS

  FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
  MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
  -----------------------------------------------------------------------*/
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
/*=========================================================================*/
/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
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

//Define the BNO080 sensor outputs format
void setReports(void) {
  Serial.println("Setting desired reports");
  if (! bno08x.enableReport(SH2_GAME_ROTATION_VECTOR)) {
    Serial.println("Could not enable game vector");
  }
}

void setup(void) {
  /***********Configure the microcontroller***********/
  while (!Serial and debug);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);

  pinMode(ModeSW, INPUT);
  analogReadResolution(12); //Change the ADC resolution to 12 bits
  
  /****************Configure the IMU****************/
  // Try to initialize BNO080 IMU!
  if (!bno08x.begin_I2C(0x4B)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) { delay(10); }
  }
  Serial.println("BNO08x Found!");
  setReports(); //set pre-defined report data for BNO085
  Serial.println("BNO08x Report Format Set!");
  
  /****************Configure the BLU****************/
  Serial.print(F("Initialising the Bluefruit LE module: "));
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    //Perform a factory reset to make sure everything is in a known state
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  //Disable command echo from Bluefruit
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  //Print Bluefruit information
  ble.info();

  // The mouse function requires a minimium firmware version of0.6.6
  if ( !ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    error(F("This sketch requires firmware version " MINIMUM_FIRMWARE_VERSION " or higher!"));
  }
  /* Enable HID Service (including Mouse) */
  Serial.println(F("Enable HID Service (including Mouse): "));
  if (! ble.sendCommandCheckOK(F( "AT+BleHIDEn=On"  ))) {
    error(F("Failed to enable HID (firmware >=0.6.6?)"));
  }

  //Add or remove service requires a reset
  Serial.println(F("Performing a SW reset"));
  if (! ble.reset() ) {
    error(F("Could not reset??"));
  }
  Serial.println();
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void) {
  //For enable and disable function
  while (digitalRead(ModeSW) == 1) {
    //request sensor data from BNO08X
    if (! bno08x.getSensorEvent(&sensorValue)) {
      return;
    }
    //map the quaternions readings to 2d movement
    _r1 = sensorValue.un.gameRotationVector.real;
    _x1 = sensorValue.un.gameRotationVector.k;
    _y1 = sensorValue.un.gameRotationVector.i;
    _z1 = sensorValue.un.gameRotationVector.j;
    //only 2 ratation axis are required for mouse demonstration
    _y2 = _y1 - _y0; //compare the current reading to previous reading to see how far hand was moved
    _x2 = _x1 - _x0;
    _y0 = _y1;  //Store current readings for next cycle
    _x0 = _x1;

    if (_y2 != 0 or _x2 != 0) {
      xMove = -(_x2) * sensitivity * 1.5;  //screen has greater length than width, times 1.5(ratio of the screen)
      yMove = -(_y2) * sensitivity * 1;
      //remove slight drifting when hand is not moving.
      if (yMove > 0) {
        drift = 1;
      } else if (yMove < 0) {
        drift = -1;
      } else {
        drift = 0;
      }

      if (xMove > 0) {
        drift = 1;
      } else if (xMove < 0) {
        drift = -1;
      } else {
        drift = 0;
      }
      cmove(xMove - drift, yMove - drift); // send data and move mouse
    }
    
    if ((LeftClick() == 1) && (leftClick == 0)) {
      leftClick = 1;
      ble.sendCommandCheckOK(F("AT+BleHidMouseButton=L,press"));
      
    }
    if ((LeftClick() == 0) && (leftClick == 1)) {
      leftClick = 0;
      ble.sendCommandCheckOK(F("AT+BleHidMouseButton=0"));
    }

    if (debug) {
      //print debug readings
      Serial.print(xMove - drift);
      Serial.print(",");
      Serial.print(yMove - drift);
      Serial.print(",");
      Serial.print(fingerRead(index, 0));
      Serial.print(",");
      Serial.print(fingerRead(middle, 0));
      Serial.print(",");
      Serial.print(fingerRead(ring, 0));
      Serial.print(",");
      Serial.println(LeftClick());
    }
    if (stream) {
      //print real time IMU and flex sensor readings to serial port
      Serial.print(_r1);
      Serial.print(",");
      Serial.print(_y1);
      Serial.print(",");
      Serial.print(_z1);
      Serial.print(",");
      Serial.print(_x1);
      Serial.print(",");
      Serial.print(fingerRead(index, 0));
      Serial.print(",");
      Serial.print(fingerRead(middle, 0));
      Serial.print(",");
      Serial.print(fingerRead(ring, 0));
      Serial.print(",");
      Serial.println(LeftClick());
    }

    delay(25);
  }
  //second loop
  while (digitalRead(ModeSW) == 0) {
    FadeIn(rgbRed);
    delay(1000);
    FadeIn(rgbGreen);
    delay(1000);
    FadeIn(rgbBlue);
    delay(1000);

    FadeOut(rgbRed);
    delay(1000);
    FadeOut(rgbGreen);
    delay(1000);
    FadeOut(rgbBlue);
    delay(1000);
  }
}
