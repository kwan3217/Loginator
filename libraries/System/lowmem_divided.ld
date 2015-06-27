/* Memory Definitions */
MEMORY
{
/*  SFE_BOOT (RX)  : ORIGIN = 0x00000000, LENGTH = 64K */
/*  ROM (RX)       : ORIGIN = 0x00010000, LENGTH = 512K-64K-16K */
  ROM      (RX)  : ORIGIN = 0x00000000, LENGTH = 512K-16K
  DEVICE_ID(RX)  : ORIGIN = 0x0007C000, LENGTH = 4K
  NXP_BOOT (RX)  : ORIGIN = 0x0007D000, LENGTH = 12K
  RAM_VECT (RWX) : ORIGIN = 0x40000000, LENGTH = 64
  RAM      (RWX) : ORIGIN = 0x40000000+64, LENGTH = 32K-32-64
  RAM_IAP  (RWX) : ORIGIN = 0x40000000+32K-32, LENGTH = 32
  RAM_USB  (RWX) : ORIGIN = 0x7FD00000, LENGTH = 8K
}

/* After consultation with the Internet, I have discovered that _[A-Z].* and __.* 
are reserved identifiers to the implementation, meaning the compiler/linker/system library.
I have therefore concluded that since this linker script is used across all 
Loginator projects, any symbol defined in this script IS part of the 
implementation, and all symbols created in this linker script are reserved. 
Further, symbols which are not used should not be created, including "customary"
but unused symbols such as _etext. */

/* Section Definitions */
SECTIONS
{

    /* first section is .text which is used for code */
    .text :
    {
        image_start = . ;
         KEEP(*(.vectors))          /* required, keep the vectors where they belong */ 
	*Startup.o(.text .text.*)  /* Just customary, put startup code close to front of section */
        :*.o(.text.*)           /* remaining code, only from non-library object files (my code) */
        *(.gnu.linkonce.t.*)
	. = ALIGN(16);
    }
    > ROM

    .vtable :
    {
        :*.o(.rodata._ZTV*)
	. = ALIGN(16);
    }
    > ROM

    .rodata_str :
    {
        :*.o(.rodata.str*)                 /* read-only data (constants) */
	. = ALIGN(16);
    }
    > ROM

    .rodata :
    {
        :*.o(.rodata .rodata.*)                 /* read-only data (constants) */
	. = ALIGN(16);
    }
    > ROM

    /* .ctors .dtors are used for c++ constructors/destructors */
    .ctors :
    {
        _ctors_start = .;
        KEEP(:*.o(.init_array))  /*This is valid for arm-none-eabi*/
        KEEP(:*.o(.ctors))       /*This is valid for arm-elf */
        _ctors_end = .;
	. = ALIGN(16);
    }
    >ROM

    .rope :
    {
        KEEP(*(.rope .rope.*))                 /* read-only data (constants) */
	. = ALIGN(16);
    }
    > ROM

    .source : 
    {
	source_start = .;
        KEEP(*(.source))
	source_end = .;
	. = ALIGN(16);
    }
    >ROM

/* All code and read-only data which is supposed to be in rom must be
before this point. Next section is initialized data, which is loaded 
in rom but relocated to ram. Unfortunately this means that the source
is in between read-only data and initialized data. */
    _bdata = . ;

    /* .data section which is used for initialized data */
    .data : 
    {
        _data = .;
        :*.o(.data)
        :*.o(.data.*)
        :*.o(.gnu.linkonce.d*)
        _edata = . ;
	. = ALIGN(16);
    }
    > RAM AT>ROM	

    /* .bss section which is used for uninitialized (zeroed) data */
    .bss (NOLOAD) :
    {
        _bss_start = . ;
        :*.o(.bss .bss.*)           /* all zeroed sections */
        :*.o(.gnu.linkonce.b*)
        :*.o(COMMON)
        _bss_end = . ;
	. = ALIGN(16);
    }
    > RAM

    .text_lib : /* This specifies code from a library (not my code) */
    {
       *.a:(.text .text.*)
	. = ALIGN(16);
    }
    > ROM

    .ARM.exidx : 
    {
       *(.ARM.exidx.text .ARM.exidx.text.*)
	. = ALIGN(16);
    }
    > ROM

    .ARM.extab : 
    {
       *(.ARM.extab.text .ARM.extab.text.*)
	. = ALIGN(16);
    }
    > ROM

    .glue :
    {
        *(.glue_7)
        *(.glue_7t)
	. = ALIGN(16);
    }
    > ROM

    .vtable_lib :
    {
        *.a:(.rodata._ZTV*)
	. = ALIGN(16);
    }
    > ROM

    .rtti_info :
    {
        *(.rodata._ZTI*)
	. = ALIGN(16);
    }
    > ROM

    .rtti_name :
    {
        *(.rodata._ZTS*)
	. = ALIGN(16);
    }
    > ROM

    .rodata_str_lib :
    {
        *.a:(.rodata.str*)                 /* read-only data (constants) */
	. = ALIGN(16);
    }
    > ROM

    .rodata_lib :
    {
        *.a:(.rodata .rodata.*)                 /* read-only data (constants) */
	. = ALIGN(16);
    }
    > ROM

    .ctors_lib :
    {
        _ctors_lib_start = .;
        KEEP(*.a:(.init_array))  /*This is valid for arm-none-eabi*/
        KEEP(*.a:(.ctors))       /*This is valid for arm-elf */
        _ctors_lib_end = .;
	. = ALIGN(16);
    }
    >ROM

    .gcc_rodata :
    {
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
	. = ALIGN(16);
    }
    > ROM

    _bdata_lib = . ;

    .data_lib :
    {
        _data_lib = .;
        *.a:(.data)
        *.a:(.data.*)
        *.a:(.gnu.linkonce.d*)
        _edata_lib = . ;
	. = ALIGN(16);
    }
    > RAM AT>ROM
    
    /* Must be after the last real section loaded in ROM */
    .end.image : {
      image_end = . ;
    } 
    > ROM
    
    /* .bss section which is used for uninitialized (zeroed) data */
    .bss_lib (NOLOAD) :
    {
        _bss_lib_start = . ;
        *.a:(.bss .bss.*)           /* all zeroed sections */
        *.a:(.gnu.linkonce.b*)
        *.a:(COMMON)
        _bss_lib_end = . ;
    }
    > RAM

    .stack (NOLOAD):
    {
        *(.stack .stack.*)
    }
    > RAM

    .ram_iap (NOLOAD):
    {
        _ram_end = .;
        *(.ram_iap .ram_iap.*)
    }
    > RAM_IAP

    .usb (NOLOAD):
    {
        *(.usb .usb.*)
    }
    > RAM_USB


    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
         Symbols in the DWARF debugging sections are relative to the beginning
         of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
}
