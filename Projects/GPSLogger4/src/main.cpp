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

#ifdef USE_TINYGPS
  #include <TinyGPS++.h>
  TinyGPSPlus tinyGps;
#endif

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

File logFile;

int gpsHour, gpsMin, gpsSec;
int gpsDay, gpsMonth, gpsYear;

char strbuf[BUFSIZE];
char s1[BUF2SIZE];
char filename[13];
char substr[7];
bool fileEnable;
bool runMode;
bool logFileOpened;

// Prototypes
void doLogging();
void recvStr();
int strip_NMEA(char *orig, int offset, int count);
bool checkSDFile();
void send_nmea_command(const char *p);
void send_PUBX_packet(const char *p);
void dateTime(uint16_t* date, uint16_t* time);
void gpsTime(const char *utcTime);
void gpsDate(const char *utcDate);
void UTCtoLT();
int mysubstr(char *t, const char *s, int pos, unsigned int len );
bool setFileName();
void flushSD();
void log_to_file();
static void smartDelay(unsigned long ms);
void configure_GP8013T();

void gpsTime0(uint16_t UTC);
void gpsDate0(uint16_t dateRead);
static void printInt(unsigned long val, bool valid, int len);
static void printFloat(float val, bool valid, int len, int prec);
static void printStr(const char *str, int len);
static void printDateTime(TinyGPSDate &d, TinyGPSTime &t);

