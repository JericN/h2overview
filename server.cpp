#include "server.h"
#include <ESP8266Firebase.h>

#define REFERENCE_URL "https://h2overview-iot-default-rtdb.asia-southeast1.firebasedatabase.app/"
Firebase firebase(REFERENCE_URL);
String device = "device1";
String pref_path = "devices/" + device + "/preference";
String flag_path = "devices/" + device + "/flags";
String data_path = "devices/" + device + "/data"

typedef struct
{
    unsigned long timestamp; // Epoch time
    float value;
} Waterflow;


typedef struct {
    int duration;
    bool auto_detection;
    unsigned long start_time;
    unsigned long end_time;
} SmallLeak;


// =============================================================
// ========================= PREFENCES =========================
// =============================================================

bool get_preference1(){
    return firebase.getBool(pref_path + "/is_preference1");
}

bool get_preference2(){
    return firebase.getBool(pref_path + "/is_preference2");
}

SmallLeak get_small_leak_preference(){
    String path = pref_path + "/leak_small";
    String data = firebase.getString(path);

    // Deserialize the data.
    // Consider using Arduino Json Assistant
    // https://arduinojson.org/v6/assistant/
    const size_t prefs = JSON_OBJECT_SIZE(4) + 50;
    DynamicJsonDocument doc(prefs);

    deserializeJson(prefs, data);

    // Store the deserialized data.
    unsigned long duration = prefs["duration"];
    bool auto = prefs["auto"];
    unsigned long start_time = prefs["start_time"];
    unsigned long end_time = prefs["end_time"];

    SmallLeak small_leak_pref = {duration, auto, start_time, end_time};
    return small_leak_pref;
}

// ==============================================================
// =========================== STATES ===========================
// ==============================================================

bool get_valve_state(){
    return firebase.getBool(flag_path + "/valve_state");
}

bool get_big_leak_scanner(){
    return firebase.getBool(flag_path + "/big_leak_scanner");
}

bool get_small_leak_scanner(){
    return firebase.getBool(flag_path + "/small_leak_scanner");
}

bool get_big_leak_detected(){
    return firebase.getBool(flag_path + "/big_leak_detected");
}

bool get_small_leak_detected(){
    return firebase.getBool(flag_path + "/small_leak_detected");
}


void set_valve_state(bool state){
    if(firebase.setBool(flag_path + "/valve_state", state)){
        Serial.println("Valve State set to Firebase");
    }else{
        Serial.println("Failed to set Valve State to Firebase");
    }
}

void set_big_leak_scanner(bool state){
    if(firebase.setBool(flag_path + "/big_leak_scanner", state)){
        Serial.println("Big Leak Scanner set to Firebase");
    }else{
        Serial.println("Failed to set Big Leak Scanner to Firebase");
    }
}

void set_small_leak_scanner(bool state){
    if(firebase.setBool(flag_path + "/small_leak_scanner", state)){
        Serial.println("Small Leak Scanner set to Firebase");
    }else{
        Serial.println("Failed to set Small Leak Scanner to Firebase");
    }
}

void set_big_leak_detected(bool state){
    if(firebase.setBool(flag_path + "/big_leak_detected", state)){
        Serial.println("Big Leak Detected flag set to Firebase");
    }else{
        Serial.println("Failed to set Big Leak Detected flag to Firebase");
    }
}

void set_small_leak_detected(bool state){
    if(firebase.setBool(flag_path + "/small_leak_detected", state)){
        Serial.println("Small Leak Detected flag set to Firebase");
    }else{
        Serial.println("Failed to set Small Leak Detected flag to Firebase");
    }
}


// =============================================================
// ============================= DATA ==========================
// =============================================================

void send_waterflow(Waterflow data){
    String key = String(data.timestamp);
    String path = data_path + "/water_flow/" + key;

    if(firebase.pushFloat(path, data.value)){
        Serial.println("Waterflow data sent to Firebase");
    }else{
        Serial.println("Failed to send Waterflow data to Firebase");
    }
}
