################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.5

ORDERED_OBJS += \
"./main.obj" \
"./msp432_startup_ccs.obj" \
"./driverlib/MSP432P4xx/adc14.obj" \
"./driverlib/MSP432P4xx/aes256.obj" \
"./driverlib/MSP432P4xx/comp_e.obj" \
"./driverlib/MSP432P4xx/cpu.obj" \
"./driverlib/MSP432P4xx/crc32.obj" \
"./driverlib/MSP432P4xx/cs.obj" \
"./driverlib/MSP432P4xx/dma.obj" \
"./driverlib/MSP432P4xx/flash.obj" \
"./driverlib/MSP432P4xx/fpu.obj" \
"./driverlib/MSP432P4xx/gpio.obj" \
"./driverlib/MSP432P4xx/i2c.obj" \
"./driverlib/MSP432P4xx/interrupt.obj" \
"./driverlib/MSP432P4xx/mpu.obj" \
"./driverlib/MSP432P4xx/pcm.obj" \
"./driverlib/MSP432P4xx/pmap.obj" \
"./driverlib/MSP432P4xx/pss.obj" \
"./driverlib/MSP432P4xx/ref_a.obj" \
"./driverlib/MSP432P4xx/reset.obj" \
"./driverlib/MSP432P4xx/rtc_c.obj" \
"./driverlib/MSP432P4xx/spi.obj" \
"./driverlib/MSP432P4xx/sysctl.obj" \
"./driverlib/MSP432P4xx/systick.obj" \
"./driverlib/MSP432P4xx/timer32.obj" \
"./driverlib/MSP432P4xx/timer_a.obj" \
"./driverlib/MSP432P4xx/uart.obj" \
"./driverlib/MSP432P4xx/wdt_a.obj" \
"../msp432p401r.cmd" \
$(GEN_CMDS__FLAG) \
-l"libc.a" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include driverlib/MSP432P4xx/subdir_vars.mk
-include subdir_rules.mk
-include driverlib/MSP432P4xx/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
MSP432_spi.out \

EXE_OUTPUTS__QUOTED += \
"MSP432_spi.out" \

BIN_OUTPUTS += \
MSP432_spi.hex \

BIN_OUTPUTS__QUOTED += \
"MSP432_spi.hex" \


# All Target
all: MSP432_spi.out secondary-outputs

# Tool invocations
MSP432_spi.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: MSP432 Linker'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --advice:power=all -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --diag_wrap=off --diag_warning=225 --display_error_number -z -m"MSP432_spi.map" --heap_size=1024 --stack_size=512 -i"C:/ti/ccsv6/ccs_base/arm/include" -i"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/lib" -i"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/include" --reread_libs --display_error_number --diag_wrap=off --warn_sections --xml_link_info="MSP432_spi_linkInfo.xml" --rom_model -o "MSP432_spi.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

MSP432_spi.hex: $(EXE_OUTPUTS)
	@echo 'Invoking: MSP432 Hex Utility'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/bin/armhex"  -o "MSP432_spi.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)$(BIN_OUTPUTS__QUOTED)
	-$(RM) "main.pp" "msp432_startup_ccs.pp" "driverlib\MSP432P4xx\adc14.pp" "driverlib\MSP432P4xx\aes256.pp" "driverlib\MSP432P4xx\comp_e.pp" "driverlib\MSP432P4xx\cpu.pp" "driverlib\MSP432P4xx\crc32.pp" "driverlib\MSP432P4xx\cs.pp" "driverlib\MSP432P4xx\dma.pp" "driverlib\MSP432P4xx\flash.pp" "driverlib\MSP432P4xx\fpu.pp" "driverlib\MSP432P4xx\gpio.pp" "driverlib\MSP432P4xx\i2c.pp" "driverlib\MSP432P4xx\interrupt.pp" "driverlib\MSP432P4xx\mpu.pp" "driverlib\MSP432P4xx\pcm.pp" "driverlib\MSP432P4xx\pmap.pp" "driverlib\MSP432P4xx\pss.pp" "driverlib\MSP432P4xx\ref_a.pp" "driverlib\MSP432P4xx\reset.pp" "driverlib\MSP432P4xx\rtc_c.pp" "driverlib\MSP432P4xx\spi.pp" "driverlib\MSP432P4xx\sysctl.pp" "driverlib\MSP432P4xx\systick.pp" "driverlib\MSP432P4xx\timer32.pp" "driverlib\MSP432P4xx\timer_a.pp" "driverlib\MSP432P4xx\uart.pp" "driverlib\MSP432P4xx\wdt_a.pp" 
	-$(RM) "main.obj" "msp432_startup_ccs.obj" "driverlib\MSP432P4xx\adc14.obj" "driverlib\MSP432P4xx\aes256.obj" "driverlib\MSP432P4xx\comp_e.obj" "driverlib\MSP432P4xx\cpu.obj" "driverlib\MSP432P4xx\crc32.obj" "driverlib\MSP432P4xx\cs.obj" "driverlib\MSP432P4xx\dma.obj" "driverlib\MSP432P4xx\flash.obj" "driverlib\MSP432P4xx\fpu.obj" "driverlib\MSP432P4xx\gpio.obj" "driverlib\MSP432P4xx\i2c.obj" "driverlib\MSP432P4xx\interrupt.obj" "driverlib\MSP432P4xx\mpu.obj" "driverlib\MSP432P4xx\pcm.obj" "driverlib\MSP432P4xx\pmap.obj" "driverlib\MSP432P4xx\pss.obj" "driverlib\MSP432P4xx\ref_a.obj" "driverlib\MSP432P4xx\reset.obj" "driverlib\MSP432P4xx\rtc_c.obj" "driverlib\MSP432P4xx\spi.obj" "driverlib\MSP432P4xx\sysctl.obj" "driverlib\MSP432P4xx\systick.obj" "driverlib\MSP432P4xx\timer32.obj" "driverlib\MSP432P4xx\timer_a.obj" "driverlib\MSP432P4xx\uart.obj" "driverlib\MSP432P4xx\wdt_a.obj" 
	-@echo 'Finished clean'
	-@echo ' '

secondary-outputs: $(BIN_OUTPUTS)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

