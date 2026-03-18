#include "switch_pin.h"

void initSwitch(SwitchDef swDef, bool initInOnState) {
  if (initInOnState) {
    switchOn(swDef);
  } else {
    switchOff(swDef);
  }
  pinMode(swDef.pinNo, OUTPUT);
}

void switchOn(SwitchDef swDef) {
  bool onState = swDef.isActiveHigh;
  digitalWrite(swDef.pinNo, onState ? HIGH : LOW);
}

void switchOff(SwitchDef swDef) {
  bool offState = !swDef.isActiveHigh;
  digitalWrite(swDef.pinNo, offState ? HIGH : LOW);
}

void switchOnOrOff(SwitchDef swDef, bool on) {
  if (on) switchOn(swDef);
  else switchOff(swDef);
}
