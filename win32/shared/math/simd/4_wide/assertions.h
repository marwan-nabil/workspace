#ifdef ENABLE_ASSERTIONS

inline void
AssertGoodMask(u32_lane Mask, u32 FaultIndex)
{
    if
    (
        ((U32FromU32Lane(Mask, 0) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 0) != 0)) ||
        ((U32FromU32Lane(Mask, 1) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 1) != 0)) ||
        ((U32FromU32Lane(Mask, 2) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 2) != 0)) ||
        ((U32FromU32Lane(Mask, 3) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 3) != 0))
    )
    {
        printf("\n ============== something bad happened at %d ============== \n", FaultIndex);
        printf("Mask[0] == %d", U32FromU32Lane(Mask, 0));
        printf("Mask[1] == %d", U32FromU32Lane(Mask, 1));
        printf("Mask[2] == %d", U32FromU32Lane(Mask, 2));
        printf("Mask[3] == %d", U32FromU32Lane(Mask, 3));
        fflush(stdout);
        Assert(0);
    }
}

#else

#define AssertGoodMask(Mask, FaultIndex)

#endif // ENABLE_ASSERTIONS