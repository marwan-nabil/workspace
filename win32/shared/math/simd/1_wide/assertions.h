#pragma once

#include <stdio.h>
#include "portable\shared\basic_defines.h"
#include "win32\shared\math\simd\1_wide\conversions.h"

#ifdef ENABLE_ASSERTIONS

inline void
AssertGoodMask(u32_lane Mask, u32 FaultIndex)
{
    if ((U32FromU32Lane(Mask, 0) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 0) != 0))
    {
        printf("\n ============== something bad happened at %d ============== \n", FaultIndex);
        printf("Mask[0] == %d", U32FromU32Lane(Mask, 0));
        fflush(stdout);
        Assert(0);
    }
}

#else

#define AssertGoodMask(Mask, FaultIndex)

#endif