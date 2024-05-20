#ifndef PHYSICAL_H
#define PHYSICAL_H

#define PRESSURE_SENSOR_PIN A0
#define FLOW_SENSOR_PIN D2
#define SOLENOID_BUTTON_PIN D3
#define SOLENOID_RELAY_PIN D4

#include <Arduino.h>

class Physical {
 public:
  Physical();
  void initialize_pins();
  int get_solenoid_state();
  void set_solenoid_state(int state);
  float read_waterflow_rate();
  float read_water_pressure();

 private:
  // Interrupt handler
  static void pulse_counter();

  // Global variable
  static unsigned long PULSE;

  // Calibration constants
  const float PRESSURE_OFFSET = 0;
  const float BASELINE_PRESSURE_VOLTAGE = 0;
};

#endif