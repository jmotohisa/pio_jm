/*
  ESP32 XPT TouchScreen 
*/

/*
 * Hardware
 *  ESP32 DevKit
 *  ILI9341 LCD module (with SD and XPT2046 Touchscreen)
*/

#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
// #include <Time.h>
#include <TimeLib.h>

// Wiring
/*
ILI9341 module | ESP32+XPT2046 Touchscreen
-------|---------------
1 (VCC)    | 3V3
2 (GND)    | GND
3 (CS)     | 14
4 (RESET)  | 33
5 (D/C)    | 27
6 (SDI(MOSI)) | 23
7 (SCK)    | 18
8 (LED)    | 3V3 (with 100ohm)
9 (SDO(MISO) | 19
10 (T_CLK) | 18 SCLK
11 (T_CS)  | 5
12 (T_DIN) | 23 MOSI
13 (T_DO)  | 19 MISO
14 (T_IRQ) | NC(4)

R1 (CS)     | 17
R2 (MOSI)   | MOSI
R3 (MISO)   | MISO
R4(J4) (SCK)    | SCK

*/
#define TFT_CS 14
#define TFT_RST 33
#define TFT_DC 27
#define TFT_MOSI 23
#define TFT_CLK 18
#define TFT_MISO 19

#define TOUCH_SCLK TFT_CLK
#define TOUCH_CS 5
#define TOUCH_DIN TFT_MOSI
#define TOUCH_DOUT TFT_MISO
#define TOUCH_IRQ 4

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC,TFT_RST);

XPT2046_Touchscreen ts= XPT2046_Touchscreen(TOUCH_CS, TOUCH_IRQ);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

int xmin,xmax,ymin,ymax;
int i=0;
double Tmin=10.0,Tmax=30.0;
int interval=10;

// globals
unsigned long whenCountStarted = 0;

typedef struct BUTTON{
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t t;
  char key[8];
  uint8_t status;
} BUTTON;

#define LOOP 0
#define WAIT 1
#define SETTING 1
BUTTON key0_0 = { 195, 220,45,100,"setting",WAIT};
BUTTON *key0_array[1] = { &key0_0 };

#define KEYIN  0
#define KEYCLR 1
#define KINFIN 2
#define SKIP   3
BUTTON key1_7 = { 20, 20,45,45,"7",KEYIN};
BUTTON key1_8 = { 70, 20,45,45,"8",KEYIN};
BUTTON key1_9 = {120, 20,45,45,"9",KEYIN};
BUTTON key1_4 = { 20, 70,45,45,"4",KEYIN};
BUTTON key1_5 = { 70, 70,45,45,"5",KEYIN};
BUTTON key1_6 = {120, 70,45,45,"6",KEYIN};
BUTTON key1_1 = { 20,120,45,45,"1",KEYIN};
BUTTON key1_2 = { 70,120,45,45,"2",KEYIN};
BUTTON key1_3 = {120,120,45,45,"3",KEYIN};
BUTTON key1_0 = { 20,170,45,45,"0",KEYIN};
BUTTON key1_clr = { 70,170,45,45,"CL",KEYCLR};
BUTTON key1_ok1 = {120,170,45,45,"OK",KINFIN};

BUTTON *key1_array[12] = { &key1_0, &key1_1, &key1_2, &key1_3, &key1_4, &key1_5, &key1_6, &key1_7,
			   &key1_8, &key1_9, &key1_clr, &key1_ok1};

#define INTERVAL  0
#define SETTMIN 1
#define SETTMAX 2
#define EXIT   3
BUTTON key2_0 = { 20, 20,45,45,"time",INTERVAL};
BUTTON key2_1 = { 70, 20,45,45,"Tmin",SETTMIN};
BUTTON key2_2 = {120, 20,45,45,"Tmax",KEYIN};
BUTTON key2_3 = { 20, 70,45,45,"exit",EXIT};
BUTTON *key2_array[4] = { &key2_0, &key2_1, &key2_2, &key2_3};

#define CALIBRATION_POINT 20

float calibration_x1;
float calibration_y1;
float delta_x,delta_y;
float convert_x (float);
float convert_y (float);

void drawCrossPoint(uint16_t , uint16_t ,uint16_t );
int touched2(BUTTON **, int );
void setRTC();

#define MAXLEN 256
char strbuf[MAXLEN];
boolean touch_pressed;

