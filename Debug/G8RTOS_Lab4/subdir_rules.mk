################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
G8RTOS_Lab4/%.obj: ../G8RTOS_Lab4/%.s $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1210/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/KnockCloud/Documents/Classes/Microprocessor Applications 2/Final Project/uP2_FinalProject" --include_path="C:/Users/KnockCloud/workspace_v12/uP2_FinalProject/BoardSupport/inc" --include_path="C:/Users/KnockCloud/workspace_v12/uP2_FinalProject/BoardSupport" --include_path="C:/Users/KnockCloud/workspace_v12/uP2_FinalProject/G8RTOS_Lab4" --include_path="C:/ti/TivaWare_C_Series-2.2.0.295" --include_path="C:/ti/ccs1210/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs="ccs" --define=PART_TM4C123GH6PM -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="G8RTOS_Lab4/$(basename $(<F)).d_raw" --obj_directory="G8RTOS_Lab4" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

G8RTOS_Lab4/%.obj: ../G8RTOS_Lab4/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1210/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/KnockCloud/Documents/Classes/Microprocessor Applications 2/Final Project/uP2_FinalProject" --include_path="C:/Users/KnockCloud/workspace_v12/uP2_FinalProject/BoardSupport/inc" --include_path="C:/Users/KnockCloud/workspace_v12/uP2_FinalProject/BoardSupport" --include_path="C:/Users/KnockCloud/workspace_v12/uP2_FinalProject/G8RTOS_Lab4" --include_path="C:/ti/TivaWare_C_Series-2.2.0.295" --include_path="C:/ti/ccs1210/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs="ccs" --define=PART_TM4C123GH6PM -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="G8RTOS_Lab4/$(basename $(<F)).d_raw" --obj_directory="G8RTOS_Lab4" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


