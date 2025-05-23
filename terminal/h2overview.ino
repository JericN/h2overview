#include <ESP8266WiFi.h>
#include <PubSubClient.h>
// #include <WiFi.h>

#include "feature.h"
#include "hardware.h"
#include "server.h"

Hardware hardware;
MQTTserver server(hardware);
Feature feature(hardware, server);

#define DEVICE_ID "H2O-12345"

bool is_scheduled_health_scan = false;
int health_scan_schedule_count = 0;
ScheduleEntry health_scan_schedules[50];

bool is_scheduled_valve_control = false;
int valve_control_schedule_count = 0;
ScheduleEntry valve_control_schedules[50];

// const char* ssid = "Rex Judicata";
// const char* password = "93291123aaaA.";
// const char* ssid = "Jeric";
// const char* password = "12121212";
const char* ssid = "..";
const char* password = "qqwweerr";
//const char* ssid = "Raspberry";
//const char* password = "54321edcba";
//const char* ssid = "4studentstoo";
//const char* password = "W1F14students";
// const char* ssid = "DragonsDen";
// const char* password = "iotcup2024fusrodah";
// const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* mqtt_server = "test.mosquitto.org";

int timezone = 8 * 3600;
int dst = 0;

void setup_wifi() {
  int led_state = 0;
  delay(1000);
  Serial.println();
  Serial.print("[LOGS] Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (led_state == 0) {
      hardware.set_connection_led_state(HIGH);
      led_state = 1;
    } else {
      hardware.set_connection_led_state(LOW);
      led_state = 0;
    }
  }

  randomSeed(micros());
  Serial.println("\n[LOGS] WiFi connected");
  Serial.print("[LOGS] IP address: ");
  Serial.println(WiFi.localIP());

  hardware.set_connection_led_state(HIGH);
}

// =============================================================================
// ============================== MQTT CALLBACK ================================
// =============================================================================

void callback(char* topic, byte* payload, unsigned int length) {
  // copy the payload to a string
  char value[length + 1];
  for (int i = 0; i < length; i++) {
    value[i] = (char)payload[i];
  }
  value[length] = '\0';

  Serial.print("[LOGS] From topic [");
  Serial.print(topic);
  Serial.print("] Message arrived [");
  Serial.print(value);
  Serial.println("]");

  // This is the routing logic for the MQTT messages
  if (strcmp(topic, "h2overview/H2O-12345/is_valve_open") == 0) {
    feature.remote_valve_control(value);

  } else if (strcmp(topic, "h2overview/H2O-12345/is_manual_leak_scan_running") == 0) {
    feature.manual_leak_scan(value);

  } else if (strcmp(topic, "h2overview/H2O-12345/scheduled_valve_control") == 0) {
    feature.set_scheduled_valve_control(value);
    for (int i = 0; i < valve_control_schedule_count; i++) {
      int num_days = valve_control_schedules[i].num_days;
      int* days = valve_control_schedules[i].days;
      int start_time = valve_control_schedules[i].start_time;
      int end_time = valve_control_schedules[i].end_time;

      Serial.print("[OUTPUT] Scheduled valve control: ");
      Serial.print("Num Day: ");
      Serial.print(num_days);
      Serial.print(" Day: ");
      for (int j = 0; j < num_days; j++) {
        Serial.print(days[j]);
        Serial.print(" ");
      }
      Serial.print(" Start time: ");
      Serial.print(start_time);
      Serial.print(" End time: ");
      Serial.println(end_time);
    }

  } else if (strcmp(topic, "h2overview/H2O-12345/scheduled_health_scan") == 0) {
    feature.set_scheduled_health_scan(value);
    for (int i = 0; i < health_scan_schedule_count; i++) {
      int num_days = health_scan_schedules[i].num_days;
      int* days = health_scan_schedules[i].days;
      int start_time = health_scan_schedules[i].start_time;
      int end_time = health_scan_schedules[i].end_time;

      Serial.print("[OUTPUT] Scheduled health scan: ");
      Serial.print("Num Day: ");
      Serial.print(num_days);
      Serial.print(" Day: ");
      for (int j = 0; j < num_days; j++) {
        Serial.print(days[j]);
        Serial.print(" ");
      }
      Serial.print(" Start time: ");
      Serial.print(start_time);
      Serial.print(" End time: ");
      Serial.println(end_time);
    }

  } else if (strcmp(topic, "h2overview/H2O-12345/is_alive") == 0) {
    feature.is_alive();

  } else {
    Serial.println("[ERROR] Invalid topic");
  }

  Serial.print("[LOGS] Callback done from topic [");
  Serial.print(topic);
  Serial.println("]");
}

// =============================================================================
// ============================== SETUP AND LOOP ===============================
// =============================================================================

void setup() {
  delay(3000);
  Serial.begin(9600);
  delay(3000);

  // Initialize hardware pins
  Serial.println("[LOGS] Initializing hardware pins...");
  hardware.initialize_pins();
  Serial.println("[LOGS] Hardware pins initialized");

  // Setup WiFi
  Serial.println("[LOGS] Setting up WiFi...");
  setup_wifi();
  Serial.println("[LOGS] WiFi setup complete");

  // Setup MQTT
  Serial.println("[LOGS] Setting up MQTT...");
  server.setup_mqtt(mqtt_server, callback);
  Serial.println("[LOGS] MQTT setup complete");

  // Setup Time
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for Internet time");

  while (!time(nullptr)) {
    Serial.print("*");
    delay(1000);
  }

  Serial.println("\n[LOGS] Time response....OK");
}

void loop() {
  server.loop();
  Serial.println("looping...");

  feature.local_valve_control();
  feature.check_scheduled_valve_control();
  feature.check_scheduled_health_scan();
  feature.send_waterflow_data();
  feature.send_pressure_data();

  delay(200);
}

// for (int i = 0; i < valve_control_schedule_count; i++) {
//   int day = valve_control_schedules[i].day;
//   int start_time = valve_control_schedules[i].start_time;
//   int end_time = valve_control_schedules[i].end_time;
//   Serial.print("[LOGS] Scheduled valve control: ");
//   Serial.print("Day: ");
//   Serial.print(day);
//   Serial.print(" Start time: ");
//   Serial.print(start_time);
//   Serial.print(" End time: ");
//   Serial.println(end_time);
// }