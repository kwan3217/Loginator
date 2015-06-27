LIBMAKE+=../libraries/crc/Makefile
EXTRAINCDIRS+=../libraries/crc/

crc: crc.cpp crc.h
	g++ -std=c++11 -O3 -o crc.o -c crc.cpp -ffunction-sections -fdata-sections
	g++ -O3 -o crc crc.o -Wl,--gc-sections -Wl,-Map,crc.map

crc.lss: crc
	objdump -h $< > $@
	objdump -S -j .text $< |tail -n +4 >> $@
	objdump -s -j .vtable -j .rodata_str -j .rodata -j .ctors $< | tail -n +4 >> $@
	objdump -s                        -j .data $< | tail -n +4 >> $@
	objdump -S -j .text_lib $< |tail -n +4 >> $@
	objdump -s -j .ARM.exidx -j .ARM.extab -j .glue -j .vtable_lib -j .rtti_info -j .rtti_name -j .rodata_str_lib -j .rodata_lib -j .ctors_lib $< | tail -n +4 >> $@
	objdump -s                        -j .data_lib $< | tail -n +4 >> $@
	objdump -t $< | grep ^[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f] | sort >> $@