void setup() {
  // int offset;
  // char utcTime[10], utcDate[7];
  // char latitude[11], longtude[12];
  // char NS[2], WE[2];
  // char statGPS[2];
  // char GPSStr[7];
  // char localTime0[9], localDate0[9];
  // char *strbuf2;

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

  // initialize Software Serial and GPS
  GPSRaw.begin(9600);
  while(!GPSRaw);
  delay(1000);

  oled.print("GPS ready");
#ifdef DEBUG_SERIAL
  #ifdef USE_TINYGPS
  Serial.println("GPS ready");
  Serial.println(F("Sats HDOP  Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum"));
  Serial.println(F("           (deg)      (deg)       Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail"));
  Serial.println(F("----------------------------------------------------------------------------------------------------------------------------------------"));
  #else
  Serial.println("GPS ready");
  #endif
#endif

  // wait until GPSdata becomes valid
#ifdef USE_TINYGPS
  do {
    delay(500);
    oled.setCursor(0,0);
    oled.print("Waiting Valid data");
    oled.setCursor(0,6);
    oled.print(tinyGps.location.lat());
    oled.print(" ");
    oled.print(tinyGps.location.lng());
    oled.setCursor(0,12);
    oled.print(tinyGps.date.year());
    oled.print(tinyGps.date.month());
    oled.print(tinyGps.date.day());
    #ifdef DEBUG_SERIAL
    Serial.println("Waiting for valid location and date data.");
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
#else
  do {
    delay(500);
    oled.setCursor(0,0);
    oled.print("waiting Valid data");
#ifdef DEBUG_SERIAL
    Serial.println("waiting for Valid data");
#endif
    if (GPSRaw.available()) {  // if recived serial signal
      recvStr();   // read serial data to string buffer
    }
#ifdef DEBUG_SERIAL
    Serial.print(strbuf);
#endif
    strbuf2=strstr(strbuf,"$GNRMC");
    if (strbuf2) { // if RMC line
      offset = strip_NMEA(strbuf2, 0, 1); // s1 should should be $GNRMC
      offset = strip_NMEA(strbuf2, offset, 1); // utcTime
      strcpy(utcTime, s1);
      offset = strip_NMEA(strbuf2, offset, 1); // status
      strcpy(statGPS, s1);
      offset = strip_NMEA(strbuf2, offset, 1); // latitue
      strcpy(latitude, s1);
      offset = strip_NMEA(strbuf2, offset, 1); // N/W
      strcpy(NS, s1);
      offset = strip_NMEA(strbuf2, offset, 1); // longitude
      strcpy(longtude, s1);
      offset = strip_NMEA(strbuf2, offset, 1); // W/E
      strcpy(WE, s1);
      offset = strip_NMEA(strbuf2, offset, 3); // utcDate
      strcpy(utcDate, s1);
      gpsDate(utcDate);
      gpsTime(utcTime);
      break;
    }
  } while(1);
#endif

// initialize SD card
  filename[0]='\0';
  oled.clear();
  oled.setCursor(0,0);
  oled.print(PROGRAM);
  oled.print(" version: ");
  oled.print(VERSION);
  oled.setCursor(0,6);
  oled.print("Init SD card");
  pinMode(SD_CHIP_SELECT, OUTPUT);
  if (SD.begin(SD_CHIP_SELECT)) {
    fileEnable = checkSDFile();
#ifdef DEBUG_SERIAL
    Serial.println("SD begin succeeded.");
    Serial.print("file name:");
    Serial.println(filename);
#endif
  } else {
    oled.setCursor(0,12);
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
  doLogging();
}

void doLogging()
{
  char utcTime[10], utcDate[7];
  char latitude[11], longtude[12];
  char NS[2], WE[2];
  char statGPS[2];
  char localTime0[9], localDate0[9];
  int offset;
  char *strbuf2;

  if (GPSRaw.available()) {  // if recived serial signal
//    digitalWrite(LED_PIN_NO,LED_ON);
    recvStr();   // read serial data to string buffer
#ifdef DEBUG_SERIAL
    Serial.print(strbuf);
#endif
    if (logFileOpened == true) {
      logFile.print(strbuf);
      logFile.flush();
    }
    // digitalWrite(LED_PIN_NO,LED_OFF);

    // Display date/time/latitude/longitude
    strbuf2=strstr(strbuf,"$GNRMC");
    if (strbuf2) { // if RMC line
      offset = strip_NMEA(strbuf2, 0, 1); // s1 should should be $GNRMC
      offset = strip_NMEA(strbuf, offset, 1); // utcTime
      strcpy(utcTime, s1);
      offset = strip_NMEA(strbuf, offset, 1); // status
      strcpy(statGPS, s1);
      offset = strip_NMEA(strbuf, offset, 1); // latitue
      strcpy(latitude, s1);
      offset = strip_NMEA(strbuf, offset, 1); // N/W
      strcpy(NS, s1);
      offset = strip_NMEA(strbuf, offset, 1); // longitude
      strcpy(longtude, s1);
      offset = strip_NMEA(strbuf, offset, 1); // W/E
      strcpy(WE, s1);
      offset = strip_NMEA(strbuf, offset, 3); // utcDate
      strcpy(utcDate, s1);

      gpsDate(utcDate);
      gpsTime(utcTime);
      UTCtoLT();

      oled.setCursor(0,0);
      sprintf(localTime0, "%02d:%02d:%02d", gpsHour,gpsMin,gpsSec);
      oled.print(localTime0);

      oled.setCursor(64,0);
      sprintf(localDate0, "%02d/%02d/%02d", gpsYear,gpsMonth,gpsDay);
      oled.print(localDate0);

      if (strchr(statGPS, 'A')) {
        oled.setCursor(0,2);
        oled.print(NS);
        oled.print(latitude);
        oled.setCursor(0,4);
        oled.print(WE);
        oled.print(longtude);
#ifdef DEBUG_SERIAL
        Serial.print(NS);
        Serial.print(latitude);
        Serial.print(" ");
        Serial.print(WE);
        Serial.println(longtude);
#endif
      } else {
        oled.setCursor(0,2);
        oled.print("GPS invalid");
      }
    }
  }
}

// recieve string from GPS
void recvStr()
{
  int i = 0;
  char c;
  while (1) {
    if (GPSRaw.available()) {
      c = GPSRaw.read();
      strbuf[i] = c;
      if (c == '\n') break;
      i++;
    }
  }
  i++;
  strbuf[i] = '\0';  // \0: end of string
}

// get info from NMEA sentence
int strip_NMEA(char *orig, int offset, int count)
{
  char *str0, *s0;
  int i, len;

  for (i = 0; i < count; i++) {
    str0 = orig + offset;
    s0 = strchr(str0, ',');
    len = strlen(str0) - strlen(s0);
    strncpy(s1, str0, len); // len < BUF2SIZE +1 assumed
    s1[len] = '\0';
    offset += strlen(s1) + 1;
  }

  return offset;
}

bool checkSDFile()
{
  // check log file
  // int i;
  // int loopmax = 100;
  bool fileNameSet;

  oled.setCursor(0,6);

  fileNameSet=setFileName();
  if(strlen(filename)==0)
    strcpy(filename,"GPtemp.txt");
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

// UTC -> (gpsHour, gpsMin, gpsSec)
void gpsTime0(uint16_t UTC)
{
  gpsHour = int(UTC / 10000);
  gpsMin = int(UTC % 10000 / 100);
  gpsSec = UTC % 100;
}
// Date ->(gpsYear,gpsMonth,gpsYear)
void gpsDate0(uint16_t dateRead)
{
  gpsDay = int(dateRead / 10000);
  gpsMonth = int(dateRead % 10000 / 100);
  gpsYear = dateRead % 100; //last 2 digits, e.g. 2013-> 13
}

void gpsTime(const char *utcTime)
{
  char s[3];
  mysubstr(s,utcTime,0,2);
  gpsHour=atoi(s);
  mysubstr(s,utcTime,2,2);
  gpsMin=atoi(s);
  mysubstr(s,utcTime,4,2);
  gpsSec=atoi(s);
}

void gpsDate(const char *utcDate)
{
  char s[3];
  mysubstr(s,utcDate,4,2);
  gpsYear = atoi(s);
  mysubstr(s,utcDate,2,2);
  gpsMonth = atoi(s);
  mysubstr(s,utcDate,0,2);
  gpsDay = atoi(s);
}
/*
  // Latitude/Longitude
  void gpsLatLong(int lat1, int lat2, int long1, int long2)
  {
  gpsLat = int(lat1/100) + (lat1%100)/60.0 + float(lat2)/10000.0/60.0;
  gpsLong = int(long1/100) + (long1%100)/60.0 + float(long2)/10000.0/60.0;
  }
*/

void UTCtoLT()
{
//  int leap = (gpsYear % 4) * 4 / gpsYear - (gpsYear % 100) * 100 / gpsYear + (gpsYear % 400) * 400 / gpsYear ;
  int leap = (gpsYear / 4) * 4 / gpsYear ;
  int DaysAMonth[12] = {31, 28 + leap, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  gpsHour += TimeZone;
  if (gpsHour < 0)
  {
    gpsHour += 24;
    gpsDay -= 1;
    if (gpsDay < 1)
    {
      if (gpsMonth == 1)
      {
        gpsMonth = 12;
        gpsYear -= 1;
      }
      else
      {
        gpsMonth -= 1;
      }
      gpsDay = DaysAMonth[gpsMonth - 1];
    }
  }
  if (gpsHour >= 24)
  {
    gpsHour -= 24;
    gpsDay += 1;
    if (gpsDay > DaysAMonth[gpsMonth - 1])
    {
      gpsDay = 1;
      gpsMonth += 1;
      if (gpsMonth > 12) {
        gpsYear += 1;
        gpsMonth = 1;
      }
    }
  }
}

int mysubstr(char *t, const char *s, int pos, unsigned int len )
{
//    if( pos < 0 || len < 0 || len > strlen(s) )
    if( pos < 0 || len > strlen(s) )
        return -1;
    for( s += pos; *s != '\0' && len > 0; len-- )
        *t++ = *s++;
    *t = '\0';
    return 0;
}

bool setFileName()
{
  char tmp[32];
#ifdef USE_TINYGPS
  TinyGPSDate d;
  d=tinyGps.date;

  if(d.isValid()) {
    gpsMonth = d.month();
    gpsDay   = d.day();
    gpsYear  = d.year()-2000;
#ifdef DEBUG_SERIAL
    sprintf(tmp,"%d/%d/%d",gpsYear,gpsMonth,gpsDay);
    Serial.println("");
    Serial.print("UTC:");
    Serial.println(tmp);
#endif
    UTCtoLT();
#ifdef DEBUG_SERIAL
    sprintf(tmp,"%d/%d/%d",gpsYear,gpsMonth,gpsDay);
    Serial.print("local time:");
    Serial.println(tmp);
#endif
    sprintf(filename, "GP%02d%02d%02d.txt", gpsYear,gpsMonth,gpsDay);
    return true;
  } else {
    sprintf(filename,"GP000000.txt");
    return false;
  }
#else
#ifdef DEBUG_SERIAL
    sprintf(tmp,"%d/%d/%d",gpsYear,gpsMonth,gpsDay);
    Serial.println("");
    Serial.print("UTC:");
    Serial.println(tmp);
#endif
    UTCtoLT();
#ifdef DEBUG_SERIAL
    sprintf(tmp,"%d/%d/%d",gpsYear,gpsMonth,gpsDay);
    Serial.print("local time:");
    Serial.println(tmp);
#endif
    sprintf(filename, "GP%02d%02d%02d.txt", gpsYear,gpsMonth,gpsDay);
    return true;
#endif
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
  #ifdef USE_TINYGPS
  unsigned long start = millis();
  do
  {
    while (GPSRaw.available())
      tinyGps.encode(GPSRaw.read());
  } while (millis() - start < ms);
  #else
  delay(ms);
  #endif
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

#ifdef USE_TINYGPS
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
#endif

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}


#include <TinyGPSPlus.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

static const int RXPin = 4;   // GPS TX -> Arduino RX
static const int TXPin = 3;   // GPS RX -> Arduino TX
static const int SD_CS = 10;  // SDカードCS

SoftwareSerial gpsSerial(RXPin, TXPin);
TinyGPSPlus gps;

File logfile;

void setup()
{
  Serial.begin(115200);
  GPSRaw.begin(9600);

  Serial.println("GPS Logger Start");

  //-----------------------------------
  // SDカード初期化
  //-----------------------------------
  if (!SD.begin(SD_CS))
  {
    Serial.println("SD init failed");
    while (1);
  }

  //-----------------------------------
  // ファイル作成
  //-----------------------------------
  logfile = SD.open("gpslog.csv", FILE_WRITE);

  if (!logfile)
  {
    Serial.println("Cannot open file");
    while (1);
  }

  // ファイルが空ならヘッダを書く
  if (logfile.size() == 0)
  {
    logfile.println(
      "Date_UTC,Time_UTC,Latitude,Longitude,Altitude,Speed,Course");
    logfile.flush();
  }

  Serial.println("Logging started");
}

void loop()
{
  while (GPSRaw.available())
  {
    gps.encode(GPSRaw.read());
  }

  //-----------------------------------
  // 位置と時刻が有効なとき保存
  //-----------------------------------
  static unsigned long lastLog = 0;

  if (gps.location.isValid() &&
      gps.date.isValid() &&
      gps.time.isValid())
  {
    if (millis() - lastLog > 1000)
    {
      lastLog = millis();

      logfile.print(gps.date.year());
      logfile.print("-");

      if (gps.date.month() < 10) logfile.print("0");
      logfile.print(gps.date.month());
      logfile.print("-");

      if (gps.date.day() < 10) logfile.print("0");
      logfile.print(gps.date.day());

      logfile.print(",");

      if (gps.time.hour() < 10) logfile.print("0");
      logfile.print(gps.time.hour());
      logfile.print(":");

      if (gps.time.minute() < 10) logfile.print("0");
      logfile.print(gps.time.minute());
      logfile.print(":");

      if (gps.time.second() < 10) logfile.print("0");
      logfile.print(gps.time.second());

      logfile.print(",");

      logfile.print(gps.location.lat(), 6);
      logfile.print(",");

      logfile.print(gps.location.lng(), 6);
      logfile.print(",");

      logfile.print(gps.altitude.meters(), 1);
      logfile.print(",");

      logfile.print(gps.speed.kmph(), 1);
      logfile.print(",");

      logfile.println(gps.course.deg(), 1);

      logfile.flush();

      Serial.print("Logged: ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(",");
      Serial.println(gps.location.lng(), 6);
    }
  }
}