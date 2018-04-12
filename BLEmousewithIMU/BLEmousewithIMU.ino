/*
  This example shows how to send HID (keyboard/mouse/etc) data via BLE
  Note that not all devices support BLE Mouse!
  - OSX, Windows 10 both work
  - Android has limited support
  - iOS completely ignores mouse
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
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

//Set IMU sample rate and rename the sensor in bno
#define BNO055_SAMPLERATE_DELAY_MS (10)
Adafruit_BNO055 bno = Adafruit_BNO055();

//define all the pin numbers

#define thumb A1
#define index A2
#define middle A3
#define ring A4
#define pinky A5

#define ModeSW 12

#define rgbRed 11
#define rgbGreen 10
#define rgbBlue 9

//settings for the project
const int sensitivity = 22; //sensitivity of the movement
const int debug = 1; //Debug flag

//initialize all global variables 
int xMove, yMove, drift, b;
float _x0, _y0;
float _x1, _y1, _z1, _x2, _y2;

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         0
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
/*=========================================================================*/


// Create the bluefruit object, either software serial...uncomment these lines

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/

int getBattery(void){
//Convert battery voltage into 0~255 reading	
	int value;
	value = (analogRead(A7)*6.6/4096);
	return map(value, 3.2, 4.2, 0, 255);
}

int LEDWrite(int R, int G){
//lit RGB LED with Green and Red.	
	analogWrite(rgbRed, R);
	analogWrite(rgbGreen, G);
}

int FadeIn(int Pin){
  //Lit cycle for RGB LED throught mixture of red, green and blue
  for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
    // sets the value (range from 0 to 255):
    analogWrite(Pin, fadeValue);
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }
  return 0;
}

int FadeOut(int Pin){
//Off cycle for RGB LED throught mixture of red, green and blue
  for (int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5) {
    // sets the value (range from 0 to 255):
    analogWrite(Pin, fadeValue);
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }
  return 0;    
}

int cmove(int x, int y){
//send move data of mouse curser to bluetooth module according to the given x,y value.
     ble.print(F("AT+BleHidMouseMove="));
     ble.print(x);
     ble.print(",");
     ble.println(y);
     return 0;
}

int fingerRead(int finger, int offset){
//read one finger and give back a reading inbetween 0~4095
  int reading;
  reading = analogRead(finger)-offset;

  return reading; //map(reading,0,4095,0,512);
}

int LeftClick(void){
//read fingers and compare to the thresholds
  int data = 0;
  for( int i=0; i <= 10; i++){
    if(fingerRead(middle, 0)> 2000 && fingerRead(ring, 0)> 2300){
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
  if(data/10 >= 0.85){
	return 1;
  } else {
	return 0;
  }
}

void setup(void){

  while (!Serial and debug);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);

  pinMode(ModeSW,INPUT);
  
  ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_2X_Val; //Set internal gain
  ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC0_Val;
  //ADC->AVGCTRL.bit.SAMPLENUM = ADC_AVGCTRL_SAMPLENUM_8_Val; // average from 4 samples
  analogReadResolution(12); //Change the ADC resolution to 12 bits 

  //initialize IMU module
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  bno.setExtCrystalUse(true); //set IMU module to use external crystal 

  //Initialise the BLU module
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) ) //
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  // This demo only available for firmware from 0.6.6
  if ( !ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    error(F("This sketch requires firmware version " MINIMUM_FIRMWARE_VERSION " or higher!"));
  }

  /* Enable HID Service (including Mouse) */
  Serial.println(F("Enable HID Service (including Mouse): "));
  if (! ble.sendCommandCheckOK(F( "AT+BleHIDEn=On"  ))) {
    error(F("Failed to enable HID (firmware >=0.6.6?)"));
  }

  /* Add or remove service requires a reset */
  Serial.println(F("Performing a SW reset (service changes require a reset): "));
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
void loop(void){
//Two main loops for enable and disable function	
  while(digitalRead(ModeSW) == 1){

    imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER); //get euler angle from IMU

    _x1 = (euler.x());
    _y1 = (euler.y());
    _z1 = (euler.z());

    _y2 = _y1- _y0;  //compare the current reading to previous reading to see how far hand was moved
    _x2 = _x1- _x0;
    _y0 = _y1;  //Store current readings for next cycle
    _x0 = _x1; 
	
    if (_y2 != 0 or _x2 != 0){
	xMove = _x2 * sensitivity * 1.5;  //screen has greater length than width, times 1.5
        yMove = -(_y2) * sensitivity * 1;
	//remove slight drifting when hand is not moving.    
	if(yMove > 0){
		drift = 1;
	} else if(yMove < 0){
		drift = -1;
	} else {drift = 0;}
		
	if(xMove > 0){
		drift = 1;
	} else if(xMove < 0){
		drift = -1;
	} else {drift = 0;}
		
        cmove(xMove-drift, yMove-drift); // send data and move mouse
	}
	
    /******************************************************/
    /*if (_y2 != 0){
        yMove = -(_y2) * sensitivity * 1.8;
		if(yMove > 0){
			drift = 1;
		} else if(yMove < 0){
			drift = -1;
		} else {drift = 0;}
		
        cmove(0, yMove-drift); // move mouse on y axis
	}
	  
    if (_x2 != 0){
        xMove = _x2 * sensitivity;
		if(xMove > 0){
			drift = 1;
		} else if(xMove < 0){
			drift = -1;
		} else {drift = 0;}
		cmove(xMove-drift, 0); // move mouse on x axis
	}*/
	/******************************************************/
    
    if(LeftClick() == 1){
      ble.sendCommandCheckOK(F("AT+BleHidMouseButton=L,press"));
    }
    if(LeftClick() == 0){
      ble.sendCommandCheckOK(F("AT+BleHidMouseButton=0"));
    }
	
    if(debug){
    //print real time reading to serial port(may lagging the movement)
	Serial.print(xMove-drift);
	Serial.print(",");
	Serial.print(yMove-drift);
	Serial.print(",");
	Serial.print(fingerRead(index, 0));
	Serial.print(",");
	Serial.print(fingerRead(middle, 0));
	Serial.print(",");
	Serial.print(fingerRead(ring, 0));
	Serial.print(",");
	Serial.println(LeftClick());
    }
  }  
  //second main loop
  while(digitalRead(ModeSW) == 0){
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
