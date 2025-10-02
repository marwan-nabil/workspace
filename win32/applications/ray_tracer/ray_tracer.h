#pragma once

#include "portable\shared\base_types.h"
#include "win32\shared\math\vector3.h"
#include "win32\shared\math\simd\shared\random.h"

#define BOUNCES_PER_RAY 8
#define RAYS_PER_PIXEL 256
#define RAY_BATCHES_PER_PIXEL (RAYS_PER_PIXEL / SIMD_NUMBEROF_LANES)

#pragma pack(push, 1)
struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    i32 Width;
    i32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    i32 HorizontalResolution;
    i32 VerticalResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
};
#pragma pack(pop)

struct image_u32
{
    u32 WidthInPixels;
    u32 HeightInPixels;
    u32 *Pixels;
};

struct material
{
    v3 EmmissionColor;
    v3 ReflectionColor;
    brdf_table *BrdfTable;
    f32 Specularity;
};

struct plane
{
    v3 Normal;
    v3 Tangent;
    v3 BiTangent;
    f32 Distance;
    u32 MaterialIndex;
};

struct sphere
{
    v3 Position;
    f32 Radius;
    u32 MaterialIndex;
};

struct camera
{
    v3 Position;
    coordinate_set CoordinateSet;
};

struct film
{
    v3 Center;
    f32 DistanceFromCamera;
    f32 HalfWidth;
    f32 HalfHeight;
    f32 HalfPixelWidth;
    f32 HalfPixelHeight;
};

struct world
{
    coordinate_set CoordinateSet;
    camera Camera;
    film Film;

    material *Materials;
    u32 MaterialsCount;

    plane *Planes;
    u32 PlanesCount;

    sphere *Spheres;
    u32 SpheresCount;
};

struct rendering_parameters
{
    u8 CoreCount;

    u32 TileWidthInPixels;
    u32 TileHeightInPixels;
    u32 TileCountX;
    u32 TileCountY;
    u32 TotalTileCount;

    f32 HitDistanceLowerLimit;
    f32 ToleranceToZero;
};

struct work_order
{
    world *World;
    image_u32 *Image;
    rendering_parameters *RenderingParameters;
    random_series_lane Entropy;

    u32 StartPixelX;
    u32 StartPixelY;
    u32 EndPixelX;
    u32 EndPixelY;
};

struct work_queue
{
    u32 WorkOrderCount;
    work_order *WorkOrders;

    volatile i64 TotalRayBouncesComputed;
    volatile i64 TotalLoopsComputed;
    volatile i64 TotalTilesDone;
    volatile i64 NextWorkOrderIndex;
};