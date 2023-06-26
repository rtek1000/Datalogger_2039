################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/1-Version.c \
../Core/Src/AES_func.c \
../Core/Src/BMP_to_USB.c \
../Core/Src/DS18B20.c \
../Core/Src/DS3231.c \
../Core/Src/EEPROM_EXTERN.c \
../Core/Src/ILI9341.c \
../Core/Src/IWDG_Delay.c \
../Core/Src/MPU6050.c \
../Core/Src/SCREEN.c \
../Core/Src/SN74HC595.c \
../Core/Src/STM32F4_UID.c \
../Core/Src/base64.c \
../Core/Src/clock.c \
../Core/Src/dwt_stm32_delay.c \
../Core/Src/export_file_csv.c \
../Core/Src/fonts.c \
../Core/Src/key_map.c \
../Core/Src/log_uart.c \
../Core/Src/main.c \
../Core/Src/num_base_64.c \
../Core/Src/screen_menu.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/w25qxx.c 

OBJS += \
./Core/Src/1-Version.o \
./Core/Src/AES_func.o \
./Core/Src/BMP_to_USB.o \
./Core/Src/DS18B20.o \
./Core/Src/DS3231.o \
./Core/Src/EEPROM_EXTERN.o \
./Core/Src/ILI9341.o \
./Core/Src/IWDG_Delay.o \
./Core/Src/MPU6050.o \
./Core/Src/SCREEN.o \
./Core/Src/SN74HC595.o \
./Core/Src/STM32F4_UID.o \
./Core/Src/base64.o \
./Core/Src/clock.o \
./Core/Src/dwt_stm32_delay.o \
./Core/Src/export_file_csv.o \
./Core/Src/fonts.o \
./Core/Src/key_map.o \
./Core/Src/log_uart.o \
./Core/Src/main.o \
./Core/Src/num_base_64.o \
./Core/Src/screen_menu.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/w25qxx.o 

C_DEPS += \
./Core/Src/1-Version.d \
./Core/Src/AES_func.d \
./Core/Src/BMP_to_USB.d \
./Core/Src/DS18B20.d \
./Core/Src/DS3231.d \
./Core/Src/EEPROM_EXTERN.d \
./Core/Src/ILI9341.d \
./Core/Src/IWDG_Delay.d \
./Core/Src/MPU6050.d \
./Core/Src/SCREEN.d \
./Core/Src/SN74HC595.d \
./Core/Src/STM32F4_UID.d \
./Core/Src/base64.d \
./Core/Src/clock.d \
./Core/Src/dwt_stm32_delay.d \
./Core/Src/export_file_csv.d \
./Core/Src/fonts.d \
./Core/Src/key_map.d \
./Core/Src/log_uart.d \
./Core/Src/main.d \
./Core/Src/num_base_64.d \
./Core/Src/screen_menu.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/w25qxx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/1-Version.cyclo ./Core/Src/1-Version.d ./Core/Src/1-Version.o ./Core/Src/1-Version.su ./Core/Src/AES_func.cyclo ./Core/Src/AES_func.d ./Core/Src/AES_func.o ./Core/Src/AES_func.su ./Core/Src/BMP_to_USB.cyclo ./Core/Src/BMP_to_USB.d ./Core/Src/BMP_to_USB.o ./Core/Src/BMP_to_USB.su ./Core/Src/DS18B20.cyclo ./Core/Src/DS18B20.d ./Core/Src/DS18B20.o ./Core/Src/DS18B20.su ./Core/Src/DS3231.cyclo ./Core/Src/DS3231.d ./Core/Src/DS3231.o ./Core/Src/DS3231.su ./Core/Src/EEPROM_EXTERN.cyclo ./Core/Src/EEPROM_EXTERN.d ./Core/Src/EEPROM_EXTERN.o ./Core/Src/EEPROM_EXTERN.su ./Core/Src/ILI9341.cyclo ./Core/Src/ILI9341.d ./Core/Src/ILI9341.o ./Core/Src/ILI9341.su ./Core/Src/IWDG_Delay.cyclo ./Core/Src/IWDG_Delay.d ./Core/Src/IWDG_Delay.o ./Core/Src/IWDG_Delay.su ./Core/Src/MPU6050.cyclo ./Core/Src/MPU6050.d ./Core/Src/MPU6050.o ./Core/Src/MPU6050.su ./Core/Src/SCREEN.cyclo ./Core/Src/SCREEN.d ./Core/Src/SCREEN.o ./Core/Src/SCREEN.su ./Core/Src/SN74HC595.cyclo ./Core/Src/SN74HC595.d ./Core/Src/SN74HC595.o ./Core/Src/SN74HC595.su ./Core/Src/STM32F4_UID.cyclo ./Core/Src/STM32F4_UID.d ./Core/Src/STM32F4_UID.o ./Core/Src/STM32F4_UID.su ./Core/Src/base64.cyclo ./Core/Src/base64.d ./Core/Src/base64.o ./Core/Src/base64.su ./Core/Src/clock.cyclo ./Core/Src/clock.d ./Core/Src/clock.o ./Core/Src/clock.su ./Core/Src/dwt_stm32_delay.cyclo ./Core/Src/dwt_stm32_delay.d ./Core/Src/dwt_stm32_delay.o ./Core/Src/dwt_stm32_delay.su ./Core/Src/export_file_csv.cyclo ./Core/Src/export_file_csv.d ./Core/Src/export_file_csv.o ./Core/Src/export_file_csv.su ./Core/Src/fonts.cyclo ./Core/Src/fonts.d ./Core/Src/fonts.o ./Core/Src/fonts.su ./Core/Src/key_map.cyclo ./Core/Src/key_map.d ./Core/Src/key_map.o ./Core/Src/key_map.su ./Core/Src/log_uart.cyclo ./Core/Src/log_uart.d ./Core/Src/log_uart.o ./Core/Src/log_uart.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/num_base_64.cyclo ./Core/Src/num_base_64.d ./Core/Src/num_base_64.o ./Core/Src/num_base_64.su ./Core/Src/screen_menu.cyclo ./Core/Src/screen_menu.d ./Core/Src/screen_menu.o ./Core/Src/screen_menu.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/w25qxx.cyclo ./Core/Src/w25qxx.d ./Core/Src/w25qxx.o ./Core/Src/w25qxx.su

.PHONY: clean-Core-2f-Src

