/* * * * * * * * * * * * * * * * *
*  Author: Hugs Not Bugs         *
*  Description: Lab 03 firmware  *
* * * * * * * * * * * * * * * * */

#include <EEPROM.h> //for read/write to EEPROM
#include <Adafruit_GFX.h>    // Core graphics library from adafruit
#include <SPI.h>       // this is needed for display communication
#include <Adafruit_ILI9341.h>  //display library from adafruit
#include <Wire.h>      // this is needed for cap touch communication
#include <Adafruit_FT6206.h>  //cap touch library from adafruit
#include <RTClib.h>  //real time clock library from adafruit
#include "./lab03constants.h"
#include "./lab03pointevents.h"

//DIGITAL PINS
#define TFT_CS 10  //for display SPI
#define TFT_DC 9  //for display SPI
#define BACKLIGHT 8  //for display backlight
#define RED_LED 3
#define BLUE_LED 4
//ANALOG PINS
#define TEMP_SENSOR A0

//struct for setpoints; contains addresses for EEPROM values
typedef struct {
  unsigned int m_enabled;
  unsigned int m_hour;
  unsigned int m_minute;
  unsigned int m_temp;
} Setpoint;

//Screens enum for checking where we are in the program
enum screen {
  HOME,
  CONFIG,
  SET_TIME,
  SETPOINT_MENU,
  SET_SETPOINT
};

//global state variables
const int POWER_SAVE_INTERVAL_SECONDS = 60;
const int HVAC_MIN = 13;
const int HVAC_MAX = 32;
const String DAYS[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int t_counter;
int t_span;
int currentMinute;
bool powerSaveMode = false;
Adafruit_FT6206 ctp = Adafruit_FT6206();
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
RTC_DS3231 rtc;
Setpoint allSetpoints[4];  //array of allSetpoints that one is viewing (Weekday OR Weekend)
Setpoint editingSetpoint;  //an instance of Setpoint type for a setpoint to be edited
screen currentScreen = HOME;

//bool for pressed buttons
bool heatIsPressed = false;
bool coolIsPressed = false;
bool autoIsPressed = false;
bool offIsPressed = true;
bool holdIsPressed = false;
//bool for indicating which set of setpoints being adjusted
bool adjustingWeekday = false;
bool adjustingWeekend = false;
//global int for current temperature in Celsius
int tempC;

void setup(void) {
  pinMode(TEMP_SENSOR,INPUT);
  pinMode(BACKLIGHT,OUTPUT);
  pinMode(RED_LED,OUTPUT);
  pinMode(BLUE_LED,OUTPUT);   
  digitalWrite(BACKLIGHT,HIGH);
  digitalWrite(RED_LED,LOW);
  digitalWrite(BLUE_LED,LOW);

  tempC = readTempC();

  tft.begin();

  if (! ctp.begin(30)) {  // pass in 'sensitivity' coefficient *DEFAULT: 40*
    while (1);
  }
  if (! rtc.begin()) {    
    while (1);
  }
  if (rtc.lostPower()) {
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
    
  tft.setRotation(3);
  
  t_counter = rtc.now().second();
  t_span = t_counter;
  currentMinute = rtc.now().minute();
  renderHome();
}

void loop() {
  checkTime();
  DateTime t_now = rtc.now();
  t_counter = t_now.second();
  
  if(t_counter - t_span > POWER_SAVE_INTERVAL_SECONDS) {    
    powerSaveMode = true;
    digitalWrite(BACKLIGHT,LOW);
  }
  // Wait for a touch
  if (! ctp.touched()) {
    return;
  }
    
  t_span = t_now.second();
  
  if(powerSaveMode) {
    powerSaveMode = false;
    digitalWrite(BACKLIGHT,HIGH);
    currentScreen = HOME;
    adjustingWeekday = false;
    adjustingWeekend = false;
    renderHome();
    return;
  }
  // Retrieve a point
  TS_Point p = ctp.getPoint();
  // rotate points to match our screen rotation
  int ty = p.x;
  p.x = map(p.y, 0, 240, 0, 240);
  p.y = map(240-ty, 0, 320, 0, 320);
  
  switch(currentScreen) {
    case HOME:
      handleAllHomeButtons(p);
      break;
    case CONFIG:
      if(pressedBack(p.x,p.y)) {
        currentScreen = HOME;
        renderHome();
      }
      else {
        handleConfigButtons(p);
      }
      break;
    case SET_TIME:
      if(pressedBack(p.x,p.y)) {
        currentScreen = CONFIG;
        renderConfig();
      }
      else {
        setTimeHandleButtons(p);
      }
      
      break;
    case SETPOINT_MENU:
      if(pressedBack(p.x,p.y)) {
        adjustingWeekday = false; adjustingWeekend = false;
        currentScreen = CONFIG;
        renderConfig();
      }
      else {
        setpointMenuHandleEnableButtons(p);
        setpointMenuHandleEditButtons(p);
      }
      break;
    case SET_SETPOINT:
      if(pressedBack(p.x,p.y)) {
        currentScreen = SETPOINT_MENU;
        renderSetpointMenu();
      }
      else {
        handleSetSetpointButtons(p);
      }
      break;
  }
}

void renderHome() {
  bool displayF = EEPROM.read(M_DISPLAY_FAHRENHEIT);
  tft.fillScreen(BACKGROUND);
  //day time
  tft.setCursor(4,4);
  tft.setTextColor(DARK_GREY);
  tft.setTextSize(1);
  tft.print(DAYS[rtc.now().dayOfTheWeek()]); tft.print(" ");
  int h = rtc.now().hour();
  int m = rtc.now().minute();
  printTimeOneLine(h,m);  
  //currentTemp
  tft.setCursor(4,20);
  tft.setTextColor(DARK_GREY);
  tft.setTextSize(2);
  tft.print("Current Temp: ");
  displayF ? tft.print((int)(tempC*1.8+32)) : tft.print(tempC);
  displayF ? tft.print(" F") : tft.print(" C");
  //config button
  tft.fillRect(CONFIG_X,CONFIG_Y,CONFIG_WIDTH,CONFIG_HEIGHT,LITE_GREY);
  tft.drawRect(CONFIG_X,CONFIG_Y,CONFIG_WIDTH,CONFIG_HEIGHT,BLACK);
  tft.setCursor(CONFIG_X + 12,CONFIG_HEIGHT/3);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);  
  tft.print("Settings");
  //middle box
  tft.setCursor(120,50);
  tft.setTextSize(2);
  tft.print("Set Temp");
  tft.setCursor(125,80);
  tft.setTextSize(4);
  int currentSetTemp = EEPROM.read(M_CURRENT_SETPOINT_C);
  displayF ? tft.print((int)(currentSetTemp*1.8+32)) : tft.print(currentSetTemp);
  displayF ? tft.print(" F") : tft.print(" C");    
  //arrow buttons
  //down
  tft.fillRect(HOME_SP_DOWN_X,HOME_SP_Y,HOME_SP_WIDTH,HOME_SP_HEIGHT,LITE_GREY);
  tft.drawRect(HOME_SP_DOWN_X,HOME_SP_Y,HOME_SP_WIDTH,HOME_SP_HEIGHT,BLACK);
  tft.fillTriangle(HOME_SP_DOWN_X + 30,HOME_SP_Y + 17,HOME_SP_DOWN_X + 10,HOME_SP_Y + 7,HOME_SP_DOWN_X + 50,HOME_SP_Y + 7,BLACK);
  //up
  tft.fillRect(HOME_SP_UP_X,HOME_SP_Y,HOME_SP_WIDTH,HOME_SP_HEIGHT,LITE_GREY);
  tft.drawRect(HOME_SP_UP_X,HOME_SP_Y,HOME_SP_WIDTH,HOME_SP_HEIGHT,BLACK);
  tft.fillTriangle(HOME_SP_UP_X + 30,HOME_SP_Y + 6,HOME_SP_UP_X + 10,HOME_SP_Y + 16,HOME_SP_UP_X + 50,HOME_SP_Y + 16,BLACK);
  //hold
  tft.fillCircle(HOLD_X + 12,HOLD_Y + 12,12, holdIsPressed ? HOLD_1 : HOLD_0);
  tft.drawCircle(HOLD_X + 12,HOLD_Y + 12,12,BLACK);
  tft.setCursor(HOLD_X - 25,HOLD_Y + 30);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  tft.print("Hold Set Temp");
  //main buttons box  
  //heat
  tft.fillRect(HEAT_X,MBUTTON_Y,MBUTTON_WIDTH,MBUTTON_HEIGHT, heatIsPressed ? HEAT_1 : HEAT_0);
  tft.drawRect(HEAT_X,MBUTTON_Y,MBUTTON_WIDTH,MBUTTON_HEIGHT,BLACK);
  tft.setCursor(HEAT_X + 4,MBUTTON_Y + 10);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Heat");
  //cool
  tft.fillRect(COOL_X,MBUTTON_Y,MBUTTON_WIDTH,MBUTTON_HEIGHT, coolIsPressed ? COOL_1 : COOL_0);
  tft.drawRect(COOL_X,MBUTTON_Y,MBUTTON_WIDTH,MBUTTON_HEIGHT,BLACK);
  tft.setCursor(COOL_X + 4,MBUTTON_Y + 10);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);  
  tft.print("Cool");
  //auto
  tft.fillRect(AUTO_X,MBUTTON_Y,MBUTTON_WIDTH,MBUTTON_HEIGHT, autoIsPressed ? AUTO_1 : AUTO_0);
  tft.drawRect(AUTO_X,MBUTTON_Y,MBUTTON_WIDTH,MBUTTON_HEIGHT,BLACK);
  tft.setCursor(AUTO_X + 4,MBUTTON_Y + 10);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Auto");
  //off
  tft.fillRect(OFF_X,MBUTTON_Y,MBUTTON_WIDTH,MBUTTON_HEIGHT, offIsPressed ? GREY_1 : GREY_0);
  tft.drawRect(OFF_X,MBUTTON_Y,MBUTTON_WIDTH,MBUTTON_HEIGHT,BLACK);
  tft.setCursor(OFF_X + 10,MBUTTON_Y + 10);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Off");
}

