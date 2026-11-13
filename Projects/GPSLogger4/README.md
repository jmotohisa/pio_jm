# GPSLogger4

GPS Logger using : Arduino MEGA, GPS u-Blox GM-8013T, LCD M096P4BL, and SD card
Ported from project in Arduino_sketches:

/* 
 *  Arudino Mega 2560
 *  with
 *  u-Blox GM-8013T
 *  SDcard
 *  M096P4BL
 *  
 */


Comment from main.cpp
/* note
  Arudino-SDcard
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11 on Arduino Uno/Duemilanove/Diecimila, Mega: ICSP
 ** MISO - pin 12 on Arduino Uno/Duemilanove/Diecimila, Mega: ICSP
 ** CLK - pin 13 on Arduino Uno/Duemilanove/Diecimila,  Mega: ICSP
 ** CS - depends on your SD card shield or module. ->10
*/

/* Arduino Mega 2560 - GM-8013T
 Hardware Serial (Serial 1)
  RX: 19 - 3 (TTL output)
  TX: 18 - 4 (TTL input)
*/

/* Arudio - M096PBL
    A5 - SCL
    A4 - SDA
  */
