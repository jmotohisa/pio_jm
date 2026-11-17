// GPS Logger4 version 2.1

#define PROGRAM "GPSlogger4"
#define VERSION "2.1"

/*
  GPS Logger for Arudino Mega 2560
    with
      u-Blox GM-8013T
      SDcard
      M096P4BL
 */

/*   The circuit: Hardware connection
  (1) Arudino -SDcard: SPI
    SD card attached to SPI bus as follows:
 ** MOSI - pin 11 on Arduino Uno/Duemilanove/Diecimila, Mega: ICSP
 ** MISO - pin 12 on Arduino Uno/Duemilanove/Diecimila, Mega: ICSP
 ** CLK - pin 13 on Arduino Uno/Duemilanove/Diecimila,  Mega: ICSP
 ** CS - depends on your SD card shield or module. ->10

  (2) Arduino Mega 2560 - GM-8013T: Hardware Serial (Serial 1)
      RX: 19 - 3 (TTL output)
      TX: 18 - 4 (TTL input)

  (3) Arduino - M96PBL: I2C
       A5 - SCL
       A4 - SDA
*/


// uncomment if debug and use serial
#define DEBUG_SERIAL

// uncomment if flush timer enable
#define FLUSH_TIMER_ENABLE

// use TinyGPS++ for extracting initial data
#define USE_TINYGPS

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "SSD1306Ascii.h"
//#include "SSD1306AsciiWire.h"
#include "SSD1306AsciiAvrI2c.h"

#include <TinyGPS++.h>

#ifdef FLUSH_TIMER_ENABLE
  #include <MsTimer2.h>
#endif

#define LED_PIN_NO 7
#define SW_PIN_NO 6
#define LED_ON HIGH
#define LED_OFF LOW
#define SD_CHIP_SELECT 10

#define TimeZone (9)
// interval for logging
#define INTERVAL 5

#define BUFSIZE 256
#define BUF2SIZE 100

// initialize the library with the numbers of the interface pins
// SoftwareSerial gps(11, 9); // RX, TX
#define GPSRaw Serial1

// setup u8g object
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

//SSD1306AsciiWire oled;
SSD1306AsciiAvrI2c oled;
TinyGPSPlus tinyGps;

File logFile;

int gpsHour, gpsMin, gpsSec;
int gpsDay, gpsMonth, gpsYear;

int localHour, localMin, localSec;
int localDay, localMonth, localYear;

char strbuf[BUFSIZE];
char s1[BUF2SIZE];
char filename[13];
char substr[7];
bool fileEnable;
bool runMode;
bool logFileOpened;

// Prototypes
bool checkSDFile();
void send_nmea_command(const char *p);
void send_PUBX_packet(const char *p);
void dateTime(uint16_t* date, uint16_t* time);
void UTCtoLT();
bool setFileName();
void flushSD();
static void smartDelay(unsigned long ms);
void configure_GP8013T();
void doLogging2();

static void printInt(unsigned long val, bool valid, int len);
static void printFloat(float val, bool valid, int len, int prec);
static void printStr(const char *str, int len);
static void printDateTime(TinyGPSDate &d, TinyGPSTime &t);

