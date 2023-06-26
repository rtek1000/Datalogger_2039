################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid.c \
../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_custom.c \
../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_keybd.c \
../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_mouse.c \
../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_parser.c 

OBJS += \
./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid.o \
./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_custom.o \
./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_keybd.o \
./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_mouse.o \
./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_parser.o 

C_DEPS += \
./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid.d \
./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_custom.d \
./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_keybd.d \
./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_mouse.d \
./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_parser.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/%.o Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/%.su Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/%.cyclo: ../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/%.c Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-ST-2f-STM32_USB_Host_Library-2f-Class-2f-HID-2f-Src

clean-Middlewares-2f-ST-2f-STM32_USB_Host_Library-2f-Class-2f-HID-2f-Src:
	-$(RM) ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid.cyclo ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid.d ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid.o ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid.su ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_custom.cyclo ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_custom.d ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_custom.o ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_custom.su ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_keybd.cyclo ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_keybd.d ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_keybd.o ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_keybd.su ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_mouse.cyclo ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_mouse.d ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_mouse.o ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_mouse.su ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_parser.cyclo ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_parser.d ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_parser.o ./Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_parser.su

.PHONY: clean-Middlewares-2f-ST-2f-STM32_USB_Host_Library-2f-Class-2f-HID-2f-Src

