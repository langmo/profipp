#ifndef ZUMODEVICE_H
#define ZUMODEVICE_H
#pragma once

#include "zumocom.h"
#include <stdint.h>

namespace profizumo
{
class ZumoDevice
{
public:
  ZumoDevice();
  void ProcessInput(ZumoInput command, int16_t value);
  void Run();
  /**
   * Initializes the controller.
   * outputProcessor is a callback which gets notified whenever sensory data changes.
   */
  void Init(void (*outputProcessor)(ZumoOutput, int16_t));
private:
  //Outputs
  int16_t leftSpeed{0};
  int16_t rightSpeed{0};
  void (*outputProcessor)(ZumoOutput, int16_t) {nullptr};
};
}
#endif
