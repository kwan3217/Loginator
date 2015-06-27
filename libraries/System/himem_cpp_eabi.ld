/***********************************************************************/
/*  ROM.ld:  Linker Script File                                        */
/***********************************************************************/

/*
   Based on scripts found in the LPC yahoo-group and the Keil examples
   modified by Martin Thomas, Kaiserslautern, Germany
   http://www.siwawi.arubi.uni-kl.de/avr_projects
*/

/* Memory Definitions */
MEMORY
{
  SFE_BOOT (RX)  : ORIGIN = 0x00000000, LENGTH = 64K 
  ROM      (RX)  : ORIGIN = 0x00000000+64K, LENGTH = 512K-16K-64K
  DEVICE_ID(RX)  : ORIGIN = 0x0007C000, LENGTH = 4K
  NXP_BOOT (RX)  : ORIGIN = 0x0007D000, LENGTH = 12K
  RAM_VECT (RWX) : ORIGIN = 0x40000000, LENGTH = 64
  RAM      (RWX) : ORIGIN = 0x40000000+64, LENGTH = 32K-32-64
  RAM_IAP  (RWX) : ORIGIN = 0x40000000+32K-32, LENGTH = 32
  RAM_USB  (RWX) : ORIGIN = 0x7FD00000, LENGTH = 8K
}

/* Section Definitions */
SECTIONS
{
    /* first section is .text which is used for code */
    .text :
    {
        KEEP(*(.vectors))
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
    /* added by Martin Thomas 4/2005 based on Anglia Design example */
    .ctors :
    {
        PROVIDE(__ctors_start__ = .);
        KEEP(*(.init_array))  /*This is valid for arm-none-eabi*/
        KEEP(*(.ctors))       /*This is valid for arm-elf */
        PROVIDE(__ctors_end__ = .);
        . = ALIGN(16);
    }
    >ROM

    .dtors :
    {
        PROVIDE(__dtors_start__ = .);
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
        PROVIDE(__dtors_end__ = .);
        . = ALIGN(16);
    }
    >ROM

    . = ALIGN(16);
    /* mthomas - end */

    _etext = . ;
    PROVIDE (etext = .);

    /* .data section which is used for initialized data */
    .data : AT (_etext)
    {
        _data = .;
        KEEP(*(.vectmapped)) /* mthomas: when code in ROM and vectors remapped */
        . = ALIGN(4);
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d*)
        . = ALIGN(4);
    }
    > RAM

    . = ALIGN(4);
    _edata = . ;
    PROVIDE (edata = .);

    /* .bss section which is used for uninitialized (zeroed) data */
    .bss (NOLOAD) :
    {
        __bss_start = . ;
        __bss_start__ = . ;
        *(.bss .bss.*)           /* all zeroed sections */
        *(.gnu.linkonce.b*)
        *(COMMON)
    }
    > RAM

    . = ALIGN(4);
    __bss_end__ = . ;
    PROVIDE (__bss_end = .);

    .stack ALIGN(256) (NOLOAD):
    {
        PROVIDE (__stack_start__ = .);
        *(.stack)
        PROVIDE (_stack = .);
        PROVIDE (__stack_end__ = .);
        . = ALIGN(4);
    }
    > RAM

    _end = . ;
    PROVIDE (end = .);

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
