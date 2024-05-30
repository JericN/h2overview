#ifndef FEATURE_H
#define FEATURE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "hardware.h"
#include "server.h"

// Solenoid state
#define CLOSE 0
#define OPEN 1

// Sensor threshold error
#define PRESSURE_LEAK_THRESHOLD 4
#define FLOW_LEAK_THRESHOLD 1

// Scan settings
// #define QUICK_LEAK_SCAN_TIME 30 * 1000
// #define RECOMMENDED_LEAK_SCAN_TIME 15 * 60 * 1000
// #define LONG_LEAK_SCAN_FLOW_TIME 5 * 60 * 1000
// #define LONG_LEAK_SCAN_PRESSURE_TIME 25 * 60 * 1000
// #define LONG_LEAK_SCAN_REPEATE 2

#define QUICK_LEAK_SCAN_TIME 30 * 1000
#define RECOMMENDED_LEAK_SCAN_TIME 1 * 60 * 1000
#define LONG_LEAK_SCAN_FLOW_TIME 30 * 1000
#define LONG_LEAK_SCAN_PRESSURE_TIME 1 * 60 * 1000
#define LONG_LEAK_SCAN_REPEATE 2

typedef struct {
  int start_time;
  int end_time;
  int days[20];
  int num_days;
} ScheduleEntry;

// Global variables defined in h2overview.ino
extern bool is_scheduled_health_scan;
extern int health_scan_schedule_count;
extern ScheduleEntry health_scan_schedules[50];
extern bool is_scheduled_valve_control;
extern int valve_control_schedule_count;
extern ScheduleEntry valve_control_schedules[50];

class Feature {
 public:
  Feature(Hardware& hardware, MQTTserver& server);

  void local_valve_control();
  void remote_valve_control(char* value);
  void set_scheduled_health_scan(char* value);
  void set_scheduled_valve_control(char* value);
  void manual_leak_scan(char* value);
  void check_scheduled_valve_control();
  void check_scheduled_health_scan();
  void is_alive();

  void send_waterflow_data();
  void send_pressure_data();

 private:
  Hardware& hardware;
  MQTTserver& server;
  static unsigned long waterflow_lastTime;
  static unsigned long pressure_lastTime;

  int waterflow_leak_scanner(int duration);
  int pressure_leak_scanner(int duration);
  int deep_leak_scanner(int repeat);
};
#endif