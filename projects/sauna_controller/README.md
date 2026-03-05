# Sauna Controller
Consists of two modules
* Main board
* Remote board

## Main board
Is responsible for handling AC 3 phase power.
Consists of / responsible for:
* relay that enables contactor to provide power to TRIACs
* TRIACs to provide power to heater
* monitor temperature of every simistor
* monitor temperature inside power enclosure
* monitor temperature outside enclosure
* operate main fan
* operate TRIAC fans
* RS485 to communicate with remote
** Heater requested on/off state
** Heating temperature request (yes/no)
** Fire hazard (yes/no)
* operate alarm (when fire hazard reported by remote)
* operate led to display status (green - active, yellow - TRIACs powered, blinking red - fault)
* operate zoomer to sound fault condition

## Remote board
Is user facing interface. Receives commands from user.
Consists of / responsible for:
* power on/off button - to activate main board
* led status - green - ready, yellow - powered, red blinking - error
* temperature display - target temperature
* temperature control buttons - up/down to adjust target temperature
* temperature sensors (two to better approximate Sauna's inside temperature)
* fire hazard temperature sensor (sensor hardwired to engage at 100°C)
* RS485 to report its current state (power,heat request,fire) to Main board upon request
