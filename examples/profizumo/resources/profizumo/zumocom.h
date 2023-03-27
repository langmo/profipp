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
  leftMotorSpeed = 'l',
  rightMotorSpeed = 'r'
}; 
bool constexpr IsZumoInput(char v)
{
  using TestCheck = EnumCheck<ZumoInput, ZumoInput::error, ZumoInput::leftMotorSpeed, ZumoInput::rightMotorSpeed>;
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
  ultrasoundDistance = 'u'
}; 
bool constexpr IsZumoOutput(char v)
{
  using TestCheck = EnumCheck<ZumoOutput, ZumoOutput::error,
    ZumoOutput::accelerationX, ZumoOutput::accelerationY, ZumoOutput::accelerationZ,
    ZumoOutput::gyroX, ZumoOutput::gyroY, ZumoOutput::gyroZ, 
    ZumoOutput::magnetometerX, ZumoOutput::magnetometerY , ZumoOutput::magnetometerZ,
    ZumoOutput::ultrasoundDistance>;
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
