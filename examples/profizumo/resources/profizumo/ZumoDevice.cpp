#include "ZumoDevice.h"

#include <Wire.h>
#include <ZumoShield.h>
#include <Arduino.h>

#define MAX_SPEED 400
#define ULTRASOUND_ECHO_PIN 4 // Echo pin of the ultrasound distance sensor
#define ULTRASOUND_TRIGGER_PIN 2 // Trigger pin of the ultrasound distance sensor

namespace profizumo
{
static ZumoMotors motors;
// accelerometer, magnetometer, and gyro
static ZumoIMU imu;

void InitUltrasound()
{
  pinMode(ULTRASOUND_TRIGGER_PIN, OUTPUT);
  pinMode(ULTRASOUND_ECHO_PIN, INPUT);
}
int16_t GetUltrasoundDistance_mm()
{
  const int MAX_DISTANCE_MM = 3000;
  const int MIN_DISTANCE_MM = 20;
  // Abstandsmessung wird mittels des 10us langen Triggersignals gestartet
  digitalWrite(ULTRASOUND_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASOUND_TRIGGER_PIN, LOW);
  // Nun wird am Echo-Eingang gewartet, bis das Signal aktiviert wurde
  // und danach die Zeit gemessen, wie lang es aktiviert bleibt
  unsigned long duration_mus = pulseIn(ULTRASOUND_ECHO_PIN, HIGH);
  // Nun wird der Abstand mittels der aufgenommenen Zeit berechnet
  int16_t distance_mm = duration_mus/5.82;
  // Überprüfung ob gemessener Wert innerhalb der zulässingen Entfernung liegt
  if (distance_mm >= MAX_DISTANCE_MM || distance_mm <= MIN_DISTANCE_MM) 
    return -1;
  else
    return distance_mm;
  // Falls nicht wird eine Fehlermeldung ausgegeben.
}

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

  InitUltrasound();
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
    // Ultrasound distance
    outputProcessor(ZumoOutput::ultrasoundDistance, GetUltrasoundDistance_mm());
    
  }
}
}
