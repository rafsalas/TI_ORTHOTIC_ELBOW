################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
simpliciti/Applications/application/End\ Device/main_ED_BM.obj: ../simpliciti/Applications/application/End\ Device/main_ED_BM.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.5/bin/cl430" --cmd_file="C:\Users\rafael\SkyDrive\Documents\Sports Watch\simpliciti\Applications\configuration\smpl_nwk_config.dat" --cmd_file="C:\Users\rafael\SkyDrive\Documents\Sports Watch\simpliciti\Applications\configuration\End Device\smpl_config.dat"  -vmspx --abi=coffabi -O4 --opt_for_speed=0 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.5/include" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/include" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/logic" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/driver" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/bluerobin" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Applications/application/End Device" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/bsp" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/bsp/boards" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/bsp/boards/CC430EM" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/bsp/boards/CC430EM/bsp_external" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/bsp/drivers" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/bsp/drivers/code" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/bsp/mcus" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/mrfi" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/mrfi/radios" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/mrfi/radios/family5" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/mrfi/smartrf" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/nwk" --include_path="C:/Users/rafael/SkyDrive/Documents/Sports Watch/simpliciti/Components/nwk_applications" -g --define=__CC430F6137__ --define=__CCE__ --define=ISM_US --define=MRFI_CC430 --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU18 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --call_assumptions=0 --gen_opt_info=2 --printf_support=minimal --preproc_with_compile --preproc_dependency="simpliciti/Applications/application/End Device/main_ED_BM.pp" --obj_directory="simpliciti/Applications/application/End Device" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


