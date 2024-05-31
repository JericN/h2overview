#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
// #include <WiFi.h>
#include <PubSubClient.h>
#include "hardware.h"

// Device serial number
#define DEVICE_SERIAL "H2O-12345"

typedef struct {
  unsigned long timestamp;  // Epoch time
  float value;
} Waterflow;

typedef struct {
  unsigned long timestamp;  // Epoch time
  float value;
} Pressure;


class MQTTserver {
 public:
  MQTTserver(Hardware& hardware);

  void set_valve_state(int state);
  void set_manual_leak_scan_running(int state);
  void set_automated_scan_running(int state);

  void set_manual_scan_result(int result, const char* scan_type);
  void set_health_scan_result(int result);
  
  void send_waterflow(Waterflow flow);
  void send_pressure(Pressure pressure);
  void setup_mqtt(const char* mqtt_server, void (*callback)(char*, uint8_t*, unsigned int));

  void set_alive();

  void loop();

 private:
  void reconnect();
  Hardware& hardware;
  WiFiClient espClient;
  PubSubClient client;
};

#endif  // SERVER_H
