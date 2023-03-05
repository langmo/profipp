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

void loop()
{
  controller.Run();
}

/*
  Called whenever new data comes in the hardware serial RX. This
  routine is run between each time loop() runs.
*/
void serialEvent() 
{
  while (Serial.available()>=4) 
  {
    // Each command must start with a STOP_BYTE
    if(((byte)Serial.read()) != profizumo::stopByte)
    {
      controller.ProcessInput(profizumo::ZumoInput::error, 0);
      continue;
    }
    // get the command (one 8bit char)
    char command = (char)Serial.read();
    
    // get the value (one 16bit int)
    // network endianess is big endian, Arduino uses little endian
    int16_t value = (Serial.read() << 8) | Serial.read();
    
    controller.ProcessInput(profizumo::ToZumoInput(command), value);
  }
}
/**
 * Send data over serial. Used as feedback for controller.
 */
void serialOutput(profizumo::ZumoOutput command, int16_t value)
{
  Serial.write(profizumo::stopByte);
  Serial.write(profizumo::FromZumoOutput(command));
  // Little endian to network/big endian conversion
  Serial.write((value >> 8 ) & 0xFF);
  Serial.write((value      ) & 0xFF);
}
