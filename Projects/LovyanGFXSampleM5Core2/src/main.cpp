// https://logikara.blog/lovyangfx_m5stack/

#include <M5Core2.h>    // CORE2使用の場合有効
// #include <M5Stack.h>       // M5Stack GRAY使用の場合有効
// #include <M5StickCPlus.h>  // M5StickC Plus使用の場合有効

#define LGFX_AUTODETECT // 自動認識(D-duino-32 XS, PyBadgeはパネルID読取れないため自動認識の対象から外れているそうです)
#define LGFX_USE_V1     // v1.0.0を有効に(v0からの移行期間の特別措置とのこと。書かない場合は旧v0系で動作)
#include <LovyanGFX.hpp>          // lovyanGFXのヘッダを準備
#include <LGFX_AUTODETECT.hpp>    // クラス"LGFX"を準備
static LGFX lcd;                  // LGFXのインスタンスを作成（クラスLGFXを使ってlcdコマンドでいろいろできるようにする）
static LGFX_Sprite canvas(&lcd);  // スプライトを使う場合はLGFX_Spriteのインスタンスを作成

// 変数設定
int cnt;        // 本体ボタンON回数格納用
int x = 0;      // x軸座標格納用
uint16_t color; // 登録色格納用
// 初期設定 -----------------------------------------
void setup() {
  M5.begin();   // 本体初期化
  // LCD初期設定
  lcd.init();                 // LCD初期化
  lcd.setRotation(2);         // 画面向き設定（0～3で設定、4～7は反転)　※CORE2、GRAYの場合
  //lcd.setRotation(1);         // 画面向き設定（0～3で設定、4～7は反転)　※M5StickC Plusの場合
  canvas.setColorDepth(8);    // カラーモード設定（書かなければ初期値16bit。24bit（パネル性能によっては18bit）は対応していれば選択可）
                              // CORE2 GRAY のスプライトは16bit以上で表示されないため8bitに設定
  canvas.setTextWrap(false);  // 改行をしない（画面をはみ出す時自動改行する場合はtrue）
  canvas.setTextSize(1);      // 文字サイズ（倍率）
  canvas.createSprite(lcd.width(), lcd.height()); // canvasサイズ（メモリ描画領域）設定（画面サイズに設定）
}
// メイン -----------------------------------------
void loop() {
  M5.update();                      // 本体ボタン状態更新
  // LCD表示処理（canvas.で指定してメモリ内の仮想画面に描画していく）
  canvas.fillScreen(BLACK);         // 背景塗り潰し
  canvas.setTextColor(WHITE);       // 文字色と背景を指定（文字色, 背景（省略可））
  // 日本語表示
  canvas.setCursor(0, 0);                         // 座標を指定（x, y）
  canvas.setFont(&fonts::lgfxJapanGothic_24);     // ゴシック体（8,12,16,20,24,28,32,36,40）
  canvas.println("液晶表示 ゴシック体");            // 表示内容をcanvasに準備
  canvas.setCursor(0, 25);                        // 座標を指定（x, y）
  canvas.setFont(&fonts::lgfxJapanMincho_24);     // 明朝体（8,12,16,20,24,28,32,36,40）
  canvas.println("液晶表示 明朝体");               // 表示内容をcanvasに準備
  // 本体ボタンON/OFF状態表示
  if (M5.BtnA.isPressed()) {                      // ボタンを押していれば
    canvas.setTextColor(CYAN);                    // 文字色指定
    canvas.drawString("BTN=ON", 5, 56, &Font4);   // 本体ボタンON表示
  } else {                                        // ボタンを押してなければ
    canvas.setTextColor(WHITE);                   // 文字色指定
    canvas.drawString("BTN=OFF", 5, 56, &Font4);  // 本体ボタンOFF表示
  }
  // 本体ボタンON回数カウント表示
  if (M5.BtnA.wasPressed()) {         // ボタンが押されていたら
    cnt++;                            // カウント+1
  }
  canvas.setTextColor(WHITE);         // 文字色指定
  canvas.setCursor(125, 56, &Font4);  // 座標とフォントを指定（x, y, フォント）
  canvas.print("CNT=");               //「CNT=」表示
  canvas.setTextSize(0.5);            // 文字倍率変更
  canvas.setCursor(190, 53, &Font7);  // 座標とフォントを指定（x, y, フォント）
  canvas.printf("%03d", cnt);         // カウント数表示
  canvas.setTextSize(1);              // 文字倍率を戻す
  // 線（色は3種類の方法で指定）
  canvas.drawLine(0, 50, 240, 50, lcd.color332(255, 255, 255)); // 線（始点x,始点y,終点x,終点y,色）
  canvas.drawFastVLine(120, 50, 30, (uint8_t)0xFF);             // 線（始点x,始点y,始点からの垂線長さ,色）
  canvas.drawFastHLine(0, 80, 240, WHITE);                      // 線（始点x,始点y,始点からの平行線長さ,色）
  // 円
  canvas.drawCircle(20, 98, 13, WHITE); // 円（始点x,始点y,半径,色）
  canvas.fillCircle(51, 98, 13, WHITE); // 塗り潰し円（始点x,始点y,半径,色）
  // 三角
  canvas.drawTriangle(69, 110, 85, 86, 101, 110, WHITE);    // 三角（x0, y0, x1, y1, x2, y2, 色）
  canvas.fillTriangle(105, 110, 121, 86, 137, 110, WHITE);  // 塗り潰し三角（x0, y0, x1, y1, x2, y2, 色）
  // 四角
  canvas.drawRect(145, 85, 26, 26, WHITE);  // 四角（始点x,始点y,縦長さ,横長さ）
  canvas.fillRect(175, 85, 26, 26, WHITE);  // 塗り潰し四角（始点x,始点y,縦長さ,横長さ,色）
  // 色名登録色表示（TFT_付きは除く）
  x = 0;                          // x座標リセット
  for (int i = 0 ; i < 19; i++) { // 全19色（TFT_付は除く）分表示繰り返し
    switch (i) {
      case 0:   color = BLACK      ;  break;  //   0,   0,   0
      case 1:   color = NAVY       ;  break;  //   0,   0, 128
      case 2:   color = DARKGREEN  ;  break;  //   0, 128,   0
      case 3:   color = DARKCYAN   ;  break;  //   0, 128, 128
      case 4:   color = MAROON     ;  break;  // 128,   0,   0
      case 5:   color = PURPLE     ;  break;  // 128,   0, 128
      case 6:   color = OLIVE      ;  break;  // 128, 128,   0
      case 7:   color = LIGHTGREY  ;  break;  // 192, 192, 192
      case 8:   color = DARKGREY   ;  break;  // 128, 128, 128
      case 9:   color = BLUE       ;  break;  //   0,   0, 255
      case 10:  color = GREEN      ;  break;  //   0, 255,   0
      case 11:  color = CYAN       ;  break;  //   0, 255, 255
      case 12:  color = RED        ;  break;  // 255,   0,   0
      case 13:  color = MAGENTA    ;  break;  // 255,   0, 255
      case 14:  color = YELLOW     ;  break;  // 255, 255,   0
      case 15:  color = WHITE      ;  break;  // 255, 255, 255
      case 16:  color = ORANGE     ;  break;  // 255, 165,   0
      case 17:  color = GREENYELLOW;  break;  // 173, 255,  47
      case 18:  color = PINK       ;  break;  // 255, 0  , 255
    }
    canvas.fillRect(x , 115, 12, 20, color);  // 塗り潰し四角でcolorを表示
    x = x + 12;                               // x座標を+12
  }
  // グラデーション表示（スプライト有り、仮想画面に描画）
  for (int i = 0; i < 80; i++) {
    canvas.drawGradientLine( 0, 150 + i, 79, 150 + i, RED, GREEN);    // 赤から緑へのグラデーション直線
    canvas.drawGradientLine( 80, 150 + i, 159, 150 + i, GREEN, BLUE); // 緑から青へのグラデーション直線
    canvas.drawGradientLine( 160, 150 + i, 240, 150 + i, BLUE, RED);  // 青から赤へのグラデーション直線
  }
  canvas.pushSprite(0, 0);  // メモリ内に描画したcanvasを座標を指定して表示する

  // グラデーション表示（スプライト無し、直接表示）
  for (int i = 0; i < 80; i++) {
    lcd.drawGradientLine( 0, 240 + i, 79, 240 + i, RED, GREEN);     // 赤から緑へのグラデーション直線
    lcd.drawGradientLine( 80, 240 + i, 159, 240 + i, GREEN, BLUE);  // 緑から青へのグラデーション直線
    lcd.drawGradientLine( 160, 240 + i, 240, 240 + i, BLUE, RED);   // 青から赤へのグラデーション直線
  }
  delay(100); // 遅延時間（ms）
}
