#ifndef PINOUTCONFIG_H // include guard
#define PINOUTCONFIG_H

/************************************************************
 *  Pin Number Enumeration
 ***********************************************************/
#define thumb A0
#define index A1
#define middle A2
#define ring A3
#define pinky A4

#define ModeSW 12
#define BNO08X_RESET 4

#define rgbRed 11
#define rgbGreen 10
#define rgbBlue 9

/************************************************************
 * Setting: Common
 * -------------------------
 * These settings are used in both SW UART, HW UART and SPI mode
 ***********************************************************/
#define BUFSIZE                        128   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   false  // If set to 'true' enables debug output

/*
 * Setting: Software UART
 * -------------------------
 * Pins for 'SW' serial. Options for connecting the UART Friend to an UNO
 */
#define BLUEFRUIT_SWUART_RXD_PIN       9    // Required for software serial!
#define BLUEFRUIT_SWUART_TXD_PIN       10   // Required for software serial!
#define BLUEFRUIT_UART_CTS_PIN         11   // Required for software serial!
#define BLUEFRUIT_UART_RTS_PIN         -1   // Optional, set to -1 if unused


/*
 * Setting: Hardware UART
 * -------------------------
 * The following macros declare the HW serial port you are using. Uncomment
 * this line if you are connecting the BLE to Leonardo/Micro or Flora
 */
#ifdef Serial1    // this makes it not complain on compilation if there's no Serial1
#define BLUEFRUIT_HWSERIAL_NAME      Serial1
#endif

/*
 * Setting: Shared UART
 * -------------------------
 * The following sets the optional Mode pin, its recommended but not required
 */
#define BLUEFRUIT_UART_MODE_PIN        12    // Set to -1 if unused

/*
 * Setting: Hardware SPI
 * -------------------------
 * The following macros declare the pins to use for HW SPI communication.
 * SCK, MISO and MOSI should be connected to the HW SPI pins on the Uno, etc.
 * This should be used with nRF51822 based Bluefruit LE modules that use SPI.
 */
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4    // Optional but recommended, set to -1 if unused

/************************************************************
 *  Initialize ble
 ***********************************************************/
//hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

#endif
