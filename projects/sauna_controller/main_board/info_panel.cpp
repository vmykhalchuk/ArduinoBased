#include "info_panel.h"

namespace InfoPanel {

  static SwitchDef sw_Led1 = SW_DEF_EMPTY, sw_Buzzer = SW_DEF_EMPTY;

  void init(SwitchDef sw_Led1, SwitchDef sw_Buzzer) {
    InfoPanel::sw_Led1 = sw_Led1;
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
