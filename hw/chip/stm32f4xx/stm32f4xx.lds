OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
	      "elf32-littlearm")
OUTPUT_ARCH(arm)


MEMORY
{
RAM1 (xrw)      : ORIGIN = 0x20000000, LENGTH = 192K
RAM2 (xrw)      : ORIGIN = 0x10000000, LENGTH = 64K
FLASH (rx)      : ORIGIN = 0x8004000, LENGTH = 1*1024*1024
}


SECTIONS
{
  .text : {
    KEEP(*(.isr_vector));
    *(.start);
    *(.text);
    *(.text*);
    *(SORT(.table.*));
    *(.eh_frame);
    *(.init);
    *(.fini);
    
    _etext = .;
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) }
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH
  
  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  .rodata : {
    __tests_start = .;
    *(.rodata.tests);
    __tests_end = .;
    *(SORT(.rodata.*));
    *(.rodata);
    *(.jcr);
    *(.tm_clone_table);
    *(.igot.plt);
    _erodata = .;  
  } >FLASH

  _data_lma = ADDR(.rodata) + SIZEOF(.rodata);
  _sidata = ADDR(.rodata) + SIZEOF(.rodata);
  .data : {
    _data_vma = .;
    _sdata = .;
    *(.data);
    *(.data*);
    *(.jcr);
    *(.tm_clone_table);
    *(.igot.plt);
    . = ALIGN(16);
    *(.version_string.1);
    . = ALIGN(16);
    *(.version_string.2);
    . = ALIGN(16);
    _edata = .;
  } >RAM1 AT>FLASH
  _data_size = SIZEOF(.data);
  
  .bss : {
    _bss = .;
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss);
    *(.bss*);
    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
    _bssend = .;
    _ebss = .;
  } >RAM1 AT> FLASH
  _bss_size = SIZEOF(.bss);
  
  _sstack = .;
  . += 0x3000;
  _estack = .;
  _end = .;  
  
   _siccmram = (_sidata + SIZEOF(.data));
  
  /* Initialized CCM-RAM section
   * 
   * IMPORTANT NOTE! 
   * If initialized variables will be placed in this section, 
   * the startup code needs to be modified to copy the init-values.  
   */
  .ccmram : AT ( _sidata + SIZEOF(.data))
  {
    . = ALIGN(4);
    _sccmidata = .;        /* create a global symbol at data start */
    *(.ccmram)             /* .data sections */
    *(.ccmram*)            /* .data* sections */
    . = ALIGN(4);
    _eccmidata = .;        /* define a global symbol at data end */
  } >RAM2
  
  _ram_top = 0x20000000 + 192*1024;
  _flash_top = 0x08000000 + 1*1024*1024;
  _ccm_top = 0x10000000 + 64*1024;
  
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
