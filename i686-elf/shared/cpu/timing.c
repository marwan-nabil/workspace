#include "i686-elf\shared\base_types.h"
#include "i686-elf\shared\basic_defines.h"

void SpinlockWait(u32 SleepLoops)
{
    SleepLoops *= 1000;
    for (u32 Index = 0; Index < SleepLoops; Index++) {;}
}