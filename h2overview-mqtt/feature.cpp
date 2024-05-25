#include "feature.h"

unsigned long Feature::waterflow_lastTime = 0;

// Constructor implementation
Feature::Feature(Hardware& hardware, MQTTserver& server) : hardware(hardware), server(server) {}

void Feature::local_valve_control() {
  Serial.println("> In local valve control");
  int buttonPress = hardware.get_solenoid_button_press();
  if (buttonPress) {
    int state = hardware.get_solenoid_state();
    hardware.set_solenoid_state(!state);
    hardware.set_led_state(!state);
    server.set_valve_state(!state);
    Serial.print("Valve state changed to: ");
    Serial.println(!state);
  }
}

void Feature::check_scheduled_valve_control() {
  Serial.println("> In auto schedule checker");

  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  int now_minute = p_tm->tm_hour * 60 + p_tm->tm_min;
   
  // Serial.print("Now minute: ");
  // Serial.println(now_minute);
  // Serial.print("Scheduled valve start: ");
  // Serial.println(scheduled_valve_start);
  // Serial.print("Scheduled valve end: ");
  // Serial.println(scheduled_valve_end);

  if (!is_scheduled_valve_control) {
    return;
  }

  if (now_minute == scheduled_valve_start) {
    Serial.println("Closing valve");
    hardware.set_solenoid_state(CLOSE);
    hardware.set_led_state(CLOSE);
    server.set_valve_state(CLOSE);
  }
  if(now_minute == scheduled_valve_end) {
    Serial.println("Opening valve");
    hardware.set_solenoid_state(OPEN);
    hardware.set_led_state(OPEN);
    server.set_valve_state(OPEN);
  }

  Serial.print("Done checking scheduled valve control");
}

void Feature::remote_valve_control(char* value) {
  Serial.println("> In remote valve control");
  Serial.print("Value: ");
  Serial.println(value);
  int remoteState = strcmp(value, "True") == 0 ? 1 : 0;
  int localState = hardware.get_solenoid_state();

  if (remoteState != localState) {
    hardware.set_solenoid_state(remoteState);
    hardware.set_led_state(remoteState);
    Serial.print("Valve state changed to: ");
    Serial.println(remoteState);
  }
}

void Feature::set_health_scan(char* value) {
  DynamicJsonDocument doc(200);
  deserializeJson(doc, value);
  is_health_scan = doc["state"];
  health_scan_time = doc["start_time"];
  Serial.print("Health scan state changed to: ");
  Serial.println(is_health_scan);
  Serial.print("Health scan time changed to: ");
  Serial.println(health_scan_time);
}

void Feature::set_scheduled_valve_control(char* value) {
  DynamicJsonDocument doc(200);
  deserializeJson(doc, value);
  is_scheduled_valve_control = doc["state"];;
  scheduled_valve_start = doc["start_time"];;
  scheduled_valve_end = doc["end_time"];;
  Serial.print("Scheduled valve control state changed to: ");
  Serial.println(is_scheduled_valve_control);
  Serial.print("Scheduled valve start time changed to: ");
  Serial.println(scheduled_valve_start);
  Serial.print("Scheduled valve end time changed to: ");
  Serial.println(scheduled_valve_end);
}


void Feature::send_waterflow_data() {
  Serial.println("In waterflow data sender");

  if (millis() - Feature::waterflow_lastTime < 60000) {
    return;
  }
  Feature::waterflow_lastTime = millis();

  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);

  int now_minute = p_tm->tm_hour * 60 + p_tm->tm_min;
  Serial.print("Now minute: ");
  Serial.println(now_minute);
  // Serial.print("TARGET minute: ");
  // Serial.println(p_tm->tm_min);

  // if (p_tm->tm_min == 30) {
    float total_water = hardware.read_cummulative_water();
    Serial.print("mililiters: ");
    Serial.println(total_water); 

    Waterflow waterflow_data;
    waterflow_data.timestamp = now;
    waterflow_data.value = total_water;
    server.send_waterflow(waterflow_data);
  // }
}

