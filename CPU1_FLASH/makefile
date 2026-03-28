################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := D:/software/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS

GEN_OPTS__FLAG := --cmd_file="syscfg/board.opt" --cmd_file="syscfg/c2000ware_libraries.opt" 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./syscfg/board.obj" \
"./syscfg/c2000ware_libraries.obj" \
"./main.obj" \
"./Core/src/communication/host_rs232.obj" \
"./Core/src/communication/serviceport.obj" \
"./Core/src/dsp_it.obj" \
"./Core/src/func_exec.obj" \
"./Core/src/elmo/elmo_can.obj" \
"./Core/src/elmo/elmo_ctrl.obj" \
"./Core/src/elmo/elmo_rs232.obj" \
"./Core/src/mode_ctrl/mode_calib.obj" \
"./Core/src/mode_ctrl/mode_ctrl.obj" \
"./Core/src/mode_ctrl/mode_position.obj" \
"./Core/src/mode_ctrl/mode_press.obj" \
"./ECAT/9252_HW.obj" \
"./ECAT/PIC32_EtherCAT_Slave.obj" \
"./ECAT/SPIDriver.obj" \
"./ECAT/src/coeappl.obj" \
"./ECAT/src/ecatappl.obj" \
"./ECAT/src/ecatcoe.obj" \
"./ECAT/src/ecateoe.obj" \
"./ECAT/src/ecatfoe.obj" \
"./ECAT/src/ecatslv.obj" \
"./ECAT/src/emcy.obj" \
"./ECAT/src/eoeappl.obj" \
"./ECAT/src/foeappl.obj" \
"./ECAT/src/mailbox.obj" \
"./ECAT/src/objdef.obj" \
"./ECAT/src/sdoserv.obj" \
"./device/F2837xD_CodeStartBranch.obj" \
"./device/device.obj" \
"../2837xD_FLASH_lnk_cpu1.cmd" \
"D:/software/ti/c2000/C2000Ware_6_00_01_00/driverlib/f2837xd/driverlib/ccs/Debug/driverlib.lib" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include Core/src/communication/subdir_vars.mk
-include Core/src/subdir_vars.mk
-include Core/src/elmo/subdir_vars.mk
-include Core/src/mode_ctrl/subdir_vars.mk
-include ECAT/subdir_vars.mk
-include ECAT/src/subdir_vars.mk
-include device/subdir_vars.mk
-include subdir_rules.mk
-include Core/src/communication/subdir_rules.mk
-include Core/src/subdir_rules.mk
-include Core/src/elmo/subdir_rules.mk
-include Core/src/mode_ctrl/subdir_rules.mk
-include ECAT/subdir_rules.mk
-include ECAT/src/subdir_rules.mk
-include device/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
F28377D.out 

EXE_OUTPUTS__QUOTED += \
"F28377D.out" 


# All Target
all: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "F28377D.out"

# Tool invocations
F28377D.out: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: C2000 Linker'
	"D:/software/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -Ooff --advice:performance=all --define=DEBUG --define=CPU1 --define=_FLASH --c99 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi -z -m"F28377D.map" --heap_size=0x300 --stack_size=0x3f8 --warn_sections -i"D:/software/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/lib" -i"D:/software/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --reread_libs --diag_wrap=off --display_error_number --xml_link_info="F28377D_linkInfo.xml" --entry_point=code_start --rom_model -o "F28377D.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_MISC_FILES__QUOTED)$(GEN_FILES__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "syscfg\board.obj" "syscfg\c2000ware_libraries.obj" "main.obj" "Core\src\communication\host_rs232.obj" "Core\src\communication\serviceport.obj" "Core\src\dsp_it.obj" "Core\src\func_exec.obj" "Core\src\elmo\elmo_can.obj" "Core\src\elmo\elmo_ctrl.obj" "Core\src\elmo\elmo_rs232.obj" "Core\src\mode_ctrl\mode_calib.obj" "Core\src\mode_ctrl\mode_ctrl.obj" "Core\src\mode_ctrl\mode_position.obj" "Core\src\mode_ctrl\mode_press.obj" "ECAT\9252_HW.obj" "ECAT\PIC32_EtherCAT_Slave.obj" "ECAT\SPIDriver.obj" "ECAT\src\coeappl.obj" "ECAT\src\ecatappl.obj" "ECAT\src\ecatcoe.obj" "ECAT\src\ecateoe.obj" "ECAT\src\ecatfoe.obj" "ECAT\src\ecatslv.obj" "ECAT\src\emcy.obj" "ECAT\src\eoeappl.obj" "ECAT\src\foeappl.obj" "ECAT\src\mailbox.obj" "ECAT\src\objdef.obj" "ECAT\src\sdoserv.obj" "device\F2837xD_CodeStartBranch.obj" "device\device.obj" 
	-$(RM) "syscfg\board.d" "syscfg\c2000ware_libraries.d" "main.d" "Core\src\communication\host_rs232.d" "Core\src\communication\serviceport.d" "Core\src\dsp_it.d" "Core\src\func_exec.d" "Core\src\elmo\elmo_can.d" "Core\src\elmo\elmo_ctrl.d" "Core\src\elmo\elmo_rs232.d" "Core\src\mode_ctrl\mode_calib.d" "Core\src\mode_ctrl\mode_ctrl.d" "Core\src\mode_ctrl\mode_position.d" "Core\src\mode_ctrl\mode_press.d" "ECAT\9252_HW.d" "ECAT\PIC32_EtherCAT_Slave.d" "ECAT\SPIDriver.d" "ECAT\src\coeappl.d" "ECAT\src\ecatappl.d" "ECAT\src\ecatcoe.d" "ECAT\src\ecateoe.d" "ECAT\src\ecatfoe.d" "ECAT\src\ecatslv.d" "ECAT\src\emcy.d" "ECAT\src\eoeappl.d" "ECAT\src\foeappl.d" "ECAT\src\mailbox.d" "ECAT\src\objdef.d" "ECAT\src\sdoserv.d" "device\device.d" 
	-$(RM) "device\F2837xD_CodeStartBranch.d" 
	-$(RMDIR) $(GEN_MISC_DIRS__QUOTED)
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

