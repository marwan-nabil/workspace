#pragma once

#include "portable\shared\base_types.h"
#include "win32\shared\math\floats.h"
#include "win32\shared\math\rectangle2.h"
#include "win32\shared\math\vector3.h"

struct rectangle3
{
	v3 MinPoint;
	v3 MaxPoint;
};

inline rectangle3
RectangleFromMinMax(v3 Min, v3 Max)
{
	rectangle3 Result = {Min, Max};
	return Result;
}

inline rectangle3
RectCenterRadius(v3 Center, v3 Radius)
{
	rectangle3 Result = {};
	Result.MinPoint = Center - Radius;
	Result.MaxPoint = Center + Radius;
	return Result;
}

inline rectangle3
RectCenterDiameter(v3 Center, v3 Diameter)
{
	rectangle3 Result = {};
	Result.MinPoint = Center - 0.5f * Diameter;
	Result.MaxPoint = Center + 0.5f * Diameter;
	return Result;
}

inline rectangle3
RectMinDiameter(v3 MinPoint, v3 Diameter)
{
	rectangle3 Result = {};
	Result.MinPoint = MinPoint;
	Result.MaxPoint = MinPoint + Diameter;
	return Result;
}

inline b32
IsInRectangle(rectangle3 Rectangle, v3 TestPoint)
{
	b32 Result =
	(
		TestPoint.X >= Rectangle.MinPoint.X &&
		TestPoint.Y >= Rectangle.MinPoint.Y &&
		TestPoint.Z >= Rectangle.MinPoint.Z &&
		TestPoint.X < Rectangle.MaxPoint.X &&
		TestPoint.Y < Rectangle.MaxPoint.Y &&
		TestPoint.Z < Rectangle.MaxPoint.Z
	);
	return Result;
}

inline rectangle3
ExpandRectangle(rectangle3 Rectangle, v3 Increment)
{
	rectangle3 Result = Rectangle;
	v3 HalfIncrementVector = 0.5f * Increment;
	Result.MinPoint -= HalfIncrementVector;
	Result.MaxPoint += HalfIncrementVector;
	return Result;
}

inline v3
GetMinCorner(rectangle3 Rect)
{
	v3 Result = Rect.MinPoint;
	return Result;
}

inline v3
GetMaxCorner(rectangle3 Rect)
{
	v3 Result = Rect.MaxPoint;
	return Result;
}

inline v3
GetCenter(rectangle3 Rect)
{
	v3 Result = 0.5f * (Rect.MinPoint + Rect.MaxPoint);
	return Result;
}

inline b32
DoRectanglesOverlap(rectangle3 A, rectangle3 B)
{
	b32 NotOverlappingCondition =
	(
		(B.MaxPoint.X <= A.MinPoint.X) || (B.MinPoint.X >= A.MaxPoint.X) ||
		(B.MaxPoint.Y <= A.MinPoint.Y) || (B.MinPoint.Y >= A.MaxPoint.Y) ||
		(B.MaxPoint.Z <= A.MinPoint.Z) || (B.MinPoint.Z >= A.MaxPoint.Z)
	);

	return !NotOverlappingCondition;
}

inline v3
GetbarycentricPoint(rectangle3 Volume, v3 Point)
{
	v3 Result;
	Result.X = SafeRatio0((Point.X - Volume.MinPoint.X), (Volume.MaxPoint.X - Volume.MinPoint.X));
	Result.Y = SafeRatio0((Point.Y - Volume.MinPoint.Y), (Volume.MaxPoint.Y - Volume.MinPoint.Y));
	Result.Z = SafeRatio0((Point.Z - Volume.MinPoint.Z), (Volume.MaxPoint.Z - Volume.MinPoint.Z));
	return Result;
}

inline rectangle2
Rectangle3ToRectangle2(rectangle3 A)
{
    rectangle2 Result;
    Result.MinPoint = A.MinPoint.XY;
    Result.MaxPoint = A.MaxPoint.XY;
    return Result;
}