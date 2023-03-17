#include "ZumoDevice.h"

#include <Wire.h>
#include <ZumoShield.h>

#define MAX_SPEED 400

namespace profizumo
{
static ZumoMotors motors;
// accelerometer, magnetometer, and gyro
static ZumoIMU imu;

ZumoDevice::ZumoDevice()
{
}
void ZumoDevice::Init(void (*outputProcessor_)(ZumoOutput, int16_t))
{
  outputProcessor = outputProcessor_;
  // Initialize the Wire library and join the I2C bus as a master
  Wire.begin();
  // Init IMU sensors.
  if (!imu.init())
  {
    // If initialization failed, send error message in endless loop.
    while(1)
    {
      outputProcessor(profizumo::ZumoOutput::error, -1);
      delay(1000);
    }
  }
  imu.enableDefault();
  
}
void ZumoDevice::ProcessInput(ZumoInput command, int16_t value)
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
void ZumoDevice::Run()
{
  motors.setLeftSpeed(leftSpeed);
  motors.setRightSpeed(rightSpeed);

  if(outputProcessor != nullptr)
  {
    imu.read();
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
  }
}
}
