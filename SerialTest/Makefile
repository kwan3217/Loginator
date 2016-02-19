#Since the whole point of this project is to test the LPC4078, set the submodel here
SUBMDL=LPC4078
OPTLEVEL = 3
PROJECT_BRIEF="Serial port test, mostly only useful for LPC4078"
HWDESCCSV=Loginator2368Hardware.csv Loginator2368Onboard.csv
#include ../libraries/LPCduino/Makefile
include ../libraries/peripherals/Serial/Makefile
#include ../libraries/dump/Makefile
#include ../libraries/hardwareDesc/Makefile
include ../libraries/System/Makefile
ARM_SYMBOLS+=--add-symbol '$$d=.arm:0x000,local'
ARM_SYMBOLS+=--add-symbol '$$t=.arm:0x01c,local'
ARM_SYMBOLS+=--add-symbol '$$d=.arm:0x030,local'
ARM_SYMBOLS+=--add-symbol '$$t=.arm:0x03C,local'
ARM_SYMBOLS+=--add-symbol '$$d=.arm:0x0b0,local'
ARM_SYMBOLS+=--add-symbol '$$t=.arm:0x0e0,local'
ARM_SYMBOLS+=--add-symbol '&stack=.arm:0x000'
ARM_SYMBOLS+=--add-symbol '&reset=.arm:0x004'
ARM_SYMBOLS+=--add-symbol '&nmi=.arm:0x008'
ARM_SYMBOLS+=--add-symbol '&hardfault=.arm:0x00C'
ARM_SYMBOLS+=--add-symbol '&mmfault=.arm:0x010'
ARM_SYMBOLS+=--add-symbol '&busfault=.arm:0x014'
ARM_SYMBOLS+=--add-symbol '&usagefault=.arm:0x018'
ARM_SYMBOLS+=--add-symbol 'reset=.arm:0x01c'
ARM_SYMBOLS+=--add-symbol 'emulate_reset1=.arm:0x09e'
ARM_SYMBOLS+=--add-symbol 'emulate_reset2=.arm:0x0a6'
ARM_SYMBOLS+=--add-symbol '&crp=.arm:0x0b0'
ARM_SYMBOLS+=--add-symbol '&boot[43c]=.arm:0x0b4'
ARM_SYMBOLS+=--add-symbol '&boot[5d0]=.arm:0x0b8'
ARM_SYMBOLS+=--add-symbol '&usercrp=.arm:0x0c8'
ARM_SYMBOLS+=--add-symbol 'crp2=.arm:0x0cc'
ARM_SYMBOLS+=--add-symbol 'crp3=.arm:0x0d0'
ARM_SYMBOLS+=--add-symbol 'crp1=.arm:0x0d4'
ARM_SYMBOLS+=--add-symbol 'crp4=.arm:0x0dc'
ARM_SYMBOLS+=--add-symbol 'nmi=.arm:0x0e0'
ARM_SYMBOLS+=--add-symbol 'hardfault=.arm:0x0e2'
ARM_SYMBOLS+=--add-symbol 'mmfault=.arm:0x0e4'
ARM_SYMBOLS+=--add-symbol 'busfault=.arm:0x0e6'
ARM_SYMBOLS+=--add-symbol 'usagefault=.arm:0x0e8'
ARM_SYMBOLS+=--add-symbol 'autobaud=.arm:0x1a2'
ARM_SYMBOLS+=--add-symbol 'isp_entry=.arm:0x2ec'
ARM_SYMBOLS+=--add-symbol 'clear_r4_r8_and_return=.arm:0x2ac'
ARM_SYMBOLS+=--add-symbol 'setup_clocks_and_peripherals=.arm:0x2b0'
ARM_SYMBOLS+=--add-symbol 'start_usercode_if_checksum_good=.arm:0x346'
ARM_SYMBOLS+=--add-symbol 'setup_emc_pins=.arm:0x388'
ARM_SYMBOLS+=--add-symbol 'setup_emc=.arm:0x3e6'
ARM_SYMBOLS+=--add-symbol 'usercode_entry=.arm:0x44c'
ARM_SYMBOLS+=--add-symbol 'reset2=.arm:0x46e'
ARM_SYMBOLS+=--add-symbol '$$d=.arm:0x4f4,local'
ARM_SYMBOLS+=--add-symbol '$$t=.arm:0x558,local'
ARM_SYMBOLS+=--add-symbol '$$d=.arm:0x5f4,local'
ARM_SYMBOLS+=--add-symbol '$$t=.arm:0x62C,local'
ARM_SYMBOLS+=--add-symbol 'rounded_divide=.arm:0x782'
ARM_SYMBOLS+=--add-symbol 'routine118c=.arm:0x118c'
ARM_SYMBOLS+=--add-symbol 'routine12b0=.arm:0x12b0'
ARM_SYMBOLS+=--add-symbol '$$d=.arm:0x12bc,local'
ARM_SYMBOLS+=--add-symbol '$$t=.arm:0x12c0,local'
ARM_SYMBOLS+=--add-symbol '$$d=.arm:0x1640,local'
ARM_SYMBOLS+=--add-symbol '$$t=.arm:0x1650,local'
ARM_SYMBOLS+=--add-symbol 'delay=.arm:0x16da'
ARM_SYMBOLS+=--add-symbol 'clear_0x200000_bit6_and_delay=.arm:0x16ee'
ARM_SYMBOLS+=--add-symbol 'set_0x200000_bit6_and_delay=.arm:0x16fe'
ARM_SYMBOLS+=--add-symbol 'routine170e=.arm:0x170e'
ARM_SYMBOLS+=--add-symbol '$$d=.arm:0x1784,local'
ARM_SYMBOLS+=--add-symbol '$$t=.arm:0x17a0,local'
ARM_SYMBOLS+=--add-symbol '$$d=.arm:0x1c7c,local'
ARM_SYMBOLS+=--add-symbol '$$t=.arm:0x1c80,local'
ARM_SYMBOLS+=--add-symbol 'real_iap=.arm:0x17a0'
ARM_SYMBOLS+=--add-symbol 'IAP_LOCATION=.arm:0x1ff0'

bootstrap.o: bootstrap.hex Makefile
	arm-none-eabi-objcopy $< -O elf32-little -B arm --rename-section .sec1=.arm --set-section-flags .sec1=alloc,contents,load,code,rom $(ARM_SYMBOLS) $@

bootstrap.disasm: bootstrap.o Makefile notes.txt ../DumpIap/asm_note.pl
	cp $@ notes.txt
	arm-none-eabi-objdump -d -marm -z $< | perl ../DumpIap/asm_note.pl notes.txt > $@


