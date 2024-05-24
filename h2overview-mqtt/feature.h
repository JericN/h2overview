#ifndef FEATURE_H
#define FEATURE_H

#include <Arduino.h>
#include "Physical.h"
class Feature {
 public:
  Feature(Physical& hardware);

  void local_valve_control();
  void remote_valve_control(int remoteState);

 private:
  Physical& hardware;
};

#endif