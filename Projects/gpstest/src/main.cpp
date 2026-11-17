// GPS test for M5Stack Core2
// Using TinyGPS++ library to read GPS data from a GPS module connected to the M5Stack Core2

#include <M5Unified.h>
#include <TinyGPS++.h>
#define LGFX_AUTODETECT
#include <SD.h>
// #include <LovyanGFX.hpp>

// #include "mysetting.h"

// test of M5Stack Core2 and GPS
// also test of TinyGPS++ to serial output
#define pi 3.141592653589793

// HardwareSerial GPSRaw(2);
#define GPSRaw Serial2
TinyGPSPlus gps;

static M5GFX lcd;

char data[200];
bool isGPSValidDetedcted = false;
int cnt=0;
int z = 12;


void setup() {
  M5.begin();
  lcd.init();
  lcd.setRotation(1);
//  GPSRaw.begin(9600, SERIAL_8N1, 33, 32);
  GPSRaw.begin(9600);
  Serial.begin(115200);
  lcd.setCursor(0,0);
  lcd.setTextScroll(true);
  lcd.fillScreen(BLACK);
  lcd.printf("### GPS TEST %d\n", cnt++);
}

void recvStr(char *buf)
{
  int i = 0;
  char c;
  while (1) {
    if (GPSRaw.available()) {
      c = GPSRaw.read();
      buf[i] = c;
      if (c == '\n') break;
      i++;
    }
  }
  buf[i] = '\0';  // \0: end of string
}

void loop() {
  char buf[200];

  if(!isGPSValidDetedcted) {
    while(GPSRaw.available()>0) {
      char c = GPSRaw.read();
      Serial.write(c);
      if(gps.encode(c)) {
        break;
      }
    }
    if(gps.location.isValid()) {
      isGPSValidDetedcted = true;
      Serial.println("GPS Valid Detected");
      lcd.printf("LAT:%.6f\n", gps.location.lat() );
      lcd.printf("LNG:%.6f\n", gps.location.lng() );
      lcd.printf("ALT:%.2f\n", gps.altitude.meters() );
      lcd.printf("SPEED:%.4f\n",gps.speed.kmph());
      lcd.printf("DATE:%04d/%02d/%02d\n",gps.date.year(),gps.date.month(),gps.date.day());
    }
  } else {
    while(GPSRaw.available()>0) {
      recvStr(buf);
      Serial.println(buf);
      lcd.println(buf);
    }
  }
  
  delay(1000);
}
