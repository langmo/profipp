#include "ZumoDevice.h"
#include "zumocom.h"
#include <stdint.h>

// Constants
#define LED_PIN 13

// Forward declactions
void serialOutput(profizumo::ZumoOutput command, int value);

// Global controller object.
profizumo::ZumoDevice controller{};

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial) 
  {
    // busy wait for serial port to connect.
  }
  controller.Init(&serialOutput);
}
void receiveSerial();
void loop()
{
  controller.Run();
  receiveSerial();
}

/*
  Called whenever new data comes in the hardware serial RX. This
  routine is run between each time loop() runs.
*/
void receiveSerial()//serialEvent() 
{
  static int step = 0;
  static char command = 'e';
  static int firstMessage = 0;
  static int secondMessage = 0;

  while(Serial.available()>=1)
  { 
    int val = Serial.read();
    bool commandComplete = false;
    switch(step)
    {
      case 0:
        command = (char)val;
        step++;
        break;
      case 1:
        firstMessage = val;
        step++;
        break;
      case 2:
        secondMessage = val;
        commandComplete = true;
        step++;
        break;
      default:
        if(((byte)val) == profizumo::stopByte)
          step=0;
        break;
    }
    if(commandComplete)
    {
      // get the value (one 16bit int)
      // network endianess is big endian, Arduino uses little endian
      int16_t value = (firstMessage << 8) | secondMessage;
      controller.ProcessInput(profizumo::ToZumoInput(command), value);
    }
  }
}
/**
 * Send data over serial. Used as feedback for controller.
 */
void serialOutput(profizumo::ZumoOutput command, int16_t value)
{
  Serial.write(profizumo::FromZumoOutput(command));
  // Little endian to network/big endian conversion
  Serial.write((value >> 8 ) & 0xFF);
  Serial.write((value      ) & 0xFF);
  Serial.write(profizumo::stopByte);
}
