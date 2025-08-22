/******************************************/
/*                  Addition              */
/******************************************/
inline u32_lane
operator+(u32_lane A, u32 B)
{
    u32_lane Result = A + U32LaneFromU32(B);
    return Result;
}

inline u32_lane
operator+(u32 A, u32_lane B)
{
    u32_lane Result = B + A;
    return Result;
}

inline u32_lane &
operator+=(u32_lane &A, u32_lane B)
{
    A = A + B;
    return A;
}

inline u32_lane &
operator+=(u32_lane &A, u32 B)
{
    A = A + B;
    return A;
}

/******************************************/
/*                  Subtraction           */
/******************************************/
inline u32_lane
operator-(u32_lane A, u32 B)
{
    u32_lane Result = A - U32LaneFromU32(B);
    return Result;
}

inline u32_lane
operator-(u32 A, u32_lane B)
{
    u32_lane Result = U32LaneFromU32(A) - B;
    return Result;
}

inline u32_lane
operator-(u32_lane A)
{
    u32_lane Result = U32LaneFromU32(0) - A;
    return Result;
}

inline u32_lane &
operator-=(u32_lane &A, u32_lane B)
{
    A = A - B;
    return A;
}

inline u32_lane &
operator-=(u32_lane &A, u32 B)
{
    A = A - U32LaneFromU32(B);
    return A;
}

/******************************************/
/*                  bitwise and           */
/******************************************/
inline u32_lane
operator&(u32_lane A, u32 B)
{
    u32_lane Result = A & U32LaneFromU32(B);
    return Result;
}

inline u32_lane
operator&(u32 A, u32_lane B)
{
    u32_lane Result = U32LaneFromU32(A) & B;
    return Result;
}

inline u32_lane &
operator&=(u32_lane &A, u32_lane B)
{
    A = A & B;
    return A;
}

inline u32_lane &
operator&=(u32_lane &A, u32 B)
{
    A = A & U32LaneFromU32(B);
    return A;
}

/******************************************/
/*                  bitwise andnot        */
/******************************************/
inline u32_lane
AndNot(u32_lane A, u32 B)
{
    u32_lane Result = AndNot(A, U32LaneFromU32(B));
    return Result;
}

inline u32_lane
AndNot(u32 A, u32_lane B)
{
    u32_lane Result = AndNot(U32LaneFromU32(A), B);
    return Result;
}

/******************************************/
/*                  bitwise or            */
/******************************************/
inline u32_lane
operator|(u32_lane A, u32 B)
{
    u32_lane Result = A | U32LaneFromU32(B);
    return Result;
}

inline u32_lane
operator|(u32 A, u32_lane B)
{
    u32_lane Result = U32LaneFromU32(A) | B;
    return Result;
}

inline u32_lane &
operator|=(u32_lane &A, u32_lane B)
{
    A = A | B;
    return A;
}

inline u32_lane &
operator|=(u32_lane &A, u32 B)
{
    A = A | U32LaneFromU32(B);
    return A;
}

/******************************************/
/*                  bitwise xor           */
/******************************************/
inline u32_lane
operator^(u32_lane A, u32 B)
{
    u32_lane Result = A ^ U32LaneFromU32(B);
    return Result;
}

inline u32_lane
operator^(u32 A, u32_lane B)
{
    u32_lane Result = U32LaneFromU32(A) ^ B;
    return Result;
}

inline u32_lane &
operator^=(u32_lane &A, u32_lane B)
{
    A = A ^ B;
    return A;
}

inline u32_lane &
operator^=(u32_lane &A, u32 B)
{
    A = A ^ U32LaneFromU32(B);
    return A;
}

/******************************************/
/*             other operations           */
/******************************************/
inline void
ConditionalAssign(u32_lane *Destination, u32_lane Source, u32_lane Mask)
{
    *Destination = AndNot(Mask, *Destination) | (Mask & Source);
}

inline u32_lane
Max(u32_lane A, u32_lane B)
{
    u32_lane Result;
    u32_lane ComparisonMask = (A > B);
    ConditionalAssign(&Result, A, ComparisonMask);
    ConditionalAssign(&Result, B, ~ComparisonMask);
    return Result;
}

inline u32_lane
Min(u32_lane A, u32_lane B)
{
    u32_lane Result;
    u32_lane ComparisonMask = (A < B);
    ConditionalAssign(&Result, A, ComparisonMask);
    ConditionalAssign(&Result, B, ~ComparisonMask);
    return Result;
}

inline u32_lane
Clamp(u32_lane Value, u32_lane Minimum, u32_lane Maximum)
{
	u32_lane Result = Value;
    Result = Max(Value, Minimum);
    Result = Min(Value, Maximum);
	return Result;
}