void setup() {
#ifdef DEBUG_SERIAL
  Serial.begin(9600);
  delay(1000);
  Serial.print(PROGRAM);
  Serial.print(" version: ");
  Serial.println(VERSION);
  Serial.println("GPS Logger Started.");
#endif

  pinMode(LED_PIN_NO, OUTPUT) ;      // LEDに接続
  pinMode(SW_PIN_NO, INPUT_PULLUP ) ; // SW に接続し内部プルアップに設定

  // initialize OLED Display M096P4BL
  Wire.begin();
  Wire.setClock(400000L);
#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0
  // Call oled.setI2cClock(frequency) to change from the default frequency.

  oled.setFont(X11fixed7x14);
//  oled.setFont(System5x7);
  oled.clear();
  oled.setCursor(0,0);
  oled.print(PROGRAM);
  oled.setCursor(0,2);
  oled.print("Version: ");
  oled.print(VERSION);
 
  // initialize Software Serial and GPS
  GPSRaw.begin(9600);
  while(!GPSRaw);
  delay(1000);
  oled.clear();
  oled.setCursor(0,0);
  oled.print("GPS ready");
  oled.setCursor(0,2);
  oled.print("Waiting valid data");

#ifdef DEBUG_SERIAL
  Serial.println("GPS ready");
  Serial.println(F("Sats HDOP  Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum"));
  Serial.println(F("           (deg)      (deg)       Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail"));
  Serial.println(F("----------------------------------------------------------------------------------------------------------------------------------------"));
#endif

  // wait until GPSdata becomes valid
  do {
    delay(1000);
//    oled.setCursor(0,18);
//    oled.print(tinyGps.location.lat());
//    oled.print(" ");
//    oled.print(tinyGps.location.lng());
    while (GPSRaw.available()) {
      tinyGps.encode(GPSRaw.read());
    }
    gpsYear = tinyGps.date.year();
    gpsMonth = tinyGps.date.month();
    gpsDay = tinyGps.date.day();
    gpsHour = tinyGps.time.hour();
    gpsMin = tinyGps.time.minute();
    gpsSec = tinyGps.time.second();

    oled.setCursor(0,4);
    oled.print("UTC:");
    oled.print(gpsYear);
    oled.print(gpsMonth);
    oled.print(gpsDay);
    #ifdef DEBUG_SERIAL
    printInt(tinyGps.satellites.value(), tinyGps.satellites.isValid(), 5);
    printFloat(tinyGps.hdop.hdop(), tinyGps.hdop.isValid(), 6, 1);
    printFloat(tinyGps.location.lat(), tinyGps.location.isValid(), 11, 6);
    printFloat(tinyGps.location.lng(), tinyGps.location.isValid(), 12, 6);
    printInt(tinyGps.location.age(), tinyGps.location.isValid(), 5);
    printDateTime(tinyGps.date, tinyGps.time);
    printFloat(tinyGps.altitude.meters(), tinyGps.altitude.isValid(), 7, 2);
    printFloat(tinyGps.course.deg(), tinyGps.course.isValid(), 7, 2);
    printFloat(tinyGps.speed.kmph(), tinyGps.speed.isValid(), 6, 2);
    printStr(tinyGps.course.isValid() ? TinyGPSPlus::cardinal(tinyGps.course.deg()) : "*** ", 6);
    Serial.println("");
    #endif
  } while(!tinyGps.location.isValid() || !tinyGps.date.isValid());

// initialize SD card
  filename[0]='\0';
  oled.clear();
  oled.print("Init SD card");
  pinMode(SD_CHIP_SELECT, OUTPUT);
  UTCtoLT();
  if (SD.begin(SD_CHIP_SELECT)) {
    fileEnable = checkSDFile();
    // ファイルが空ならヘッダを書く
    if (logFile.size() == 0)
    {
      logFile.println(
        "Date_UTC,Time_UTC,Latitude,Longitude,Altitude,Speed,Course");
      logFile.flush();
    }
#ifdef DEBUG_SERIAL
    Serial.println("SD begin succeeded.");
    Serial.print("file name:");
    Serial.println(filename);
#endif
  } else {
//    oled.setCursor(0,12);
//    oled.clearToEOL();
    oled.println("SD begin failed.");
#ifdef DEBUG_SERIAL
    Serial.println("SD begin failed.");
#endif
    fileEnable=false;
  }
  logFileOpened=false;

  // configure output of GM - 8013T
  configure_GP8013T();

  oled.setCursor(0,0);
  oled.clearToEOL();

  // enable timer2
#ifdef FLUSH_TIMER_ENABLE
  MsTimer2::set(60000, flushSD);
  MsTimer2::start();
#endif

  runMode = 0;
  digitalWrite(LED_PIN_NO, LED_OFF); // LED off
}

void loop()
{
  int swStatus;

  swStatus = digitalRead(SW_PIN_NO);
  if (runMode == 1 && swStatus == LOW) {
    runMode = 0; // logging turn off
#ifdef DEBUG_SERIAL
    Serial.println("Logging stopped");
#endif
   digitalWrite(LED_PIN_NO, LOW);
    if (logFileOpened == true) {
      logFile.close();
      logFileOpened = false;
    }
  } else if (runMode == 0 && swStatus == HIGH) { // logging turn on
    runMode = 1;
    if (logFileOpened == false) {
      if(!fileEnable) {
        fileEnable = checkSDFile();
      }
      if(fileEnable) {
        logFile = SD.open(filename, FILE_WRITE);
      }

      if(!logFile || !fileEnable) {
        oled.setCursor(0,6);
        oled.print("Can't open logfile");
      } else {
        logFileOpened=true;
#ifdef DEBUG_SERIAL
        Serial.println("Logging Started.");
#endif
        digitalWrite(LED_PIN_NO, LED_ON);
      }
    }
  }
  while (GPSRaw.available()) {
    tinyGps.encode(GPSRaw.read());
  }
  doLogging2();
  smartDelay(5000);
}

