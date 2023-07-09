# Datalogger 2039
This is an 8-channel Datalogger for collecting temperature data (DS18B20 sensor).

-------

- Note: Work in progress, just demonstrative, not functional, sorry.

-------

## Features:
- 12-bit temperature sensor [from -55°C to +125°C (-67°F to +257°F)]
- - Needs two cycles to reset fault (Cycle 1: Start converting; Cycle 2: Read)
- Short circuit protection for sensors (power and data line).
- Short-circuit protection for the USB ports (power line).
- 56 characters channel label [(firm. ver. 0.0.8): special character supported (À/Á/Ã/Â/Ä); Ctrl+C/Ctrl+C supported].
- 53895 readings (37 days, 10 hours and 15 minutes @ 1 minute interval).
- Control access to settings and data export via password.
- Export data via USBdisk (*.dat). Before removing the USB disk It is recommended to exit the Export screen to completely disable the USB port.
- - Need to format the USB disk in FAT, or it will give an error.
- - Output file with part of the data encoded in Base64. Need to decode the data to import into a spreadsheet, see the File Converter App.
- Edit parameters via USB keyboard.
- - Keyboard may take a few seconds to recognize.
- Cabinet tilt monitoring.
- Battery and charger voltage monitoring.
- Battery polarity reversal protection (In case the Datalogger 2039 is using two batteries, it is not recommended to reverse any of them, or it may damage the battery).
- Monitoring of data in colored graph.
- Screenshot of graph to save to USB disk (Other screens can be captured, USBdisk is triggered only during capture).
- Multiple languages (English, Portuguese and Spanish).
- Integrated firmware integrity check (CRC).
- Energy saving for prolonged battery charge and durability of the LCD backlight.
- - Microcontroller operates in two modes, low power at only 40MHz clock and normal power at 168MHz clock.
- - The display is activated only when the microcontroller is clocked at 168MHz.
- - The display backlight has 3 operating states: high brightness, low brightness and off.
- - When the sensor is read, a conversion interval of 750ms is required, during which time the microcontroller sleeps.
- Temperature offset. On the sensors screen, press the number between 1 and 8 to open the offset adjustment screen.

