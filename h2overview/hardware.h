#ifndef HARDWARE_H
#define HARDWARE_H

// #define PRESSURE_SENSOR_PIN A0
// #define SOLENOID_RELAY_PIN D5
// #define SOLENOID_BUTTON_PIN D4
// #define FLOW_SENSOR_PIN D6
// #define LED_1 D3

#define PRESSURE_SENSOR_PIN 32
#define FLOW_SENSOR_PIN 4
#define SOLENOID_BUTTON_PIN 6
#define SOLENOID_RELAY_PIN 5
#define LED_1 7

#include <Arduino.h>

class Hardware {
 public:
  Hardware();

  void initialize_pins();
  int get_solenoid_state();
  int get_solenoid_button_press();
  void set_solenoid_state(int state);
  void set_led_state(int state);
  float read_current_water_pressure();
  float read_average_water_pressure(int duration);
  float read_waterflow_rate(int time);
  float read_cummulative_water();

 private:
  // interrupt handler
  static void IRAM_ATTR pulse_counter();
  // static void pulse_counter();

  // Global variable
  static unsigned long PULSE;
  static unsigned long HOURLY_PULSE;

  // Debounce variables
  int buttonState;
  int lastButtonState;
  unsigned long lastDebounceTime;
  const unsigned long DEBOUNCE_DELAY = 50;

  // Calibration constants
  const float PRESSURE_OFFSET = 0.483;
  const float BASELINE_PRESSURE_VOLTAGE = 0.45;
};

#endif