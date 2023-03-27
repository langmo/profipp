# profizumo
A profinet interface for the Pololu Zumo robot.

## Licence
profizumo itself is licenced under the GPL version 3. See LICENSE for details. 

## Installation on Raspberry Pi
Needed: 
- Raspberry Pi 3 Model B+ or higher
- Pololu Zumo 32U4. The Arduino version should also work, but the 32U4 is assumed in the following.
Steps:
- Install profi++
- Install the Arduino IDE on the Raspberry: https://www.arduino.cc
- In the Arduino IDE, install the board driver of the 32U:
  - Open File->Preferences. Enter https://files.pololu.com/arduino/package_pololu_index.json in the "Additional Board Manager URLs" text box. Press OK.
  - In Tools->Boards->Boards Manager, search and install "Pololu A-Star Boards"
  - In Tools->Board, select "Pololu A-Star 32U4"
- In the Arduino IDE, upload the Arduino program (~/profipp/examples/profizumo/resources/profizumo/
- Compile profizumo
- Generate the GSDML file:
  - Run ``./profizumo -e ./``
  - Copy the GSDML (e.g. via a USB stick) to the computer of the PLC.
  - Start TIA Portal, install the GSDML and add it to the installed devices.