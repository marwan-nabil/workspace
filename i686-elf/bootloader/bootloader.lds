ENTRY(Entry)
OUTPUT_FORMAT("binary")

__load_address = 0x00000500;

SECTIONS
{
    . = __load_address;
    .entry :
    {
        __entry_start = .;
        *(.entry)
    }
    .text :
    {
        __text_start = .;
        *(.text)
    }
    .data :
    {
        __data_start = .;
        *(.data)
    }
    .rodata :
    {
        __rodata_start = .;
        *(.rodata)
    }
    .bss :
    {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }
    __end = .;
}