bool checkSDFile()
{
  bool fileNameSet;

  oled.setCursor(0,6);
  fileNameSet=setFileName();
  if(strlen(filename)==0)
    strcpy(filename,"GPtemp.csv");
  oled.println("Opening log file");
#ifdef DEBUG_SERIAL
  Serial.print("Logfile:");
  Serial.print(filename);
  Serial.println(" opened");
#endif

// open log file
  logFile = SD.open(filename, FILE_WRITE);
  if (!logFile) {
    oled.setCursor(0,6);
    oled.print("Can't open logfile");
#ifdef DEBUG_SERIAL
    Serial.println("Can't open logfile");
#endif
    return false;
  }

// logifle opened succesfully
  oled.clear();
  oled.setCursor(0,0);
  oled.print("Log file opened.");
  oled.setCursor(0,6);
  oled.clearToEOL();
  oled.print(filename);
  logFile.close();
  if(fileNameSet) {
    SdFile::dateTimeCallback( &dateTime );
  }
#ifdef DEBUG_SERIAL
  Serial.println("log file opened.");
#endif

  return true;
}

// send NMEA command with checksum to gps (hardware serial)
void send_nmea_command(const char *p)
{
  uint8_t checksum = 0;
  GPSRaw.print('$');
  do {
    char c = *p++;
    if (c) {
      checksum ^= (uint8_t)c;
      GPSRaw.print(c);
    }
    else {
      break;
    }
  }
  while (1);
  GPSRaw.print('*');
  GPSRaw.println(checksum, HEX);
  //  GPSRaw.print("\n\r");
}

void send_PUBX_packet(const char *p)
{
  uint8_t checksum = 0;
  GPSRaw.print('$');
  do {
    char c = *p++;
    if (c) {
      checksum ^= (uint8_t)c;
      GPSRaw.print(c);
    }
    else {
      break;
    }
  }
  while (1);
  GPSRaw.print('*');
  GPSRaw.println(checksum, HEX);
}

// $GNRMC, $GNVTG, $GNGGA, $GPGSV, $GLGSV, $GNGLL, $GNGSA
void configure_GP8013T()
{
  // set NMEA sentence output rate
  // "$PUBX,40", followed by
  // msgID, 
  // output rate on DDC
  // output rate on USART1
  // output rate on USART2
  // output rate on USB
  // output rate on SPI
  // 0 (reserved)
  // and "*" + checksum + CRLF

  /*
  sprintf(strbuf,"PUBX,40,RMC,0,%d,0,0,0,0",INTERVAL);send_PUBX_packet(strbuf);Serial.println(strbuf);
  sprintf(strbuf,"PUBX,40,VTG,0,%d,0,0,0,0",INTERVAL);send_PUBX_packet(strbuf);Serial.println(strbuf);
  sprintf(strbuf,"PUBX,40,GGA,0,%d,0,0,0,0",INTERVAL);send_PUBX_packet(strbuf);Serial.println(strbuf);
  sprintf(strbuf,"PUBX,40,GSV,0,%d,0,0,0,0",INTERVAL);send_PUBX_packet(strbuf);Serial.println(strbuf);
  sprintf(strbuf,"PUBX,40,GLL,0,%d,0,0,0,0",INTERVAL);send_PUBX_packet(strbuf);Serial.println(strbuf);
  sprintf(strbuf,"PUBX,40,GSA,0,%d,0,0,0,0",INTERVAL);send_PUBX_packet(strbuf);Serial.println(strbuf);
*/
  send_PUBX_packet("PUBX,40,RMC,0,5,0,0,0,0");
  send_PUBX_packet("PUBX,40,VTG,0,5,0,0,0,0");
  send_PUBX_packet("PUBX,40,GGA,0,5,0,0,0,0");
  send_PUBX_packet("PUBX,40,GSV,0,5,0,0,0,0");
  send_PUBX_packet("PUBX,40,GLL,0,5,0,0,0,0");
  send_PUBX_packet("PUBX,40,GSA,0,5,0,0,0,0");
}

