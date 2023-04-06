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

#ifndef ZUMOCOMMUNICATION_H
#define ZUMOCOMMUNICATION_H
#pragma once

namespace profizumo
{
  constexpr char stopByte{'\n'};
template<typename EnumType, EnumType... Values> class EnumCheck;

template<typename EnumType> class EnumCheck<EnumType>
{
public:
  template<typename IntType> static bool constexpr IsValue(IntType) 
  { 
    return false; 
  }
};

template<typename EnumType, EnumType V, EnumType... Next>
class EnumCheck<EnumType, V, Next...> : private EnumCheck<EnumType, Next...>
{
  using super = EnumCheck<EnumType, Next...>;
public:
  template<typename IntType> static bool constexpr IsValue(IntType v)
  {
      return v == static_cast<IntType>(V) || super::IsValue(v);
  }
};

enum class ZumoInput: char
{
  error = 'e',
  leftMotorSetSpeed = 'l',
  rightMotorSetSpeed = 'r'
}; 
bool constexpr IsZumoInput(char v)
{
  using TestCheck = EnumCheck<ZumoInput, ZumoInput::error, ZumoInput::leftMotorSetSpeed, ZumoInput::rightMotorSetSpeed>;
  return TestCheck::IsValue(v);
}
ZumoInput constexpr ToZumoInput(char v)
{
  return IsZumoInput(v) ? static_cast<ZumoInput>(v) : ZumoInput::error;
}
char constexpr FromZumoInput(ZumoInput output)
{
  return static_cast<char>(output);
}

enum class ZumoOutput: char
{
  error = 'e',
  // Acceleration
  accelerationX = 'a',
  accelerationY = 'b',
  accelerationZ = 'c',
  // Gyro 
  gyroX = 'g',
  gyroY = 'h',
  gyroZ = 'i',
  // Magnetometer 
  magnetometerX = 'm',
  magnetometerY = 'n',
  magnetometerZ = 'o',
  // Ultrasound distance
  ultrasoundDistance = 'u',
  // Encoder
  leftMotorIsSpeed = 'l',
  rightMotorIsSpeed = 'r'
}; 
bool constexpr IsZumoOutput(char v)
{
  using TestCheck = EnumCheck<ZumoOutput, ZumoOutput::error,
    ZumoOutput::accelerationX, ZumoOutput::accelerationY, ZumoOutput::accelerationZ,
    ZumoOutput::gyroX, ZumoOutput::gyroY, ZumoOutput::gyroZ, 
    ZumoOutput::magnetometerX, ZumoOutput::magnetometerY , ZumoOutput::magnetometerZ,
    ZumoOutput::ultrasoundDistance,
    ZumoOutput::leftMotorIsSpeed, ZumoOutput::rightMotorIsSpeed>;
  return TestCheck::IsValue(v);
}
ZumoOutput constexpr ToZumoOutput(char v)
{
  return IsZumoOutput(v) ? static_cast<ZumoOutput>(v) : ZumoOutput::error;
}
char constexpr FromZumoOutput(ZumoOutput output)
{
  return static_cast<char>(output);
}

}
#endif
