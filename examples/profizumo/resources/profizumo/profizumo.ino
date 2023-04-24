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

#include "ProfizumoDevice.h"
#include "zumocom.h"
#include <stdint.h>
#include <Wire.h>

// Constants
#define LED_PIN 13
// Which serial port to use. Set to "Serial" for communication via microUSB, and to "Serial1" for GPIO based com.
#define SERIAL_PORT Serial1

// Forward declactions
void serialOutput(profizumo::ZumoOutput command, int value);

// Global controller object.
profizumo::ProfizumoDevice controller{};

void setup()
{
  // Initialize the Wire library and join the I2C bus as a master
  Wire.begin();
  
  pinMode(LED_PIN, OUTPUT);
  SERIAL_PORT.begin(9600, SERIAL_8N1);
  while (!SERIAL_PORT) 
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

  while(SERIAL_PORT.available()>=1)
  { 
    int val = SERIAL_PORT.read();
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
  SERIAL_PORT.write(profizumo::FromZumoOutput(command));
  // Little endian to network/big endian conversion
  SERIAL_PORT.write((value >> 8 ) & 0xFF);
  SERIAL_PORT.write((value      ) & 0xFF);
  SERIAL_PORT.write(profizumo::stopByte);
}
