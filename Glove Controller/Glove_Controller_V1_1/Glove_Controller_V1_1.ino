/*
  This is the program for Adafruit Feather M0 Bluefruit LE with Adafruit BNO085 IMU Module.
  In theory, it can be directly uses on Arduino M0 with Adafruit Bluefruit LE SPI Friend and Adafruit BNO085 IMU Module.
*/

//include libraries for microcontroller and BLE module
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

//include libraries for IMU
#include <Wire.h>
#include <Adafruit_BNO08x.h>

//Project libraries
#include "Config.h"
#include "Peripherals.h"
#include "Command.h"

//settings for the project
const int sensitivity = 700;    //sensitivity of the movement
const bool debug_mode = false;  //Debug flag (Debug Mode | 3D rotational data stream)

static float _x0, _y0;          //Global variables to store previous position

Adafruit_BNO08x  bno08x(BNO08X_RESET);// Set the delay between fresh samples
sh2_SensorValue_t sensorValue;  //declear variable for the sensor data

/*=========================================================================
  APPLICATION SETTINGS

  FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
  MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
  -----------------------------------------------------------------------*/
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
/*=========================================================================*/

// Error output
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

//Define the BNO080 sensor outputs format
void setReports(void) {
  Serial.println("Setting desired reports");
  if (! bno08x.enableReport(SH2_GAME_ROTATION_VECTOR)) {
    Serial.println("Could not enable game vector");
  }
}

/**************************************************************************
 *  Initialize all the modules
 **************************************************************************/
void setup(void) {
  /***********Configure the microcontroller***********/
  while (!Serial and debug_mode);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);

  pinMode(ModeSW, INPUT);
  analogReadResolution(12); //Change the ADC resolution to 12 bits
  
  /****************Configure the IMU****************/
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

  //Perform a factory reset to make sure everything is in a known state
  if ( FACTORYRESET_ENABLE ){
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  //Disable command echo from Bluefruit
  ble.echo(false);
  
  //Print Bluefruit information
  Serial.println("Requesting Bluefruit info:");
  ble.info();

  // The mouse function requires a minimium firmware version of0.6.6
  if ( !ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) ){
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

/**************************************************************************
 *  Main Loop Function
 **************************************************************************/
void loop(void) {
  //For enable and disable function
  while (digitalRead(ModeSW) == 1) {
    //request sensor data from BNO08X
    if (! bno08x.getSensorEvent(&sensorValue)) {
      return;
    }
    //map the quaternions readings to 2D movement
    float _r1 = sensorValue.un.gameRotationVector.real;
    float _x1 = sensorValue.un.gameRotationVector.k;
    float _y1 = sensorValue.un.gameRotationVector.i;
    float _z1 = sensorValue.un.gameRotationVector.j;
    //Hand movement through comparing the current with previous reading and update previous readings
    float _y2 = _y1 - _y0; _y0 = _y1;  
    float _x2 = _x1 - _x0; _x0 = _x1;

    int xMove, yMove;

    if (_y2 != 0 || _x2 != 0) {
      xMove = -(_x2) * sensitivity * 1.5;  //screen has greater length than width, times 1.5(ratio of the screen)
      yMove = -(_y2) * sensitivity * 1;

      //Remove slight drifting to ignore the hand shaking
      int drift = 1; // Drift index
      xMove = (xMove > 0) ? xMove - drift
            : (xMove < 0) ? xMove + drift
            : 0;
      yMove = (yMove > 0) ? yMove - drift
            : (yMove < 0) ? yMove + drift
            : 0;
      MouseMove(xMove, yMove); // send data and move mouse
    }
    
    ble.sendCommandCheckOK(F((LeftClick()?"AT+BleHidMouseButton=L,press" : "AT+BleHidMouseButton=0")));

    if (debug_mode) {
      //print debug readings
			String debug_output = String(String(xMove)+","+String(yMove)+","
					+String(FingerRead(index, 0))+","+String(FingerRead(middle, 0))
					+","+String(FingerRead(middle, 0))+","+String(FingerRead(ring, 0))
					+","+String(LeftClick()));
			Serial.println(debug_output);   
    }else{
      //print real time IMU and flex sensor readings to serial port
			String stream_output = String(String(_r1)+","+String(_y1)+","
				+String(_z1)+","+String(_x1)+","+String(_r1)+","+String(_y1)+","
				+String(FingerRead(index, 0))+","+String(FingerRead(middle, 0))
				+","+String(FingerRead(middle, 0))+","+String(FingerRead(ring, 0))
				+","+String(LeftClick()));
      Serial.println(stream_output);
    }
    delay(25);
  }

  /*LED control for continously FadeIn & FadeOut*/
  while (digitalRead(ModeSW) == 0) {
    FadeIn(rgbRed);     delay(1000);
    FadeIn(rgbGreen);   delay(1000);
    FadeIn(rgbBlue);    delay(1000);

    FadeOut(rgbRed);    delay(1000);
    FadeOut(rgbGreen);  delay(1000);
    FadeOut(rgbBlue);   delay(1000);
  }
}
