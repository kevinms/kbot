# kbot

The robot brainz to perform autonomous obstacle navigation and environment mapping.

## Hardware Setup

The brainz of the kbot is a Teensy 3.0 microcontroller. It is responsible
for interfacing with the various sensors and modules aboard the robot. The Teensy
will gather all necessary information about kbot's surroundings and make
informed decisions based on this information. The sensors and modules are connected
to the Teensy through various interfaces including: SPI, UART, ADC and others.

#### Motor Controller:
A Pololu Qik2s9v1 motor controller is used to control the speed and direction of
kbot's motors. This controller has a simple serial serial interface. The folks at
Pololu have been kind enough to provide an Arduino library, [qik-arduino](https://github.com/pololu/qik-arduino), for it as well! This
Arduino library uses a `SoftwareSerial` class since many Arduino boads only have a
single UART (hardware serial) which is usually used for serial over USB (for 
programming and debugging) The Teensy 3.0 has three UART ports at its disposle.
For kbot, the motor controller library has been modified to use the third UART. `HardwareSerial3`
The modified library has been included in this repository.

Below is a wiring chart for the Qik motor controller:


```
-------------------------------
 Qik            Teensy
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
```

#### Radio:
The kbot is equiped with a 2.4 GHz nrf24l01+ tranceiver radio for remote control
and kill switch. This can be helpful for debugging kbot and just having a little
fun :) The radio will communicate to the Teensy over an SPI bus. It is connected
to the Teensy's single SPI port. The CE (chip enable) pin on the radio, which
is not part of the SPI bus, switches between RX and TX mode. An Aruino library,
[RF24](https://github.com/maniacbug/RF24/), has been slightly modified for the Teensy and included in this repository.

Below is a wiring chart for the radio:


```
-------------------------------
 Radio         Teensy
-------------------------------
GND  <-> (logic supply ground)
VCC  <-> (logic voltage)
CE   <-> pin 4
CSN  <-> pin 10 (CS0)
SCK  <-> pin 13 (SCK)
MOSI <-> pin 11 (DOUT)
MISO <-> pin 12 (DIN)
IRQ  <-> unused
```

## Software Setup

- `kbot.ino`   : main arduino sketch
- `PololuQik/` : motor controller library (modified)
- `RF24/`      : radio library (modified)

kbot runs Arduino sketches on the Teensy 3.0. This is possible by using the Teensyduino add-on
for the Arduino IDE. You will first need to install the latest Arduino IDE and then install the
latest Teensyduino add-on. Follow the detailed instructions on the Teensy website:

http://www.pjrc.com/teensy/td_download.html

Once, you have the IDE setup, the motor controller and radio libraries need to be copied to:

\<arduino-ide-folder\>/libraries/

With the libraries installed, open `kbot.ino` in the Arduino IDE. Now, compile and upload your sketch to the Teensy 3.0 :D


## Contributors
- Kevin M. Smith
- Zach DeGroot
