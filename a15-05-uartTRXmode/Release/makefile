################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
a15-05-uartTRXmode.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
a15-05-uartTRXmode.list \

OBJCOPY_BIN += \
a15-05-uartTRXmode.bin \


# All Target
all: a15-05-uartTRXmode.elf secondary-outputs

# Tool invocations
a15-05-uartTRXmode.elf: $(OBJS) $(USER_OBJS) C:\ljh\STM32F103RBTX_FLASH.ld
	arm-none-eabi-gcc -o "a15-05-uartTRXmode.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"C:\ljh\STM32F103RBTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="a15-05-uartTRXmode.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

a15-05-uartTRXmode.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "a15-05-uartTRXmode.list"
	@echo 'Finished building: $@'
	@echo ' '

a15-05-uartTRXmode.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "a15-05-uartTRXmode.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
