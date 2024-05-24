#include "feature.h"

// Constructor implementation
Feature::Feature(Hardware& hardware, FirebaseServer& firebase) : hardware(hardware), firebase(firebase) {}

void Feature::local_valve_control(){
  int buttonPress = hardware.get_solenoid_button_press();
  if (buttonPress) {
    int state = hardware.get_solenoid_state();
    hardware.set_solenoid_state(!state);
    hardware.set_led_state(!state);
    Serial.print("Valve state changed to: ");
    Serial.println(!state);
  }
}

void Feature::remote_valve_control(int remoteState) {
  int localState = hardware.get_solenoid_state();

  if (remoteState != localState) {
    hardware.set_solenoid_state(remoteState);
    hardware.set_led_state(remoteState);
    Serial.print("Valve state changed to: ");
    Serial.println(remoteState);
  }
}
