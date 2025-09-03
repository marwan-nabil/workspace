/******************************************/
/*          arithmetic operations         */
/******************************************/
inline u32_lane
operator+(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm256_add_epi32(A, B);
    return Result;
}

inline u32_lane
operator-(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm256_sub_epi32(A, B);
    return Result;
}

/******************************************/
/*             bitwise operations         */
/******************************************/
inline u32_lane
operator&(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm256_and_si256(A, B);
    return Result;
}

inline u32_lane
AndNot(u32_lane A, u32_lane B)
{
    // (~A) & B
    u32_lane Result = _mm256_andnot_si256(A, B);
    return Result;
}

inline u32_lane
operator|(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm256_or_si256(A, B);
    return Result;
}

inline u32_lane
operator^(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm256_xor_si256(A, B);
    return Result;
}

inline u32_lane
operator~(u32_lane A)
{
    u32_lane Result = _mm256_xor_si256(A, _mm256_set1_epi32(0xFFFFFFFF));
    return Result;
}

inline u32_lane
operator<<(u32_lane Value, u32 Shift)
{
    u32_lane Result = _mm256_slli_epi32(Value, Shift);
    return Result;
}

inline u32_lane
operator>>(u32_lane Value, u32 Shift)
{
    u32_lane Result = _mm256_srli_epi32(Value, Shift);
    return Result;
}

/******************************************/
/*           comparison operations        */
/******************************************/
inline u32_lane
operator<(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm256_cmpgt_epi32(B, A);
    return Result;
}

inline u32_lane
operator>(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm256_cmpgt_epi32(A, B);
    return Result;
}

inline u32_lane
operator==(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm256_cmpeq_epi32(A, B);
    return Result;
}

inline u32_lane
operator!=(u32_lane A, u32_lane B)
{
    u32_lane Result = _mm256_xor_si256
    (
        _mm256_cmpeq_epi32(A, B),
        _mm256_set1_epi32(0xFFFFFFFF)
    );
    return Result;
}

/******************************************/
/*             other operations           */
/******************************************/
inline b32
MaskIsAllZeroes(u32_lane Mask)
{
    b32 Result = (_mm256_movemask_epi8(Mask) == 0);
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
//         (u64)ElementPointer[3] +
//         (u64)ElementPointer[4] +
//         (u64)ElementPointer[5] +
//         (u64)ElementPointer[6] +
//         (u64)ElementPointer[7];
//     return NarrowValue;
// }

inline u32
HorizontalAdd(u32_lane WideValue)
{
    WideValue = _mm256_hadd_epi32
    (
        _mm256_hadd_epi32(WideValue, _mm256_set1_epi32(0)),
        _mm256_set1_epi32(0)
    );
    u32 NarrowValue = _mm256_extract_epi32(WideValue, 0) + _mm256_extract_epi32(WideValue, 4);
    return NarrowValue;
}