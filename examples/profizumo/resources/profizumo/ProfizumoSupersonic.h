// Copyright 2023 Moritz Lang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef PROFIZUMOSUPERSONIC_H
#define PROFIZUMOSUPERSONIC_H
#pragma once

#include <stdint.h> // int16_t

#define SUPERSONIC_ECHO_PIN 11 // Echo pin of the ultrasound distance sensor
#define SUPERSONIC_TRIGGER_PIN 12 // Trigger pin of the ultrasound distance sensor

namespace profizumo
{
class ProfizumoSupersonic
{
public:
  ProfizumoSupersonic(long minTimeBetweenPulses_us = 16000, long maxTimeBetweenPulses_us = 32000);
  /**
   * Should be called before device is used.
   */
  void Init();
  /**
   * Should be called regularly. Triggers new pulses. However, measureing pulse length is done in OnInterrupt().
   */
  void Run();
  /**
   * Should be called when a hardware interrupt on the echo pin is triggered, e.g. in 
   * ISR(PCINT0_vect).
   * Measures pulse length.
   */
  void OnInterrupt();
  /**
   * Returns distance in mm. Returns -1 if no measurement was done, yet,
   * or if the last measurement was invalid.
   */
  int16_t GetLastDistance_mm();
private:
  volatile long echoStart_us{0};                         // Records start of echo pulse in us
  volatile long echoEnd_us{0};                           // Records end of echo pulse in us
  volatile long echoDuration_us{0};                      // Duration - difference between end and start in us
  volatile bool lastEcho{false};
  long minTimeBetweenPulses_us; // us
  long maxTimeBetweenPulses_us; // us
  int16_t maxDistance_mm{3000};
  int16_t minDistance_mm{20};
};
}
#endif
