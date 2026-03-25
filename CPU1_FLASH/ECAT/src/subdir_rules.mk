################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
ECAT/src/%.obj: ../ECAT/src/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"D:/software/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -Ooff --include_path="E:/Project/MO269/SysCfg/F28377D" --include_path="E:/Project/MO269/SysCfg/F28377D/device" --include_path="D:/software/ti/c2000/C2000Ware_6_00_01_00/driverlib/f2837xd/driverlib" --include_path="D:/software/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --advice:performance=all --define=DEBUG --define=CPU1 --define=_FLASH --c99 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="ECAT/src/$(basename $(<F)).d_raw" --include_path="E:/Project/MO269/SysCfg/F28377D/CPU1_FLASH/syscfg" --obj_directory="ECAT/src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


