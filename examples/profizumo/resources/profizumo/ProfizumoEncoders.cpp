// Original work Copyright (c) 2015-2022 Pololu Corporation (www.pololu.com)
// Modified work Copyright 2023 Moritz Lang
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

#include "ProfizumoEncoders.h"
#include <FastGPIO.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#define LEFT_XOR   8
#define LEFT_B     IO_E2
#define RIGHT_XOR  7
#define RIGHT_B    23

namespace profizumo
{
void ProfizumoEncoders::OnLeftInterrupt()
{
  bool newLeftXOR = FastGPIO::Pin<LEFT_XOR>::isInputHigh();
  if(newLeftXOR == lastLeftXOR)
    return;

  bool newLeftB = FastGPIO::Pin<LEFT_B>::isInputHigh();
  bool newLeftA = newLeftXOR ^ newLeftB;

  countLeft += (newLeftA ^ lastLeftB) - (lastLeftA ^ newLeftB);

  if((lastLeftA ^ newLeftA) & (lastLeftB ^ newLeftB))
  {
      errorLeft = true;
  }

  lastLeftA = newLeftA;
  lastLeftB = newLeftB;
  lastLeftXOR = newLeftXOR;
}

void ProfizumoEncoders::OnRightInterrupt()
{
  bool newRightXOR = FastGPIO::Pin<RIGHT_XOR>::isInputHigh();
  if(newRightXOR == lastRightXOR)
    return;
  
  bool newRightB = FastGPIO::Pin<RIGHT_B>::isInputHigh();
  bool newRightA = newRightXOR ^ newRightB;

  countRight += (newRightA ^ lastRightB) - (lastRightA ^ newRightB);

  if((lastRightA ^ newRightA) & (lastRightB ^ newRightB))
  {
      errorRight = true;
  }

  lastRightA = newRightA;
  lastRightB = newRightB;
  lastRightXOR = newRightXOR;
}

void ProfizumoEncoders::Init()
{
  // Set the pins as pulled-up inputs.
  FastGPIO::Pin<LEFT_XOR>::setInputPulledUp();
  FastGPIO::Pin<LEFT_B>::setInputPulledUp();
  FastGPIO::Pin<RIGHT_XOR>::setInputPulledUp();
  FastGPIO::Pin<RIGHT_B>::setInputPulledUp();


  // Initialize the variables.
  lastLeftB = FastGPIO::Pin<LEFT_B>::isInputHigh();
  lastLeftA = FastGPIO::Pin<LEFT_XOR>::isInputHigh() ^ lastLeftB;
  countLeft = 0;
  errorLeft = 0;

  lastRightB = FastGPIO::Pin<RIGHT_B>::isInputHigh();
  lastRightA = FastGPIO::Pin<RIGHT_XOR>::isInputHigh() ^ lastRightB;
  countRight = 0;
  errorRight = 0;
  
  // Configure hardware interrupt when pin 8 cgabges state
  // pin 8 -> PCINT4, which is at PCMSK0, Bit 4
  // Unset global interrupt enable bits (I) in the status register (SREG), such that
  // no interrupts will be thrown while changing configuration
  cli();
  // Set PCIE0 bit in the bit change interrupt control register (PCICR)
  PCICR |= (1 << PCIE0);
  // Set PCINT4 bt (=bit 4) in the bit change enable mask (PCMSK). The first
  // 8 bits are in PCMSK0, the next 8 in PCMSK1,...
  PCMSK0 |= (1 << PCINT4);
  // set global interrupt enable bits (I) in the status register (SREG), such that
  // interrupts are enabled again
  sei();

  // Note: Right interrupt already attached...
}

int16_t ProfizumoEncoders::GetCountsLeft()
{
    return totalCountsLeft;
}

int16_t ProfizumoEncoders::GetCountsRight()
{
    return totalCountsRight;
}

int16_t ProfizumoEncoders::GetCountsPerSecondLeft()
{
    return countsPerSecondLeft;
}

int16_t ProfizumoEncoders::GetCountsPerSecondRight()
{
    return countsPerSecondRight;
}
void ProfizumoEncoders::Run()
{
  long time_us = micros();
  // Make local copies of counts while interrupts are disabled
  cli();
  int16_t deltaCountsRight = countRight;
  countRight = 0;
  int16_t deltaCountsLeft = countLeft;
  countLeft = 0;
  sei();
  
  totalCountsRight += deltaCountsRight;
  totalCountsLeft += deltaCountsLeft;
  if(lastRun_us == 0)
  {
    countsPerSecondRight = 0;
    countsPerSecondLeft = 0;
  }
  else
  {
    double timeFactor = 1000000.0 / (time_us-lastRun_us);
    countsPerSecondRight = deltaCountsRight*timeFactor;
    countsPerSecondLeft = deltaCountsLeft*timeFactor;
  }
  lastRun_us = time_us;
}

bool ProfizumoEncoders::CheckErrorLeft()
{
    bool error = errorLeft;
    errorLeft = false;
    return error;
}

bool ProfizumoEncoders::CheckErrorRight()
{
    bool error = errorRight;
    errorRight = false;
    return error;
}
}
