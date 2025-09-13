#pragma once

#include "win32\shared\base_types.h"
#include "win32\shared\math\floats.h"

union v2
{
	struct
	{
		f32 X;
		f32 Y;
	};
	f32 E[2];
};

inline v2
V2(f32 X, f32 Y)
{
    v2 Result;
    Result.X = X;
    Result.Y = Y;
    return Result;
}

inline v2
operator+(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    return Result;
}

inline v2 &
operator+=(v2 &A, v2 B)
{
    A.X += B.X;
    A.Y += B.Y;
    return A;
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    return Result;
}

inline v2
operator-(v2 A)
{
    v2 Result;
    Result.X = -A.X;
    Result.Y = -A.Y;
    return Result;
}

inline v2 &
operator-=(v2 &A, v2 B)
{
    A.X -= B.X;
    A.Y -= B.Y;
    return A;
}

inline v2
operator*(f32 A, v2 B)
{
    v2 Result;
    Result.X = A * B.X;
    Result.Y = A * B.Y;
    return Result;
}

inline v2
operator*(v2 A, f32 B)
{
    v2 Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    return Result;
}

inline v2 &
operator*=(v2 &A, f32 B)
{
    A.X *= B;
    A.Y *= B;
    return A;
}

inline v2
operator/(v2 A, f32 B)
{
    v2 Result;
    Result.X = A.X / B;
    Result.Y = A.Y / B;
    return Result;
}

inline v2 &
operator/=(v2 &A, f32 B)
{
    A.X /= B;
    A.Y /= B;
    return A;
}

inline v2
HadamardProduct(v2 A, v2 B)
{
    v2 Result;
    Result.X = A.X * B.X;
    Result.Y = A.Y * B.Y;
    return Result;
}

inline f32
InnerProduct(v2 A, v2 B)
{
    f32 Result = A.X * B.X + A.Y * B.Y;
    return Result;
}

inline f32
LengthSquared(v2 A)
{
    f32 Result = InnerProduct(A, A);
    return Result;
}

inline f32
Length(v2 A)
{
    f32 Result = SquareRoot(LengthSquared(A));
    return Result;
}

inline v2
Normalize(v2 A)
{
    v2 Result;
    Result = A / SquareRoot(LengthSquared(A));
    return Result;
}

inline v2
Clamp01(v2 A)
{
	v2 Result;
	Result.X = Clamp01(A.X);
	Result.Y = Clamp01(A.Y);
	return Result;
}