void renderConfig() {
  tft.fillScreen(BACKGROUND);
  renderGoBack();
  tft.fillRect(CONFIG_BOX_X,CONFIG_BOX_Y1,CONFIG_BOX_WIDTH,CONFIG_BOX_HEIGHT,LITE_GREY);
  tft.drawRect(CONFIG_BOX_X,CONFIG_BOX_Y1,CONFIG_BOX_WIDTH,CONFIG_BOX_HEIGHT,BLACK);
  tft.setCursor(CONFIG_BOX_X + 36,CONFIG_BOX_Y1 + 9);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  tft.print("Adjust Date & Time");
  //fahrenheit checkbox
  tft.setCursor(80,92);
  tft.print("Display Fahrenheit");
  tft.fillRect(CHECKBOX_X,CHECKBOX_Y1,CHECKBOX_SIDE,CHECKBOX_SIDE,WHITE);
  tft.drawRect(CHECKBOX_X,CHECKBOX_Y1,CHECKBOX_SIDE,CHECKBOX_SIDE,BLACK);  
  if( EEPROM.read(M_DISPLAY_FAHRENHEIT) ) {
    //draw "X" in box
    tft.drawLine(CHECKBOX_X, CHECKBOX_Y1 + CHECKBOX_SIDE, CHECKBOX_X + CHECKBOX_SIDE, CHECKBOX_Y1, BLACK);
    tft.drawLine(CHECKBOX_X, CHECKBOX_Y1, CHECKBOX_X + CHECKBOX_SIDE, CHECKBOX_Y1 + CHECKBOX_SIDE, BLACK);
  }
  //setpoints checkbox
  tft.setCursor(80,122);
  tft.print("Enable Saved Setpoints");
  tft.fillRect(CHECKBOX_X,CHECKBOX_Y2,CHECKBOX_SIDE,CHECKBOX_SIDE,WHITE);
  tft.drawRect(CHECKBOX_X,CHECKBOX_Y2,CHECKBOX_SIDE,CHECKBOX_SIDE,BLACK);
  if( EEPROM.read(M_SETPOINTS_ENABLED) ) {
    //draw "X" in box
    tft.drawLine(CHECKBOX_X, CHECKBOX_Y2 + CHECKBOX_SIDE, CHECKBOX_X + CHECKBOX_SIDE, CHECKBOX_Y2, BLACK);
    tft.drawLine(CHECKBOX_X, CHECKBOX_Y2, CHECKBOX_X + CHECKBOX_SIDE, CHECKBOX_Y2 + CHECKBOX_SIDE, BLACK);
  }
  tft.fillRect(CONFIG_BOX_X,CONFIG_BOX_Y2,CONFIG_BOX_WIDTH,CONFIG_BOX_HEIGHT,LITE_GREY);
  tft.drawRect(CONFIG_BOX_X,CONFIG_BOX_Y2,CONFIG_BOX_WIDTH,CONFIG_BOX_HEIGHT,BLACK);  
  tft.setCursor(CONFIG_BOX_X + 18,CONFIG_BOX_Y2 + 9);
  tft.print("Adjust Weekday Setpoints");
  tft.fillRect(CONFIG_BOX_X,CONFIG_BOX_Y3,CONFIG_BOX_WIDTH,CONFIG_BOX_HEIGHT,LITE_GREY);
  tft.drawRect(CONFIG_BOX_X,CONFIG_BOX_Y3,CONFIG_BOX_WIDTH,CONFIG_BOX_HEIGHT,BLACK);
  tft.setCursor(CONFIG_BOX_X + 18,CONFIG_BOX_Y3 + 9);
  tft.print("Adjust Weekend Setpoints");
}

