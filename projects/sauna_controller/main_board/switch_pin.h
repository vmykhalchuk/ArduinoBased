#ifndef SWITCH_PIN_H
#define SWITCH_PIN_H

#include <Arduino.h>

struct SwitchDef {
  int pinNo;
  bool isActiveHigh;
};

const bool IS_ACTIVE_HIGH = true;
const bool IS_ACTIVE_LOW = false;

void initSwitch(SwitchDef swDef, bool initInOnState = false);

void switchOn(SwitchDef swDef);
void switchOff(SwitchDef swDef);

void switchOnOrOff(SwitchDef swDef, bool on);
#endif
