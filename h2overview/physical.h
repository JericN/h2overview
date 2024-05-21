#ifndef PHYSICAL_H
#define PHYSICAL_H

// #define PRESSURE_SENSOR_PIN A0
#define PRESSURE_SENSOR_PIN 3
// #define FLOW_SENSOR_PIN D2
#define FLOW_SENSOR_PIN 4
// #define SOLENOID_BUTTON_PIN D3
#define SOLENOID_BUTTON_PIN 6
// #define SOLENOID_RELAY_PIN D4
#define SOLENOID_RELAY_PIN 21

#include <Arduino.h>

class Physical {
 public:
  Physical();

  void initialize_pins();
  int get_solenoid_state();
  int get_solenoid_button_press();
  void set_solenoid_state(int state);
  float read_waterflow_rate();
  float read_water_pressure();

 private:
  // Interrupt handler
  static void pulse_counter();

  // Global variable
  static unsigned long PULSE;

  // Debounce variables
  int buttonState;
  int lastButtonState;
  unsigned long lastDebounceTime;
  const unsigned long DEBOUNCE_DELAY = 50;

  // Calibration constants
  const float PRESSURE_OFFSET = 0;
  const float BASELINE_PRESSURE_VOLTAGE = 0;
};

#endif