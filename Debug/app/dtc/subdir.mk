################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app/dtc/dtc_mgr.c 

OBJS += \
./app/dtc/dtc_mgr.o 

C_DEPS += \
./app/dtc/dtc_mgr.d 


# Each subdirectory must supply rules for building sources it contributes
app/dtc/%.o app/dtc/%.su app/dtc/%.cyclo: ../app/dtc/%.c app/dtc/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/STM32G4xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I"D:/Project/VECU_STM32/app/body_control" -I"D:/Project/VECU_STM32/app/inc" -I"D:/Project/VECU_STM32/config" -I"D:/Project/VECU_STM32/app/command" -I"D:/Project/VECU_STM32/app/diagnostics" -I"D:/Project/VECU_STM32/app/dtc" -I"D:/Project/VECU_STM32/app/fault_injection" -I"D:/Project/VECU_STM32/app/health" -I"D:/Project/VECU_STM32/app/nvm" -I"D:/Project/VECU_STM32/app/vehicle_state" -I"D:/Project/VECU_STM32/middleware/can_codec" -I"D:/Project/VECU_STM32/platform/can_if" -I"D:/Project/VECU_STM32/app/cli" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-app-2f-dtc

clean-app-2f-dtc:
	-$(RM) ./app/dtc/dtc_mgr.cyclo ./app/dtc/dtc_mgr.d ./app/dtc/dtc_mgr.o ./app/dtc/dtc_mgr.su

.PHONY: clean-app-2f-dtc

