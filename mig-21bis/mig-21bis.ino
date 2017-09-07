/*
  Tell DCS-BIOS to use a serial connection and use interrupt-driven
  communication. The main program will be interrupted to prioritize
  processing incoming data.
  
  This should work on any Arduino that has an ATMega328 controller
  (Uno, Pro Mini, many others).
 */
#define DCSBIOS_DEFAULT_SERIAL

#include "DcsBios.h"

#include <TM1638.h>
#define TM1638_COLOR_RED    2
#define TM1638_COLOR_GREEN  1

#define POS(SV,EV,SA,EA,A) (float((float(SV - EV) / (SA - EA) * (A - SA) + SV)))

TM1638 module0(9,8,7,true,1);
bool updateModule0 = true;
char module0Text[9] = "mach ias";

TM1638 module1(9,8,6,true,1);
bool updateModule1 = true;
char module1Text[9] = "baro alt";

TM1638 module2(9,8,5,true,1);
bool updateModule2 = true;
char module2Text[9] = "Course";

TM1638 module3(9,8,4,true,1);
bool updateModule3 = true;
char module3Text[9] = "engs rpm";

/*
TM1638 module4(9,8,3,true,1);
bool updateModule4 = true;
char module4Text[9];

TM1638 module5(9,8,2,true,1);
bool updateModule5 = true;
char module5Text[9];
*/

// ################# module0 #################

unsigned int iasInd = 0;

void onIasIndChange(unsigned int newValue) {
  iasInd = 2000.0 * newValue / 65535.0;
  updateModule0 = true;
}
DcsBios::IntegerBuffer iasIndBuffer(0x2266, 0xffff, 0, onIasIndChange);

unsigned int mInd = 0.0;

void onMIndChange(unsigned int newValue) {
    mInd = 300.0 * newValue / 65535.0;
  updateModule0 = true;
}
DcsBios::IntegerBuffer mIndBuffer(0x226a, 0xffff, 0, onMIndChange);


// ################# module1 #################


int baroKmAlt = 30000;

void onBaroAltKmChange(unsigned int newValue) {
  baroKmAlt = 30000.0 * newValue / 65535.0;
  updateModule2 = true;
}
DcsBios::IntegerBuffer baroAltKmBuffer(0x226e, 0xffff, 0, onBaroAltKmChange);

int baroMAlt = 1000;

void onBaroAltMChange(unsigned int newValue) {
  baroMAlt = 1000.0 * newValue / 65535.0;
  updateModule1 = true;
}
DcsBios::IntegerBuffer baroAltMBuffer(0x226c, 0xffff, 0, onBaroAltMChange);

void onRaltNdChange(unsigned int newValue) {
/*
0

10 RRRR
20 RRRR G
30 RRRR GG
40 RRRR GGG
50 RRRR GGGG

60 RRRG GGGG
70 RRGG GGGG
80 RGGG GGGG

100 GGGG GGGG
150 GGGG GGG
250 GGGG GG

300 GGGG G
400 GGGG
500 GGG
600 GG
*/
    /* your code here */
}
DcsBios::IntegerBuffer raltNdBuffer(0x2286, 0xffff, 0, onRaltNdChange);

// ################# module2 #################

int nppCrsInd = 359;

void onNppCrsIndChange(unsigned int newValue) {
  nppCrsInd = 360 - (int(360.0 * newValue / 65535.0) % 360);
}
DcsBios::IntegerBuffer nppCrsIndBuffer(0x2270, 0xffff, 0, onNppCrsIndChange);

unsigned int vviP = 0;

void onDa200VviChange(unsigned int newValue) {
  vviP = newValue;
  updateModule2 = true;
}
DcsBios::IntegerBuffer da200VviBuffer(0x228c, 0xffff, 0, onDa200VviChange);

void progressVVI(TM1638 module, unsigned int value) {
  const unsigned int MIN = 0;
  const unsigned int CENTER = 32768;
  const unsigned int MAX = 65535;
  const unsigned int MARK = 4096;

  if        (value > CENTER +  1000) {
         if (value >   MAX  -  2768) module.setLEDs(0b1111000000000000);
    else if (value > CENTER + 24000) module.setLEDs(0b1111000000010000);
    else if (value > CENTER + 18000) module.setLEDs(0b1111000000110000);
    else if (value > CENTER + 10000) module.setLEDs(0b1111000001110000);
    else if (value > CENTER +  5000) module.setLEDs(0b1111000011110000);
    else if (value > CENTER +  3000) module.setLEDs(0b0111000001110000);
    else if (value > CENTER +  2000) module.setLEDs(0b0011000000110000);
    else if (value > CENTER +  1000) module.setLEDs(0b0001000000010000);
  }
  else   if (value < CENTER -  1000) {
         if (value <   MIN  +  2768) module.setLEDs(0b0000111100000000);
    else if (value < CENTER - 24000) module.setLEDs(0b0000011100001000);
    else if (value < CENTER - 18000) module.setLEDs(0b0000001100001100);
    else if (value < CENTER - 10000) module.setLEDs(0b0000000100001110);
    else if (value < CENTER -  5000) module.setLEDs(0b0000000000001111);
    else if (value < CENTER -  3000) module.setLEDs(0b0000000000001110);
    else if (value < CENTER -  2000) module.setLEDs(0b0000000000001100);
    else if (value < CENTER -  1000) module.setLEDs(0b0000000000001000);
  }
  else                               module.setLEDs(0b0001000000011000);
}

