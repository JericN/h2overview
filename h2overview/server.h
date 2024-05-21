#ifndef FIREBASE_SERVER_H
#define FIREBASE_SERVER_H

#include <Arduino.h>
#include <ArduinoJson.h>
// #include <ESP8266Firebase.h>
#include <ESP32Firebase.h>

// Firebase endpoint
#define REFERENCE_URL "https://h2overview-iot-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Device serial number
#define DEVICE_SERIAL "device_serial"

#define SUCCESS 200
#define FAILURE 400

typedef struct {
  unsigned long timestamp;  // Epoch time
  float value;
} Waterflow;

typedef struct {
  bool scheduled;
  unsigned long duration;
  unsigned long start_time;
  unsigned long end_time;
} LeakPref;

class FirebaseServer {
 public:
  FirebaseServer();

  // Preferences
  LeakPref get_small_leak_preference();

  // Flags
  bool get_valve_state();
  bool is_leak_scanning();
  bool is_leak_detected();
  void set_valve_state(bool state);
  void set_leak_scanning(bool state);
  void set_leak_detected(bool state);

  // Data
  void send_waterflow(Waterflow data);

 private:
  static Firebase firebase;
  static String pref_path;
  static String flag_path;
  static String data_path;
};

#endif  // FIREBASE_SERVER_H
