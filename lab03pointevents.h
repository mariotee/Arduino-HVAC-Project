//contains checks for button presses
#include "./lab03constants.h"

//NAVIGATION
bool pressedConfig(int px, int py) {
  return (px >= CONFIG_X && px <= CONFIG_X + CONFIG_WIDTH && py >= CONFIG_Y && py <= CONFIG_Y + CONFIG_HEIGHT);
}
bool pressedBack(int px, int py) {
  return (px >= GO_BACK_X && px <= GO_BACK_X + GO_BACK_WIDTH && py >= GO_BACK_Y && py <= GO_BACK_Y + GO_BACK_HEIGHT);
}
bool pressedAdjustDateTime(int px, int py) {
  return (px >= CONFIG_BOX_X && px <= CONFIG_BOX_X + CONFIG_BOX_WIDTH && py >= CONFIG_BOX_Y1 && py <= CONFIG_BOX_Y1 + CONFIG_BOX_HEIGHT);
}
bool pressedAdjustWeekdaySP(int px, int py) {
  return (px >= CONFIG_BOX_X && px <= CONFIG_BOX_X + CONFIG_BOX_WIDTH && py >= CONFIG_BOX_Y2 && py <= CONFIG_BOX_Y2 + CONFIG_BOX_HEIGHT);
}
bool pressedAdjustWeekendSP(int px, int py) {
  return (px >= CONFIG_BOX_X && px <= CONFIG_BOX_X + CONFIG_BOX_WIDTH && py >= CONFIG_BOX_Y3 && py <= CONFIG_BOX_Y3 + CONFIG_BOX_HEIGHT);
}
bool pressedEditSetpoint1(int px, int py) {
  return (px >= EDIT_BUTTON_X && px <= EDIT_BUTTON_X + EDIT_BUTTON_W && py >= EDIT_BUTTON_Y1 && py <= EDIT_BUTTON_Y1 + EDIT_BUTTON_H);
}
bool pressedEditSetpoint2(int px, int py) {
  return (px >= EDIT_BUTTON_X && px <= EDIT_BUTTON_X + EDIT_BUTTON_W && py >= EDIT_BUTTON_Y2 && py <= EDIT_BUTTON_Y2 + EDIT_BUTTON_H);
}
bool pressedEditSetpoint3(int px, int py) {
  return (px >= EDIT_BUTTON_X && px <= EDIT_BUTTON_X + EDIT_BUTTON_W && py >= EDIT_BUTTON_Y3 && py <= EDIT_BUTTON_Y3 + EDIT_BUTTON_H);
}
bool pressedEditSetpoint4(int px, int py) {
  return (px >= EDIT_BUTTON_X && px <= EDIT_BUTTON_X + EDIT_BUTTON_W && py >= EDIT_BUTTON_Y4 && py <= EDIT_BUTTON_Y4 + EDIT_BUTTON_H);
}
//UI CHANGE
//home
bool pressedHomeSpDown(int px, int py) {
  return (px >= HOME_SP_DOWN_X && px <= HOME_SP_DOWN_X + HOME_SP_WIDTH && py >= HOME_SP_Y && py <= HOME_SP_Y + HOME_SP_HEIGHT);
}
bool pressedHomeSpUp(int px, int py) {
  return (px >= HOME_SP_UP_X && px <= HOME_SP_UP_X + HOME_SP_WIDTH && py >= HOME_SP_Y && py <= HOME_SP_Y + HOME_SP_HEIGHT);
}
bool pressedHeat(int px, int py) {
  return (px >= HEAT_X && px <= HEAT_X + MBUTTON_WIDTH && py >= MBUTTON_Y && py <= MBUTTON_Y + MBUTTON_HEIGHT);
}
bool pressedCool(int px, int py) {
  return (px >= COOL_X && px <= COOL_X + MBUTTON_WIDTH && py >= MBUTTON_Y && py <= MBUTTON_Y + MBUTTON_HEIGHT);
}
bool pressedAuto(int px, int py) {
  return (px >= AUTO_X && px <= AUTO_X + MBUTTON_WIDTH && py >= MBUTTON_Y && py <= MBUTTON_Y + MBUTTON_HEIGHT);
}
bool pressedOff(int px, int py) {
  return (px >= OFF_X && px <= OFF_X + MBUTTON_WIDTH && py >= MBUTTON_Y && py <= MBUTTON_Y + MBUTTON_HEIGHT);
}
bool pressedHold(int px, int py) {
  return (px >= HOLD_X && px <= HOLD_X + HOLD_WIDTH && py >= HOLD_Y && py <= HOLD_Y + HOLD_HEIGHT);
}
//setpoint menu
bool pressedDisplayFahrenheit(int px, int py) {
  return (px >= CHECKBOX_X && px <= CHECKBOX_X + CHECKBOX_SIDE && py >= CHECKBOX_Y1 && py <= CHECKBOX_Y1 + CHECKBOX_SIDE);
}
bool pressedEnableAllSetpoints(int px, int py) {
  return (px >= CHECKBOX_X && px <= CHECKBOX_X + CHECKBOX_SIDE && py >= CHECKBOX_Y2 && py <= CHECKBOX_Y2 + CHECKBOX_SIDE);
}
bool pressedEnableSetpoint1(int px, int py) {
  return (px >= SP_EN_X - SP_EN_RADIUS && px <= SP_EN_X + SP_EN_RADIUS && py >= SP_EN_Y1 - SP_EN_RADIUS && py <= SP_EN_Y1 + SP_EN_RADIUS);  
}
bool pressedEnableSetpoint2(int px, int py) {
  return (px >= SP_EN_X - SP_EN_RADIUS && px <= SP_EN_X + SP_EN_RADIUS && py >= SP_EN_Y2 - SP_EN_RADIUS && py <= SP_EN_Y2 + SP_EN_RADIUS);
}
bool pressedEnableSetpoint3(int px, int py) {
  return (px >= SP_EN_X - SP_EN_RADIUS && px <= SP_EN_X + SP_EN_RADIUS && py >= SP_EN_Y3 - SP_EN_RADIUS && py <= SP_EN_Y3 + SP_EN_RADIUS);
}
bool pressedEnableSetpoint4(int px, int py) {
  return (px >= SP_EN_X - SP_EN_RADIUS && px <= SP_EN_X + SP_EN_RADIUS && py >= SP_EN_Y4 - SP_EN_RADIUS && py <= SP_EN_Y4 + SP_EN_RADIUS);
}
//set setpoint
bool pressedSetSetpointHourUp(int px, int py) {
  return (px >= SSP_HH_X && px <= SSP_HH_X + SSP_TIME_W && py >= SSP_TIME_UP_Y && py <= SSP_TIME_UP_Y + SSP_TIME_H);
}
bool pressedSetSetpointHourDown(int px, int py) {
  return (px >= SSP_HH_X && px <= SSP_HH_X + SSP_TIME_W && py >= SSP_TIME_DOWN_Y && py <= SSP_TIME_DOWN_Y + SSP_TIME_H);
}
bool pressedSetSetpointMinuteUp(int px, int py) {
  return (px >= SSP_MM_X && px <= SSP_MM_X + SSP_TIME_W && py >= SSP_TIME_UP_Y && py <= SSP_TIME_UP_Y + SSP_TIME_H);
}
bool pressedSetSetpointMinuteDown(int px, int py) {
  return (px >= SSP_MM_X && px <= SSP_MM_X + SSP_TIME_W && py >= SSP_TIME_DOWN_Y && py <= SSP_TIME_DOWN_Y + SSP_TIME_H);
}
bool pressedSetSetpointTempUp(int px, int py) {
  return (px >= SSP_TEMP_UP_X && px <= SSP_TEMP_UP_X + SSP_TEMP_W && py >= SSP_TEMP_Y && py <= SSP_TEMP_Y + SSP_TEMP_H);
}
bool pressedSetSetpointTempDown(int px, int py) {
  return (px >= SSP_TEMP_DOWN_X && px <= SSP_TEMP_DOWN_X + SSP_TEMP_W && py >= SSP_TEMP_Y && py <= SSP_TEMP_Y + SSP_TEMP_H);
}
//set time
bool pressedMonthUp(int px, int py) {  
  return (px >= DT_MM_X && px <= DT_MM_X + DT_TOP_W && py >= DT_TOP_UP_Y && py <= DT_TOP_UP_Y + DT_TOP_H);
}
bool pressedMonthDown(int px, int py) {
  return (px >= DT_MM_X && px <= DT_MM_X + DT_TOP_W && py >= DT_TOP_DOWN_Y && py <= DT_TOP_DOWN_Y + DT_TOP_H);
}
bool pressedDayUp(int px, int py) {
  return (px >= DT_DD_X && px <= DT_DD_X + DT_TOP_W && py >= DT_TOP_UP_Y && py <= DT_TOP_UP_Y + DT_TOP_H);
}
bool pressedDayDown(int px, int py) {
  return (px >= DT_DD_X && px <= DT_DD_X + DT_TOP_W && py >= DT_TOP_DOWN_Y && py <= DT_TOP_DOWN_Y + DT_TOP_H);
}
bool pressedYearUp(int px, int py) {
  return (px >= DT_YYYY_X && px <= DT_YYYY_X + DT_TOP_W && py >= DT_TOP_UP_Y && py <= DT_TOP_UP_Y + DT_TOP_H);
}
bool pressedYearDown(int px, int py) {
  return (px >= DT_YYYY_X && px <= DT_YYYY_X + DT_TOP_W && py >= DT_TOP_DOWN_Y && py <= DT_TOP_DOWN_Y + DT_TOP_H);
}
bool pressedHourUp(int px, int py) {
  return (px >= DT_HH_X && px <= DT_HH_X + DT_BOTTOM_W && py >= DT_BOTTOM_UP_Y && py <= DT_BOTTOM_UP_Y + DT_BOTTOM_H);
}
bool pressedHourDown(int px, int py) {
  return (px >= DT_HH_X && px <= DT_HH_X + DT_BOTTOM_W && py >= DT_BOTTOM_DOWN_Y && py <= DT_BOTTOM_DOWN_Y + DT_BOTTOM_H);
}
bool pressedMinuteUp(int px, int py) {
  return (px >= DT_NN_X && px <= DT_NN_X + DT_BOTTOM_W && py >= DT_BOTTOM_UP_Y && py <= DT_BOTTOM_UP_Y + DT_BOTTOM_H);
}
bool pressedMinuteDown(int px, int py) {
  return (px >= DT_NN_X && px <= DT_NN_X + DT_BOTTOM_W && py >= DT_BOTTOM_DOWN_Y && py <= DT_BOTTOM_DOWN_Y + DT_BOTTOM_H);
}
