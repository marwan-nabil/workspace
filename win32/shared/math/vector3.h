#pragma once

#include "portable\shared\base_types.h"
#include "win32\shared\math\floats.h"
#include "win32\shared\math\vector2.h"

union v3
{
	struct
	{
		f32 X;
		f32 Y;
		f32 Z;
	};
	struct
	{
		f32 Red;
		f32 Green;
		f32 Blue;
	};
	struct
	{
		v2 XY;
		f32 Ignored0;
	};
	struct
	{
		f32 Ignored1;
		v2 YZ;
	};
	f32 E[3];
};

struct coordinate_set
{
    v3 X;
    v3 Y;
    v3 Z;
};

inline v3
V3(f32 X, f32 Y, f32 Z)
{
    v3 Result;
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    return Result;
}

inline v3
V3(v2 XY, f32 Z)
{
	v3 Result;
	Result.X = XY.X;
	Result.Y = XY.Y;
	Result.Z = Z;
	return Result;
}

inline v3
operator+(v3 A, v3 B)
{
    v3 Result;
    Result.X = A.X + B.X;
    Result.Y = A.Y + B.Y;
    Result.Z = A.Z + B.Z;
    return Result;
}

inline v3 &
operator+=(v3 &A, v3 B)
{
    A.X += B.X;
    A.Y += B.Y;
    A.Z += B.Z;
    return A;
}

inline v3
operator-(v3 A, v3 B)
{
    v3 Result;
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    Result.Z = A.Z - B.Z;
    return Result;
}

inline v3
operator-(v3 A)
{
    v3 Result;
    Result.X = -A.X;
    Result.Y = -A.Y;
    Result.Z = -A.Z;
    return Result;
}

inline v3 &
operator-=(v3 &A, v3 B)
{
    A.X -= B.X;
    A.Y -= B.Y;
    A.Z -= B.Z;
    return A;
}

inline v3
operator*(f32 A, v3 B)
{
    v3 Result;
    Result.X = A * B.X;
    Result.Y = A * B.Y;
    Result.Z = A * B.Z;
    return Result;
}

inline v3
operator*(v3 A, f32 B)
{
    v3 Result;
    Result.X = A.X * B;
    Result.Y = A.Y * B;
    Result.Z = A.Z * B;
    return Result;
}

inline v3 &
operator*=(v3 &A, f32 B)
{
    A.X *= B;
    A.Y *= B;
    A.Z *= B;
    return A;
}

inline v3
operator/(v3 A, f32 B)
{
    v3 Result;
    Result.X = A.X / B;
    Result.Y = A.Y / B;
    Result.Z = A.Z / B;
    return Result;
}

inline v3 &
operator/=(v3 &A, f32 B)
{
    A.X /= B;
    A.Y /= B;
    A.Z /= B;
    return A;
}

inline f32
InnerProduct(v3 A, v3 B)
{
    f32 Result = A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    return Result;
}

inline v3
HadamardProduct(v3 A, v3 B)
{
    v3 Result = V3(A.X * B.X, A.Y * B.Y, A.Z * B.Z);
    return Result;
}

inline v3
CrossProduct(v3 A, v3 B)
{
    v3 Result;

    Result.X = A.Y * B.Z - A.Z * B.Y;
    Result.Y = A.Z * B.X - A.X * B.Z;
    Result.Z = A.X * B.Y - A.Y * B.X;

    return Result;
}

inline f32
LengthSquared(v3 A)
{
    f32 Result = InnerProduct(A, A);
    return Result;
}

inline f32
Length(v3 A)
{
    f32 Result = SquareRoot(LengthSquared(A));
    return Result;
}

inline v3
Normalize(v3 A)
{
    v3 Result;
    Result = A / SquareRoot(LengthSquared(A));
    return Result;
}

inline v3
Lerp(v3 A, v3 B, f32 t)
{
    v3 Result = (1.0f - t) * A + t * B;
    return Result;
}

inline v3
Clamp01(v3 A)
{
	v3 Result;
	Result.X = Clamp01(A.X);
	Result.Y = Clamp01(A.Y);
	Result.Z = Clamp01(A.Z);
	return Result;
}