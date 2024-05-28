#ifndef FEATURE_H
#define FEATURE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "hardware.h"
#include "server.h"

#define CLOSE 0
#define OPEN 1

extern bool is_health_scan;
extern unsigned int health_scan_time;
extern bool is_scheduled_valve_control;
extern unsigned int scheduled_valve_start;
extern unsigned int scheduled_valve_end;



class Feature {
 public:
  Feature(Hardware& hardware, MQTTserver& server);

  void local_valve_control();
  void remote_valve_control(char* value);
  void set_health_scan(char* value);
  void set_scheduled_valve_control(char* value);
  void check_scheduled_valve_control();
  void send_waterflow_data();
  int manual_leak_detection();
  int scan_leak();
  int small_leak_scan(char* value);
  int big_leak_scan(char* value);

 private:
  Hardware& hardware;
  MQTTserver& server;
  static unsigned long waterflow_lastTime;

};
#endif