################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Blinky/Blinky.obj: ../Blinky/Blinky.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/c2000_15.12.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv6/tools/compiler/c2000_15.12.3.LTS/include" --include_path="C:/Users/martisek/Desktop/TI_projects/Example_2833xLEDBlink/Console" --include_path="C:/Users/martisek/Desktop/TI_projects/Example_2833xLEDBlink/Blinky" --include_path="C:/Users/martisek/Desktop/TI_projects/Example_2833xLEDBlink/QP_include" --include_path="C:/Users/martisek/Desktop/TI_projects/Example_2833xLEDBlink/QP_port" --include_path="C:/Users/martisek/Desktop/TI_projects/Example_2833xLEDBlink/QP_qep" --include_path="C:/Users/martisek/Desktop/TI_projects/Example_2833xLEDBlink/QP_qf" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v140/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v140/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" -g --define="_DEBUG" --define="LARGE_MODEL" --quiet --diag_suppress=10063 --diag_warning=225 --verbose_diagnostics --issue_remarks --output_all_syms --cdebug_asm_data --preproc_with_compile --preproc_dependency="Blinky/Blinky.d" --obj_directory="Blinky" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Blinky/bsp.obj: ../Blinky/bsp.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv6/tools/compiler/c2000_15.12.3.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="C:/ti/ccsv6/tools/compiler/c2000_15.12.3.LTS/include" --include_path="C:/Users/martisek/Desktop/TI_projects/Example_2833xLEDBlink/Console" --include_path="C:/Users/martisek/Desktop/TI_projects/Example_2833xLEDBlink/Blinky" --include_path="C:/Users/martisek/Desktop/TI_projects/Example_2833xLEDBlink/QP_include" --include_path="C:/Users/martisek/Desktop/TI_projects/Example_2833xLEDBlink/QP_port" --include_path="C:/Users/martisek/Desktop/TI_projects/Example_2833xLEDBlink/QP_qep" --include_path="C:/Users/martisek/Desktop/TI_projects/Example_2833xLEDBlink/QP_qf" --include_path="/packages/ti/xdais" --include_path="C:/ti/controlSUITE/device_support/f2833x/v140/DSP2833x_headers/include" --include_path="C:/ti/controlSUITE/device_support/f2833x/v140/DSP2833x_common/include" --include_path="C:/ti/controlSUITE/libs/math/IQmath/v15c/include" --include_path="C:/ti/controlSUITE/libs/math/FPUfastRTS/V100/include" -g --define="_DEBUG" --define="LARGE_MODEL" --quiet --diag_suppress=10063 --diag_warning=225 --verbose_diagnostics --issue_remarks --output_all_syms --cdebug_asm_data --preproc_with_compile --preproc_dependency="Blinky/bsp.d" --obj_directory="Blinky" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


