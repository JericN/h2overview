#include "hardware.h"

// Initialize static variable
unsigned long Hardware::PULSE = 0;

Hardware::Hardware() {
  // Constructor
}

// Interrupt handler
void IRAM_ATTR Hardware::pulse_counter() {
  Hardware::PULSE++;
}

// Initialize pins
void Hardware::initialize_pins() {
  pinMode(FLOW_SENSOR_PIN, INPUT);
  pinMode(SOLENOID_RELAY_PIN, OUTPUT);
  pinMode(SOLENOID_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_1, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), Hardware::pulse_counter, RISING);
}

// Get the state of the solenoid
int Hardware::get_solenoid_state() {
  return digitalRead(SOLENOID_RELAY_PIN);
}

// Get the state of the solenoid button
int Hardware::get_solenoid_button_press() {
  int reading = digitalRead(SOLENOID_BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

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

// Set the state of the solenoid
void Hardware::set_solenoid_state(int state) {
  digitalWrite(SOLENOID_RELAY_PIN, state);
}

void Hardware::set_led_state(int state) {
  digitalWrite(LED_1, state);
}

// Read the water flow rate
float Hardware::read_waterflow_rate() {
  Hardware::PULSE = 0;
  delay(1000);
  return 2.663 * Hardware::PULSE;
}

// Read the water flow rate
float Hardware::read_waterflow_rate_timed(int time) {
  Hardware::PULSE = 0;
  delay(time);
  return 2.663 * Hardware::PULSE;
}

// Read the water pressure
float Hardware::read_water_pressure() {
  float V = analogRead(PRESSURE_SENSOR_PIN) * 5.00 / 1023;
  // FIXME: calibrate pressure
  float P = (V - 0.83) * 400;
  return P;
}
