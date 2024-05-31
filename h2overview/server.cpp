#include "server.h"

MQTTserver::MQTTserver() : client(espClient) {
  // Constructor implementation (if needed)
}

void MQTTserver::setup_mqtt(const char* mqtt_server, void (*callback)(char*, uint8_t*, unsigned int)) {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.subscribe("h2overview/H2O-12345/#");
}

void MQTTserver::reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("[LOGS] Attempting MQTT connection...");
    // Create a random client ID
    char clientId[50];
    sprintf(clientId, "ESP8266Client-%04X", random(0xffff));
    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.println("[LOGS] MQTT connected");
      client.subscribe("h2overview/H2O-12345/#");
    } else {
      Serial.print("[ERROR] failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
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

void MQTTserver::set_manual_scan_result(int result, const char* scan_type) {
  if (!client.connected()) {
    reconnect();
  }
  const char* res;
  if (result == 0) {
    res = "no_leak";
  } else if (result == 1) {
    res = "leak_detected";
  } else {
    res = "failed";
  }

  char* payload = (char*)malloc(100);
  if (payload == nullptr) {
    return;
  }
  snprintf(payload, 100, "{\"scan_type\": \"%s\", \"result\": \"%s\"}", scan_type, res);
  client.publish("h2overview/out/H2O-12345/manual_results", payload);
  free(payload);
}

void MQTTserver::set_health_scan_result(int result) {
  if (!client.connected()) {
    reconnect();
  }

  const char* res;
  if (result == 0) {
    res = "no_leak";
  } else if (result == 1) {
    res = "leak_detected";
  } else {
    res = "failed";
  }

  char* payload = (char*)malloc(100);
  if (payload == nullptr) {
    return;
  }
  snprintf(payload, 100, "{\"scan_type\": \"long\", \"result\": \"%s\"}", res);
  client.publish("h2overview/out/H2O-12345/auto_results", payload);
  free(payload);
}

void MQTTserver::send_waterflow(Waterflow flow) {
  if (!client.connected()) {
    reconnect();
  }

  char* payload = (char*)malloc(100);
  if (payload == nullptr) {
    return;
  }

  snprintf(payload, 100, "{\"timestamp\": %lu, \"value\": %f}", flow.timestamp, flow.value);
  client.publish("h2overview/out/H2O-12345/waterflow", payload);
  free(payload);
}

void MQTTserver::send_pressure(Pressure pressure) {
  if (!client.connected()) {
    reconnect();
  }

  char* payload = (char*)malloc(100);
  if (payload == nullptr) {
    return;
  }

  snprintf(payload, 100, "{\"timestamp\": %lu, \"value\": %f}", pressure.timestamp, pressure.value);
  client.publish("h2overview/out/H2O-12345/pressure", payload);
  free(payload);
}

void MQTTserver::set_alive() {
  if (!client.connected()) {
    reconnect();
  }
  client.publish("h2overview/out/H2O-12345/is_alive", "1");
}

void MQTTserver::loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
