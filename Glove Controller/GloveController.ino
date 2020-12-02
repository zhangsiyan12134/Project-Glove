/***************************************************************************
 *  This is the program for Adafruit Feather M0 Bluefruit LE with Adafruit BNO085 IMU Module.
 *  In theory, it can be directly uses on Arduino M0 with Adafruit Bluefruit LE SPI Friend and Adafruit BNO085 IMU Module.
****************************************************************************/
//Libraries for microcontroller, BLE module, and IMU
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <Wire.h>
#include <Adafruit_BNO08x.h>        //Based on IMU model

//Project libraries
#include "Config.hh"
#include "pheripheral.hh"
#include "command.hh"

//settings for the project
const int sensitivity = 700; //sensitivity of the movement
const bool debug_mode = false; //Debug flag (Debug Mode | 3D rotational data stream)

//initialize all global variables
float _x0, _y0; //TODO _z0 The previous position

// Set the delay between fresh samples
Adafruit_BNO08x  bno08x(BNO08X_RESET);
//declear variable for the sensor data
sh2_SensorValue_t sensorValue;

/*=========================================================================*/
/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
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

void setup(void) {
  /***********Configure the microcontroller***********/
  while (!Serial && debug_mode);  // required for Flora & Micro
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
  if ( !ble.begin(VERBOSE_MODE) ){
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE ){
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
    //map the quaternions readings to 2D movement
    float _r1 = sensorValue.un.gameRotationVector.real;
    float _x1 = sensorValue.un.gameRotationVector.k;
    float _y1 = sensorValue.un.gameRotationVector.i;
    float _z1 = sensorValue.un.gameRotationVector.j;
    //Hand movement through comparing the current with previous reading
		//Update previous reading for next cycle
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
      cmove(xMove, yMove); // send data and move mouse
    }
    
    ble.sendCommandCheckOK(F((LeftClick()?"AT+BleHidMouseButton=L,press" : "AT+BleHidMouseButton=0")));

    if (debug_mode) {
      //print debug readings
			String debug_output = String(String(xMove)+","+String(yMove)+","
					+String(fingerRead(index, 0))+","+String(fingerRead(middle, 0))
					+","+String(fingerRead(middle, 0))+","+String(fingerRead(ring, 0))
					+","+String(LeftClick()));
			Serial.println(debug_output);   
    }else{
      //print real time IMU and flex sensor readings to serial port
			String stream_output = String(String(_r1)+","+String(_y1)+","
				+String(_z1)+","+String(_x1)+","+String(_r1)+","+String(_y1)+","
				+String(fingerRead(index, 0))+","+String(fingerRead(middle, 0))
				+","+String(fingerRead(middle, 0))+","+String(fingerRead(ring, 0))
				+","+String(LeftClick()));
      Serial.println(stream_output);
    }
    delay(25);
  }

  //LED control for continously FadeIn & FadeOut
  while (digitalRead(ModeSW) == 0) {
    FadeIn(rgbRed);     delay(1000);
    FadeIn(rgbGreen);   delay(1000);
    FadeIn(rgbBlue);    delay(1000);

    FadeOut(rgbRed);    delay(1000);
    FadeOut(rgbGreen);  delay(1000);
    FadeOut(rgbBlue);   delay(1000);
  }
}
