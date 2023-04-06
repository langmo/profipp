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
#include "ProfizumoDevice.h"
#include "ProfizumoMotors.h"
#include "ProfizumoImu.h"

#include <Arduino.h>

#define MAX_SPEED 400

namespace profizumo
{
static ProfizumoMotors motors{};
// accelerometer, magnetometer, and gyro
static ProfizumoImu imu{};
static ProfizumoSupersonic superSonic{};
}

ISR(PCINT0_vect)
{
  profizumo::superSonic.OnInterrupt();
}

namespace profizumo
{
ProfizumoDevice::ProfizumoDevice()
{
}
void ProfizumoDevice::Init(void (*outputProcessor_)(ZumoOutput, int16_t))
{
  outputProcessor = outputProcessor_;
  
  // Init IMU sensors.
  if (!imu.Init())
  {
    // If initialization failed, send error message in endless loop.
    while(1)
    {
      outputProcessor(profizumo::ZumoOutput::error, -1);
      delay(1000);
    }
  }
  imu.EnableDefault();

  superSonic.Init();
  motors.Init();
}
void ProfizumoDevice::ProcessInput(ZumoInput command, int16_t value)
{
  switch(command)
  {
    case ZumoInput::leftMotorSpeed:
      leftSpeed = value;
      if(leftSpeed > MAX_SPEED)
        leftSpeed = MAX_SPEED;
      else if(leftSpeed < -MAX_SPEED)
        leftSpeed = -MAX_SPEED;
      break;
    case ZumoInput::rightMotorSpeed:
      rightSpeed = value;
      if(rightSpeed > MAX_SPEED)
        rightSpeed = MAX_SPEED;
      else if(rightSpeed < -MAX_SPEED)
        rightSpeed = -MAX_SPEED;
      break;
    case ZumoInput::error:
      leftSpeed = 0;
      rightSpeed = 0;
      break;
  }
}



void ProfizumoDevice::Run()
{
  motors.SetLeftSpeed(leftSpeed);
  motors.SetRightSpeed(rightSpeed);

  superSonic.Run();

  if(outputProcessor != nullptr)
  {
    imu.Read();
    // Accelleration
    outputProcessor(ZumoOutput::accelerationX, imu.a.x);
    outputProcessor(ZumoOutput::accelerationY, imu.a.y);
    outputProcessor(ZumoOutput::accelerationZ, imu.a.z);
    // Gyro 
    outputProcessor(ZumoOutput::gyroX, imu.g.x);
    outputProcessor(ZumoOutput::gyroY, imu.g.y);
    outputProcessor(ZumoOutput::gyroZ, imu.g.z);
    // magnetometer 
    outputProcessor(ZumoOutput::magnetometerX, imu.m.x);
    outputProcessor(ZumoOutput::magnetometerY, imu.m.y);
    outputProcessor(ZumoOutput::magnetometerZ, imu.m.z);
    // Ultrasound distance
    outputProcessor(ZumoOutput::ultrasoundDistance, superSonic.GetLastDistance_mm());
    
  }
}
}
