#pragma once

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

#endif // ENABLE_ASSERTIONS