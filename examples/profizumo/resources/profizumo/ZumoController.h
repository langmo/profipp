#ifndef ZUMOCONTROLLER_H
#define ZUMOCONTROLLER_H
#pragma once

#include "zumoCommunication.h"
#include <stdint.h>

namespace profizumo
{
class ZumoController
{
public:
  ZumoController();
  void ProcessInput(ZumoInput command, int16_t value);
  void Run();
  /**
   * Initializes the controller.
   * outputProcessor is a callback which gets notified whenever sensory data changes.
   */
  void Init(void (*outputProcessor)(ZumoOutput, int16_t));
private:
  int leftSpeed{0};
  int rightSpeed{0};
  void (*outputProcessor)(ZumoOutput, int16_t) {nullptr};
};
}
#endif
