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

#include "ProfizumoSupersonic.h"
#include <Arduino.h>

namespace profizumo
{
ProfizumoSupersonic::ProfizumoSupersonic(long minTimeBetweenPulses_us_, long maxTimeBetweenPulses_us_) : minTimeBetweenPulses_us{minTimeBetweenPulses_us_}, maxTimeBetweenPulses_us{maxTimeBetweenPulses_us_}
{
  
}
void ProfizumoSupersonic::Init()
{
  pinMode(SUPERSONIC_TRIGGER_PIN, OUTPUT);
  pinMode(SUPERSONIC_ECHO_PIN, INPUT);
  // Configure hardware interrupt when echo pin changes state
  // Echo pin 11 -> PCINT7, which is at PCMSK0, Bit 7
  // Unset global interrupt enable bits (I) in the status register (SREG), such that
  // no interrupts will be thrown while changing configuration
  cli();
  // Set PCIE0 bit in the bit change interrupt control register (PCICR)
  PCICR |= (1 << PCIE0);
  // Set PCINT7 bt (=bit 7) in the bit change enable mask (PCMSK). The first
  // 8 bits are in PCMSK0, the next 8 in PCMSK1,...
  PCMSK0 |= (1 << PCINT7);
  // set global interrupt enable bits (I) in the status register (SREG), such that
  // interrupts are enabled again
  sei();
}

void ProfizumoSupersonic::OnInterrupt()
{  
  bool echo = digitalRead(SUPERSONIC_ECHO_PIN);
  if(!lastEcho && echo)
  {
    // start of the echo pulse
    echoEnd_us = 0;                                 // Clear the end time
    echoStart_us = micros();                        // Save the start time
  }
  else if(lastEcho && !echo && echoStart_us != 0)
  {
    // echo received
    echoEnd_us = micros();                          // Save the end time
    echoDuration_us = echoEnd_us - echoStart_us;        // Calculate the pulse duration
  }
  lastEcho = echo;
}
void ProfizumoSupersonic::Run()
{
  long time_us = micros();
  if((echoEnd_us != 0 && time_us - echoStart_us >= minTimeBetweenPulses_us) // last measurement finished
    || time_us - echoStart_us >= maxTimeBetweenPulses_us) // last measurement failed
  {
    // echoStart will be overwritten by interrupt. We nevertheless set it here to prevent triggering
    // multiple pulses before OnInterrupt was called...
    echoStart_us = time_us;
    echoEnd_us = 0;
    // A measurement is triggered by keeping the trigger pin at least 10us up
    digitalWrite(SUPERSONIC_TRIGGER_PIN, HIGH);
    delayMicroseconds(12);
    digitalWrite(SUPERSONIC_TRIGGER_PIN, LOW);
  }
}
int16_t ProfizumoSupersonic::GetLastDistance_mm()
{
  cli();
  long duration = echoDuration_us;
  sei();
  
  if(duration<=0)
    return -1;
  int16_t distance_mm = duration/5.82;
  // Überprüfung ob gemessener Wert innerhalb der zulässingen Entfernung liegt
  if (distance_mm >= maxDistance_mm || distance_mm <= minDistance_mm) 
    return -1;
  else
    return distance_mm;
}
}
