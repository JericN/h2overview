#ifndef FIREBASE_SERVER_H
#define FIREBASE_SERVER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266Firebase.h>


#define REFERENCE_URL "https://h2overview-iot-default-rtdb.asia-southeast1.firebasedatabase.app/"
Firebase firebase(REFERENCE_URL);

String device = "device1";
String pref_path = "devices/" + device + "/preference";
String flag_path = "devices/" + device + "/flags";
String data_path = "devices/" + device + "/data";

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

bool get_preference1();
bool get_preference2();
SmallLeak get_small_leak_preference();

bool get_valve_state();
bool get_big_leak_scanner();
bool get_small_leak_scanner();
bool get_big_leak_detected();
bool get_small_leak_detected();

void set_valve_state(bool state);
void set_big_leak_scanner(bool state);
void set_small_leak_scanner(bool state);
void set_big_leak_detected(bool state);
void set_small_leak_detected(bool state);

void send_waterflow(Waterflow waterflow);



#endif // FIREBASE_SERVER_H