void renderSetTime() {
  tft.fillScreen(BACKGROUND);
  renderGoBack();
  //TEXT: TOP ROW
  tft.setCursor(DT_MM_X + 3,DT_TOP_UP_Y + 18);
  tft.setTextSize(4);
  int mo = rtc.now().month();
  mo < 10 ? tft.print("0") : tft.print("");
  tft.print(mo); tft.print("-");
  tft.setCursor(DT_DD_X + 3,DT_TOP_UP_Y + 18);
  int d = rtc.now().day();
  d < 10 ? tft.print("0") : tft.print("");
  tft.print(d);tft.print("-");
  tft.setCursor(DT_YYYY_X - 16,DT_TOP_UP_Y + 18);
  tft.print(rtc.now().year());
  //TEXT: BOTTOM ROW
  tft.setCursor(DT_HH_X - 5,DT_BOTTOM_UP_Y + 20);  
  int h = rtc.now().hour();
  h = get12HourFormat(h);
  h < 10 ? tft.print("0") : tft.print("");
  tft.print(h);
  tft.setCursor(DT_HH_X + 43,DT_BOTTOM_UP_Y + 22);
  tft.print(":");
  tft.setCursor(DT_NN_X - 5,DT_BOTTOM_UP_Y + 20);    
  int m = rtc.now().minute();
  m < 10 ? tft.print("0") : tft.print("");
  tft.print(m);
  tft.print(" "); tft.print(rtc.now().hour() < 12 ? "AM" : "PM");
  //TOP ROW UP ARROWS
  tft.fillRect(DT_MM_X,DT_TOP_UP_Y,DT_TOP_W,DT_TOP_H,LITE_GREY);
  tft.drawRect(DT_MM_X,DT_TOP_UP_Y,DT_TOP_W,DT_TOP_H,BLACK);  
  tft.fillTriangle(DT_MM_X + 25,DT_TOP_UP_Y + 2,DT_MM_X + 5,DT_TOP_UP_Y + 12,DT_MM_X + 45,DT_TOP_UP_Y + 12,BLACK);
  tft.fillRect(DT_DD_X,DT_TOP_UP_Y,DT_TOP_W,DT_TOP_H,LITE_GREY);
  tft.drawRect(DT_DD_X,DT_TOP_UP_Y,DT_TOP_W,DT_TOP_H,BLACK);
  tft.fillTriangle(DT_DD_X + 25,DT_TOP_UP_Y + 2,DT_DD_X + 5,DT_TOP_UP_Y + 12,DT_DD_X + 45,DT_TOP_UP_Y + 12,BLACK);
  tft.fillRect(DT_YYYY_X,DT_TOP_UP_Y,DT_TOP_W,DT_TOP_H,LITE_GREY);
  tft.drawRect(DT_YYYY_X,DT_TOP_UP_Y,DT_TOP_W,DT_TOP_H,BLACK);
  tft.fillTriangle(DT_YYYY_X + 25,DT_TOP_UP_Y + 2,DT_YYYY_X + 5,DT_TOP_UP_Y + 12,DT_YYYY_X + 45,DT_TOP_UP_Y + 12,BLACK);
  //TOP ROW DOWN ARROWS
  tft.fillRect(DT_MM_X,DT_TOP_DOWN_Y,DT_TOP_W,DT_TOP_H,LITE_GREY);
  tft.drawRect(DT_MM_X,DT_TOP_DOWN_Y,DT_TOP_W,DT_TOP_H,BLACK);
  tft.fillTriangle(DT_MM_X + 25,DT_TOP_DOWN_Y + 11,DT_MM_X + 5,DT_TOP_DOWN_Y + 3,DT_MM_X + 45,DT_TOP_DOWN_Y + 3,BLACK);
  tft.fillRect(DT_DD_X,DT_TOP_DOWN_Y,DT_TOP_W,DT_TOP_H,LITE_GREY);
  tft.drawRect(DT_DD_X,DT_TOP_DOWN_Y,DT_TOP_W,DT_TOP_H,BLACK);
  tft.fillTriangle(DT_DD_X + 25,DT_TOP_DOWN_Y + 11,DT_DD_X + 5,DT_TOP_DOWN_Y + 3,DT_DD_X + 45,DT_TOP_DOWN_Y + 3,BLACK);
  tft.fillRect(DT_YYYY_X,DT_TOP_DOWN_Y,DT_TOP_W,DT_TOP_H,LITE_GREY);
  tft.drawRect(DT_YYYY_X,DT_TOP_DOWN_Y,DT_TOP_W,DT_TOP_H,BLACK);
  tft.fillTriangle(DT_YYYY_X + 25,DT_TOP_DOWN_Y + 11,DT_YYYY_X + 5,DT_TOP_DOWN_Y + 3,DT_YYYY_X + 45,DT_TOP_DOWN_Y + 3,BLACK);
  //BOTTOM ROW UP ARROWS
  tft.fillRect(DT_HH_X,DT_BOTTOM_UP_Y,DT_BOTTOM_W,DT_BOTTOM_H,LITE_GREY);
  tft.drawRect(DT_HH_X,DT_BOTTOM_UP_Y,DT_BOTTOM_W,DT_BOTTOM_H,BLACK);
  tft.fillTriangle(DT_HH_X + 17,DT_BOTTOM_UP_Y + 2,DT_HH_X + 5,DT_BOTTOM_UP_Y + 12,DT_HH_X + 29,DT_BOTTOM_UP_Y + 12,BLACK);
  tft.fillRect(DT_NN_X,DT_BOTTOM_UP_Y,DT_BOTTOM_W,DT_BOTTOM_H,LITE_GREY);
  tft.drawRect(DT_NN_X,DT_BOTTOM_UP_Y,DT_BOTTOM_W,DT_BOTTOM_H,BLACK);
  tft.fillTriangle(DT_NN_X + 17,DT_BOTTOM_UP_Y + 2,DT_NN_X + 5,DT_BOTTOM_UP_Y + 12,DT_NN_X + 29,DT_BOTTOM_UP_Y + 12,BLACK);
  //BOTTOM ROW DOWN ARROWS  
  tft.fillRect(DT_HH_X,DT_BOTTOM_DOWN_Y,DT_BOTTOM_W,DT_BOTTOM_H,LITE_GREY);
  tft.drawRect(DT_HH_X,DT_BOTTOM_DOWN_Y,DT_BOTTOM_W,DT_BOTTOM_H,BLACK);  
  tft.fillTriangle(DT_HH_X + 17,DT_BOTTOM_DOWN_Y + 12,DT_HH_X + 5,DT_BOTTOM_DOWN_Y + 4,DT_HH_X + 29,DT_BOTTOM_DOWN_Y + 4,BLACK);
  tft.fillRect(DT_NN_X,DT_BOTTOM_DOWN_Y,DT_BOTTOM_W,DT_BOTTOM_H,LITE_GREY);
  tft.drawRect(DT_NN_X,DT_BOTTOM_DOWN_Y,DT_BOTTOM_W,DT_BOTTOM_H,BLACK);
  tft.fillTriangle(DT_NN_X + 17,DT_BOTTOM_DOWN_Y + 12,DT_NN_X + 5,DT_BOTTOM_DOWN_Y + 4,DT_NN_X + 29,DT_BOTTOM_DOWN_Y + 4,BLACK);
}