## Software:
- Firmware for STM32F407VGT6 microcontroller:
- - STM32CubeIDE - C language project (HAL).
- File Converter:
- -  ̶B̶4̶J̶ ̶-̶ ̶B̶a̶s̶i̶c̶ ̶l̶a̶n̶g̶u̶a̶g̶e̶ ̶p̶r̶o̶j̶e̶c̶t̶ ̶(̶J̶a̶v̶a̶ ̶-̶ ̶c̶r̶o̶s̶s̶ ̶p̶l̶a̶t̶f̶o̶r̶m̶)̶.̶
- - Dot Net 4.0 - [SharpDevelop 4.4.1](https://sourceforge.net/projects/sharpdevelop/files/SharpDevelop%204.x/4.4/)

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

-------

- Version 0.0.8 (experimental)
- - Fixed the last line of the exported file
- - Channel label can operate with more characters (special characters with accent: À/Á/Ã/Â/Ä)
- - Channel label can operate with copy and paste command (Ctrl+C/Ctrl+V)
- - Support for Wireless Mini Keyboard with Touchpad has been added
- - - Tested with the ABNT2 (PT-BR) character map version, it may be necessary to remap the keys for other layout versions
-
- ToDo :
- - Find out why it is not possible to go back to version 0.0.7 after installing firmware 0.0.8.
- - - On the serial port UART1 it is possible to see that an initialization loop occurs.
- - - It may be related to the W25Q64 memory data, after erasing the W25Q64 it was not possible to reproduce the fault again.
-
- - Add support for normal (+) and (-) keys for sensor offset adjustment (currently only keypad keys are supported).
- - Resolve bug: after activating a channel, when switching from one channel to another using keys F1 to F8, the state of the channel is not updated.
- - Resolve bug: in memory test, timeout occurs before all test ends.
-
- Add support for [TMP117](https://www.ti.com/product/TMP117) (+/-0.1°C; 16 bits) sensors (8CH using I2C Multiplexer IC [TCA9548A](https://www.sparkfun.com/products/16784)). More info [here](https://github.com/rtek1000/Datalogger_2039/blob/main/Hardware/TMP117/README.md).

- Implement drivers for I2C and SPI peripherals using DMA.

-------

LCD Graph screen screenshot:

![img](https://raw.githubusercontent.com/rtek1000/Datalogger_2039/main/Software/Datalogger_2039%20-%20STM32/Screenshots/2023.png)

- File conversion app has been updated with partial language support
  
![img](https://raw.githubusercontent.com/rtek1000/Datalogger_2039/main/Software/Datalogger_2039%20-%20SharpDevelop/Screenshot.png)

--------
## STM32 internal hardware:

- Regarding the hardware problem in the STM32, for new projects that can use a USB HUB in the USB port operating as a Host, it may be recommended to use an [external PHY](https://community.st.com/t5/stm32-mcu-products/is-it-possible-to-use-usb-hub-with-stm32f4-using-external-phy/m-p/569817/highlight/true#M217311): see the doc [Errata sheet - ES0206 - Rev 19 - February 2023](https://www.st.com/resource/en/errata_sheet/es0206-stm32f427437-and-stm32f429439-device-errata-stmicroelectronics.pdf).

![img](https://raw.githubusercontent.com/rtek1000/Datalogger_2039/main/Hardware/IMG_3761.jpeg)


Each model may have an ERRATA document, check on the ST website under the Documentation tab. Series of STM32 can have similar functions, as for example the [F4 and F7](https://community.st.com/t5/stm32-mcu-products/is-there-any-stm32-f7-or-h7-pin-compatible-with-stm32f407vgt6/m-p/568289/highlight/true#M216887) series:

![img](https://raw.githubusercontent.com/rtek1000/Datalogger_2039/main/Hardware/F4_F7_series_USB_internal_PHY.png)

--------

[Failures may occur in the USB port operating as a Host](https://community.st.com/t5/stm32-mcu-products/stm32-usb-host-fails-miserably-but-arduino-mega2560-and-uno-with/td-p/569199) if the connected device lacks synchronization (not very accurate time base, such as using a ceramic resonator instead of a quartz crystal). The same device (joypad) that failed with STM32, didn't fail with Arduino and MAX3421e, can be an interesting alternative: [UHS3.0](https://github.com/felis/UHS30)

- The image below shows data that the STM32 read from the device during the device recognition (enumeration) procedure, and subsequent readings are not the same.

![img](https://github.com/rtek1000/Datalogger_2039/blob/main/Hardware/STM32_Fail.png)

(Test [code](https://github.com/rtek1000/STM32F4HUB_modified/blob/main/Project-STM32CubeIDE_USBH_ASC/F407_USBH_ASC_GAMEPAD/F407_USBH_ASC/Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid.c)):

```C++
#if PRINT_HID_REPORT_DESCRIPTOR == 1
			USBH_UsrLog("Dump HID Report Descriptor start");
			printf("wItemLength %d\n", HID_Handle->HID_Desc.wItemLength);

			for(uint16_t i = 0; i < HID_Handle->HID_Desc.wItemLength; i++){
				printf("0x%02X ", phost->device.Data[i]);
			}

			printf("\n");
			USBH_UsrLog("Dump HID Report Descriptor end");
#endif // #if PRINT_HID_REPORT_DESCRIPTOR == 1
```

- Ref.: [USB Descriptor and Request Parser](https://eleccelerator.com/usbdescreqparser/) 

--------

## Licence:

#### Hardware:
Released under CERN OHL 1.2: https://ohwr.org/cernohl

#### Software:
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