void draw_button(BUTTON *button, uint16_t color) {
  tft.drawRect(button->x,button->y,button->w,button->t,color);
  tft.setCursor(button->x+15, button->y+12);
  tft.setTextColor(color);
  char s[8];
  strcpy(s,button->key);
  tft.setTextSize(2);
  tft.print(s);
}

void draw_settingButton() {
  int i;
  for(i=0;i<1;i++) {
    draw_button(key0_array[i],ILI9341_YELLOW);
  }
}

void draw_keys1() {
  int i;
  for(i=0;i<12;i++) {
    draw_button(key1_array[i],ILI9341_WHITE);
  }
}

void draw_keys2() {
  int i;
  for(i=0;i<4;i++) {
    draw_button(key2_array[i],ILI9341_WHITE);
  }
}

boolean is_in_area(BUTTON *button, TS_Point point) {
  return((convert_x(point.x) > button->x) && ((convert_x(point.x) <button->x +button->w))
	 &&(convert_y(point.y) > button->y) && ((convert_y(point.y) <button->y +button->t)));
}

void drawAxis()
{
  int w=tft.width(),h=tft.height();
  xmin=30;
  xmax=w;
  ymin=h-20;
  ymax=50;
  tft.drawFastHLine(xmin-10,ymin,xmax-xmin+10,ILI9341_RED);
  tft.drawFastHLine(xmin-10,ymax,xmax-xmin+10,ILI9341_RED);

  tft.drawFastVLine(xmin,ymax-10,ymin-ymax+20,ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(0,xmin);tft.print((int) Tmax);
  tft.setCursor(0,xmax);tft.print((int) Tmin);
}

float convert_x(float x0){
  return (CALIBRATION_POINT+(float)(x0-calibration_x1)*delta_x);
}

float convert_y(float y0){
  return (CALIBRATION_POINT+(float)(y0-calibration_y1)*delta_y);
}

void printpoint_raw(TS_Point p){
    Serial.print("Pressure = ");
    Serial.print(p.z);
    Serial.print(", x = ");
    Serial.print(p.x);
    Serial.print(", y = ");
    Serial.println(p.y);
    delay(30);
}

void printpoint_calibed(TS_Point p){
    Serial.print("Pressure = ");
    Serial.print(p.z);
    Serial.print(", x = ");
    Serial.print(convert_x(p.x));
    Serial.print(", y = ");
    Serial.print(convert_y(p.y));
    delay(30);
    Serial.println();
}

void calibration (void) {
  drawCrossPoint(CALIBRATION_POINT, CALIBRATION_POINT, ILI9341_RED);
  drawCrossPoint(tft.width()-CALIBRATION_POINT, tft.height()-CALIBRATION_POINT, ILI9341_WHITE);

  Serial.println("Calibration: touch red star:");
  
  uint8_t count = 0;
  float sum_x = 0.0;
  float sum_y = 0.0;
  while(true) {
    while(ts.touched() && count<10) {
      TS_Point point = ts.getPoint();
      printpoint_raw(point);
      sum_x += point.x;
      sum_y += point.y;
      count++;
    }
    if(count==10) break;
  }
  calibration_x1 = sum_x / 10.0;
  calibration_y1 = sum_y / 10.0;
  Serial.print("Calibration (x1,y1)=(");
  Serial.print(calibration_x1);
  Serial.print(",");
  Serial.print(calibration_y1);
  Serial.println(")");
  
  delay(1000);
  
  drawCrossPoint(CALIBRATION_POINT, CALIBRATION_POINT, ILI9341_WHITE);
  drawCrossPoint(tft.width()-CALIBRATION_POINT, tft.height()-CALIBRATION_POINT, ILI9341_RED);

  count = 0;
  sum_x = 0.0;
  sum_y = 0.0;
  while(true) {
    while(ts.touched() && count<10) {
      TS_Point point = ts.getPoint();
      printpoint_raw(point);
      sum_x += point.x;
      sum_y += point.y;
      count++;
    }
    if(count==10) break;
  }
  float calibration_x2 = sum_x / 10.0;
  float calibration_y2 = sum_y / 10.0;
  Serial.print("Calibration (x2,y2)=(");
  Serial.print(calibration_x2);
  Serial.print(",");
  Serial.print(calibration_y2);
  Serial.println(")");

  delta_x = (tft.width()-CALIBRATION_POINT*2)/ (calibration_x2-calibration_x1);
  delta_y = (tft.height()-CALIBRATION_POINT*2)/ (calibration_y2-calibration_y1);

  drawCrossPoint(CALIBRATION_POINT, CALIBRATION_POINT, ILI9341_BLACK);
  drawCrossPoint((tft.width()-CALIBRATION_POINT), (tft.height()-CALIBRATION_POINT), ILI9341_BLACK);

  char s[64];
  Serial.print("Cal(");
  Serial.print(calibration_x1);
  Serial.print(",");
  Serial.print(calibration_y1);
  Serial.println(")");
  Serial.print("Delta(");
  Serial.print(delta_x*1000.0);
  Serial.print(",");
  Serial.print(delta_y*1000.0);
  Serial.println(") at x1000");
  delay(1000);
}

void drawCrossPoint(uint16_t x, uint16_t y,uint16_t color){
  tft.drawLine(x-5,y,x+5,y,color);
  tft.drawLine(x,y-5,x,y+5,color);
  tft.drawCircle(x,y,2,color);
}

int check_key1(int val_old) {
  int status=SKIP;
  uint8_t bufptr=0;
  boolean loopstatus=true;
  char *key;
  int ikey;
  int val;

  Serial.print("Keyboard:status ");
  Serial.println(status);
  
  do {
    if((ikey=touched2(key1_array,12)) >= 0) {
      key = key1_array[ikey]->key;
      status = key1_array[ikey]->status;
    }
    switch (status) {
    case KEYIN:
      tft.setCursor(bufptr*12,0);
      tft.print(key);
      strbuf[bufptr]=*key;
      bufptr++;
      status = SKIP;
      break;
    case KEYCLR:
      tft.fillRect(0,0,bufptr*12,14,ILI9341_BLACK);
      bufptr=0;
      status=SKIP;
      break;
    case KINFIN:
      strbuf[bufptr]='\0';
      bufptr++;
      loopstatus=false;
      break;
    case SKIP:
    default:
      break;
    }
  }
  while(loopstatus);
  
  if(bufptr<=1) {
    val = val_old;
  } else {
    val = atoi(strbuf);
  }
  return val;
}

int touched2(BUTTON **key_array,int narray)
{
  TS_Point point;
  int ikey=-1;
  boolean valid=false;
  if(ts.touched()) {
    point = ts.getPoint();
    printpoint_raw(point);
    printpoint_calibed(point);
      for(uint8_t i=0;i<narray;i++) {
        if(is_in_area(key_array[i],point)) {
          valid = true;
          ikey = i;
          draw_button(key_array[ikey],ILI9341_RED);
        }
      }
  } else {
    return -1;
  }

  if(ikey<0)
    return -1;

  while(ts.touched()) {
    point = ts.getPoint();
    if(!is_in_area(key_array[ikey],point)) {
      valid = false;
      draw_button(key_array[ikey],ILI9341_WHITE);
    }
  }

  draw_button(key_array[ikey],ILI9341_WHITE);
  if(valid)
    return ikey;
  else
    return -1;
}

void setup()
{
  int i;
  
  Serial.begin(115200);
  Wire.begin();
  Serial.println();

  // start TFT
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(ILI9341_BLACK);

  // start Touchscreen
  ts.begin();
  ts.setRotation(2);

  // do calibration of Touchscreen
  calibration();
  
  drawAxis();
  draw_settingButton();
}

void loop()
{
  int status = LOOP;

  tft.setTextSize(2);

  int ikey;
  char *key;
  if((ikey = touched2(key0_array,2))>=0) {
    Serial.print("ikey=");
    Serial.println(ikey);
    key= key0_array[ikey]->key;
    status = key0_array[ikey]->status;
    Serial.println(status);
  }

  int num;
  switch (status) {
  case LOOP:
    break;
  case SETTING:
      Serial.println("Status: INPKEY");
    delay(100);
    touch_pressed = false;
    tft.fillScreen(ILI9341_BLACK);
    draw_keys1();
    num = check_key1(203);
    Serial.println(num);
    tft.fillScreen(ILI9341_BLACK);
  //  draw_keys0();
    status = LOOP;
    break;        
  default:
    break;
  }
  //  delay(50);
}
