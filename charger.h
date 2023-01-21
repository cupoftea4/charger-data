#ifndef CHARGER_H
#define CHARGER_H

#define DEBUG 0

#define PWM 11
#define CURRENT A5
#define VOLTAGE A3 
#define MAX_CURRENT 25
#define MAX_VOLTAGE 15

#define CURRENT_ERROR 0.5

#define LI_ION_MIN_VOLTAGE 12.2
#define LI_ION_MAX_VOLTAGE 12.6

#define AGM_MIN_VOLTAGE 13.5
#define AGM_MAX_VOLTAGE 14.0

#define LI_ION_CURRENT 12
#define AGM_CURRENT 2

#define BATTERY_TYPE_COUNT 2

unsigned long timer = 0;

class Charger {
public:
  enum BatteryType {
    None = 0,
    LiIon = 1,
    AGM = 2
  };

  enum BatteryState {
    Charging = 0,
    Idle = 1,
    Full = 2,
    Error = 3
  };

  void start() {
    reset();
  }

  void run() {
    if (state == Charger::Idle || state == Charger::Error) return;
  
    float voltage = getVoltage();
    float current = getCurrent();

    if (abs(current) > MAX_CURRENT) {
      error("Max current exceeded");
    }

    if (voltage > MAX_VOLTAGE) {
      error("Max voltage exceeded");
    }

    if (voltage > maxVoltageBreakpoint() && abs(current) < 0.1) {
      setBatteryIsFull();
    }

    if (millis() - timer > 3000) { // every 3 seconds
      timer = millis();
      adjustPM(voltage, current);
      if (DEBUG) 
        Serial.println(
          "PWM=" + (String)pwm + "\n" + 
          "Voltage: " + (String)voltage + " " +
          "Current: " + (String)current
        );
    }
  }

  boolean setType(BatteryType type) {
    if (type != LiIon && type != AGM) return false;
    this->type = type;
    this->neededCurrent = type == LiIon ? LI_ION_CURRENT : AGM_CURRENT;
    this->pwm = 50;
    this->state = Charging;
    if (DEBUG)
      Serial.println("Battery type changed to " + strBatteryType[type]);
    return true;
  }

  boolean setNeededCurrent(float current) {
    if (current < 0 || current > 20) return false;
    this->neededCurrent = current;
    return true;
  }

  float getCurrent() {
    int current = analogRead(CURRENT);
    return (current - 432)  * 0.088;
  }

  float getVoltage() {
    int voltage = analogRead(VOLTAGE);
    return voltage * 0.0195669;
  }

  float getNeededCurrent() {
    return this->neededCurrent;
  }

  String getStringData(String separator = ",") {
    return  (String)getCurrent() + separator + 
            (String)getVoltage() + separator + 
            (String)neededCurrent + separator +
            (String)pwm + separator + 
            (String)type + separator + 
            (String)state;
  }

  BatteryType getType() {
    return this->type;
  }

  byte getPWM() {
    return this->pwm;
  }

  void setBatteryIsFull() {
    stop(); // sets state to Idle
    this->state = Full;
  }

  BatteryState getState() {
    return this->state;
  }

  void error(String message) {
    stop(message);
    this->state = Error;
  }

  void adjustPM(float voltage, float current) {
    if (this->type == None) return;

    if (voltage >= maxVoltageBreakpoint()) {
      analogWrite(PWM, --pwm);
      reachedMaxVoltage = true;
      if (DEBUG)
        Serial.println("Reached max voltage. PWM=" + (String)pwm);
      return;
    }

    if (reachedMaxVoltage) return;

    byte adjustCurrentTo = 0;
    current = abs(current);

    if (voltage < minVoltageBreakpoint()) {
      adjustCurrentTo = this->neededCurrent;
    } else if (voltage < maxVoltageBreakpoint()) {
      adjustCurrentTo = this->neededCurrent / 2.0;
    }

    if (current - adjustCurrentTo > CURRENT_ERROR && pwm > 0) pwm--;
    else if (adjustCurrentTo - current > CURRENT_ERROR && pwm < 255) pwm++;

    analogWrite(PWM, pwm);
  }

  boolean reset() {
    this->pwm = 0;
    this->type = None;
    this->neededCurrent = 0;
    this->reachedMaxVoltage = false;
    this->state = Idle;
    analogWrite(PWM, 0);
    if (DEBUG) Serial.println("Charger started.");
    return true;
  }

  boolean stop(String message = "") {
    analogWrite(PWM, 0);
    if (message != "" && DEBUG) Serial.println(message);
    this->neededCurrent = 0;
    this->pwm = 0;
    return true;
  }

private:
  float maxVoltageBreakpoint() {
    return this->type == LiIon ? LI_ION_MAX_VOLTAGE : AGM_MAX_VOLTAGE;
  }
  
  float minVoltageBreakpoint() {
    return this->type == LiIon ? LI_ION_MIN_VOLTAGE : LI_ION_MIN_VOLTAGE;
  }

  byte pwm = 80;
  BatteryType type = BatteryType::None;
  float neededCurrent = 0;
  bool reachedMaxVoltage = false;
  const String strBatteryType[BATTERY_TYPE_COUNT + 1] = {"None", "LiIon", "AGM"};
  BatteryState state = Idle;
};

#endif
