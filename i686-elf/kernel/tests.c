#include <stdarg.h>
#include "i686-elf\shared\base_types.h"
#include "i686-elf\shared\basic_defines.h"
#include "i686-elf\shared\strings\print.h"
#include "i686-elf\shared\cpu\panic.h"
#include "i686-elf\os\kernel\main.h"

void TestInterrupts()
{
    PrintString
    (
        &GlobalPrintContext,
        "======================= interrupt tests ================\r\n"
    );
    // __asm("int $0x2");
    // __asm("int $0x3");
    // __asm("int $0x4");
    // __asm("int $50"); // will cause a segmentation exception
    // IntentionalCrash();

    PrintString
    (
        &GlobalPrintContext,
        "Interrupt tests finished.\n"
    );
}