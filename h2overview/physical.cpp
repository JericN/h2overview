#include "physical.h"

// Initialize static variable
unsigned long Physical::PULSE = 0;

Physical::Physical() {
  // Constructor
}

void Physical::pulse_counter() {
  Physical::PULSE++;
}

void Physical::initialize_pins() {
  pinMode(FLOW_SENSOR_PIN, INPUT);
  pinMode(SOLENOID_RELAY_PIN, OUTPUT);
  pinMode(SOLENOID_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), Physical::pulse_counter, RISING);
}

int Physical::get_solenoid_state() {
  return digitalRead(SOLENOID_RELAY_PIN);
}

int Physical::get_solenoid_button_press() {
  int reading = digitalRead(SOLENOID_BUTTON_PIN);

  if (reading != lastButtonState)
    lastDebounceTime = millis();

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH)
        return 1;
    }
  }

  lastButtonState = reading;
  return 0;
}

void Physical::set_solenoid_state(int state) {
  digitalWrite(SOLENOID_RELAY_PIN, state);
}

float Physical::read_waterflow_rate() {
  Physical::PULSE = 0;
  delay(1000);
  return 2.663 * Physical::PULSE;
}

float Physical::read_water_pressure() {
  float voltage = analogRead(PRESSURE_SENSOR_PIN) * 5.0 / 1023.0;
  float pressure = (voltage - PRESSURE_OFFSET) * (100.0 / (4.5 - BASELINE_PRESSURE_VOLTAGE));
  return pressure;
}
