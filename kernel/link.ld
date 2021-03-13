SECTIONS {
  . = 0xffffffff80000000;
  .text ALIGN(0x1000) : {
    text_start = .;
    *(.text)
    text_end = .;
  }
  .rodata ALIGN(0x1000) : {
    rodata_start = .;
    *(.rodata*)
    rodata_end = .;
  }
  .data ALIGN(0x1000) : {
    data_start = .;
    *(.data)
    data_end = .;
  }
  .bss ALIGN(0x1000) : {
    bss_start = .;
    *(.bss)
    *(COMMON)
    bss_end = .;
  }
}
