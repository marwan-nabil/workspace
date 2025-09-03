/******************************************/
/*         arithmetic operations          */
/******************************************/
inline f32_lane
operator+(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm256_add_ps(A, B);
    return Result;
}

inline f32_lane
operator-(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm256_sub_ps(A, B);
    return Result;
}

inline f32_lane
operator*(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm256_mul_ps(A, B);
    return Result;
}

inline f32_lane
operator/(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm256_div_ps(A, B);
    return Result;
}

/******************************************/
/*          bitwise operations            */
/******************************************/
inline f32_lane
operator&(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm256_and_ps(A, B);
    return Result;
}

inline f32_lane
AndNot(f32_lane A, f32_lane B)
{
    // (~A) & B
    f32_lane Result = _mm256_andnot_ps(A, B);
    return Result;
}

inline f32_lane
operator|(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm256_or_ps(A, B);
    return Result;
}

inline f32_lane
operator^(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm256_xor_ps(A, B);
    return Result;
}

/******************************************/
/*         comparison operations          */
/******************************************/
inline u32_lane
operator<(f32_lane A, f32_lane B)
{
    u32_lane Result = _mm256_castps_si256(_mm256_cmp_ps(A, B, _CMP_LT_OQ));
    return Result;
}

inline u32_lane
operator<=(f32_lane A, f32_lane B)
{
    u32_lane Result = _mm256_castps_si256(_mm256_cmp_ps(A, B, _CMP_LE_OQ));
    return Result;
}

inline u32_lane
operator>(f32_lane A, f32_lane B)
{
    u32_lane Result = _mm256_castps_si256(_mm256_cmp_ps(A, B, _CMP_GT_OQ));
    return Result;
}

inline u32_lane
operator>=(f32_lane A, f32_lane B)
{
    u32_lane Result = _mm256_castps_si256(_mm256_cmp_ps(A, B, _CMP_GE_OQ));
    return Result;
}

inline u32_lane
operator==(f32_lane A, f32_lane B)
{
    u32_lane Result = _mm256_castps_si256(_mm256_cmp_ps(A, B, _CMP_EQ_OQ));
    return Result;
}

inline u32_lane
operator!=(f32_lane A, f32_lane B)
{
    u32_lane Result = _mm256_castps_si256(_mm256_cmp_ps(A, B, _CMP_NEQ_OQ));
    return Result;
}

/******************************************/
/*                gathers                 */
/******************************************/
inline f32_lane
GatherF32Implementation(void *Base, u32 Stride, u32_lane Indices)
{
    u32 *IndexPointer = (u32 *)&Indices;
    f32_lane Result = F32LaneFromF32
    (
        *(f32 *)((u8 *)Base + IndexPointer[7] * Stride),
        *(f32 *)((u8 *)Base + IndexPointer[6] * Stride),
        *(f32 *)((u8 *)Base + IndexPointer[5] * Stride),
        *(f32 *)((u8 *)Base + IndexPointer[4] * Stride),
        *(f32 *)((u8 *)Base + IndexPointer[3] * Stride),
        *(f32 *)((u8 *)Base + IndexPointer[2] * Stride),
        *(f32 *)((u8 *)Base + IndexPointer[1] * Stride),
        *(f32 *)((u8 *)Base + IndexPointer[0] * Stride)
    );
    return Result;
}

/******************************************/
/*             Common Functions           */
/******************************************/
inline f32_lane
SquareRoot(f32_lane A)
{
    f32_lane Result = _mm256_sqrt_ps(A);
    return Result;
}

inline f32
HorizontalAdd(f32_lane WideValue)
{
    f32 *ElementPointer = (f32 *)&WideValue;
    f32 NarrowValue =
        ElementPointer[0] +
        ElementPointer[1] +
        ElementPointer[2] +
        ElementPointer[3] +
        ElementPointer[4] +
        ElementPointer[5] +
        ElementPointer[6] +
        ElementPointer[7];
    return NarrowValue;
}

inline f32_lane
Max(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm256_max_ps(A, B);
    return Result;
}

inline f32_lane
Min(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm256_min_ps(A, B);
    return Result;
}

inline f32_lane
Power(f32_lane A, f32 Exponent)
{
    f32_lane Result = F32LaneFromF32
    (
        Power(F32FromF32Lane(A, 7), Exponent),
        Power(F32FromF32Lane(A, 6), Exponent),
        Power(F32FromF32Lane(A, 5), Exponent),
        Power(F32FromF32Lane(A, 4), Exponent),
        Power(F32FromF32Lane(A, 3), Exponent),
        Power(F32FromF32Lane(A, 2), Exponent),
        Power(F32FromF32Lane(A, 1), Exponent),
        Power(F32FromF32Lane(A, 0), Exponent)
    );
    return Result;
}

inline f32_lane
Power(f32_lane A, f32_lane Exponent)
{
    f32_lane Result = F32LaneFromF32
    (
        Power(F32FromF32Lane(A, 7), F32FromF32Lane(Exponent, 7)),
        Power(F32FromF32Lane(A, 6), F32FromF32Lane(Exponent, 6)),
        Power(F32FromF32Lane(A, 5), F32FromF32Lane(Exponent, 5)),
        Power(F32FromF32Lane(A, 4), F32FromF32Lane(Exponent, 4)),
        Power(F32FromF32Lane(A, 3), F32FromF32Lane(Exponent, 3)),
        Power(F32FromF32Lane(A, 2), F32FromF32Lane(Exponent, 2)),
        Power(F32FromF32Lane(A, 1), F32FromF32Lane(Exponent, 1)),
        Power(F32FromF32Lane(A, 0), F32FromF32Lane(Exponent, 0))
    );
    return Result;
}