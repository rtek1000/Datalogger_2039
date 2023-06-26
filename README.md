# Datalogger 2039
This is an 8-channel Datalogger for collecting temperature data.

-------

Note: Work in progress, just demonstrative, not functional, sorry.

- ToDo:  ̶r̶e̶p̶l̶a̶c̶e̶ ̶c̶o̶d̶e̶ ̶(̶t̶o̶ ̶s̶o̶l̶v̶e̶ ̶o̶v̶e̶r̶f̶l̶o̶w̶ ̶p̶r̶o̶b̶l̶e̶m̶)̶: (version 0.0.7)
- - From if (HAL_GetTick()	> (u32_var + 500)) {
- - From if ((HAL_GetTick()	- u32_var) >= 500) {

-------

- Version 0.0.8 (experimental) has been checked and bugs found have been fixed

![img](https://raw.githubusercontent.com/rtek1000/Datalogger_2039/main/Software/Datalogger_2039%20-%20STM32/Screenshots/2023.png)

- - Support for Wireless Mini Keyboard with Touchpad has been added
- - - Tested with the ABNT2 (PT-BR) character map version, it may be necessary to remap the keys for other layout versions

![img](https://raw.githubusercontent.com/rtek1000/Datalogger_2039/main/Software/Datalogger_2039%20-%20STM32/BIN/Mini_Keyboard1.jpg)

- File conversion app has been updated with partial language support
![img](https://raw.githubusercontent.com/rtek1000/Datalogger_2039/main/Software/Datalogger_2039%20-%20SharpDevelop/Screenshot.png)

--------

## Features:
- 12-bit temperature sensor [from -55°C to +125°C (-67°F to +257°F)]
- - Needs two cycles to reset fault (Cycle 1: Start converting; Cycle 2: Read)
- Short circuit protection for sensors (power and data line).
- Short-circuit protection for the USB ports (power line).
- 56 characters channel label.
- 53895 readings (37 days, 10 hours and 15 minutes @ 1 minute interval).
- Control access to settings and data export via password.
- Export data via USBdisk (*.dat).
- - Need to format the USB disk in FAT, or it will give an error.
- - Output file with part of the data encoded in Base64. Need to decode the data to import into a spreadsheet, see the File Converter App.
- Edit parameters via USB keyboard.
- - Keyboard may take a few seconds to recognize.
- Cabinet tilt monitoring.
- Battery and charger voltage monitoring.
- Battery polarity reversal protection.
- Monitoring of data in colored graph.
- Screenshot of graph to save to USB disk.
- Multiple languages (English, Portuguese and Spanish).
- Integrated firmware integrity check (CRC).
- Energy saving for prolonged battery charge and durability of the LCD backlight.
- - Microcontroller operates in two modes, low power at only 40MHz clock and normal power at 168MHz clock.
- - The display is activated only when the microcontroller is clocked at 168MHz.
- - The display backlight has 3 operating states: high brightness, low brightness and off.
- - When the sensor is read, a conversion interval of 750ms is required, during which time the microcontroller sleeps.

## Software:
- Firmware for STM32F407VGT6 microcontroller:
- - STM32CubeIDE - C language project (HAL).
- File Converter:
- - B4J - Basic language project (Java - cross platform).

## Hardware:
- STM32F407VGT6: Dual USB port microcontroller.
- DS18B20: OneWire temperature sensor (±0.5°C Accuracy from -10°C to +85°C).
- CD74HC595: Port expansion (8-Bit Serial-In, Parallel-Out Shift)
- ILI9341: SPI TFT LCD 2.4 inch.
- W25Q64: SPI flash memory (64M-bit).
- DS3231: I2C extremely accurate real time clock - RTC.
- - CR2032: Lithium 3.0V battery backup.
- MPU6050: I2C accelerometer (ACC).
- USBLC6-2: ESD protection.
- STMPS2141: Enhanced single channel power switch (500mA continuous current)
- 18650: Li-ion 3.7V battery backup.
- TP4056: Battery charger.
- MCP1700: 1.6 µA Quiescent Current (IQ) Linear voltage regulator.
- LMR14020: High efficiency 2.2 MHz 2A Step-Down Converter (40V, 40µA IQ).
- MT3608: High Efficiency 1.2MHz 2A Step Up Converter (Up to 28V)
- LAN8720: High-Performance 10/100 Ethernet (Connectivity expansion forecast).
- 12V DC charger (9V~24V x 2A)
- Multi-functional button:
- - For manual activation when in economy mode
- - To force restart of the USB port (keyboard)
- - To force restart the microcontroller

Note:
- Series resistor on the USB line must be removed (cables must be shorter than 5 meters) see video:
- - [STM32 USB training - 07 STM32 USB hardware design]
- - https://www.youtube.com/watch?v=vwlXUOY7KY0&list=PLnMKNibPkDnFFRBVD206EfnnHhQZI4Hxa&index=7

- Calibration?
- - https://thecavepearlproject.org/2016/03/05/ds18b20-calibration-we-finally-nailed-it/

Note: If the pins PA13 (SWDIO) and PA14 (SWCLK) are used (configured for another option other than the default state) it may prevent the ST-Link from working, to solve this problem, keep the BOOT0 (pin 94) in logical state high (3.3V) before energizing, or reset the microcontroller. To not need to manually change the BOOT0 pin, keep the pins PA13 (SWDIO) and PA14 (SWCLK) unused, it also cannot be configured as input.

## Licence:

#### Hardware:
Released under CERN OHL 1.2: https://ohwr.org/cernohl

#### Software:
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
