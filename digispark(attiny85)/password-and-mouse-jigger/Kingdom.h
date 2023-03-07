// FIXME This must be a part of my library!!!
// This is a copy of same file in "arduino-sketch-war-comm-system-2"
#ifndef Kingdom_h
#define Kingdom_h

#include "Arduino.h"

// below definitions are used for State Machines (REFACTOR use inheritance for this instead)
#define CSTATE(newState)   this->state = newState; break;

class Kingdom {
  public:
    static uint16_t microsShort;
    static uint16_t millisShort;
};

#endif