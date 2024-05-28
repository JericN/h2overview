#include "server.h"

MQTTserver::MQTTserver() : client(espClient) {
  // Constructor implementation (if needed)
}

void MQTTserver::setup_mqtt(const char* mqtt_server, void (*callback)(char*, uint8_t*, unsigned int)) {
  client.setServer(mqtt_server, 1883);
  client.subscribe("h2overview/H2O-12345/#");
  client.setCallback(callback);
}

void MQTTserver::reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    char clientId[50];
    sprintf(clientId, "ESP8266Client-%04X", random(0xffff));
    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.println("connected");
      // Subscribe to the topic
      client.subscribe("h2overview/H2O-12345/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void MQTTserver::set_valve_state(int state) {
  if (!client.connected()) {
    reconnect();
  }
  client.publish("h2overview/out/H2O-12345/is_valve_open", String(state).c_str());
}

void MQTTserver::set_manual_leak_scan_running(int state) {
  if (!client.connected()) {
    reconnect();
  }
  client.publish("h2overview/out/H2O-12345/is_manual_leak_scan_running", String(state).c_str());
}

void MQTTserver::set_automated_scan_running(int state) {
  if (!client.connected()) {
    reconnect();
  }
  client.publish("h2overview/out/H2O-12345/is_automated_scan_running", String(state).c_str());
}

void MQTTserver::set_scan_result(int result) {
  if (!client.connected()) {
    reconnect();
  }
  client.publish("h2overview/out/H2O-12345/manual_results", String(result).c_str());
}


void MQTTserver::set_health_scan_result(int result) {
  if (!client.connected()) {
    reconnect();
  }
  client.publish("h2overview/out/H2O-12345/auto_results", String(result).c_str());
}


void MQTTserver::send_waterflow(Waterflow flow) {
  if (!client.connected()) {
    reconnect();
  }
  //convert WaterFlow to string
  char* payload = (char*)malloc(100);
  sprintf(payload, "{\"timestamp\": %lu, \"value\": %f}", flow.timestamp, flow.value);
  Serial.println(payload);
  client.publish("h2overview/out/H2O-12345/waterflow", payload);
  free(payload);
}



void MQTTserver::loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
