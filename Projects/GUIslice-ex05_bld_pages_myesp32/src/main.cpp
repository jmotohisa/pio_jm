//<App !Start!>
// FILE: [ex05_bld_pages.ino]
// Created by GUIslice Builder version: [0.17.b34]
//
// GUIslice Builder Generated File
//
// For the latest guides, updates and support view:
// https://github.com/ImpulseAdventure/GUIslice
//
//<App !End!>

//
// - Example 05 (Arduino):
//   - Multiple page handling, checkboxes
//   - NOTE: This is the simple version of the example without
//     optimizing for memory consumption. Therefore, it may not
//     run on Arduino devices with limited memory. 
//
// ARDUINO NOTES:
// - GUIslice_config.h must be edited to match the pinout connections
//   between the Arduino CPU and the display controller (see ADAGFX_PIN_*).
//

// Wiring
/*
ESP32 | MAX31885 peripheral
--------|---------
18      | 4 (SCK)
19      | 3 (MISO)
16      | 1 (SS)
GND     | 5 (GND)
3V3     | 6 (VCC)

ESP32 | MAX 6675
------|------
19 | MISO
16 | SS
18 | SCK
3V3 | VCC
GND | GND

ILI9341 module | ESP32
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

RTC DS1307 (via level converter) | ESP32
SCL | 22
SDA | 21
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

#define MAXCLK  TFT_CLK
#define MAXDO   TFT_MISO
#define MAXCS   16

#define SD_CS 17
#define SD_MOSI TFT_MOSI
#define SD_MISO TFT_MISO
#define SD_SCK TFT_CLK

// ------------------------------------------------
// Headers to include
// ------------------------------------------------
#include "ex05_bld_pages_GSLC.h"



// ------------------------------------------------
// Program Globals
// ------------------------------------------------
bool      m_bQuit = false;

// Free-running counter for display
unsigned  m_nCount = 0;

// Save some element references for direct access
//<Save_References !Start!>
gslc_tsElemRef* m_pElemCnt        = NULL;
gslc_tsElemRef* m_pElemProgress1  = NULL;
gslc_tsElemRef* m_pElemQuit       = NULL;
//<Save_References !End!>

// Define debug message function
static int16_t DebugOut(char ch) { if (ch == (char)'\n') Serial.println(""); else Serial.write(ch); return 0; }

// ------------------------------------------------
// Callback Methods
// ------------------------------------------------
// Common Button callback
bool CbBtnCommon(void* pvGui,void *pvElemRef,gslc_teTouch eTouch,int16_t nX,int16_t nY)
{
  gslc_tsElemRef* pElemRef = (gslc_tsElemRef*)(pvElemRef);
  gslc_tsElem* pElem = gslc_GetElemFromRef(&m_gui,pElemRef);

  if ( eTouch == GSLC_TOUCH_UP_IN ) {
    // From the element's ID we can determine which button was pressed.
    switch (pElem->nId) {
//<Button Enums !Start!>
      case E_ELEM_BTN_QUIT:
        m_bQuit = true;
        gslc_ElemSetTxtStr(&m_gui,m_pElemQuit,"DONE");
        gslc_ElemSetCol(&m_gui,m_pElemQuit,GSLC_COL_RED,GSLC_COL_BLACK,GSLC_COL_BLACK);
        break;
      case E_ELEM_BTN_EXTRA:
        gslc_SetPageCur(&m_gui,E_PG_EXTRA);
        break;
      case E_ELEM_BTN_BACK:
        gslc_SetPageCur(&m_gui,E_PG_MAIN);
        break;
//<Button Enums !End!>
        default:
        break;
    }
  }
  return true;
}
//<Checkbox Callback !Start!>
//<Checkbox Callback !End!>
//<Keypad Callback !Start!>
//<Keypad Callback !End!>
//<Spinner Callback !Start!>
//<Spinner Callback !End!>
//<Listbox Callback !Start!>
//<Listbox Callback !End!>
//<Draw Callback !Start!>
//<Draw Callback !End!>
//<Slider Callback !Start!>
//<Slider Callback !End!>
//<Tick Callback !Start!>
//<Tick Callback !End!>


void setup()
{
  // ------------------------------------------------
  // Initialize
  // ------------------------------------------------
  Serial.begin(115200);
  // Wait for USB Serial 
  //delay(1000);  // NOTE: Some devices require a delay after Serial.begin() before serial port can be used

  gslc_InitDebug(&DebugOut);


  // ------------------------------------------------
  // Load Fonts
  // ------------------------------------------------

  // ------------------------------------------------
  // Create graphic elements
  // ------------------------------------------------
  InitGUIslice_gen();



}

// -----------------------------------
// Main event loop
// -----------------------------------
void loop()
{

  char              acTxt[MAX_STR];

  // ------------------------------------------------
  // Update GUI Elements
  // ------------------------------------------------
  m_nCount++;

  // Perform drawing updates
  // - Note: we can make the updates conditional on the active
  //   page by checking gslc_GetPageCur() first.

  snprintf(acTxt,MAX_STR,"%u",m_nCount);
  gslc_ElemSetTxtStr(&m_gui,m_pElemCnt,acTxt);

  gslc_ElemXProgressSetVal(&m_gui,m_pElemProgress1,((m_nCount/2)%100));

  
  // ------------------------------------------------
  // Periodically call GUIslice update function
  // ------------------------------------------------
  gslc_Update(&m_gui);

  // Slow down updates
  delay(100);

  // In a real program, we would detect the button press and take an action.
  // For this Arduino demo, we will pretend to exit by emulating it with an
  // infinite loop. Note that interrupts are not disabled so that any debug
  // messages via Serial have an opportunity to be transmitted.
  if (m_bQuit) {
    gslc_Quit(&m_gui);
    while (1) { }
  }
  
}
