#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SPI.h>

#define sclk 13
#define mosi 11
#define cs   10
#define rst  9
#define dc   8
#define  BLACK 0x0000

//画像データ
#include "resized.h"
/*
//画像データ
const uint16_t Img_data [] PROGMEM = {
    0x0000, 0x0000,...,
};
*/

Adafruit_SSD1331 display = Adafruit_SSD1331(&SPI, cs, dc, rst);

void setup() {
  display.begin();
  display.fillScreen(BLACK);
  display.drawRGBBitmap(0, 0,Img_data, 96, 64);
//display.drawRGBBitmap(最初のx座標,y座標,画像配列名,画像の幅,画像の高さ)

  Serial.begin(9600);
}

void loop() {
  Serial.println("Hello World !");
  delay(1000);

}
