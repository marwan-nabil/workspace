#pragma once

#include "portable\shared\base_types.h"
#include "win32\shared\math\scalar_conversions.h"
#include "win32\shared\math\vector3.h"

union v4
{
	struct
	{
		f32 X;
		f32 Y;
		f32 Z;
		f32 W;
	};
	struct
	{
		v3 XYZ;
		f32 Ignored0;
	};
	struct
	{
		f32 Red;
		f32 Green;
		f32 Blue;
		f32 Alpha;
	};
	f32 E[4];
};

inline v4
V4(f32 X, f32 Y, f32 Z, f32 W)
{
	v4 Result;
	Result.X = X;
	Result.Y = Y;
	Result.Z = Z;
	Result.W = W;
	return Result;
}

inline v4
V4(v3 XYZ, f32 W)
{
	v4 Result;
	Result.X = XYZ.X;
	Result.Y = XYZ.Y;
	Result.Z = XYZ.Z;
	Result.W = W;
	return Result;
}

inline u32
PackColor(v4 Color)
{
    u32 Result =
    (
        (RoundF32ToU32(Color.Alpha * 255.0f) << 24) |
        (RoundF32ToU32(Color.Red * 255.0f) << 16) |
        (RoundF32ToU32(Color.Green * 255.0f) << 8) |
        RoundF32ToU32(Color.Blue * 255.0f)
    );
    return Result;
}