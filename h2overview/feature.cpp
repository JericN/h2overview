#include "feature.h"

unsigned long Feature::waterflow_lastTime = 0;

// Constructor implementation
Feature::Feature(Hardware& hardware, MQTTserver& server) : hardware(hardware), server(server) {}

void Feature::local_valve_control() {
  Serial.println("[LOGS] Start local valve control");

  int buttonPress = hardware.get_solenoid_button_press();
  if (buttonPress) {
    int state = hardware.get_solenoid_state();
    hardware.set_solenoid_state(!state);
    hardware.set_led_state(!state);
    server.set_valve_state(!state);
  }

  Serial.println("[LOGS] Done local valve control");
}

void Feature::remote_valve_control(char* value) {
  Serial.print("[LOGS] Start remote valve control: ");
  Serial.println(value);

  DynamicJsonDocument doc(200);
  deserializeJson(doc, value);

  // TODO: check if its "True" or "true" or "1"
  int remoteState = !strcmp(doc["value"], "true");
  int localState = hardware.get_solenoid_state();

  if (remoteState != localState) {
    hardware.set_solenoid_state(remoteState);
    hardware.set_led_state(remoteState);
  }

  Serial.println("[LOGS] Done remote valve control");
}

int Feature::waterflow_leak_scanner(int duration) {
  Serial.println("[LOGS] Start quick leak scan");

  hardware.set_solenoid_state(OPEN);
  delay(5000);

  float start_time = millis();
  bool is_leak_detected = false;
  while (millis() - start_time < duration - 5000) {
    if (hardware.read_waterflow_rate(1000) > 1) {
      is_leak_detected = true;
      break;
    }
  }

  if (is_leak_detected) {
    Serial.println("[LOGS] `Big leak` detected in quick scan!");
    hardware.set_solenoid_state(CLOSE);
    return 1;
  } else {
    Serial.println("[LOGS] `No leak` detected in quick scan!");
    return 0;
  }
}

int Feature::pressure_leak_scanner(int duration) {
  Serial.println("[LOGS] Start recommended leak scan");

  float initial_pressure = hardware.read_average_water_pressure(5000);
  hardware.set_solenoid_state(CLOSE);

  int start_time = millis();
  while (millis() - start_time < duration - 10000) {
    // TODO: if (get_interrupt()) return 0;
    delay(1000);
  }

  float final_pressure = hardware.read_average_water_pressure(5000);
  bool is_leak_detected = initial_pressure - final_pressure > PRESSURE_LEAK_THRESHOLD;

  // TODO: classify leak as big or small
  if (is_leak_detected) {
    Serial.println("[LOGS] `leak` detected in recommended scan!");
    return 1;
  } else {
    Serial.println("[LOGS] `No leak` detected in recommended scan!");
    hardware.set_solenoid_state(OPEN);
    return 0;
  }
}

int Feature::deep_leak_scanner(int repeat) {
  Serial.println("[LOGS] Start long leak scan");

  int flow_leaks[20];
  int pressure_leaks[20];

  for (int i = 0; i < repeat; i++) {
    flow_leaks[i] = waterflow_leak_scanner(LONG_LEAK_SCAN_FLOW_TIME);
    pressure_leaks[i] = pressure_leak_scanner(LONG_LEAK_SCAN_PRESSURE_TIME);
  }

  // FIXME: right now we are just checking if any of the scans detected a leak
  bool is_leak_detected = false;
  for (int i = 0; i < repeat; i++) {
    if (flow_leaks[i] || pressure_leaks[i]) {
      is_leak_detected = true;
      break;
    }
  }

  if (is_leak_detected) {
    Serial.println("[LOGS] `Big leak` detected in long scan!");
    hardware.set_solenoid_state(CLOSE);
    return 1;
  } else {
    Serial.println("[LOGS] `No leak` detected in long scan!");
    return 0;
  }
}

int Feature::manual_leak_scan(char* value) {
  DynamicJsonDocument doc(200);
  deserializeJson(doc, value);

  const char* scan_type = doc["scan_type"];
  int is_active = strcmp(doc["value"], "true") == 0 ? 1 : 0;

  if (!is_active) {
    return -1;
  }

  int is_leak_detected = 0;
  if (strcmp(scan_type, "quick") == 0) {
    is_leak_detected = waterflow_leak_scanner(QUICK_LEAK_SCAN_TIME);
  } else if (strcmp(scan_type, "recommended") == 0) {
    is_leak_detected = pressure_leak_scanner(RECOMMENDED_LEAK_SCAN_TIME);
  } else if (strcmp(scan_type, "long") == 0) {
    is_leak_detected = deep_leak_scanner(LONG_LEAK_SCAN_REPEATE);
  } else {
    Serial.println("[LOGS] Invalid scan type");
    is_leak_detected = -1;
  }

  server.set_manual_scan_result(is_leak_detected);
  server.set_manual_leak_scan_running(0);
}