void renderSetpointMenu() {  
  bool displayF = EEPROM.read(M_DISPLAY_FAHRENHEIT);
  int displayTemp;
  if(adjustingWeekday) {
    getWeekdaySetpointsFromMemory(allSetpoints);
  }

  if(adjustingWeekend) {
    getWeekendSetpointsFromMemory(allSetpoints);
  }
  
  tft.fillScreen(BACKGROUND);  
  renderGoBack();
  tft.setCursor(125,25);
  tft.setTextSize(2);
  tft.print(adjustingWeekday
    ? "Weekdays" 
    : adjustingWeekend
      ? "Weekends" 
      : "Error"
  );
  Setpoint sp1 = allSetpoints[0];
  Setpoint sp2 = allSetpoints[1];
  Setpoint sp3 = allSetpoints[2];
  Setpoint sp4 = allSetpoints[3];
  //sp1
  tft.fillCircle(SP_EN_X,SP_EN_Y1,SP_EN_RADIUS,WHITE);
  tft.fillCircle(SP_EN_X,SP_EN_Y1,SP_EN_RADIUS - 4,EEPROM.read(sp1.m_enabled) ? BLACK : WHITE);
  tft.drawCircle(SP_EN_X,SP_EN_Y1,SP_EN_RADIUS,BLACK);
  tft.setCursor(70,74);
  printTimeOneLine(EEPROM.read(sp1.m_hour),EEPROM.read(sp1.m_minute));
  tft.print(" ");
  displayTemp = EEPROM.read(sp1.m_temp);
  displayF ? tft.print((int)(displayTemp*1.8+32)) : tft.print(displayTemp);
  displayF ? tft.print("F") : tft.print("C");      
  tft.fillRect(EDIT_BUTTON_X,EDIT_BUTTON_Y1,EDIT_BUTTON_W,EDIT_BUTTON_H,WHITE);
  tft.drawRect(EDIT_BUTTON_X,EDIT_BUTTON_Y1,EDIT_BUTTON_W,EDIT_BUTTON_H,BLACK);
  tft.setCursor(EDIT_BUTTON_X + 7,73);
  tft.print("EDIT");
  //sp2
  tft.fillCircle(SP_EN_X,SP_EN_Y2,SP_EN_RADIUS,WHITE);
  tft.fillCircle(SP_EN_X,SP_EN_Y2,SP_EN_RADIUS - 4,EEPROM.read(sp2.m_enabled) ? BLACK : WHITE);
  tft.drawCircle(SP_EN_X,SP_EN_Y2,SP_EN_RADIUS,BLACK);
  tft.setCursor(70,114);
  printTimeOneLine(EEPROM.read(sp2.m_hour),EEPROM.read(sp2.m_minute));
  tft.print(" "); 
  displayTemp = EEPROM.read(sp2.m_temp);
  displayF ? tft.print((int)(displayTemp*1.8+32)) : tft.print(displayTemp);
  displayF ? tft.print("F") : tft.print("C");      
  tft.fillRect(EDIT_BUTTON_X,EDIT_BUTTON_Y2,EDIT_BUTTON_W,EDIT_BUTTON_H,WHITE);
  tft.drawRect(EDIT_BUTTON_X,EDIT_BUTTON_Y2,EDIT_BUTTON_W,EDIT_BUTTON_H,BLACK);
  tft.setCursor(EDIT_BUTTON_X + 7,113);
  tft.print("EDIT");
  //sp3
  tft.fillCircle(SP_EN_X,SP_EN_Y3,SP_EN_RADIUS,WHITE);
  tft.fillCircle(SP_EN_X,SP_EN_Y3,SP_EN_RADIUS - 4,EEPROM.read(sp3.m_enabled) ? BLACK : WHITE);
  tft.drawCircle(SP_EN_X,SP_EN_Y3,SP_EN_RADIUS,BLACK);
  tft.setCursor(70,154);
  printTimeOneLine(EEPROM.read(sp3.m_hour),EEPROM.read(sp3.m_minute));
  tft.print(" ");
  displayTemp = EEPROM.read(sp3.m_temp);
  displayF ? tft.print((int)(displayTemp*1.8+32)) : tft.print(displayTemp);
  displayF ? tft.print("F") : tft.print("C");      
  tft.fillRect(EDIT_BUTTON_X,EDIT_BUTTON_Y3,EDIT_BUTTON_W,EDIT_BUTTON_H,WHITE);
  tft.drawRect(EDIT_BUTTON_X,EDIT_BUTTON_Y3,EDIT_BUTTON_W,EDIT_BUTTON_H,BLACK);
  tft.setCursor(EDIT_BUTTON_X + 7,153);
  tft.print("EDIT");
  //sp4
  tft.fillCircle(SP_EN_X,SP_EN_Y4,SP_EN_RADIUS,WHITE);
  tft.fillCircle(SP_EN_X,SP_EN_Y4,SP_EN_RADIUS - 4,EEPROM.read(sp4.m_enabled) ? BLACK : WHITE);
  tft.drawCircle(SP_EN_X,SP_EN_Y4,SP_EN_RADIUS,BLACK);
  tft.setCursor(70,194);
  printTimeOneLine(EEPROM.read(sp4.m_hour),EEPROM.read(sp4.m_minute));
  tft.print(" ");
  displayTemp = EEPROM.read(sp4.m_temp);
  displayF ? tft.print((int)(displayTemp*1.8+32)) : tft.print(displayTemp);
  displayF ? tft.print("F") : tft.print("C");      
  tft.fillRect(EDIT_BUTTON_X,EDIT_BUTTON_Y4,EDIT_BUTTON_W,EDIT_BUTTON_H,WHITE);
  tft.drawRect(EDIT_BUTTON_X,EDIT_BUTTON_Y4,EDIT_BUTTON_W,EDIT_BUTTON_H,BLACK);
  tft.setCursor(EDIT_BUTTON_X + 7,193);
  tft.print("EDIT");
}

