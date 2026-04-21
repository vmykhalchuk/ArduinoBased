#include "info_panel.h"

namespace InfoPanel {

  SwitchDef *_sw_Led1 = nullptr, *_sw_Buzzer = nullptr;

  void init(SwitchDef &sw_Led1, SwitchDef &sw_Buzzer) {
    _sw_Led1 = &sw_Led1;
    _sw_Buzzer = &sw_Buzzer;
  }

  void setCommunicationError() {
    digitalWrite(LED_BUILTIN, HIGH);
    switchOn(*_sw_Led1);
  }
  void clearCommunicationError() {
    digitalWrite(LED_BUILTIN, LOW);
  }

  void setWarningCode(uint8_t f) {}
  void clearWarningCode(uint8_t f) {}

  void tick() {
    // FIXME Implement
  }
  
}
