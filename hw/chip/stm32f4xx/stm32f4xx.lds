OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
	      "elf32-littlearm")
OUTPUT_ARCH(arm)

SECTIONS
{
  . = 0x08004000;

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
  }

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) }
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  }
  
  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  }
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  }
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }

  .rodata : {
    *(.rodata);
    *(.rodata*);
    *(.jcr);
    *(.tm_clone_table);
    *(.igot.plt);
  }
  _erodata = .;  

  . = 0x20000000;
  _data_vma = .;
  _data_lma = ADDR(.rodata) + SIZEOF(.rodata);
  _sdata = .;
  _sidata = ADDR(.rodata) + SIZEOF(.rodata);
  .data : AT (ADDR(.rodata) + SIZEOF(.rodata)) {
    *(.data);
    *(.data*);
    *(.jcr);
    *(.tm_clone_table);
    *(.igot.plt);
    . = ALIGN(4);
  }
  _edata = .;
  _data_size = SIZEOF(.data);
  
  _bss = .;
  _sbss = .;
  .bss : {
    *(.bss);
    *(.bss*);
    . = ALIGN(4);
  }
  _bss_size = SIZEOF(.bss);
  _bssend = .;
  _ebss = .;
  
  _sstack = .;
  . += 0x2000;
  _estack = .;
  _end = .;
  _ram_top = 0x20000000 + 256*1024;
  _flash_top = 0x08000000 + 1*1024*1024;

  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
