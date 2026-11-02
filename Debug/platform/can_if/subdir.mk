################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../platform/can_if/can_if_stm32g4.c \
../platform/can_if/can_scheduler.c 

OBJS += \
./platform/can_if/can_if_stm32g4.o \
./platform/can_if/can_scheduler.o 

C_DEPS += \
./platform/can_if/can_if_stm32g4.d \
./platform/can_if/can_scheduler.d 


# Each subdirectory must supply rules for building sources it contributes
platform/can_if/%.o platform/can_if/%.su platform/can_if/%.cyclo: ../platform/can_if/%.c platform/can_if/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/STM32G4xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I"D:/Project/VECU_STM32/app/body_control" -I"D:/Project/VECU_STM32/app/inc" -I"D:/Project/VECU_STM32/config" -I"D:/Project/VECU_STM32/app/command" -I"D:/Project/VECU_STM32/app/diagnostics" -I"D:/Project/VECU_STM32/app/dtc" -I"D:/Project/VECU_STM32/app/fault_injection" -I"D:/Project/VECU_STM32/app/health" -I"D:/Project/VECU_STM32/app/nvm" -I"D:/Project/VECU_STM32/app/vehicle_state" -I"D:/Project/VECU_STM32/middleware/can_codec" -I"D:/Project/VECU_STM32/platform/can_if" -I"D:/Project/VECU_STM32/app/cli" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-platform-2f-can_if

clean-platform-2f-can_if:
	-$(RM) ./platform/can_if/can_if_stm32g4.cyclo ./platform/can_if/can_if_stm32g4.d ./platform/can_if/can_if_stm32g4.o ./platform/can_if/can_if_stm32g4.su ./platform/can_if/can_scheduler.cyclo ./platform/can_if/can_scheduler.d ./platform/can_if/can_scheduler.o ./platform/can_if/can_scheduler.su

.PHONY: clean-platform-2f-can_if

