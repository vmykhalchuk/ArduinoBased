#include "info_panel.h"

namespace InfoPanel {

  static SwitchDef _sw_Led1 = SW_DEF_EMPTY, _sw_Buzzer = SW_DEF_EMPTY;

  void init(SwitchDef sw_Led1, SwitchDef sw_Buzzer) {
    _sw_Led1 = sw_Led1;
    _sw_Buzzer = sw_Buzzer;
  }

  void setCommunicationError() {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  void clearCommunicationError() {
    digitalWrite(LED_BUILTIN, LOW);
  }

  void setWarningCode(uint8_t f) {}
  void clearWarningCode(uint8_t f) {}

  void loop() {
    
  }
  
}
