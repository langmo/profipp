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

#include "ProfizumoMotors.h"
#include <FastGPIO.h>
#include <avr/io.h>

#define PWM_L 10
#define PWM_R 9
#define DIR_L 16
#define DIR_R 15

namespace profizumo
{
// initialize timer1 to generate the proper PWM outputs to the motor drivers
void ProfizumoMotors::Init()
{
    FastGPIO::Pin<PWM_L>::setOutputLow();
    FastGPIO::Pin<PWM_R>::setOutputLow();
    FastGPIO::Pin<DIR_L>::setOutputLow();
    FastGPIO::Pin<DIR_R>::setOutputLow();

    // Timer 1 configuration
    // prescaler: clockI/O / 1
    // outputs enabled
    // phase-correct PWM
    // top of 400
    //
    // PWM frequency calculation
    // 16MHz / 1 (prescaler) / 2 (phase-correct) / 400 (top) = 20kHz
    TCCR1A = 0b10100000;
    TCCR1B = 0b00010001;
    ICR1 = 400;
    OCR1A = 0;
    OCR1B = 0;
}

// enable/disable flipping of left motor
void ProfizumoMotors::FlipLeftMotor(bool flip)
{
    flipLeft = flip;
}

// enable/disable flipping of right motor
void ProfizumoMotors::FlipRightMotor(bool flip)
{
    flipRight = flip;
}

// set speed for left motor; speed is a number between -400 and 400
void ProfizumoMotors::SetLeftSpeed(int16_t speed)
{
    bool reverse = 0;

    if (speed < 0)
    {
        speed = -speed; // Make speed a positive quantity.
        reverse = 1;    // Preserve the direction.
    }
    if (speed > 400)    // Max PWM duty cycle.
    {
        speed = 400;
    }

    OCR1B = speed;

    FastGPIO::Pin<DIR_L>::setOutput(reverse ^ flipLeft);
}

// set speed for right motor; speed is a number between -400 and 400
void ProfizumoMotors::SetRightSpeed(int16_t speed)
{
    bool reverse = 0;

    if (speed < 0)
    {
        speed = -speed;  // Make speed a positive quantity.
        reverse = 1;     // Preserve the direction.
    }
    if (speed > 400)     // Max PWM duty cycle.
    {
        speed = 400;
    }

    OCR1A = speed;

    FastGPIO::Pin<DIR_R>::setOutput(reverse ^ flipRight);
}

// set speed for both motors
void ProfizumoMotors::SetSpeeds(int16_t leftSpeed, int16_t rightSpeed)
{
  SetLeftSpeed(leftSpeed);
  SetRightSpeed(rightSpeed);
}
}
