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

#ifndef PROFIZUMOMOTORS_H
#define PROFIZUMOMOTORS_H
#pragma once

#include <stdint.h>

namespace profizumo
{
class ProfizumoMotors
{
  public:

    /** \brief Flips the direction of the left motor.
     *
     * You can call this function with an argument of \c true if the left motor
     * of your Zumo was not installed in the standard way and you want a
     * positive speed argument to correspond to forward movement.
     *
     * \param flip If true, then positive motor speeds will correspond to the
     * direction pin being high.  If false, then positive motor speeds will
     * correspond to the direction pin being low.
     */
    void FlipLeftMotor(bool flip);

    /** \brief Flips the direction of the right motor.
     *
     * You can call this function with an argument of \c true if the right motor
     * of your Zumo was not installed in the standard way and you want a
     * positive speed argument to correspond to forward movement.
     *
     * \param flip If true, then positive motor speeds will correspond to the
     * direction pin being high.  If false, then positive motor speeds will
     * correspond to the direction pin being low. */
    void FlipRightMotor(bool flip);

    /** \brief Sets the speed for the left motor.
     *
     * \param speed A number from -400 to 400 representing the speed and
     * direction of the left motor.  Values of -400 or less result in full speed
     * reverse, and values of 400 or more result in full speed forward. */
    void SetLeftSpeed(int16_t speed);

    /** \brief Sets the speed for the right motor.
     *
     * \param speed A number from -400 to 400 representing the speed and
     * direction of the right motor. Values of -400 or less result in full speed
     * reverse, and values of 400 or more result in full speed forward. */
    void SetRightSpeed(int16_t speed);

    /** \brief Sets the speeds for both motors.
     *
     * \param leftSpeed A number from -400 to 400 representing the speed and
     * direction of the right motor. Values of -400 or less result in full speed
     * reverse, and values of 400 or more result in full speed forward.
     * \param rightSpeed A number from -400 to 400 representing the speed and
     * direction of the right motor. Values of -400 or less result in full speed
     * reverse, and values of 400 or more result in full speed forward. */
    void SetSpeeds(int16_t leftSpeed, int16_t rightSpeed);

    void Init();

private:
  bool flipLeft{false};
  bool flipRight{false};
};
}
#endif
