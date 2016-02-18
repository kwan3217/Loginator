/* Memory Definitions */
MEMORY
{
  ROM_VECTORS(RX) : ORIGIN = 0x00000000, LENGTH = 512 /*The first 512 bytes are subject to vector table remapping */
  ROM      (RX)   : ORIGIN = 0x00000200, LENGTH = 512K-12K-512
  DEVICE_ID(RX)   : ORIGIN = 0x0007C000, LENGTH = 12K
  RAM      (RWX)  : ORIGIN = 0x10000000, LENGTH = 32K
  RAM_IAP  (RWX)  : ORIGIN = 0x10000000+32K-32, LENGTH = 32
  PERIPH_RAM(RWX) : ORIGIN = 0x20000000, LENGTH = 32K
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
    .vectors :
    {
        KEEP(*(.vectors))          /* required, keep the vectors where they belong */ 
       "$d" = 0x00000000;
       __init_sp     = 0x00000000;
       __reset       = 0x00000004;
       __NMI         = 0x00000008;
       __HardFault   = 0x0000000C;
       __MMFault     = 0x00000010;
       __BusFault    = 0x00000014;
       __UsageFault  = 0x00000018;
       __SVCall      = 0x0000002C;
       __PendSV      = 0x00000038;
       __SysTick     = 0x0000003C;
       __IRQ         = 0x00000040;
    }
    > ROM_VECTORS

    .text :
    {
	*Startup.o(.text .text.*)  /* Just customary, put startup code close to front of section */
        :*.o(.text.*)           /* remaining code, only from non-library object files (my code) */
        *(.gnu.linkonce.t.*)
    }
    > ROM

    .vtable :
    {
        :*.o(.rodata._ZTV*)
    }
    > ROM

    .rodata_str :
    {
        :*.o(.rodata.str*)                 /* read-only data (constants) */
    }
    > ROM

    .rodata :
    {
        :*.o(.rodata .rodata.*)                 /* read-only data (constants) */
    }
    > ROM

    /* .ctors .dtors are used for c++ constructors/destructors */
    .ctors :
    {
        _ctors_start = .;
        KEEP(:*.o(.init_array))  /*This is valid for arm-none-eabi*/
        KEEP(:*.o(.ctors))       /*This is valid for arm-elf */
        _ctors_end = .;
    }
    >ROM

    .rope :
    {
        KEEP(*(.rope .rope.*))                 /* read-only data (constants) */
    }
    > ROM

    .source : 
    {
	source_start = .;
        KEEP(*(.source))
	source_end = .;
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
    }
    > RAM

    .text_lib : /* This specifies code from a library (not my code) */
    {
       *.a:(.text .text.*)
    }
    > ROM

    .ARM.exidx : 
    {
       *(.ARM.exidx.text .ARM.exidx.text.*)
    }
    > ROM

    .ARM.extab : 
    {
       *(.ARM.extab.text .ARM.extab.text.*)
    }
    > ROM

    .glue :
    {
        *(.glue_7)
        *(.glue_7t)
    }
    > ROM

    .vtable_lib :
    {
        *.a:(.rodata._ZTV*)
    }
    > ROM

    .rtti_info :
    {
        *(.rodata._ZTI*)
    }
    > ROM

    .rtti_name :
    {
        *(.rodata._ZTS*)
    }
    > ROM

    .rodata_str_lib :
    {
        *.a:(.rodata.str*)                 /* read-only data (constants) */
    }
    > ROM

    .rodata_lib :
    {
        *.a:(.rodata .rodata.*)                 /* read-only data (constants) */
    }
    > ROM

    .ctors_lib :
    {
        _ctors_lib_start = .;
        KEEP(*.a:(.init_array))  /*This is valid for arm-none-eabi*/
        KEEP(*.a:(.ctors))       /*This is valid for arm-elf */
        _ctors_lib_end = .;
    }
    >ROM

    .gcc_rodata :
    {
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
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
    }
    > RAM AT>ROM

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
        *(.stack)
    }
    > RAM

    .ram_iap (NOLOAD):
    {
        _ram_end = .;
        *(.ram_iap .ram_iap.*)
    }
    > RAM_IAP

    iap_entry = 0x1FFF1FF0;

    .usb (NOLOAD):
    {
        *(.usb)
    }
    > PERIPH_RAM


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
