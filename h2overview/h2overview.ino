#include "physical.h"
#include "server.h"

// #include <ESP8266WiFi.h>  // For D1 R1
#include <WiFi.h>   // For ESP32

Physical hardware;
FirebaseServer firebase;

#define SOLENOID_OPEN 0
#define SOLENOID_CLOSED 1

#define SCAN_RETRIES 5
#define SCAN_COUNT 5
#define BIG_LEAK_THRESHOLD 0.5

WiFiClient espClient;
const char *ssid = "dcs-students2";
const char *password = "W1F14students";


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


// =============================================================================
// ============================== PHYSICAL LAYER ===============================
// =============================================================================

// Function to check if all elements are the same
bool areResultConsistent(int arr[SCAN_COUNT]) {
  int firstElement = arr[0];
  for (int i = 1; i < SCAN_COUNT; i++) {
    if (arr[i] != firstElement) {
      return false;
    }
  }
  return true;
}

int big_leak_scan() {
  int start = millis();
  // TODO: get scan duration
  while (millis() - start < 10000) {
    float flow = hardware.read_waterflow_rate();
    if (flow > BIG_LEAK_THRESHOLD) {
      return 1;
    }
  }
  return 0;
}

int small_leak_scan() {
  float initialRead = hardware.read_water_pressure();
  int start = millis();
  // TODO: get scan duration
  while (millis() - start < 30000) {
    // TODO: if (get_interrupt()) return 0;
  }
  float finalRead = hardware.read_water_pressure();
  return finalRead < initialRead;
}

// Scan for leaks in the system
// FIXME: This function is untested
// FIXME: This function is untested
// FIXME: This function is untested
bool scan_leak() {
  // TODO: get baseline pressure
  if (hardware.read_water_pressure() < 0)
    return false;

  int big_leak_result[SCAN_COUNT];
  int small_leak_result[SCAN_COUNT];

  // Retry loop if the big leak or small leak results are inconsistent
  for (int i = 0; i < SCAN_RETRIES; i++) {
    // Scan for big and small leaks
    for (int j = 0; j < SCAN_COUNT; j++) {
      hardware.set_solenoid_state(SOLENOID_OPEN);
      big_leak_result[j] = big_leak_scan();
      hardware.set_solenoid_state(SOLENOID_CLOSED);
      small_leak_result[j] = small_leak_scan();
    }

    // Check if the results are consistent
    bool big_leak_consistent = areResultConsistent(big_leak_result);
    bool small_leak_consistent = areResultConsistent(small_leak_result);

    // Analyze the results
    if (big_leak_consistent && small_leak_consistent) {
      int big_leak = big_leak_result[0];
      int small_leak = small_leak_result[0];

      if (big_leak == 1 && small_leak == 1) {
        return 2;  // big leak
      } else if (big_leak == 0 && small_leak == 1) {
        return 1;  // small leak
      } else if (big_leak == 0 && small_leak == 0) {
        return 0;  // no leak
      } else if (big_leak == 1 && small_leak == 0) {
        continue;  // this is not possible, retry
      }
    }
  }

  // If the results are still inconsistent after retries
  return 3;
}

void valve_control() {
  // Get the valve state from the firebase
  bool valveFlag = firebase.get_valve_state();
  Serial.print("Firebase Valve State:");
  Serial.println(valveFlag);

  // Toggle the valve state if the button is pressed
  int buttonPress = hardware.get_solenoid_button_press();
  Serial.print("Button Valve State:");
  Serial.println(buttonPress);
  if (buttonPress) {
    firebase.set_valve_state(!valveFlag);
    valveFlag = !valveFlag;
  }

  // Set the valve state if the flag is different from the current state
  bool valveState = hardware.get_solenoid_state();
  Serial.print("Valve State:");
  Serial.println(valveState);
  if (valveFlag != valveState) {
    hardware.set_solenoid_state(valveFlag);
    Serial.println("FLIPPED");
  }

  hardware.set_led_state(valveFlag);
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
  delay(5000);

  Serial.println("hard start");
  hardware.initialize_pins();
  Serial.println("hard end");
  setup_wifi();
}

void loop() {
  // Serial.println("close");
  // hardware.set_solenoid_state(1);
  // delay(5000);
  // Serial.println("open");
  // hardware.set_solenoid_state(0);
  // delay(5000);

  // print_logs();
  valve_control();

  // if (firebase.is_leak_scanning()) {
  //   int res = scan_leak();
  //   firebase.set_leak_detected(res);
  // }

  // TODO: Remove the delay once done with testing
  // delay(5000);
}
