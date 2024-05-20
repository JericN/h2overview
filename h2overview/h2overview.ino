#include "physical.h"
#include "server.h"

Physical hardware;
FirebaseServer firebase;

#define SOLENOID_OPEN 0x1
#define SOLENOID_CLOSED 0x0

#define LEAK_RETRIES 5
#define LEAK_CONFIDENCE 0.5
#define LEAK_RESULT_COUNT LEAK_CONFIDENCE * 10

// =============================================================================
// ============================== PHYSICAL LAYER ===============================
// =============================================================================

// Scan for leaks in the system
bool scan_leak() {
  if (hardware.read_water_pressure() < 0) // TODO: get baseline pressure
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
      hardware.set_solenoid_state(SOLENOID_OPEN);

      // Wait for changes in the water flow rate
      for (int time = 0; time < bigLeakWaitTime * 1000; time++) {
        if (hardware.read_waterflow_rate() > 0) {
          big_leak_result[j] = 1;
          break;
        }
      }

      // Then check for small leak
      hardware.set_solenoid_state(SOLENOID_CLOSED);
      float initialRead = hardware.read_water_pressure();

      // Wait for changes in the water pressure
      for (int time = 0; time < smallLeakWaitTime * 1000; time++) {
        // Cancel if there is an interruption by user
        if (false)
          return false;  // TODO: if (get_interrupt()) return false;
        if (hardware.read_water_pressure() < initialRead) {
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

void valve_control() {
  // Get the valve state from the firebase
  bool valveFlag = firebase.get_valve_state();

  // Toggle the valve state if the button is pressed
  if (hardware.get_solenoid_button_press()) {
    firebase.set_valve_state(!valveFlag);
    valveFlag = !valveFlag;
  }

  // Set the valve state if the flag is different from the current state
  bool valveState = hardware.get_solenoid_state();
  if (valveFlag != valveState) {
    hardware.set_solenoid_state(valveFlag);
  }
}

void print_logs() {
  Serial.print("Flow Rate: ");
  Serial.print(hardware.read_waterflow_rate());
  Serial.println(" mL/s");

  Serial.print("Pressure: ");
  Serial.print(hardware.read_water_pressure());
  Serial.println(" psi");
}

// =============================================================================
// ============================== SETUP AND LOOP ===============================
// =============================================================================

void setup() {
  Serial.begin(9600);
  hardware.initialize_pins();
}

void loop() {
  print_logs();
  valve_control();

  if (firebase.is_leak_scanning()) {
    scan_leak();
  }

  // TODO: Remove the delay once done with testing
  delay(5000);
}
