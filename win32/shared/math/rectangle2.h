#pragma once

#include "win32\shared\base_types.h"
#include "win32\shared\math\floats.h"
#include "win32\shared\math\vector2.h"

struct rectangle2
{
    v2 MinPoint;
    v2 MaxPoint;
};

inline rectangle2
RectangleFromMinMax(v2 Min, v2 Max)
{
    rectangle2 Result = {Min, Max};
    return Result;
}

inline rectangle2
RectCenterRadius(v2 Center, v2 Radius)
{
    rectangle2 Result = {};
    Result.MinPoint = Center - Radius;
    Result.MaxPoint = Center + Radius;
    return Result;
}

inline rectangle2
RectCenterDiameter(v2 Center, v2 Diameter)
{
    rectangle2 Result = {};
    Result.MinPoint = Center - 0.5f * Diameter;
    Result.MaxPoint = Center + 0.5f * Diameter;
    return Result;
}

inline rectangle2
RectMinDiameter(v2 MinPoint, v2 Diameter)
{
    rectangle2 Result = {};
    Result.MinPoint = MinPoint;
    Result.MaxPoint = MinPoint + Diameter;
    return Result;
}

inline b32
IsPointInRectangle(rectangle2 Rectangle, v2 TestPoint)
{
    b32 Result =
    (
        TestPoint.X >= Rectangle.MinPoint.X &&
        TestPoint.Y >= Rectangle.MinPoint.Y &&
        TestPoint.X < Rectangle.MaxPoint.X &&
        TestPoint.Y < Rectangle.MaxPoint.Y
    );
    return Result;
}

inline rectangle2
ExpandRectangle(rectangle2 Rectangle, v2 Increment)
{
	rectangle2 Result = Rectangle;
	v2 HalfIncrementVector = 0.5f * Increment;
	Result.MinPoint -= HalfIncrementVector;
	Result.MaxPoint += HalfIncrementVector;
	return Result;
}

inline v2
GetMinCorner(rectangle2 Rect)
{
	v2 Result = Rect.MinPoint;
	return Result;
}

inline v2
GetMaxCorner(rectangle2 Rect)
{
	v2 Result = Rect.MaxPoint;
	return Result;
}

inline v2
GetCenter(rectangle2 Rect)
{
	v2 Result = 0.5f * (Rect.MinPoint + Rect.MaxPoint);
	return Result;
}

inline b32
DoRectanglesOverlap(rectangle2 A, rectangle2 B)
{
	b32 NotOverlappingCondition =
	(
		(B.MaxPoint.X < A.MinPoint.X) || (B.MinPoint.X > A.MaxPoint.X)
		||
		(B.MaxPoint.Y < A.MinPoint.Y) || (B.MinPoint.Y > A.MaxPoint.Y)
	);

	return !NotOverlappingCondition;
}

inline v2
GetbarycentricPoint(rectangle2 Volume, v2 Point)
{
	v2 Result;
	Result.X = SafeRatio0((Point.X - Volume.MinPoint.X), (Volume.MaxPoint.X - Volume.MinPoint.X));
	Result.Y = SafeRatio0((Point.Y - Volume.MinPoint.Y), (Volume.MaxPoint.Y - Volume.MinPoint.Y));
	return Result;
}

inline v2
ClampPointToRectangle(v2 Point, rectangle2 Rectangle)
{
    v2 Result;
    Result.X = Clamp(Point.X, Rectangle.MinPoint.X, Rectangle.MaxPoint.X);
    Result.Y = Clamp(Point.Y, Rectangle.MinPoint.Y, Rectangle.MaxPoint.Y);
    return Result;
}

inline b32
IsInRectangle(rectangle2 Rectangle, v2 TestPoint)
{
	b32 Result =
	(
		TestPoint.X >= Rectangle.MinPoint.X &&
		TestPoint.Y >= Rectangle.MinPoint.Y &&
		TestPoint.X < Rectangle.MaxPoint.X &&
		TestPoint.Y < Rectangle.MaxPoint.Y
	);

	return Result;
}