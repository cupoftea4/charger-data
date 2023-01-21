#ifndef REQUEST_PROCESSOR_H
#define REQUEST_PROCESSOR_H

#include "charger.h"
const char HelpMessage[] PROGMEM  = "To start charging set type of battery. \n\
  a - get all; ({current}, {voltage}, {needed current}, {pwm}, {type}, {state}) \n\
    Battery types: 0 - None, 1 - LiIon, 2 - AGM; \n\
    Charging states: 0 - charging, 1 - idle, 2 - full, 3 - error \n\
  i?x - set needed current to x; (0 - 20A) \n\
  h or help - this message; \n\
  x - stop; (0 - success) \n\
  r - reset; (0 - success) \n\
  Codes: 1 - wrong request; 2 - wrong value; 3 - unknown error; 4 - battery full; 5 - current overflow; ";

String quickStringRead(int timeout = 20) {
  String input = "";
  char c;
  unsigned long timer = millis();
  while (millis() - timer < timeout) {
    if (Serial.available()) {
      c = Serial.read();
      if (c == '\n') break;
      if (c >= 0) input += c;
      timer = millis();
    }
  }
  return input;
}


class RequestProcessor {
public:
  enum Response {
    Success = 0,
    WrongRequest = 1,
    WrongValue = 2,
    UnknownError = 3,
    BatteryFull = 4,
    CurrentOverflow = 5
  };

  static void process(Charger *charger) {
    if (!Serial.available()) return;
    String request = quickStringRead();
    if (request == "")  return;
    processRequest(request, charger); 
  }

  static void printHelp() {
    if (!DEBUG) return;
    char *buffer = (char *)malloc(sizeof(HelpMessage));
    if (buffer == NULL) return;
    strcpy_P(buffer, HelpMessage);
    Serial.println(buffer);
    free(buffer);
  }

private:
  static void processRequest(String input, Charger *charger) {
    input.trim();
    if (input == "help" || input == "h") printHelp();
    else if (input.indexOf('?') == -1 && input.length() == 1) {
      printResponse(input.charAt(0), charger);
    } else if (input.indexOf('?') == 1 && input.length() >= 3) {
      char request = input.charAt(0);
      float value = input.substring(2).toFloat();
      switch (request) {
        case 'i':
          Serial.println(charger->setNeededCurrent(value)
            ? Response::Success 
            : Response::WrongValue
          );
          break;
        default:
          Serial.println(Response::WrongRequest);
      }
    } else {
      Serial.println(Response::WrongRequest);
    }
  }

  static String getResponse(char request, Charger *charger) {
    switch (request) {
      case 'a': // get all data (comma separated)
        return (String)charger->getStringData(",");
      case 'x':
        return charger->stop() ? (String)Response::Success : (String)Response::UnknownError;
      case 'r':
        return charger->reset() ? (String)Response::Success : (String)Response::UnknownError;
      default:
        return (String)Response::WrongRequest;
    }
  }

  static void printResponse(char request, Charger *charger) {
    String response = getResponse(request, charger);
    if (response.length() > 0) Serial.println(response);
  }
};

#endif