// ################# module3 #################

unsigned int eng1Rpm = 103.0;

void onEngineRpmChange(unsigned int newValue) {
  unsigned int angle = newValue;
  #define SA 0
  #define EA 9428
  #define SV 0
  #define EV 100
  eng1Rpm = POS(SV,EV,SA,EA,angle);
  if (angle >= SA && angle < EA) eng1Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 9428
  #define EA 16529
  #define SV 100
  #define EV 200
  else if (angle >= SA && angle < EA) eng1Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 16529
  #define EA 26045
  #define SV 200
  #define EV 300
  else if (angle >= SA && angle < EA) eng1Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 26045
  #define EA 32742
  #define SV 300
  #define EV 400
  else if (angle >= SA && angle < EA) eng1Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 32742
  #define EA 37137
  #define SV 400
  #define EV 500
  else if (angle >= SA && angle < EA) eng1Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 37137
  #define EA 41292
  #define SV 500
  #define EV 600
  else if (angle >= SA && angle < EA) eng1Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 41292
  #define EA 45545
  #define SV 600
  #define EV 700
  else if (angle >= SA && angle < EA) eng1Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 45545
  #define EA 49802
  #define SV 700
  #define EV 800
  else if (angle >= SA && angle < EA) eng1Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 49802
  #define EA 53964
  #define SV 800
  #define EV 900
  else if (angle >= SA && angle < EA) eng1Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 53964
  #define EA 58951
  #define SV 900
  #define EV 1010
  else if (angle >= SA && angle < EA) eng1Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 58951
  #define EA 60777
  #define SV 1010
  #define EV 1030
  else eng1Rpm = POS(SV,EV,SA,EA,angle);
  updateModule3 = true;
}
DcsBios::IntegerBuffer engineRpmBuffer(0x22a4, 0xffff, 0, onEngineRpmChange);

unsigned eng2Rpm = 101.0;

void onEngineRpm2Change(unsigned int newValue) {
  unsigned int angle = newValue;
  #define SA 0
  #define EA 15053
  #define SV 0
  #define EV 100
  if (angle >= SA && angle < EA) eng2Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 15053
  #define EA 20794
  #define SV 100
  #define EV 200
  else if (angle >= SA && angle < EA) eng2Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 20794
  #define EA 25101
  #define SV 200
  #define EV 300
  else if (angle >= SA && angle < EA) eng2Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 25101
  #define EA 28620
  #define SV 300
  #define EV 400
  else if (angle >= SA && angle < EA) eng2Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 28620
  #define EA 31710
  #define SV 400
  #define EV 500
  else if (angle >= SA && angle < EA) eng2Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 31710
  #define EA 37393
  #define SV 500
  #define EV 600
  else if (angle >= SA && angle < EA) eng2Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 37393
  #define EA 43097
  #define SV 600
  #define EV 700
  else if (angle >= SA && angle < EA) eng2Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 43097
  #define EA 49158
  #define SV 700
  #define EV 800
  else if (angle >= SA && angle < EA) eng2Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 49158
  #define EA 54506
  #define SV 800
  #define EV 900
  else if (angle >= SA && angle < EA) eng2Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 54506
  #define EA 59846
  #define SV 900
  #define EV 990
  else if (angle >= SA && angle < EA) eng2Rpm = POS(SV,EV,SA,EA,angle);
  #define SA 59846
  #define EA 61107
  #define SV 990
  #define EV 1020
  else eng2Rpm = POS(SV,EV,SA,EA,angle);
  updateModule3 = true;
}
DcsBios::IntegerBuffer engineRpm2Buffer(0x22a6, 0xffff, 0, onEngineRpm2Change);


// ============================ OLED ===================================

#include <U8glib.h>

