#!/bin/bash 

# Windows [Dos comment: REM]:
#..\srec_cat.exe %1 -Intel -fill 0xFF 0x08000000 0x080FFFFC -STM32 0x080FFFFC -o ROM.hex -Intel
#arm-none-eabi-objcopy --input-target=ihex --output-target=binary ROM.hex DFU.bin

# Linux [Linux comment: #]:
srec_cat $1 -Intel -fill 0xFF 0x08000000 0x080FFFFC -STM32 0x080FFFFC -o ROM.hex -Intel
arm-none-eabi-objcopy --input-target=ihex --output-target=binary ROM.hex DFU.bin