void renderSetSetpoint() {
  bool displayF = EEPROM.read(M_DISPLAY_FAHRENHEIT);
  tft.fillScreen(BACKGROUND);
  renderGoBack();
  int h = EEPROM.read(editingSetpoint.m_hour);
  int m = EEPROM.read(editingSetpoint.m_minute);
  int tmp = EEPROM.read(editingSetpoint.m_temp);
  tft.setCursor(80,100);
  tft.setTextSize(4);
  //time settings
  String z = h >= 12 ? "PM" : "AM";
  h = h == 0 
    ? 12
    : h > 12
      ? h - 12
      : h;
  tft.setCursor(SSP_HH_X - 5,SSP_TIME_UP_Y + SSP_TIME_H + 5);
  h < 10 ? tft.print("0") : tft.print("");
  tft.print(h);
  tft.setCursor(SSP_HH_X + SSP_TIME_W + 8,SSP_TIME_UP_Y + SSP_TIME_H + 8);
  tft.print(":");
  tft.setCursor(SSP_MM_X - 5,SSP_TIME_UP_Y + SSP_TIME_H + 5);
  m < 10 ? tft.print("0") : tft.print("");
  tft.print(m);
  tft.print(" " + z);
  tft.fillRect(SSP_HH_X,SSP_TIME_UP_Y,SSP_TIME_W,SSP_TIME_H,LITE_GREY);
  tft.drawRect(SSP_HH_X,SSP_TIME_UP_Y,SSP_TIME_W,SSP_TIME_H,BLACK);
  tft.fillTriangle(SSP_HH_X + 17,SSP_TIME_UP_Y + 2,SSP_HH_X + 5,SSP_TIME_UP_Y + 12,SSP_HH_X + 30,SSP_TIME_UP_Y + 12,BLACK);
  tft.fillRect(SSP_HH_X,SSP_TIME_DOWN_Y,SSP_TIME_W,SSP_TIME_H,LITE_GREY);
  tft.drawRect(SSP_HH_X,SSP_TIME_DOWN_Y,SSP_TIME_W,SSP_TIME_H,BLACK);
  tft.fillTriangle(SSP_HH_X + 17,SSP_TIME_DOWN_Y + 11,SSP_HH_X + 5,SSP_TIME_DOWN_Y + 3,SSP_HH_X + 30,SSP_TIME_DOWN_Y + 3,BLACK);
  tft.fillRect(SSP_MM_X,SSP_TIME_UP_Y,SSP_TIME_W,SSP_TIME_H,LITE_GREY);
  tft.drawRect(SSP_MM_X,SSP_TIME_UP_Y,SSP_TIME_W,SSP_TIME_H,BLACK);
  tft.fillTriangle(SSP_MM_X + 17,SSP_TIME_UP_Y + 2,SSP_MM_X + 5,SSP_TIME_UP_Y + 12,SSP_MM_X + 30,SSP_TIME_UP_Y + 12,BLACK);
  tft.fillRect(SSP_MM_X,SSP_TIME_DOWN_Y,SSP_TIME_W,SSP_TIME_H,LITE_GREY);
  tft.drawRect(SSP_MM_X,SSP_TIME_DOWN_Y,SSP_TIME_W,SSP_TIME_H,BLACK);
  tft.fillTriangle(SSP_MM_X + 17,SSP_TIME_DOWN_Y + 11,SSP_MM_X + 5,SSP_TIME_DOWN_Y + 3,SSP_MM_X + 30,SSP_TIME_DOWN_Y + 3,BLACK);
  //temperature settings
  tft.setCursor(125,148);
  tft.setTextSize(4);  
  displayF ? tft.print((int)(tmp*1.8+32)) : tft.print(tmp);
  displayF ? tft.print(" F") : tft.print(" C");        
  tft.fillRect(SSP_TEMP_DOWN_X,SSP_TEMP_Y,SSP_TEMP_W,SSP_TEMP_H,LITE_GREY);
  tft.drawRect(SSP_TEMP_DOWN_X,SSP_TEMP_Y,SSP_TEMP_W,SSP_TEMP_H,BLACK);
  tft.fillTriangle(SSP_TEMP_DOWN_X + 30,SSP_TEMP_Y + 14,SSP_TEMP_DOWN_X + 5,SSP_TEMP_Y + 4,SSP_TEMP_DOWN_X + 55,SSP_TEMP_Y + 4,BLACK);
  tft.fillRect(SSP_TEMP_UP_X,SSP_TEMP_Y,SSP_TEMP_W,SSP_TEMP_H,LITE_GREY);
  tft.drawRect(SSP_TEMP_UP_X,SSP_TEMP_Y,SSP_TEMP_W,SSP_TEMP_H,BLACK);
  tft.fillTriangle(SSP_TEMP_UP_X + 30,SSP_TEMP_Y + 2,SSP_TEMP_UP_X + 5,SSP_TEMP_Y + 12,SSP_TEMP_UP_X + 55,SSP_TEMP_Y + 12,BLACK);
}

