// https://github.com/m5stack/M5StickC/blob/master/examples/Hat/TOF/TOF.ino

// please install vl53l0x lib first (https://github.com/pololu/vl53l0x-arduino)
// lib in Sketch->Includ Library->Library Manager, search for vl53l0x, Author: pololu

#include "M5StickC.h"
#include <Wire.h>
#include <VL53L0X.h>
#include "BluetoothSerial.h"

VL53L0X sensor;
TFT_eSprite img = TFT_eSprite(&M5.Lcd); 

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
//  Wire.begin(0, 26, 100000);
  Wire.begin(0,26);
  
  M5.begin();

  SerialBT.begin("M5StickC+ToF");

  img.createSprite(160, 80);
  img.fillSprite(BLACK);
  img.setTextColor(WHITE);
  img.setTextSize(2);

  sensor.setTimeout(500);
  if (!sensor.init()) {
    img.setCursor(10, 10);
    img.print("Failed");
    img.pushSprite(0, 0);
    Serial.println("Failed to detect and initialize sensor!");
    while (1) {}
  }
  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  sensor.startContinuous(100);

}
/*
void loop() {
  uint16_t distance = sensor.readRangeContinuousMillimeters();
  Serial.print(distance);
  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  Serial.println();
  img.fillSprite(BLACK);
  img.setCursor(10, 10);
  img.print(distance);
  img.pushSprite(0, 0);
  SerialBT.println(distance);
  delay(100);
}
  */
void loop() {
  // シリアルバッファにデータがあるか確認
  if (Serial.available() > 0) {
    // 1文字読み取り
    char cmd = Serial.read();

    // 'M' を受け取った場合のみ処理を実行
    if (cmd == 'M') {
      uint16_t distance = sensor.readRangeContinuousMillimeters();

      // --- シリアル出力 ---
      Serial.print(distance);
      if (sensor.timeoutOccurred()) { 
        Serial.print(" TIMEOUT"); 
      }
      Serial.println();

      // --- 画面更新 (LCD/Sprite) ---
      img.fillSprite(BLACK);
      img.setCursor(10, 10);
      img.print(distance);
      img.pushSprite(0, 0);

      // --- Bluetooth送信 ---
      SerialBT.println(distance);
    }
    
    // 改行コード(\n, \r)など、'M'以外の残った文字をバッファからクリア
    while(Serial.available() > 0 && Serial.peek() < 32) {
      Serial.read();
    }
  }
  
  // CPU負荷を抑えるための微小な待ち時間（必要に応じて）
  delay(10);
}
