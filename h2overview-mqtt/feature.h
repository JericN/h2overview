#ifndef FEATURE_H
#define FEATURE_H

#include <Arduino.h>
#include "hardware.h"
#include "server.h"

class Feature {
 public:
  Feature(Hardware& hardware, FirebaseServer& firebase);

  void local_valve_control();
  void remote_valve_control(int remoteState);

 private:
  Hardware& hardware;
  FirebaseServer& firebase;
};

#endif