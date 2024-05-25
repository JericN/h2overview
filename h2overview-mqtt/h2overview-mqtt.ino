#include <ESP8266WiFi.h>  // For D1 R1
#include <PubSubClient.h>
// #include <WiFi.h>   // For ESP32

#include "feature.h"
#include "hardware.h"
#include "server.h"

Hardware hardware;
FirebaseServer firebase;
Feature feature(hardware, firebase);

#define DEVICE_ID "H2O-12345"

#define SOLENOID_OPEN 0
#define SOLENOID_CLOSED 1

#define SCAN_RETRIES 5
#define SCAN_COUNT 5
#define BIG_LEAK_THRESHOLD 0.5

WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = "dcs-students2";
const char* password = "W1F14students";
const char* mqtt_server = "broker.mqtt-dashboard.com";

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

void reconnect_mqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    char clientId[50];
    sprintf(clientId, "ESP8266Client-%04X", random(0xffff));

    if (client.connect(clientId)) {
      Serial.println("connected");
      // TODO: make the topic dynamic
      client.subscribe("h2overview/H2O-12345/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
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
  while (millis() - start < 5000) {
    float flow = hardware.read_waterflow_rate();
    Serial.print("Flow rate: ");
    Serial.println(flow);
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
int scan_leak() {
  // TODO: get baseline pressure
  // FIXME: replace correct calibration
  int pressure = hardware.read_water_pressure();
  Serial.print("Pressure reading: ");
  Serial.println(pressure);
  if (pressure < 0) {
    Serial.println("No water pressure detected");
    return 3;
  }

  Serial.println("Scanning for leaks...");

  int big_leak_result[SCAN_COUNT];
  int small_leak_result[SCAN_COUNT];

  // Retry loop if the big leak or small leak results are inconsistent
  for (int i = 0; i < SCAN_RETRIES; i++) {
    Serial.print("Retry: ");
    Serial.println(i);

    // Scan for big and small leaks
    for (int j = 0; j < SCAN_COUNT; j++) {
      Serial.print("Scan: ");
      Serial.println(j);

      hardware.set_solenoid_state(SOLENOID_OPEN);
      big_leak_result[j] = big_leak_scan();
      Serial.print("Big leak scan done: ");
      Serial.println(big_leak_result[j]);

      delay(2000);
      hardware.set_solenoid_state(SOLENOID_CLOSED);
      delay(2000);

      // small_leak_result[j] = small_leak_scan();
      // Serial.println("Small leak scan done");
      // Serial.println(small_leak_result[j]);
    }

    Serial.println("Done scanning for leaks...");

    // Check if the results are consistent
    // print big leak values
    for (int j = 0; j < SCAN_COUNT; j++) {
      Serial.print(big_leak_result[j]);
      Serial.print(" ");
    }
    Serial.println();

    bool big_leak_consistent = areResultConsistent(big_leak_result);
    Serial.print("Big leak consistent: ");
    Serial.println(big_leak_consistent);

    if (big_leak_consistent) {
      int big_leak = big_leak_result[0];
      if (big_leak == 1) {
        return 2;  // big leak
      } else if (big_leak == 0) {
        return 0;  // no leak
      }
    }

    // bool small_leak_consistent = areResultConsistent(small_leak_result);

    // // Analyze the results
    // if (big_leak_consistent && small_leak_consistent) {
    //   int big_leak = big_leak_result[0];
    //   int small_leak = small_leak_result[0];

    //   if (big_leak == 1 && small_leak == 1) {
    //     return 2;  // big leak
    //   } else if (big_leak == 0 && small_leak == 1) {
    //     return 1;  // small leak
    //   } else if (big_leak == 0 && small_leak == 0) {
    //     return 0;  // no leak
    //   } else if (big_leak == 1 && small_leak == 0) {
    //     continue;  // this is not possible, retry
    //   }
    // }
  }

  // If the results are still inconsistent after retries
  return 3;
}

// =============================================================================
// ============================== MQTT CALLBACK ================================
// =============================================================================

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  int value = atoi((char*)payload);

  Serial.print("From topic [");
  Serial.print(topic);
  Serial.print("] Message arrived [");
  Serial.print(value);
  Serial.println("]");

  // This is the routing logic for the MQTT messages
  if (strcmp(topic, "h2overview/H2O-12345/valve_state") == 0) {
    feature.remote_valve_control(value);
  } else if (strcmp(topic, "h2overview/H2O-12345/leak_scan") == 0) {
    Serial.println("Scanning for leaks...");
  } else {
    Serial.println("Invalid topic");
  }

  Serial.print("Callback done from topic [");
  Serial.print(topic);
  Serial.print("]");
}

// =============================================================================
// ============================== SETUP AND LOOP ===============================
// =============================================================================

void setup() {
  Serial.begin(9600);
  delay(5000);

  hardware.initialize_pins();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  // TODO: make the topic dynamic
  client.subscribe("h2overview/H2O-12345/#");
  client.setCallback(callback);
}

void loop() {
  if (!client.connected())
    reconnect_mqtt();
  client.loop();

  feature.local_valve_control();

  int leak_flag = firebase.is_leak_scanning();
  if (leak_flag) {
    int res = scan_leak();
    firebase.set_leak_detected(res);
    firebase.set_leak_scanning(0);
  }

  // TODO: Remove the delay once done with testing
  // delay(5000);
}
