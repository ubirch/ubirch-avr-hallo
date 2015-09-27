# ubirch-hallo

Written to the sounds of 

- [kors k (ST2B recording)](https://soundcloud.com/kors-k) 
- [Afterhour Sounds](https://soundcloud.com/afterhour-sounds/lars-neubert-presents-nr66)
- [Nils Frajm](https://soundcloud.com/nils_frahm)
- ... and others they follow and reference

## Parts

# Adafruit Trinket Audio

- [Adafruit MicroSD/Audio Breakout](http://adafru.it/1381)
- [SIM800 cell-network chip](http://adafru.it/fix2468) (similar, onboard ubirch #1) 
- ...

## Wiring

You will need to refer to @chaosblog's wiring sheet for the pins used on the ubirch #1.

- default pins (SPI)
    * VCC -> any power pin (DIGITAL #3) 
    * GND -> ISP #5 GND
    * CLK -> ISP #3 SCK
    * MISO -> ISP #4 MISO
    * MOSI -> ISP #2 D.I2C (MOSI)

- special pins
    * RST -> unconnected (works apparently)
    * CS -> ADC #2 (A1)
    * XDCS -> ADC #1 (A0)
    * SDCS -> DIGITAL #2 (D1)
    * DREQ -> DIGITAL #1 (D0)

## Building

The build process is based upon [CMake](http://www.cmake.org/), which plays nice with a number
of IDEs. However, as we have to create some special dependencies there is a certain directory
structure to be followed:

```
<project>
 +-sketches
   +-ubirch-hallo (the sketch source code)
   +-libraries (directory with 3rd party Arduino libs, see CMakeLists.txt in ubirch-hallo sketch)
```

If you need a different path for the Arduino SDK, you can set an environment variable or change
the top ```CMakeLists.txt```. This is necessary to setup the correct targets for the sketches.

> Be aware that sketches also need a ```CMakeLists.txt``` build file declaring dependencies.

Default SDK libraries are available, like ```SoftwareSerial```,```Wire```, etc. The core library is
called ```arduino-core``` and must be declared for Sketches that have no other dependencies.
 
### Prerequisits

Install the toolchain:

For MacOS X the simplest way to get the toolchain is [homebew](http://brew.sh/):

```
brew tap larsimmisch/avr
brew install avr-binutils avr-gcc avr-libc avrdude
brew install cmake
```

If you want to be able to compile [Arduino](https://www.arduino.cc/) sketches you will also need
the [Arduino SDK](https://www.arduino.cc/en/Main/Software) and set the environment variable ARDUINO_SDK_PATH.

### Make

1. Edit CMakeLists.txt and set PROGRAMMER and SERIAL_DEV to your flash programmer and device.
2. follow the following procedure:

```
mkdir build
cd build
cmake ..
make
```

To flash the code: ```make ubirch-hallo-flash```

> Unless you make significant changes to the CMakeLists.txt you only need to run ```make``` in
> the build directory from now on.

## LICENSE

    Copyright 2015 ubirch GmbH (http://www.ubirch.com)
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
