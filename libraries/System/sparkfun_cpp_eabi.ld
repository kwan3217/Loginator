/* Linker script used for when the SFE bootloader is in place, 
and the target firmware is to be loaded and booted by that */
/* Memory Definitions */
MEMORY
{
  SFE_BOOT (RX)  : ORIGIN = 0x00000000, LENGTH = 64K 
  ROM (RX)       : ORIGIN = 0x00010000, LENGTH = 512K-64K-16K 
/*  ROM      (RX)  : ORIGIN = 0x00000000, LENGTH = 512K-16K */
  DEVICE_ID(RX)  : ORIGIN = 0x0007C000, LENGTH = 4K
  NXP_BOOT (RX)  : ORIGIN = 0x0007D000, LENGTH = 12K
  RAM_VECT (RWX) : ORIGIN = 0x40000000, LENGTH = 64
  RAM      (RWX) : ORIGIN = 0x40000000+64, LENGTH = 32K-32-64
  RAM_IAP  (RWX) : ORIGIN = 0x40000000+32K-32, LENGTH = 32
  RAM_USB  (RWX) : ORIGIN = 0x7FD00000, LENGTH = 8K
}

/* After consultation with the Internet, I have discovered that _[A-Z].* and __.* 
are reserved identifiers to the implementation, meaning the compiler/linker/system library.
I have therefore concluded that my code is not part of the implementation, and
since there is very little system, all symbols created in this linker script are
normal, not reserved. Further, symbols which are not used should not be created,
including "customary" but unused symbols such as etext. */

/* Section Definitions */
SECTIONS
{
    /* first section is .text which is used for code */
    .text :
    {
        btext = .;
        KEEP(*(.vectors))          /* required, keep the vectors where they belong */ 
	*Startup.o(.text .text.*)  /* Just customary, put startup code close to front of section */
        *(.text .text.*)           /* remaining code */
        *(.gnu.linkonce.t.*)
        . = ALIGN(16);
    }
    > ROM

    .glue :
    {
        *(.glue_7)
        *(.glue_7t)
        . = ALIGN(16);
    }

    .vtable :
    {
        *(.rodata._ZT*)
        . = ALIGN(16);
    }
    > ROM

    .rodata :
    {
        *(.rodata .rodata.*)                 /* read-only data (constants) */
        . = ALIGN(16);
    }
    > ROM

    .gcc_rodata :
    {
        *(.gcc_except_table)
        *(.gnu.linkonce.r.*)
        . = ALIGN(16);
    }
    > ROM

    /* .ctors .dtors are used for c++ constructors/destructors */
    .ctors :
    {
        ctors_start = .;
        KEEP(*(.init_array))  /*This is valid for arm-none-eabi*/
        KEEP(*(.ctors))       /*This is valid for arm-elf */
        ctors_end = .;
        . = ALIGN(16);
    }
    >ROM

/*The program never returns control to an external system, therefore
global destructors are never called. */
/*
    .dtors :
    {
        dtors_start = .;
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
        dtors_end = .;
        . = ALIGN(16);
    }
    >ROM
*/


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
    bdata = . ;

    /* .data section which is used for initialized data */
    .data : AT (bdata)
    {
        data = .;
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d*)
        edata = . ;
/*        . = ALIGN(16);*/
    }
    > RAM

    /* .bss section which is used for uninitialized (zeroed) data */
    .bss (NOLOAD) :
    {
        bss_start = . ;
        *(.bss .bss.*)           /* all zeroed sections */
        *(.gnu.linkonce.b*)
        *(COMMON)
        . = ALIGN(16);
        bss_end = . ;
    }
    > RAM

    .stack (NOLOAD):
    {
        *(.stack)
        . = ALIGN(16);
    }
    > RAM

    .usb (NOLOAD):
    {
        *(.usb)
        . = ALIGN(16);
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
