kbot
===
Kevin M. Smith
Zach DeGroot

Table of Contents:
  1. Hardware
  2. Software
    a. Libraries

========================================================================
Hardware
========================================================================
Teensy 3.0 Microcontroller
  - Hardware UART
      + PololuQik2s9v1
  - SPI Bus
      + nrf24l01+


-------------------------------
Qik       Teensy
-------------------------------
GND   <-> (logic supply ground)
VCC   <-> (logic voltage)
RX    <-> pin 8 (TX3)
TX    <-> pin 9 (RX3)
RESET <-> unused
ERR   <-> unused
M1    <-> (to motor 1)
M2    <-> (to motor 2)
GND   <-> (motor supply ground)
VMOT  <-> (motor voltage)

-------------------------------
Radio    Teensy
-------------------------------
GND  <-> (logic supply ground)
VCC  <-> (logic voltage)
CE   <-> pin 4
CSN  <-> pin 10 (CS0)
SCK  <-> pin 13 (SCK)
MOSI <-> pin 11 (DOUT)
MISO <-> pin 12 (DIN)
IRQ  <-> unused

========================================================================
Software
========================================================================
kbot.ino  - main arduino sketch
PololuQik - motor controller library (modified)
RF24      - radio library (modified)

You will need to install Teensyduino add-on for the Arduino IDE. Follow
the instructions here: http://www.pjrc.com/teensy/td_download.html

Open kbot.ino in the Arduino IDE. Copy the motor controller and radio
libraries to <arduino-ide-folder>/libraries/

Now compile and upload your sketch to the Teensy 3.0 :D

========================================================================
Libraries
========================================================================

PololuQik:
https://github.com/pololu/qik-arduino


RF24:
https://github.com/maniacbug/RF24/