void renderGoBack() {
  tft.fillRect(GO_BACK_X,GO_BACK_Y,GO_BACK_WIDTH,GO_BACK_HEIGHT,LITE_GREY);
  tft.drawRect(GO_BACK_X,GO_BACK_Y,GO_BACK_WIDTH,GO_BACK_HEIGHT,BLACK);
  tft.setCursor(8,8);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Go Back");
}

void handleAllHomeButtons(TS_Point p) {
  if(pressedConfig(p.x,p.y)) {
    currentScreen = CONFIG;
    renderConfig();
  }
  if(pressedHomeSpDown(p.x,p.y)) {
    int newTemp = EEPROM.read(M_CURRENT_SETPOINT_C) - 1;
    newTemp = newTemp < HVAC_MIN ? HVAC_MIN : newTemp;
    EEPROM.write(M_CURRENT_SETPOINT_C, newTemp);
    checkLeds();
    renderHome();
  }
  if(pressedHomeSpUp(p.x,p.y)) {
    int newTemp = EEPROM.read(M_CURRENT_SETPOINT_C) + 1;
    newTemp = newTemp > HVAC_MAX ? HVAC_MAX : newTemp;
    EEPROM.write(M_CURRENT_SETPOINT_C, newTemp);
    checkLeds();
    renderHome();
  }
  if(pressedHeat(p.x,p.y)) {
    heatIsPressed = true; coolIsPressed = false; autoIsPressed = false; offIsPressed = false;
    checkLeds();
    renderHome();    
  }
  if(pressedCool(p.x,p.y)) {
    heatIsPressed = false; coolIsPressed = true; autoIsPressed = false; offIsPressed = false;
    checkLeds();
    renderHome();    
  }
  if(pressedAuto(p.x,p.y)) {
    heatIsPressed = false; coolIsPressed = false; autoIsPressed = true; offIsPressed = false;
    checkLeds();
    renderHome();    
  }
  if(pressedOff(p.x,p.y)) {        
    heatIsPressed = false; coolIsPressed = false; autoIsPressed = false; offIsPressed = true;
    checkLeds();
    renderHome();    
  }
  if(pressedHold(p.x,p.y)) {
    holdIsPressed = !holdIsPressed;
    renderHome();
  }
}

void handleConfigButtons(TS_Point p) {
  if(pressedAdjustDateTime(p.x,p.y)) {
    currentScreen = SET_TIME;
    renderSetTime();
  }
  if(pressedEnableAllSetpoints(p.x,p.y)) {
    EEPROM.write(M_SETPOINTS_ENABLED, !EEPROM.read(M_SETPOINTS_ENABLED));
    renderConfig();
  }
  if(pressedDisplayFahrenheit(p.x,p.y)) {
    EEPROM.write(M_DISPLAY_FAHRENHEIT, !EEPROM.read(M_DISPLAY_FAHRENHEIT));
    renderConfig();
  }
  if(pressedAdjustWeekdaySP(p.x,p.y)) {
    adjustingWeekday = true; adjustingWeekend = false;
    currentScreen = SETPOINT_MENU;
    renderSetpointMenu();
  }
  if(pressedAdjustWeekendSP(p.x,p.y)) {
    adjustingWeekday = false; adjustingWeekend = true;
    currentScreen = SETPOINT_MENU;
    renderSetpointMenu();
  }
}

