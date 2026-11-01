################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../middleware/can_codec/can_codec.c 

OBJS += \
./middleware/can_codec/can_codec.o 

C_DEPS += \
./middleware/can_codec/can_codec.d 


# Each subdirectory must supply rules for building sources it contributes
middleware/can_codec/%.o middleware/can_codec/%.su middleware/can_codec/%.cyclo: ../middleware/can_codec/%.c middleware/can_codec/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/STM32G4xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I"D:/Project/VECU_STM32/app/body_control" -I"D:/Project/VECU_STM32/app/inc" -I"D:/Project/VECU_STM32/config" -I"D:/Project/VECU_STM32/app/command" -I"D:/Project/VECU_STM32/app/diagnostics" -I"D:/Project/VECU_STM32/app/dtc" -I"D:/Project/VECU_STM32/app/fault_injection" -I"D:/Project/VECU_STM32/app/health" -I"D:/Project/VECU_STM32/app/nvm" -I"D:/Project/VECU_STM32/app/vehicle_state" -I"D:/Project/VECU_STM32/middleware/can_codec" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-middleware-2f-can_codec

clean-middleware-2f-can_codec:
	-$(RM) ./middleware/can_codec/can_codec.cyclo ./middleware/can_codec/can_codec.d ./middleware/can_codec/can_codec.o ./middleware/can_codec/can_codec.su

.PHONY: clean-middleware-2f-can_codec

