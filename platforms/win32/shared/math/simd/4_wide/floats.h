/******************************************/
/*         arithmetic operations          */
/******************************************/
inline f32_lane
operator+(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm_add_ps(A, B);
    return Result;
}

inline f32_lane
operator-(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm_sub_ps(A, B);
    return Result;
}

inline f32_lane
operator*(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm_mul_ps(A, B);
    return Result;
}

inline f32_lane
operator/(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm_div_ps(A, B);
    return Result;
}

/******************************************/
/*          bitwise operations            */
/******************************************/
inline f32_lane
operator&(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm_and_ps(A, B);
    return Result;
}

inline f32_lane
AndNot(f32_lane A, f32_lane B)
{
    // (~A) & B
    f32_lane Result = _mm_andnot_ps(A, B);
    return Result;
}

inline f32_lane
operator|(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm_or_ps(A, B);
    return Result;
}

inline f32_lane
operator^(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm_xor_ps(A, B);
    return Result;
}

/******************************************/
/*         comparison operations          */
/******************************************/
inline u32_lane
operator<(f32_lane A, f32_lane B)
{
    u32_lane Result = _mm_castps_si128(_mm_cmplt_ps(A, B));
    return Result;
}

inline u32_lane
operator<=(f32_lane A, f32_lane B)
{
    u32_lane Result = _mm_castps_si128(_mm_cmple_ps(A, B));
    return Result;
}

inline u32_lane
operator>(f32_lane A, f32_lane B)
{
    u32_lane Result = _mm_castps_si128(_mm_cmpgt_ps(A, B));
    return Result;
}

inline u32_lane
operator>=(f32_lane A, f32_lane B)
{
    u32_lane Result = _mm_castps_si128(_mm_cmpge_ps(A, B));
    return Result;
}

inline u32_lane
operator==(f32_lane A, f32_lane B)
{
    u32_lane Result = _mm_castps_si128(_mm_cmpeq_ps(A, B));
    return Result;
}

inline u32_lane
operator!=(f32_lane A, f32_lane B)
{
    u32_lane Result = _mm_castps_si128(_mm_cmpneq_ps(A, B));
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
    f32_lane Result = _mm_sqrt_ps(A);
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
        ElementPointer[3];
    return NarrowValue;
}

inline f32_lane
Max(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm_max_ps(A, B);
    return Result;
}

inline f32_lane
Min(f32_lane A, f32_lane B)
{
    f32_lane Result = _mm_min_ps(A, B);
    return Result;
}

inline f32_lane
Power(f32_lane A, f32 Exponent)
{
    f32_lane Result = F32LaneFromF32
    (
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
        Power(F32FromF32Lane(A, 3), F32FromF32Lane(Exponent, 3)),
        Power(F32FromF32Lane(A, 2), F32FromF32Lane(Exponent, 2)),
        Power(F32FromF32Lane(A, 1), F32FromF32Lane(Exponent, 1)),
        Power(F32FromF32Lane(A, 0), F32FromF32Lane(Exponent, 0))
    );
    return Result;
}