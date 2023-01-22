/*
  * Communication protocol
  * 1.  Request: "help" or "h";
  *     Response:  
  * "     To start charging set type of battery.;
  *         a - get all; ({current}, {voltage}, {needed current}, {percentage}, {pwm}, {type}, {state})
  *           Battery types: 0 - None, 1 - LiIon, 2 - AGM;
  *           Charging states: 0 - charging, 1 - idle, 2 - full, 3 - error
  *         i?x - set needed current to x; (0 - 20A)
  *         h - this message; 
  *         x - stop; (0 - success)
  *         r - reset; (0 - success)
  *     Codes: 1 - wrong request; 2 - wrong value; 3 - unknown error; 4 - battery full; 5 - current overflow; 
  * "
  * 2.  Request: {current}; Response: {float; (0 - 25)};
  *     Request: {voltage}; Response: {float; (0 - 15)};
  *     Request: {needed current}; Response: {float; (0 - 15)};
  *     Request: {percentage}; Response: {float; (0 - 100)};
  *     Request: {pwm}; Response: {byte; (0 - 255)};
  *     Request: {type}; Response: {byte; (1 - 2)};
  *     Request: {state}; Response: {byte; (0 - 3)};
  *     Request: "i?x"; Response: {float; (0 - success)};
  *     Request: "x"; Response: {bool; (0 - success)};
  *     Request: "r"; Response: {bool; (0 - success)};
  * 3.  Code: {byte; (1)}; 1 - wrong request; 2 - wrong value; 3 - unknown error; 4 - battery full; 5 - current overflow; 
  */

#include "request_processor.h"

#define CHARGER_TYPE_INPUT 2

Charger * charger = new Charger();

void setup() {
  Serial.begin(9600);
  pinMode(PWM, OUTPUT);
  pinMode(CHARGER_TYPE_INPUT, INPUT_PULLUP);
  analogReference(INTERNAL);
  charger->start();
  RequestProcessor::printHelp();
}

void loop() {
  RequestProcessor::process(charger);

  auto type = static_cast<Charger::BatteryType>(
    !digitalRead(CHARGER_TYPE_INPUT) + 1
  ); // 1 - LiIon, 2 - AGM
  if (type != charger->getType()) charger->setType(type);

  charger->run();
}
