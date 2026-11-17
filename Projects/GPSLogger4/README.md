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