void setTimeHandleButtons(TS_Point p) {  
  if(pressedMonthUp(p.x,p.y)) {
    DateTime t = rtc.now();
    int m = t.month();
    m += 1;
    m = m > 12 ? 1 : m;
    rtc.adjust(DateTime(t.year(),m,t.day(),t.hour(),t.minute(),0));    
    renderSetTime();
  }
  if(pressedMonthDown(p.x,p.y)) {
    DateTime t = rtc.now();
    int m = t.month();
    m -= 1;
    m = m < 1 ? 12 : m;
    rtc.adjust(DateTime(t.year(),m,t.day(),t.hour(),t.minute(),0));    
    renderSetTime();
  }
  if(pressedDayUp(p.x,p.y)) {
    DateTime t = rtc.now();
    int threshold = 31;
    int m = t.month();
    int d = t.day();
    int y = t.year();

    switch(m) {
      case 2: threshold = (y % 4) ? 29 : 28; break;
      case 4: threshold = 30; break;
      case 6: threshold = 30; break;
      case 9: threshold = 30; break;
      case 11: threshold = 30; break;
    }

    d += 1;
    d = d > threshold ? 1 : d;
    
    rtc.adjust(DateTime(y,m,d,t.hour(),t.minute(),0));    
    renderSetTime();
  }
  if(pressedDayDown(p.x,p.y)) {
    DateTime t = rtc.now();
    int threshold = 31;
    int m = t.month();
    int d = t.day();
    int y = t.year();

    switch(m) {
      case 2: threshold = (y % 4) ? 29 : 28; break;
      case 4: threshold = 30; break;
      case 6: threshold = 30; break;
      case 9: threshold = 30; break;
      case 11: threshold = 30; break;
    }

    d -= 1;
    d = d < 1 ? threshold : d;
    
    rtc.adjust(DateTime(y,m,d,t.hour(),t.minute(),0));
    renderSetTime();
  }
  if(pressedYearUp(p.x,p.y)) {
    DateTime t = rtc.now();
    int y = t.year();
    y += 1;
    rtc.adjust(DateTime(y,t.month(),t.day(),t.hour(),t.minute(),0));
    renderSetTime();
  }
  if(pressedYearDown(p.x,p.y)) {
    DateTime t = rtc.now();
    int y = t.year();
    y -= 1;
    rtc.adjust(DateTime(y,t.month(),t.day(),t.hour(),t.minute(),0));
    renderSetTime();
  }  
  if(pressedHourUp(p.x,p.y)) {
    DateTime t = rtc.now();
    int h = t.hour();
    h += 1;
    h = h > 23 ? 0 : h;
    rtc.adjust(DateTime(t.year(),t.month(),t.day(),h,t.minute(),0));
    renderSetTime();
  }
  if(pressedHourDown(p.x,p.y)) {
    DateTime t = rtc.now();
    int h = t.hour();
    h -= 1;
    h = h < 0 ? 23 : h;
    rtc.adjust(DateTime(t.year(),t.month(),t.day(),h,t.minute(),0));
    renderSetTime();
  }
  if(pressedMinuteUp(p.x,p.y)) {
    DateTime t = rtc.now();
    int m = t.minute();
    m += 1;
    m = m > 60 ? 0 : m;
    rtc.adjust(DateTime(t.year(),t.month(),t.day(),t.hour(),m,0));
    renderSetTime();
  }
  if(pressedMinuteDown(p.x,p.y)) {
    DateTime t = rtc.now();
    int m = t.minute();
    m -= 1;
    m = m < 60 ? m : 59;
    rtc.adjust(DateTime(t.year(),t.month(),t.day(),t.hour(),m,0));
    renderSetTime();
  }
}

void setpointMenuHandleEnableButtons(TS_Point p) {
  if(pressedEnableSetpoint1(p.x,p.y)) {
    int address = adjustingWeekday 
      ? M_WEEKDAY_SP1_ENB 
      : adjustingWeekend 
        ? M_WEEKEND_SP1_ENB 
        : NULL;  //this null will catch errors
    EEPROM.write(address, !(EEPROM.read(address)));
    renderSetpointMenu();
  }
  if(pressedEnableSetpoint2(p.x,p.y)) {
    int address = adjustingWeekday 
      ? M_WEEKDAY_SP2_ENB 
      : adjustingWeekend 
        ? M_WEEKEND_SP2_ENB 
        : NULL;  //this null will catch errors
    EEPROM.write(address, !(EEPROM.read(address)));
    renderSetpointMenu();
  }
  if(pressedEnableSetpoint3(p.x,p.y)) {    
    int address = adjustingWeekday 
      ? M_WEEKDAY_SP3_ENB 
      : adjustingWeekend 
        ? M_WEEKEND_SP3_ENB 
        : NULL;  //this null will catch errors
    EEPROM.write(address, !(EEPROM.read(address)));
    renderSetpointMenu();
  }
  if(pressedEnableSetpoint4(p.x,p.y)) {    
    int address = adjustingWeekday
      ? M_WEEKDAY_SP4_ENB 
      : adjustingWeekend 
        ? M_WEEKEND_SP4_ENB 
        : NULL;  //this null will catch errors
    EEPROM.write(address, !(EEPROM.read(address)));
    renderSetpointMenu();
  }
}

void setpointMenuHandleEditButtons(TS_Point p) {
  if(pressedEditSetpoint1(p.x,p.y)) {
    editingSetpoint = allSetpoints[0];
    currentScreen = SET_SETPOINT;
    renderSetSetpoint();
  }
  if(pressedEditSetpoint2(p.x,p.y)) {
    editingSetpoint = allSetpoints[1];
    currentScreen = SET_SETPOINT;
    renderSetSetpoint();
  }
  if(pressedEditSetpoint3(p.x,p.y)) {
    editingSetpoint = allSetpoints[2];
    currentScreen = SET_SETPOINT;
    renderSetSetpoint();
  }
  if(pressedEditSetpoint4(p.x,p.y)) {
    editingSetpoint = allSetpoints[3];
    currentScreen = SET_SETPOINT;
    renderSetSetpoint();
  }
}

