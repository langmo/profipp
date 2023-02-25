/*
 * This example uses the ZumoMotors library to drive each motor on the Zumo
 * forward, then backward. The yellow user LED is on when a motor should be
 * running forward and off when a motor should be running backward. If a
 * motor on your Zumo has been flipped, you can correct its direction by
 * uncommenting the call to flipLeftMotor() or flipRightMotor() in the setup()
 * function.
 */

#include <Wire.h>
#include <ZumoShield.h>

#define LED_PIN 13

ZumoMotors motors;

void setup()
{
  pinMode(LED_PIN, OUTPUT);

  // uncomment one or both of the following lines if your motors' directions need to be flipped
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop()
{
  static int speedLeft = 0;
  static int speedRight = 0;
  if (Serial.available() > 0) 
  {
    // get incoming byte:
    int val = Serial.read();
    if(((char)val)!='\n')
    {
      speedLeft = val;
      if(speedLeft > 400)
        speedLeft = 400;
      else if(speedLeft < -400)
        speedLeft = -400;
    }
  }
  motors.setLeftSpeed(speedLeft);
  motors.setRightSpeed(speedLeft);
  delay(10);
}
