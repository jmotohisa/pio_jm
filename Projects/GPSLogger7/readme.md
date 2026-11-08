# GPSLogger7

Ported from project in Arduino_sketches:

Hardware connection: 

 • SSD1306（I2C接続）
	• SCL: PB6
	• SDA: PB7
• SDカード（SPI1接続）
	• SCK: PA5
	• MISO: PA6
	• MOSI: PA7
	• CS: PA4（コード側で任意のGPIOに変更可能）
HardwareSerial (serial2)
	• HardwareSerial Serial2(PA3, PA2);
    RX2: PA3
    TX2: PA2


Taken from original comment in GPSLogger7.ino
~~~~~

// my parser and TinyGPS mixed version

/*
  Arduino or Bule pill (STM32duino)
  with
   u-Blox GM-8013T or GT-902PMGG (u-Blox 8)
   SDcard
   M096P4BL (SSD1306)
*/

/* note
  Arudino - SDcard - Blue Pill
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11 on Arduino Uno/Duemilanove/Diecimila   - PA7
 ** MISO - pin 12 on Arduino Uno/Duemilanove/Diecimila   - PA6
 ** CLK - pin 13 on Arduino Uno/Duemilanove/Diecimila    - PA5
 ** CS - depends on your SD card shield or module. ->10  - PA4

 microSD
 1 DAT2
 2 CD/DAT3: CS
 3 CMD: MOSI
 4 VDD: 3.3
 5 CLK: CLK
 6 VSS: GND
 7 DAT: MISO
 8 DAT1
 9 Card A
 10 Card B

*/

/* Arduino  (with software serial) - GM-8013T - Blue pill (Serial2/UART2)
    RX: 8 - 3 (TTL output) - PA3
    TX: 9 - 4 (TTL input) -  PA2
*/

/* Arudio - M096PBL - Blue Pill
    A5 - SCL - B6 
    A4 - SDA - B7
  */

/* flow
  初期化ステップ1(シリアルポート、ソフトウェアシリアル、OLEDの初期化)
  初期化ステップ2(GPSの設定：データの受信間隔・GPSより出力/ログするセンテンスの設定)
  初期化ステップ3(SD・ファイルの初期化)
  データを受信し、GPRMCもしくはGNRMCセンテンスのチェック→有効なRMCセンテンスならば日付を得る
  日付、および既存ファイルと重複しないようファイル名を決定
  ファイル作成日を指定していったんファイルをopen→close

  loop
  データ受信
  LED点灯
  ファイルが有効ならば
    ファイルをopen
    データをファイルに記録(シリアルに出力)
    ファイルをclose
    LED消灯
  GPRMCもしくはGNRMCセンテンスのチェック、RMCセンテンスならば
    有効なRMCセンテンスならば
      validのindicator をOLEDに表示
      緯度・経度・日付・時刻を得て、OLEDに情報を表示
    でなければ
      invalid dataであることの表示
  loop end

*/
~~~~