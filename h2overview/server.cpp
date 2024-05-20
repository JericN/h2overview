#include "server.h"

// Initialize static variable
Firebase FirebaseServer::firebase(REFERENCE_URL);
String FirebaseServer::pref_path = "devices/" + String(DEVICE_SERIAL) + "/preference";
String FirebaseServer::flag_path = "devices/" + String(DEVICE_SERIAL) + "/flags";
String FirebaseServer::data_path = "devices/" + String(DEVICE_SERIAL) + "/data";

FirebaseServer::FirebaseServer() {
  // Constructor
}

// =================================================================
// =========================== PREFENCES ===========================
// =================================================================

LeakPref FirebaseServer::get_small_leak_preference() {
  // Fetch the data from Firebase
  String data = firebase.getString(pref_path + "/leak_scan");

  // Deserialize the data
  JsonDocument doc;
  deserializeJson(doc, data);

  // Extract the deserialized data
  bool scheduled = doc["scheduled"];
  unsigned long duration = doc["duration"];
  unsigned long start_time = doc["start_time"];
  unsigned long end_time = doc["end_time"];

  return LeakPref{scheduled, duration, start_time, end_time};
}

// =================================================================
// ============================= FLAGS =============================
// =================================================================

bool FirebaseServer::get_valve_state() {
  return firebase.getInt(flag_path + "/valve_state");
}

bool FirebaseServer::is_leak_scanning() {
  return firebase.getInt(flag_path + "/leak_scanning");
}

bool FirebaseServer::is_leak_detected() {
  return firebase.getInt(flag_path + "/leak_detected");
}

void FirebaseServer::set_valve_state(bool state) {
  int res = firebase.setInt(flag_path + "/valve_state", state);
  if (res == FAILURE) {
    Serial.println("[ERROR] Firebase: Failed to set valve_state flag");
  }
}

void FirebaseServer::set_leak_scanning(bool state) {
  int res = firebase.setInt(flag_path + "/leak_scanning", state);
  if (res == FAILURE) {
    Serial.println("[ERROR] Firebase: Failed to set leak_scanning flag");
  }
}

void FirebaseServer::set_leak_detected(bool state) {
  int res = firebase.setInt(flag_path + "/leak_detected", state);
  if (res == FAILURE) {
    Serial.println("[ERROR] Firebase: Failed to set leak_detected flag");
  }
}

// ==================================================================
// ============================== DATA ==============================
// ==================================================================

void FirebaseServer::send_waterflow(Waterflow data) {
  int res = firebase.setFloat(data_path + "/water_flow/" + String(data.timestamp), data.value);
  if (res == FAILURE) {
    Serial.println("Failed to send Waterflow data to Firebase");
  }
}