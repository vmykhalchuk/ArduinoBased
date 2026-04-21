#include "clock.h"
uint32_t ClockHR::now = _ClockMillis::_millis(); // Define the storage
uint16_t ClockLR::now = _ClockMillis::_millis();

namespace _ClockMillis {
  uint32_t _millis() {
    return millis();
  }
}
