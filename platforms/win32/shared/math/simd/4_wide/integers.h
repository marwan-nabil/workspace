/******************************************/
/*          arithmetic operations         */
/******************************************/
inline u32_lane
operator+(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm_add_epi32(A, B);
    return Result;
}

inline u32_lane
operator-(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm_sub_epi32(A, B);
    return Result;
}

/******************************************/
/*             bitwise operations         */
/******************************************/
inline u32_lane
operator&(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm_and_si128(A, B);
    return Result;
}

inline u32_lane
AndNot(u32_lane A, u32_lane B)
{
    // (~A) & B
    u32_lane Result = _mm_andnot_si128(A, B);
    return Result;
}

inline u32_lane
operator|(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm_or_si128(A, B);
    return Result;
}

inline u32_lane
operator^(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm_xor_si128(A, B);
    return Result;
}

inline u32_lane
operator~(u32_lane A)
{
    u32_lane Result = _mm_xor_si128(A, _mm_set1_epi32(0xFFFFFFFF));
    return Result;
}

inline u32_lane
operator<<(u32_lane Value, u32 Shift)
{
    u32_lane Result = _mm_slli_epi32(Value, Shift);
    return Result;
}

inline u32_lane
operator>>(u32_lane Value, u32 Shift)
{
    u32_lane Result = _mm_srli_epi32(Value, Shift);
    return Result;
}

/******************************************/
/*           comparison operations        */
/******************************************/
inline u32_lane
operator<(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm_cmplt_epi32(A, B);
    return Result;
}

inline u32_lane
operator>(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm_cmpgt_epi32(A, B);
    return Result;
}

inline u32_lane
operator==(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm_cmpeq_epi32(A, B);
    return Result;
}

inline u32_lane
operator!=(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm_xor_epi32
    (
        _mm_cmpeq_epi32(A, B),
        _mm_set1_epi32(0xFFFFFFFF)
    );
    return Result;
}

/******************************************/
/*             other operations           */
/******************************************/
inline b32
MaskIsAllZeroes(u32_lane Mask)
{
    b32 Result = (_mm_movemask_epi8(Mask) == 0);
    return Result;
}

// TODO: decide which implmenetation of HorizontalAdd() is better
// inline u64
// HorizontalAdd(u32_lane WideValue)
// {
//     u32 *ElementPointer = (u32 *)&WideValue;
//     u32 NarrowValue =
//         (u64)ElementPointer[0] +
//         (u64)ElementPointer[1] +
//         (u64)ElementPointer[2] +
//         (u64)ElementPointer[3];
//     return NarrowValue;
// }

inline u32
HorizontalAdd(u32_lane WideValue)
{
    u32 NarrowValue = _mm_extract_epi32
    (
        _mm_hadd_epi32
        (
            _mm_hadd_epi32(WideValue, _mm_set1_epi32(0)),
            _mm_set1_epi32(0)
        ),
        0
    );
    return NarrowValue;
}