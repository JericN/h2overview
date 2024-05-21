#include "physical.h"

// Initialize static variable
unsigned long Physical::PULSE = 0;

Physical::Physical() {
  // Constructor
}

// Interrupt handler
void Physical::pulse_counter() {
  Physical::PULSE++;
}

// Initialize pins
void Physical::initialize_pins() {
  // pinMode(FLOW_SENSOR_PIN, INPUT);
  pinMode(SOLENOID_RELAY_PIN, OUTPUT);
  pinMode(SOLENOID_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_1, OUTPUT);
  // attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), Physical::pulse_counter, RISING);
}

// Get the state of the solenoid
int Physical::get_solenoid_state() {
  return digitalRead(SOLENOID_RELAY_PIN);
}

// Get the state of the solenoid button
int Physical::get_solenoid_button_press() {
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
void Physical::set_solenoid_state(int state) {
  digitalWrite(SOLENOID_RELAY_PIN, state);
}

void Physical::set_led_state(int state) {
  digitalWrite(LED_1, state);
}

// Read the water flow rate
float Physical::read_waterflow_rate() {
  Physical::PULSE = 0;
  delay(1000);
  return 2.663 * Physical::PULSE;
}

// Read the water pressure
float Physical::read_water_pressure() {
  float voltage = analogRead(PRESSURE_SENSOR_PIN);
  Serial.print("Voltage: ");
  Serial.println(voltage);
  float pressure = ((voltage * 5.0 / 1023.0) - 0.483) * (100.0 / (3 - 0.45));
  return pressure;
}
