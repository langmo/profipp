# profizumo Arduino Code
The code which has to run on the Zumo32U4 board/Arduino when using Profizumo.

## Contributions and Licence
The Arduino control program was written by Moritz Lang. However, parts of it were taken over and
modified from the official zumo-32u4-arduino-library from pololu, which can be found here:
https://github.com/pololu/zumo-32u4-arduino-library

The zumo-32u4-arduino-library from pololu is licensed under the MIT license. Thus, for convenience,
the whole Arduino control program is licensed under the MIT license, too. See LICENSE for details.

The code taken over and modified from pololu mainly concerns the control of hardware components which
already come with the Zumo32U4 board. Files containing such code can be identified by the corresponding copyright 
notices at the top of the file. Other parts, especially the ones concerning communication with the Raspberry Pi as
well as code controlling additional sensors, were written from skretch.

It was necessary to take over and modify code (instead of just using
the Zumo32U4 Arduino library) since the hardware of the Profizumo bot is slightly changed as compared to
the original Zumo32U4 bot. Specifically, the Profizumo bot has an additional supersonic distance sensor,
and hardware interrupts are used for non-blocking measurements of the distance (instead of busy waiting).
The corresponding interrupt routines, i.e. ``ISR(PCINT0_vect)``, were however already defined in the
Zumo32U4 library, which led to multiple definition errors. Since this made a fork necessary, some other
modifications were done to the code, too.