// 6 lines
#define FONT u8g_font_8x13 // 16x6
#define FONT u8g_font_7x13 // 18x6
#define FONT u8g_font_6x13 // 21x6
#define FONT_HEIGHT 9 // font height in pixels
#define LINE_SPACING 2 // spacing between lines
#define LINE_SPACING 4 // spacing between lines
/*
// 7 lines
#define FONT u8g_font_profont12 // 21x7
#define FONT u8g_font_ncenR08 // 25x7
#define FONT_HEIGHT 8 // font height in pixels
#define LINE_SPACING 1 // spacing between lines

// 8 lines
#define FONT u8g_font_profont11 // 21x7
//#define FONT u8g_font_ncenR08 // 25x7
#define FONT_HEIGHT 7 // font height in pixels
#define LINE_SPACING 1 // spacing between lines
*/
/*
// 9 lines
#define FONT u8g_font_profont10 // 21x7
#define FONT_HEIGHT 6 // font height in pixels
#define LINE_SPACING 1 // spacing between lines
*/
/*
// 10 lines
#define FONT u8g_font_baby // 21x7
#define FONT_HEIGHT 5 // font height in pixels
#define LINE_SPACING 1 // spacing between lines
*/
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

String line0 = "12345678901234567890123456";
String line1 = "234567890123456";
String line2 = "34567890123456";
String line3 = "4567890123456";
String line4 = "567890123456";
String line5 = "67890123456";
String line6 = "7890123456";
String line7 = "890123456";
String line8 = "90123456";
String line9 = "0123456";
String line10 = "123456";

/*
line0 = "START    <80l        ";
line1 = "FORSAZ E FORSAZ VFUEL";
line2 = "DC GEN AC GEN   1FUEL";
line3 = "POZAR  NOZZLE   <450l";
line4 = "B HYD  M HYD    3FUEL";
*/

String uhfFreq = "225.00";
String vhfComm = "126.675";
String vhfNav = "107.00";
bool redraw = true;

void onUhfFreqChange(char* newValue) {
  uhfFreq = newValue;
  line0 = "UHF: " + uhfFreq;
  redraw = true;
}
DcsBios::StringBuffer<6> uhfFreqBuffer(0x14e2, onUhfFreqChange);

void onVhfcommFreqChange(char* newValue) {
  vhfComm = newValue;
  line1 = "VHF: " + vhfComm;
  redraw = true;
}
DcsBios::StringBuffer<7> vhfcommFreqBuffer(0x14d4, onVhfcommFreqChange);

void onVhfnavFreqChange(char* newValue) {
  vhfNav = newValue;
  line3 = "NAV: " + vhfNav;
  redraw = true;
}
DcsBios::StringBuffer<6> vhfnavFreqBuffer(0x14f0, onVhfnavFreqChange);

String fm1 = "3";
String fm2 = "0";
String fm3 = "0";
String fm4 = "0";
String fmComm = "30.00";

void onVhffmFreq1Change(char* newValue) {
  fm1 = newValue;
  fmComm = fm1 + fm2 + '.' + fm3 + fm4;
  line2 = "FM:   " + fmComm;
  redraw = true;
}
DcsBios::StringBuffer<1> vhffmFreq1StrBuffer(0x14ea, onVhffmFreq1Change);

void onVhffmFreq2Change(unsigned int newValue) {
  fm2 = String(newValue);
  fmComm = fm1 + fm2 + '.' + fm3 + fm4;
  line2 = "FM:   " + fmComm;
  redraw = true;
}
DcsBios::IntegerBuffer vhffmFreq2Buffer(0x14de, 0x1e00, 9, onVhffmFreq2Change);

void onVhffmFreq3Change(unsigned int newValue) {
  fm3 = String(newValue);
  fmComm = fm1 + fm2 + '.' + fm3 + fm4;
  line2 = "FM:   " + fmComm;
  redraw = true;
}
DcsBios::IntegerBuffer vhffmFreq3Buffer(0x14ea, 0x0f00, 8, onVhffmFreq3Change);

void onVhffmFreq4Change(char* newValue) {
  fm4 = newValue;
  fmComm = fm1 + fm2 + '.' + fm3 + fm4;
  line2 = "FM:   " + fmComm;
  redraw = true;
}
DcsBios::StringBuffer<1> vhffmFreq4StrBuffer(0x14ec, onVhffmFreq4Change);

#define B190 0 // 190 200 220
#define B400 1 // 400 420 450 500
#define B850 2 // 850 900 1000

String adfNav = "450.0";
String adfSig = "-1";
int adfBand = 0;
unsigned int adfFreq = 0;

#define ADF_FREQ(SF,EF,SV,EV,V) (String(int(float(SF - EF) / (SV - EV) * (V - SV) + SF)))

