// Test of GPS at Port C of M5Core2
// based on sketch_sep09a

#include <M5Core2.h>
#include <TinyGPS++.h>
#define LGFX_AUTODETECT
#include <SD.h>
#include <LovyanGFX.hpp>

#include "mysetting.h"

// test of M5Stack Core2 and GPS
// also test of TinyGPS++ to serial output
// based on a sketch sketch_apr24a (in 2021)
#define pi 3.141592653589793

// HardwareSerial GPSRaw(2);
#define GPSRaw Serial2
TinyGPSPlus gps;

static LGFX lcd;

char data[200];

int cnt=0;
int z = 12;


void setup() {
  M5.begin();
  lcd.init();
  lcd.setRotation(1);
//  GPSRaw.begin(9600, SERIAL_8N1, 33, 32);
  GPSRaw.begin(9600);
  Serial.begin(115200);
}

void loop() {
  lcd.setCursor(0,0);
  lcd.fillScreen(BLACK);
  lcd.printf("### GPS TEST %d\n", cnt++);
  while(GPSRaw.available()>0) {
    char c = GPSRaw.read();
    Serial.write(c);
    if(gps.encode(c)) {
      break;
    }
  }
  if(gps.location.isValid()) {
    lcd.printf("LAT:%.6f\n", gps.location.lat() );
    lcd.printf("LNG:%.6f\n", gps.location.lng() );
    lcd.printf("ALT:%.2f\n", gps.altitude.meters() );
    lcd.printf("SPEED:%.4f\n",gps.speed.kmph());
  } else {
    lcd.printf("INVALID\n");
  }
  //Aボタンを押すとズーム倍率を上げる
  M5.update();
  if(M5.BtnA.wasPressed()){
    if(z!=18){
      z++;
    }
  }
  //Bボタンを押すとズーム倍率を下げる
  if(M5.BtnB.wasPressed()){
    if(z!=12) {
      z--;
    }
  }

  //変数の定義
  double la;
  double ln;
  if(gps.location.isValid())
  { la = gps.location.lat();
    ln = gps.location.lng();
  } else {
    Serial.println("Invalid");
    la = MY_LATITUDE;
    ln = MY_LONGITUDE;
  }

  double L = 85.05112878;
  //緯度経度→ピクセル座標の変換計算
  double px = int(pow(2.0, z + 7.0) * ((ln / 180.0) + 1.0));
  double py = int(pow(2.0, z + 7.0) * (-1 * atanh(sin(pi * la / 180.0)) + atanh(sin(pi * L / 180.0))) / pi);
  //ピクセル座標→タイル座標の変換計算
  int tx = px / 256;
  int ty = py / 256;
  //タイル画像の中の座標を計算
  int x = int(px) % 256;
  int y = int(py) % 256;

  Serial.printf("(lat,lon)=(%f,%f)\n",la,ln);
  Serial.printf("(px,py)=(%f,%f)\n",px,py);
  Serial.printf("(tx,ty)=(%d,%d)\n",tx,ty);
  Serial.printf("(x,y)=(%d,%d)\n",x,y);
 

  //画像9枚のファイルアドレスを用意
  String filename[9];
  filename[0] = String("/std/" + String(z) + "/" + String(tx - 1) + "/" + String(ty-1) + ".png");
  filename[1] = String("/std/" + String(z) + "/" + String(tx) + "/" + String(ty-1) + ".png");
  filename[2] = String("/std/" + String(z) + "/" + String(tx + 1) + "/" + String(ty-1) + ".png");
  filename[3] = String("/std/" + String(z) + "/" + String(tx - 1) + "/" + String(ty) + ".png");
  filename[4] = String("/std/" + String(z) + "/" + String(tx) + "/" + String(ty) + ".png");
  filename[5] = String("/std/" + String(z) + "/" + String(tx + 1) + "/" + String(ty) + ".png");
  filename[6] = String("/std/" + String(z) + "/" + String(tx - 1) + "/" + String(ty+1) + ".png");
  filename[7] = String("/std/" + String(z) + "/" + String(tx) + "/" + String(ty+1) + ".png");
  filename[8] = String("/std/" + String(z) + "/" + String(tx + 1) + "/" + String(ty+1) + ".png");

  /*
  画像9枚の並びはこのようになっている
  [0][1][2]
  [3][4][5]
  [6][7][8]
  */

  //Stringからchar配列に変換
  for (int i = 0; i < 9; i++)
  {
    int str_len = filename[i].length() + 1;
    char file[9][str_len];
    filename[i].toCharArray(file[i], str_len);
  }

  //filename[4]を中心として画像を描画
  int mainx = -1 * (x-160), mainy = -1 * (y-120);
  lcd.drawPngFile(SD, filename[4], mainx, mainy);
  //他8枚の画像を描画

  if (mainx > 0 && mainy > 0)
  {
    lcd.drawPngFile(SD, filename[0], mainx - 256, mainy-256);
  }
  if (mainy > 0)
  {
    lcd.drawPngFile(SD, filename[1], mainx , mainy-256);
  }
  if (mainx + 256 < 320 && mainy >0)
  {
    lcd.drawPngFile(SD, filename[2], mainx + 256, mainy - 256);
  }


  if (mainx > 0)
  {
    lcd.drawPngFile(SD, filename[3], mainx - 256, mainy);
  }
  if (mainx + 256 < 320)
  {
    lcd.drawPngFile(SD, filename[5], mainx + 256, mainy);
  }


  if (mainx > 0 && mainy < -26)
  {
    lcd.drawPngFile(SD, filename[6], mainx - 256, mainy + 256);
  }
  if (mainy < -26)
  {
    lcd.drawPngFile(SD, filename[7], mainx, mainy + 256);
  }
  if (mainx + 256 < 320 && mainy < -26)
  {
    lcd.drawPngFile(SD, filename[8], mainx + 256, mainy + 256);
  }

  //中心に印をつける
  lcd.fillCircle(160, 120, 8, TFT_CYAN);
  lcd.fillCircle(160, 120, 5, TFT_BLUE);

  delay(5000);
}