void Feature::check_scheduled_health_scan() {
  if (!is_scheduled_health_scan) {
    return;
  }

  Serial.println("[LOGS] Start checking for scheduled health scan");

  time_t now = time(nullptr);
  struct tm* utc_tm = gmtime(&now);
  int utc_now_minute = utc_tm->tm_hour * 60 + utc_tm->tm_min;
  int utc_now_day = utc_tm->tm_wday;

  // check if the current time is in the schedule
  for (int i = 0; i < health_scan_schedule_count; i++) {
    if (health_scan_schedules[i].day == utc_now_day && health_scan_schedules[i].start_time == utc_now_minute) {
      Serial.println("[LOGS] Leak scan is started due to scheduled health scan");
      server.set_automated_scan_running(1);
      int result = deep_leak_scanner(1);
      server.set_health_scan_result(result);
      server.set_automated_scan_running(0);
      break;
    }
  }

  Serial.println("[LOGS] Done scheduled health scan");
}

void Feature::check_scheduled_valve_control() {
  if (!is_scheduled_valve_control) {
    return;
  }

  Serial.println("[LOGS] Start checking for scheduled valve control");

  time_t now = time(nullptr);
  struct tm* utc_tm = gmtime(&now);
  int utc_now_minute = utc_tm->tm_hour * 60 + utc_tm->tm_min;
  int utc_now_day = utc_tm->tm_wday;

  // TODO: make this more efficient
  for (int i = 0; i < valve_control_schedule_count; i++) {
    if (valve_control_schedules[i].day == utc_now_day && valve_control_schedules[i].start_time == utc_now_minute) {
      Serial.println("[LOGS] Valve is closed due to scheduled valve control");
      hardware.set_solenoid_state(CLOSE);
      hardware.set_led_state(CLOSE);
      server.set_valve_state(CLOSE);
      break;
    }
  }

  // TODO: make this more efficient
  for (int i = 0; i < valve_control_schedule_count; i++) {
    if (valve_control_schedules[i].day == utc_now_day && valve_control_schedules[i].end_time == utc_now_minute) {
      Serial.println("[LOGS] Valve is opened due to scheduled valve control");
      hardware.set_solenoid_state(OPEN);
      hardware.set_led_state(OPEN);
      server.set_valve_state(OPEN);
      break;
    }
  }

  Serial.println("[LOGS] Done scheduled valve control");
}

void Feature::set_scheduled_valve_control(char* value) {
  Serial.print("[LOGS] Start setting scheduled valve control: ");
  Serial.println(value);

  DynamicJsonDocument doc(1000);
  deserializeJson(doc, value);
  is_scheduled_valve_control = doc["is_enabled"];

  JsonArray schedules = doc["schedules"];
  int i = 0;
  for (JsonVariant schedule : schedules) {
    valve_control_schedules[i].day = schedule["day"];
    valve_control_schedules[i].start_time = schedule["start_time"];
    valve_control_schedules[i].end_time = schedule["end_time"];
    i++;
  }

  valve_control_schedule_count = i;
  Serial.println("[LOGS] Done setting scheduled valve control");
}

void Feature::set_scheduled_health_scan(char* value) {
  Serial.print("[LOGS] Start setting scheduled health scan: ");
  Serial.println(value);

  DynamicJsonDocument doc(200);
  deserializeJson(doc, value);
  is_scheduled_health_scan = doc["is_enabled"];

  JsonArray schedules = doc["schedules"];
  int i = 0;
  for (JsonVariant schedule : schedules) {
    health_scan_schedules[i].day = schedule["day"];
    health_scan_schedules[i].start_time = schedule["start_time"];
    health_scan_schedules[i].end_time = -1;
    i++;
  }

  health_scan_schedule_count = i;
  Serial.println("[LOGS] Done setting scheduled health scan");
}

// FIXME:
// FIXME:
void Feature::send_waterflow_data() {
  Serial.println("[LOGS] Start sending waterflow data");

  time_t now = time(nullptr);
  struct tm* utc_tm = gmtime(&now);
  if (millis() - Feature::waterflow_lastTime < 60000 && utc_tm->tm_min == 00) {
    return;
  }

  Feature::waterflow_lastTime = millis();
  float total_water = hardware.read_cummulative_water();
  Waterflow waterflow_data;
  waterflow_data.timestamp = now;
  waterflow_data.value = total_water;
  server.send_waterflow(waterflow_data);

  Serial.println("[LOGS] Done sending waterflow data");
}

void Feature::is_alive() {
  Serial.println("[LOGS] I am alive");
  server.set_alive();
}