void handleSetSetpointButtons(TS_Point p) {
  if(pressedSetSetpointHourUp(p.x,p.y)) {
    int newHour = EEPROM.read(editingSetpoint.m_hour) + 1;
    newHour = newHour > 23 ? 0 : newHour;
    EEPROM.write(editingSetpoint.m_hour, newHour);
    renderSetSetpoint();
  }
  if(pressedSetSetpointHourDown(p.x,p.y)) {
    int newHour = EEPROM.read(editingSetpoint.m_hour) - 1;
    newHour = newHour < 0 ? 23 : newHour;
    EEPROM.write(editingSetpoint.m_hour, newHour);
    renderSetSetpoint();
  }
  if(pressedSetSetpointMinuteUp(p.x,p.y)) {
    int newMinute = EEPROM.read(editingSetpoint.m_minute) + 1;
    newMinute = newMinute > 59 ? 0 : newMinute;
    EEPROM.write(editingSetpoint.m_minute, newMinute);
    renderSetSetpoint();
  }
  if(pressedSetSetpointMinuteDown(p.x,p.y)) {
    int newMinute = EEPROM.read(editingSetpoint.m_minute) - 1;
    newMinute = newMinute < 0 ? 59 : newMinute;
    EEPROM.write(editingSetpoint.m_minute, newMinute);
    renderSetSetpoint();
  }
  if(pressedSetSetpointTempUp(p.x,p.y)) {
    int newTemp = EEPROM.read(editingSetpoint.m_temp) + 1;
    newTemp = newTemp > HVAC_MAX ? HVAC_MAX : newTemp;
    EEPROM.write(editingSetpoint.m_temp, newTemp);
    renderSetSetpoint();
  }
  if(pressedSetSetpointTempDown(p.x,p.y)) {
    int newTemp = EEPROM.read(editingSetpoint.m_temp) - 1;
    newTemp = newTemp < HVAC_MIN ? HVAC_MIN : newTemp;
    EEPROM.write(editingSetpoint.m_temp, newTemp);
    renderSetSetpoint();
  }
}

//TODO: reimplement RTC module
void checkTime() {
  if(currentMinute != rtc.now().minute()) {
    tempC = readTempC();
    currentMinute = rtc.now().minute();
    checkSetpointsEnabled();   
    checkLeds();
    
    if(currentScreen == HOME && !powerSaveMode) {
      renderHome();
    }
  }
}

void checkLeds() {
  int setTemp = EEPROM.read(M_CURRENT_SETPOINT_C);
  if((heatIsPressed || autoIsPressed) && tempC < setTemp) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);
  }
  else if((coolIsPressed || autoIsPressed) && tempC > setTemp) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);
  }
  else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
  }
}

void getWeekdaySetpointsFromMemory(Setpoint setpointArray[]) {
  setpointArray[0] = { M_WEEKDAY_SP1_ENB,M_WEEKDAY_SP1_HRS,M_WEEKDAY_SP1_MNT,M_WEEKDAY_SP1_TMP };  
  setpointArray[1] = { M_WEEKDAY_SP2_ENB,M_WEEKDAY_SP2_HRS,M_WEEKDAY_SP2_MNT,M_WEEKDAY_SP2_TMP };  
  setpointArray[2] = { M_WEEKDAY_SP3_ENB,M_WEEKDAY_SP3_HRS,M_WEEKDAY_SP3_MNT,M_WEEKDAY_SP3_TMP };  
  setpointArray[3] = { M_WEEKDAY_SP4_ENB,M_WEEKDAY_SP4_HRS,M_WEEKDAY_SP4_MNT,M_WEEKDAY_SP4_TMP };
}

void getWeekendSetpointsFromMemory(Setpoint setpointArray[]) {
  setpointArray[0] = { M_WEEKEND_SP1_ENB,M_WEEKEND_SP1_HRS,M_WEEKEND_SP1_MNT,M_WEEKEND_SP1_TMP };  
  setpointArray[1] = { M_WEEKEND_SP2_ENB,M_WEEKEND_SP2_HRS,M_WEEKEND_SP2_MNT,M_WEEKEND_SP2_TMP };  
  setpointArray[2] = { M_WEEKEND_SP3_ENB,M_WEEKEND_SP3_HRS,M_WEEKEND_SP3_MNT,M_WEEKEND_SP3_TMP };  
  setpointArray[3] = { M_WEEKEND_SP4_ENB,M_WEEKEND_SP4_HRS,M_WEEKEND_SP4_MNT,M_WEEKEND_SP4_TMP };
}

void checkSetpointsEnabled() {
  if(!holdIsPressed && EEPROM.read(M_SETPOINTS_ENABLED)) {
    if(rtc.now().dayOfTheWeek() == 0 || rtc.now().dayOfTheWeek() == 6) {
      getWeekendSetpointsFromMemory(allSetpoints);  
      checkForSetpoints();    
    }
    else {
      getWeekdaySetpointsFromMemory(allSetpoints);
      checkForSetpoints();
    }
  }
}

void checkForSetpoints() {
  for(int i = 0; i < 4; ++i) {
    DateTime t = rtc.now();
    Setpoint currentSp = allSetpoints[i];
    if(rtc.now().hour() == EEPROM.read(currentSp.m_hour) && rtc.now().minute() == EEPROM.read(currentSp.m_minute) && !holdIsPressed) {
      EEPROM.write(M_CURRENT_SETPOINT_C, EEPROM.read(currentSp.m_temp));      
      if(currentScreen == HOME && !powerSaveMode) {
        renderHome();
      }
    }
  }
}

int get12HourFormat(int h) {
  if(h == 0) {
    return 12;
  }
  else if(h <= 12) {
    return h;
  }
  else {
    return h - 12;
  }
}

void printTimeOneLine(int h, int m) {
  String z = h < 12 ? " AM" : " PM";
  h = get12HourFormat(h);
  h < 10 ? tft.print("0") : tft.print("");
  tft.print(h);
  tft.print(":");  
  m < 10 ? tft.print("0") : tft.print("");
  tft.print(m);
  tft.print(z);
}

int readTempC(){
  int reading = analogRead(TEMP_SENSOR);
  double voltage = (reading * 5.0)/1024.0;
  return round(((voltage - 0.50) * 100));
}
