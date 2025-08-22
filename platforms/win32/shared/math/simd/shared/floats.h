/******************************************/
/*                  Addition              */
/******************************************/
inline f32_lane
operator+(f32_lane A, f32 B)
{
    f32_lane Result = A + F32LaneFromF32(B);
    return Result;
}

inline f32_lane
operator+(f32 A, f32_lane B)
{
    f32_lane Result = F32LaneFromF32(A) + B;
    return Result;
}

inline f32_lane &
operator+=(f32_lane &A, f32_lane B)
{
    A = A + B;
    return A;
}

inline f32_lane &
operator+=(f32_lane &A, f32 B)
{
    A = A + F32LaneFromF32(B);
    return A;
}

/******************************************/
/*                  Subtraction           */
/******************************************/
inline f32_lane
operator-(f32_lane A, f32 B)
{
    f32_lane Result = A - F32LaneFromF32(B);
    return Result;
}

inline f32_lane
operator-(f32 A, f32_lane B)
{
    f32_lane Result = F32LaneFromF32(A) - B;
    return Result;
}

inline f32_lane
operator-(f32_lane A)
{
    f32_lane Result = 0.0f - A;
    return Result;
}

inline f32_lane &
operator-=(f32_lane &A, f32_lane B)
{
    A = A - B;
    return A;
}

inline f32_lane &
operator-=(f32_lane &A, f32 B)
{
    A = A - F32LaneFromF32(B);
    return A;
}

/******************************************/
/*               Multiplication           */
/******************************************/
inline f32_lane
operator*(f32_lane A, f32 B)
{
    f32_lane Result = A * F32LaneFromF32(B);
    return Result;
}

inline f32_lane
operator*(f32 A, f32_lane B)
{
    f32_lane Result = F32LaneFromF32(A) * B;
    return Result;
}

inline f32_lane &
operator*=(f32_lane &A, f32_lane B)
{
    A = A * B;
    return A;
}

inline f32_lane &
operator*=(f32_lane &A, f32 B)
{
    A = A * F32LaneFromF32(B);
    return A;
}

/******************************************/
/*                  Division              */
/******************************************/
inline f32_lane
operator/(f32_lane A, f32 B)
{
    f32_lane Result = A / F32LaneFromF32(B);
    return Result;
}

inline f32_lane
operator/(f32 A, f32_lane B)
{
    f32_lane Result = F32LaneFromF32(A) / B;
    return Result;
}

inline f32_lane &
operator/=(f32_lane &A, f32_lane B)
{
    A = A / B;
    return A;
}

inline f32_lane &
operator/=(f32_lane &A, f32 B)
{
    A = A / F32LaneFromF32(B);
    return A;
}

/******************************************/
/*             comparison                 */
/******************************************/
inline u32_lane
operator<(f32_lane A, f32 B)
{
    u32_lane Result = A < F32LaneFromF32(B);
    return Result;
}

inline u32_lane
operator<=(f32_lane A, f32 B)
{
    u32_lane Result = A <= F32LaneFromF32(B);
    return Result;
}

inline u32_lane
operator>(f32_lane A, f32 B)
{
    u32_lane Result = A > F32LaneFromF32(B);
    return Result;
}

inline u32_lane
operator>=(f32_lane A, f32 B)
{
    u32_lane Result = A >= F32LaneFromF32(B);
    return Result;
}

/******************************************/
/*                masking                 */
/******************************************/
inline f32_lane
operator&(u32_lane Mask, f32_lane Value)
{
    f32_lane Result = StaticCastU32LaneToF32Lane(Mask) & Value;
    return Result;
}

inline f32_lane
operator&(f32_lane Value, u32_lane Mask)
{
    f32_lane Result = Mask & Value;
    return Result;
}

/******************************************/
/*             other operations           */
/******************************************/
inline f32_lane
Square(f32_lane A)
{
    return A * A;
}

inline void
ConditionalAssign(f32_lane *Destination, f32_lane Source, u32_lane Mask)
{
    ConditionalAssign((u32_lane *)Destination, *(u32_lane *)&Source, Mask);
}

inline f32_lane
Clamp(f32_lane Value, f32_lane Minimum, f32_lane Maximum)
{
	f32_lane Result = Min(Max(Value, Minimum), Maximum);
	return Result;
}

inline f32_lane
Clamp(f32_lane Value, f32 Min, f32 Max)
{
	f32_lane Result = Clamp(Value, F32LaneFromF32(Min), F32LaneFromF32(Max));
	return Result;
}

inline f32_lane
Clamp01(f32_lane Value)
{
	f32_lane Result = Clamp(Value, 0.0f, 1.0f);
    return Result;
}

inline f32_lane
TranslateLinearTosRGB(f32_lane Linear)
{
    Linear = Clamp(Linear, 0.0f, 1.0f);

	f32_lane sRGB = Linear * 12.92f;

    f32_lane Expression = 1.055f * Power(Linear, 1.0f/2.4f) - 0.055f;

    ConditionalAssign(&sRGB, Expression, Linear > 0.0031308f);

	return sRGB;
}

inline f32_lane
Lerp(f32_lane A, f32_lane B, f32_lane T)
{
	f32_lane Result = ((1.0f - T) * A) + (T * B);
	return Result;
}

inline f32_lane
SafeRatioN(f32_lane Dividend, f32_lane Divisor, f32_lane AltValue)
{
	f32_lane Result = AltValue;

    ConditionalAssign(&Result, Dividend / Divisor, (Divisor != F32LaneFromF32(0)));

	return Result;
}

inline f32_lane
SafeRatio0(f32_lane Dividend, f32_lane Divisor)
{
	return SafeRatioN(Dividend, Divisor, F32LaneFromF32(0));
}

inline f32_lane
SafeRatio1(f32_lane Dividend, f32_lane Divisor)
{
	return SafeRatioN(Dividend, Divisor, F32LaneFromF32(1.0f));
}