void dateTime(uint16_t* date, uint16_t* time)
{
  // GPSやRTCから日付と時間を取得
  // FAT_DATEマクロでフィールドを埋めて日付を返す
  *date = FAT_DATE(gpsYear + 2000, gpsMonth, gpsDay);

  // FAT_TIMEマクロでフィールドを埋めて時間を返す
  *time = FAT_TIME(gpsHour, gpsMin, gpsSec);
}

void UTCtoLT()
{
  int leap = (gpsYear / 4) * 4 / gpsYear ;
  int DaysAMonth[12] = {31, 28 + leap, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  localHour = gpsHour;
  localMin = gpsMin;
  localSec = gpsSec;
  localDay = gpsDay;
  localMonth = gpsMonth;
  localYear = gpsYear;

  localHour += TimeZone;
  if (localHour < 0)
  {
    localHour += 24;
    localDay -= 1;
    if (localDay < 1)
    {
      if (localMonth == 1)
      {
        localMonth = 12;
        localYear -= 1;
      }
      else
      {
        localMonth -= 1;
      }
      localDay = DaysAMonth[localMonth - 1];
    }
  }
  if (localHour >= 24)
  {
    localHour -= 24;
    localDay += 1;
    if (localDay > DaysAMonth[localMonth - 1])
    {
      localDay = 1;
      localMonth += 1;
      if (localMonth > 12) {
        localYear += 1;
        localMonth = 1;
      }
    }
  }
}

bool setFileName()
{
  char tmp[32];
  TinyGPSDate d;
  d=tinyGps.date;

  if(d.isValid()) {
    gpsMonth = d.month();
    gpsDay   = d.day();
    gpsYear  = d.year()-2000;
    UTCtoLT();
#ifdef DEBUG_SERIAL
    sprintf(tmp,"%d/%d/%d",gpsYear,gpsMonth,gpsDay);
    Serial.println("");
    Serial.print("UTC:");
    Serial.println(tmp);
    sprintf(tmp,"%d/%d/%d",localYear,localMonth,localDay);
    Serial.print("local time:");
    Serial.println(tmp);
#endif
    sprintf(filename, "GP%02d%02d%02d.csv", localYear,localMonth,localDay);
    return true;
  } else {
    sprintf(filename,"GP000000.csv");
    return false;
  }
}

void flushSD()
{
  if (logFileOpened == true) {
    digitalWrite(LED_PIN_NO, LED_OFF);
    logFile.flush();
    oled.setCursor(0,6);
    oled.clearToEOL();
    oled.print(filename);
    delay(500);
    digitalWrite(LED_PIN_NO, LED_ON);
  }
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GPSRaw.available())
      tinyGps.encode(GPSRaw.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
//  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }
  
  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);

  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}

void doLogging2()
{
  if (tinyGps.location.isValid() &&
      tinyGps.date.isValid() &&
      tinyGps.time.isValid())
  {
      if (tinyGps.date.month() < 10) logFile.print("0");
      logFile.print(tinyGps.date.month());
      logFile.print("-");

      if (tinyGps.date.day() < 10) logFile.print("0");
      logFile.print(tinyGps.date.day());

      logFile.print(",");

      if (tinyGps.time.hour() < 10) logFile.print("0");
      logFile.print(tinyGps.time.hour());
      logFile.print(":");

      if (tinyGps.time.minute() < 10) logFile.print("0");
      logFile.print(tinyGps.time.minute());
      logFile.print(":");

      if (tinyGps.time.second() < 10) logFile.print("0");
      logFile.print(tinyGps.time.second());

      logFile.print(",");

      logFile.print(tinyGps.location.lat(), 6);
      logFile.print(",");

      logFile.print(tinyGps.location.lng(), 6);
      logFile.print(",");

      logFile.print(tinyGps.altitude.meters(), 1);
      logFile.print(",");

      logFile.print(tinyGps.speed.kmph(), 1);
      logFile.print(",");

      logFile.println(tinyGps.course.deg(), 1);

      logFile.flush();

#ifdef DEBUG_SERIAL
      Serial.print("Logged: ");
      Serial.print(tinyGps.location.lat(), 6);
      Serial.print(",");
      Serial.println(tinyGps.location.lng(), 6);
#endif

  }
}
