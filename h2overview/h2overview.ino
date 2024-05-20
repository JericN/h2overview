
#include "physical.h"

Physical p;

#define SOLENOID_OPEN 0x1
#define SOLENOID_CLOSED 0x0

#define LEAK_RETRIES 5
#define LEAK_CONFIDENCE 0.5
#define LEAK_RESULT_COUNT LEAK_CONFIDENCE * 10

// Global variables for the water flow sensor
volatile long pulse;
float volume = 0;
float lastTime = 0.0;

// Global variables for the solenoid button
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 100;
bool buttonState = HIGH;
bool lastButtonState = HIGH;

// =============================================================================
// ============================== PHYSICAL LAYER ===============================
// =============================================================================

// Scan for leaks in the system
bool scan_leak() {
  if (p.read_water_pressure() ==)
    return false;

  int bigLeakWaitTime = 10;    // TODO: get_big_leak_pref(); returns in seconds
  int smallLeakWaitTime = 10;  // TODO: get_small_leak_pref(); returns in seconds

  int big_leak_result[LEAK_RESULT_COUNT];
  int small_leak_result[LEAK_RESULT_COUNT];

  // Retry loop if the big leak or small leak results are inconsistent
  for (int i = 0; i < LEAK_RETRIES; i++) {
    // Leak detection loop for big and small leaks
    for (int j = 0; j < LEAK_RESULT_COUNT; j++) {
      // First check for big leak
      p.set_solenoid_state(SOLENOID_OPEN);

      // Wait for changes in the water flow rate
      for (int time = 0; time < bigLeakWaitTime * 1000; time++) {
        if (p.read_waterflow_rate() > 0) {
          big_leak_result[j] = 1;
          break;
        }
      }

      // Then check for small leak
      p.set_solenoid_state(SOLENOID_CLOSED);
      float initialRead = p.read_water_pressure();

      // Wait for changes in the water pressure
      for (int time = 0; time < smallLeakWaitTime * 1000; time++) {
        // Cancel if there is an interruption by user
        if (false)
          return false;  // TODO: if (get_interrupt()) return false;
        if (p.read_water_pressure() < initialRead) {
          big_leak_result[j] = 1;
          break;
        }
      }
    }

    // Start checking for consistency and results
    bool big_leak_consistent = true;
    bool small_leak_consistent = true;

    // Check for big leak consistency and result
    for (check1 = 0; check1 < LEAK_RESULT_COUNT - 1; check1++) {
      if (big_leak_result[check1] != big_leak_result[check1 + 1]) {
        big_leak_consistent = false;
        break;
      }
    }

    if (big_leak_consistent) {
      if (big_leak_result[0] == 1) {
        // TODO: set_leak_detected(1);
        return true;
      }
      // Otherwise, continue to check for small leak or retry due to inconsistency
    }

    // Check for small leak consistency and result
    for (check2 = 0; check2 < LEAK_RESULT_COUNT - 1; check2++) {
      if (small_leak_result[check2] != small_leak_result[check2 + 1]) {
        small_leak_consistent = false;
        break;
      }
    }
    if (small_leak_consistent) {
      if (small_leak_result[0] == 1) {
        // TODO: set_leak_detected(2);
        return true;
      } else {
        // TODO: set_leak_detected(0);
        return false;
      }
      // Otherwise, retry due to inconsistency
    }
  }

  // Out of retries, failed to detect leak
  // Assumed no leak detected
  return false;
}

// =============================================================================
// ============================= DEVICE FUNCTIONS ==============================
// =============================================================================

// Once the solenoid button is pressed, the solenoid state will be toggled
void switch_solendoid_state() {
  if (p.get_solenoid_state() == SOLENOID_OPEN)
    p.set_solenoid_state(SOLENOID_CLOSED);
  else
    p.set_solenoid_state(SOLENOID_OPEN);
}

// Check solenoid button press, toggle if pressed
void get_solenoid_button_press() {
  int reading = digitalRead(solenoidButtonPin);

  if (reading != lastButtonState)
    lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay && reading != buttonState) {
    buttonState = reading;
    if (buttonState == HIGH)
      switch_solendoid_state();
  }

  lastButtonState = reading;
}

// =============================================================================
// ============================== SETUP AND LOOP ===============================
// =============================================================================

void setup() {
  // Initialize the serial communication
  Serial.begin(9600);

  // Initialize the pins
  physical.initialize_pins();
}

void loop() {
  Serial.print("Volume: ");
  Serial.print(read_water_volume());
  Serial.println(" mL");

  Serial.print("Flow Rate: ");
  Serial.print(p.read_waterflow_rate());
  Serial.println(" mL/s");

  Serial.print("Pressure: ");
  Serial.print(p.read_water_pressure());
  Serial.println(" psi");

  // Check if the valve should be opened or closed
  // No toggle if the valve is already in the desired state
  bool valveFlag = false;  // TODO: get_valve_flag();
  bool isValveOpened = p.get_solenoid_state() == SOLENOID_OPEN ? true : false;
  if (isValveOpened != valveFlag)
    switch_solendoid_state();

  // Check if leak detec should start
  bool detectLeakFlag = false;  // TODO: get_detect_leak_flag();
  if (detectLeakFlag) {
    if (scan_leak())
      Serial.println("Leak Detected");
    else
      Serial.println("No Leak Detected");
  }

  // Check if the solenoid button is pressed
  get_solenoid_button_press();

  // TODO: Remove the delay once done with testing
  delay(5000);
}

#include "physical.h"

void setup() {
  // Initialize the serial communication
  Serial.begin(9600);

  // Initialize the pins
  p.initialize_pins();
}

void loop() {
  Serial.print("Hello");
}