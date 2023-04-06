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

#ifndef PROFIZUMOENCODERS_H
#define PROFIZUMOENCDERS_H
#pragma once

#include <stdint.h>

namespace profizumo
{
/*! \brief Reads counts from the encoders on the Zumo 32U4.
 *
 * This class allows you to read counts from the encoders on the Zumo 32U4,
 * which lets you tell how much each motor has turned and in what direction.
 *
 * The encoders are monitored in the background using interrupts, so your code
 * can perform other tasks without missing encoder counts.
 *
 * To read the left encoder, this class uses an interrupt service routine (ISR)
 * for PCINT0_vect, so there will be a compile-time conflict with any other code
 * that defines a pin-change ISR.
 *
 * To read the right encoder, this class calls
 * [attachInterrupt()](http://arduino.cc/en/Reference/attachInterrupt), so there
 * will be a compile-time conflict with any other code that defines an ISR for
 * an external interrupt directly instead of using attachInterrupt(). */
class ProfizumoEncoders
{

public:

    /*! Returns the number of counts that have been detected from the left-side
     * encoder.  These counts start at 0.  Positive counts correspond to forward
     * movement of the left side of the Zumo, while negative counts correspond
     * to backwards movement.
     *
     * The count is returned as a signed 16-bit integer.  When the count goes
     * over 32767, it will overflow down to -32768.  When the count goes below
     * -32768, it will overflow up to 32767. */
    int16_t GetCountsLeft();

    /*! This function is just like getCountsLeft() except it applies to the
     *  right-side encoder. */
    int16_t GetCountsRight();

    /*! This function is just like getCountsLeft() except it also clears the
     *  counts before returning.  If you call this frequently enough, you will
     *  not have to worry about the count overflowing. */
    int16_t GetCountsAndResetLeft();

    /*! This function is just like getCountsAndResetLeft() except it applies to
     *  the right-side encoder. */
    int16_t GetCountsAndResetRight();

    /*! This function returns true if an error was detected on the left-side
     * encoder.  It resets the error flag automatically, so it will only return
     * true if an error was detected since the last time checkErrorLeft() was
     * called.
     *
     * If an error happens, it means that both of the encoder outputs changed at
     * the same time from the perspective of the ISR, so the ISR was unable to
     * tell what direction the motor was moving, and the encoder count could be
     * inaccurate.  The most likely cause for an error is that the interrupt
     * service routine for the encoders could not be started soon enough.  If
     * you get encoder errors, make sure you are not disabling interrupts for
     * extended periods of time in your code. */
    bool CheckErrorLeft();

    /*! This function is just like checkErrorLeft() except it applies to
     *  the right-side encoder. */
    bool CheckErrorRight();

    void OnLeftInterrupt();
    void OnRightInterrupt();

    void Init();
private:
    volatile bool lastLeftXOR{false};
    volatile bool lastLeftA{false};
    volatile bool lastLeftB{false};

    volatile bool lastRightXOR{false};
    volatile bool lastRightA{false};
    volatile bool lastRightB{false};
    
    volatile bool errorLeft{false};
    volatile bool errorRight{false};
    
    // These count variables are uint16_t instead of int16_t because
    // signed integer overflow is undefined behavior in C++.
    volatile uint16_t countLeft;
    volatile uint16_t countRight;
};
}
#endif
