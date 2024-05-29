#include "hardware.h"

// Initialize static variable
unsigned long Hardware::PULSE = 0;
unsigned long Hardware::HOURLY_PULSE = 0;

Hardware::Hardware() {
  // Constructor
}

// Interrupt handler
void IRAM_ATTR Hardware::pulse_counter() {
  Hardware::PULSE++;
  Hardware::HOURLY_PULSE++;
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

// Set the state of the solenoid
void Hardware::set_solenoid_state(int state) {
  digitalWrite(SOLENOID_RELAY_PIN, state);
}

// Set the state of the LED
void Hardware::set_led_state(int state) {
  digitalWrite(LED_1, state);
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

// Read the water pressure
float Hardware::read_current_water_pressure() {
  int V = analogRead(PRESSURE_SENSOR_PIN);
  float pressure = map(V, 0, 1023, 0, 100);
  return pressure;
}

// Read the water pressure
float Hardware::read_average_water_pressure(int duration) {
  int start_time = millis();
  float readings[100];
  int i = 0;

  // Read the pressure every 200ms
  while (millis() - start_time < duration) {
    int V = analogRead(PRESSURE_SENSOR_PIN);
    float pressure = map(V, 0, 1023, 0, 100);
    readings[i++] = pressure;
    delay(200);
  }

  // Calculate the average pressure
  float sum = 0;
  for (int j = 0; j < i; j++) {
    sum += readings[j];
  }

  float average_pressure = sum / i;
  return average_pressure;
}

// Read the water flow rate
float Hardware::read_waterflow_rate(int time) {
  Hardware::PULSE = 0;
  delay(time);
  return 8.5714 * Hardware::PULSE;
}

// Read the cummulative water flow
float Hardware::read_cummulative_water() {
  float output = 8.5714 * Hardware::HOURLY_PULSE;
  Hardware::HOURLY_PULSE = 0;
  return output;
}