String getAdfFreq(int band, unsigned int val) {
  switch(band) {
    case B190:
      #define SV 0
      #define EV 6200
      #define SF 190
      #define EF 200
      if (val >= SV && val < EV) return ADF_FREQ(SF,EF,SV,EV,val);
      #define SV 6200
      #define EV 9320
      #define SF 200
      #define EF 210
      else if (val >= SV && val < EV) return ADF_FREQ(SF,EF,SV,EV,val);
      #define SV 9320
      #define EV 48300
      #define SF 210
      #define EF 342
      else if (val >= SV && val < EV) return ADF_FREQ(SF,EF,SV,EV,val);
      #define SV 48300
      #define EV 65535
      #define SF 342
      #define EF 400
      else return ADF_FREQ(SF,EF,SV,EV,adfFreq);
      break;
    case B400:
      #define SV 0
      #define EV 6681
      #define SF 400
      #define EF 420
      if (val >= SV && val < EV) return ADF_FREQ(SF,EF,SV,EV,val);
      #define SV 6681
      #define EV 13569
      #define SF 420
      #define EF 450
      else if (val >= SV && val < EV) return ADF_FREQ(SF,EF,SV,EV,val);
      #define SV 13569
      #define EV 16809
      #define SF 450
      #define EF 477
      else if (val >= SV && val < EV) return ADF_FREQ(SF,EF,SV,EV,val);
      #define SV 16809
      #define EV 65535
      #define SF 477
      #define EF 850
      else return ADF_FREQ(SF,EF,SV,EV,adfFreq);
      break;
    case B850:
      #define SV 0
      #define EV 6681
      #define SF 850
      #define EF 900
      if (val >= SV && val < EV) return ADF_FREQ(SF,EF,SV,EV,val);
      #define SV 6681
      #define EV 12858
      #define SF 900
      #define EF 986
      else if (val >= SV && val < EV) return ADF_FREQ(SF,EF,SV,EV,val);
      #define SV 12858
      #define EV 65535
      #define SF 986
      #define EF 1800
      else return ADF_FREQ(SF,EF,SV,EV,adfFreq);
      break;
  }
  return String('?');
}

void onAdfBandChange(unsigned int newValue) {
  adfBand = newValue;
  adfNav = getAdfFreq(adfBand,adfFreq);
  line4 = "NAV: " + adfNav + " " + adfSig;
  redraw = true;
}
DcsBios::IntegerBuffer adfBandBuffer(0x14fc, 0x0003, 0, onAdfBandChange);

void onAdfFreqChange(unsigned int newValue) {
  adfFreq = newValue;
  adfNav = getAdfFreq(adfBand,adfFreq);
  line4 = "NAV: " + adfNav + " " + adfSig;
  redraw = true;
}
DcsBios::IntegerBuffer adfFreqBuffer(0x1426, 0xffff, 0, onAdfFreqChange);

void onAdfSignalChange(unsigned int newValue) {
  adfSig = String((float)newValue / 65535.0);
  line4 = "NAV: " + adfNav + " " + adfSig;
  redraw = true;
}
DcsBios::IntegerBuffer adfSignalBuffer(0x1428, 0xffff, 0, onAdfSignalChange);

/* paste code snippets from the reference documentation here */
void setup() {
  DcsBios::setup();
  if (updateModule0) {
    module0.setDisplayToString(module0Text);
    updateModule0 = false;
  }
  if (updateModule1) {
    module1.setDisplayToString(module1Text);
    updateModule1 = false;
  }
  if (updateModule2) {
    module2.setDisplayToString(module2Text);
    updateModule2 = false;
  }
  if (updateModule3) {
    module3.setDisplayToString(module3Text);
    updateModule3 = false;
  }
}

void loop() {
  DcsBios::loop();

  if (updateModule0) {
    snprintf(module0Text, sizeof(module0Text), "%4d%4d", mInd, iasInd);
    module0.setDisplayToString(module0Text, 64);
    updateModule0 = false;
  }
  if (updateModule1) {
    snprintf(module1Text, sizeof(module1Text), "%8d", baroKmAlt);
    module1.setDisplayToString(module1Text);
    updateModule1 = false;
  }
  if (updateModule2) {
    snprintf(module2Text, sizeof(module2Text), "%8d", nppCrsInd);
    module2.setDisplayToString(module2Text);
    progressVVI(module2,vviP);
    updateModule2 = false;
  }
  if (updateModule3) {
    snprintf(module3Text, sizeof(module3Text), "%4d%4d", eng1Rpm, eng2Rpm);
    module3.setDisplayToString(module3Text, 2+32);
    updateModule3 = false;
  }
}
