################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-143660623: ../cfg.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"D:/software/ti/ccs1281/ccs/utils/sysconfig_1.26.2/sysconfig_cli.bat" --script "E:/Project/MO269/SysCfg/F28377D/cfg.syscfg" -o "syscfg" -s "D:/software/ti/c2000/C2000Ware_6_00_01_00/.metadata/sdk.json" -d "F2837xD" --package F2837xD_176PTP --part F2837xD_176PTP --compiler ccs
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/board.c: build-143660623 ../cfg.syscfg
syscfg/board.h: build-143660623
syscfg/board.cmd.genlibs: build-143660623
syscfg/board.opt: build-143660623
syscfg/board.json: build-143660623
syscfg/pinmux.csv: build-143660623
syscfg/epwm.dot: build-143660623
syscfg/adc.dot: build-143660623
syscfg/c2000ware_libraries.cmd.genlibs: build-143660623
syscfg/c2000ware_libraries.opt: build-143660623
syscfg/c2000ware_libraries.c: build-143660623
syscfg/c2000ware_libraries.h: build-143660623
syscfg/clocktree.h: build-143660623
syscfg: build-143660623

syscfg/%.obj: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"D:/software/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -Ooff --include_path="E:/Project/MO269/SysCfg/F28377D" --include_path="E:/Project/MO269/SysCfg/F28377D/device" --include_path="D:/software/ti/c2000/C2000Ware_6_00_01_00/driverlib/f2837xd/driverlib" --include_path="D:/software/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --include_path="E:/Project/MO269/SysCfg/F28377D/Core/inc" --advice:performance=all --define=DEBUG --define=CPU1 --define=_FLASH --c99 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="syscfg/$(basename $(<F)).d_raw" --include_path="E:/Project/MO269/SysCfg/F28377D/CPU1_FLASH/syscfg" --obj_directory="syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"D:/software/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -Ooff --include_path="E:/Project/MO269/SysCfg/F28377D" --include_path="E:/Project/MO269/SysCfg/F28377D/device" --include_path="D:/software/ti/c2000/C2000Ware_6_00_01_00/driverlib/f2837xd/driverlib" --include_path="D:/software/ti/ccs1281/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --include_path="E:/Project/MO269/SysCfg/F28377D/Core/inc" --advice:performance=all --define=DEBUG --define=CPU1 --define=_FLASH --c99 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="E:/Project/MO269/SysCfg/F28377D/CPU1_FLASH/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


