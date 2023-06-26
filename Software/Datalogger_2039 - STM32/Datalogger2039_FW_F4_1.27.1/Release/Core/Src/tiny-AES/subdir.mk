################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/tiny-AES/aes.c 

OBJS += \
./Core/Src/tiny-AES/aes.o 

C_DEPS += \
./Core/Src/tiny-AES/aes.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/tiny-AES/%.o Core/Src/tiny-AES/%.su Core/Src/tiny-AES/%.cyclo: ../Core/Src/tiny-AES/%.c Core/Src/tiny-AES/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-tiny-2d-AES

clean-Core-2f-Src-2f-tiny-2d-AES:
	-$(RM) ./Core/Src/tiny-AES/aes.cyclo ./Core/Src/tiny-AES/aes.d ./Core/Src/tiny-AES/aes.o ./Core/Src/tiny-AES/aes.su

.PHONY: clean-Core-2f-Src-2f-tiny-2d-AES
