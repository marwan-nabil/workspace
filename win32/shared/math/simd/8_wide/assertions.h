#ifdef ENABLE_ASSERTIONS

inline void
AssertGoodMask(u32_lane Mask, u32 FaultIndex)
{
    if
    (
        ((U32FromU32Lane(Mask, 0) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 0) != 0)) ||
        ((U32FromU32Lane(Mask, 1) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 1) != 0)) ||
        ((U32FromU32Lane(Mask, 2) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 2) != 0)) ||
        ((U32FromU32Lane(Mask, 3) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 3) != 0)) ||
        ((U32FromU32Lane(Mask, 4) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 4) != 0)) ||
        ((U32FromU32Lane(Mask, 5) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 5) != 0)) ||
        ((U32FromU32Lane(Mask, 6) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 6) != 0)) ||
        ((U32FromU32Lane(Mask, 7) != 0xFFFFFFFF) && (U32FromU32Lane(Mask, 7) != 0))
    )
    {
        printf("\n ============== something bad happened at %d ============== \n", FaultIndex);
        printf("Mask[0] == %d", U32FromU32Lane(Mask, 0));
        printf("Mask[1] == %d", U32FromU32Lane(Mask, 1));
        printf("Mask[2] == %d", U32FromU32Lane(Mask, 2));
        printf("Mask[3] == %d", U32FromU32Lane(Mask, 3));
        printf("Mask[4] == %d", U32FromU32Lane(Mask, 4));
        printf("Mask[5] == %d", U32FromU32Lane(Mask, 5));
        printf("Mask[6] == %d", U32FromU32Lane(Mask, 6));
        printf("Mask[7] == %d", U32FromU32Lane(Mask, 7));
        fflush(stdout);
        Assert(0);
    }
}

#else

#define AssertGoodMask(Mask, FaultIndex)

#endif // ENABLE_ASSERTIONS