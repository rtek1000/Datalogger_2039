The file F407VG_USB_MSC_DFU_host_bkp1.zip is a basic bootloader for testing, the program to be loaded needs to be written to operate on FLASH address 0x08008000, otherwise it will not work. This bootloader reads a binary image file.
Described in this tutorial:
[STM32 USB training - 11.3 USB MSC DFU host labs]
https://www.youtube.com/watch?v=CGUC1wqSLCE


The STM32F407VGT6_Bootloader.zip file is a basic bootloader for testing, the program to be loaded needs to be written to operate on FLASH address 0x08008000, otherwise it will not work. This bootloader does not read a binary image file, you have to use a program like STM32CubeProgrammer to burn the additional program from address 0x08008000. Configuring the IDE to generate a .hex file after compilation will make it easier to use the STM32CubeProgrammer program.

Tutorial:
[How to Create a Super Simple Bootloader]
https://www.youtube.com/playlist?list=PLnMKNibPkDnEb1sphpdFJ3bR9dNy7S6mO


The STM32F407VGT6_Bootloader_Led_Blink.zip file is a program written to operate at FLASH address 0x08008000, this program must be loaded via USB flash drive (if the bootloader has access to the USB Host port, configure the IDE to generate a BIN file) or using a program such as STM32CubeProgrammer to burn the additional program from address 0x08008000. Configuring the IDE to generate a .hex file after compilation will make it easier to use the STM32CubeProgrammer program.
Described in this tutorial:
[STM32 USB training - 09.9 USB DFU device labs]
https://www.youtube.com/watch?v=n_LDXOQHerU
