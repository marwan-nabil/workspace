// TODO: fix BRDF table lookup or use a different BRDF implementation
#include <Windows.h>
#include <intrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>
#include <math.h>
#include <float.h>
#include <time.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
#include "win32\shared\file_system\files.h"
#include "win32\shared\math\constants.h"
#include "win32\shared\math\integers.h"
#include "win32\shared\math\floats.h"
#include "win32\shared\math\scalar_conversions.h"
#include "win32\shared\math\transcendentals.h"
#include "win32\shared\math\random.h"
#include "win32\shared\math\vector2.h"
#include "win32\shared\math\vector3.h"
#include "win32\shared\math\vector4.h"

#if (SIMD_NUMBEROF_LANES == 1)
#   include "win32\shared\math\simd\1_wide\math.h"
#elif (SIMD_NUMBEROF_LANES == 4)
#   include "win32\shared\math\simd\4_wide\math.h"
#elif (SIMD_NUMBEROF_LANES == 8)
#   include "win32\shared\math\simd\8_wide\math.h"
#else
#   error "the defined SIMD_NUMBEROF_LANES is still not supported"
#endif // SIMD_NUMBEROF_LANES == 1

#include "win32\shared\math\simd\shared\math.h"

#if (SIMD_NUMBEROF_LANES == 1)
#   include "win32\shared\math\simd\1_wide\conversions.h"
#   include "win32\shared\math\simd\1_wide\integers.h"
#   include "win32\shared\math\simd\1_wide\floats.h"
#   include "win32\shared\math\simd\1_wide\assertions.h"
#elif (SIMD_NUMBEROF_LANES == 4)
#   include "win32\shared\math\simd\4_wide\conversions.h"
#   include "win32\shared\math\simd\4_wide\integers.h"
#   include "win32\shared\math\simd\4_wide\floats.h"
#   include "win32\shared\math\simd\4_wide\assertions.h"
#   include "win32\shared\math\simd\4_wide\vector3.h"
#elif (SIMD_NUMBEROF_LANES == 8)
#   include "win32\shared\math\simd\8_wide\conversions.h"
#   include "win32\shared\math\simd\8_wide\integers.h"
#   include "win32\shared\math\simd\8_wide\floats.h"
#   include "win32\shared\math\simd\8_wide\assertions.h"
#   include "win32\shared\math\simd\8_wide\vector3.h"
#else
#   error "the defined SIMD_NUMBEROF_LANES is still not supported"
#endif // SIMD_NUMBEROF_LANES == 1

#include "win32\shared\math\simd\shared\conversions.h"
#include "win32\shared\math\simd\shared\integers.h"
#include "win32\shared\math\simd\shared\floats.h"
#include "win32\shared\math\simd\shared\vector3.h"
#include "win32\shared\math\simd\shared\random.h"

#include "brdf.h"
#include "ray_tracer.h"

inline void
WriteBitmapImage(u32 *Pixels, u32 WidthInPixels, u32 HeightInPixels, char *FileName)
{
    u32 OutputPixelSize = WidthInPixels * HeightInPixels * sizeof(u32);

    bitmap_header BitmapHeader = {};
    BitmapHeader.FileType = 0x4D42;
    BitmapHeader.FileSize = sizeof(BitmapHeader) + OutputPixelSize;
    BitmapHeader.BitmapOffset = sizeof(BitmapHeader);
    BitmapHeader.Size = sizeof(BitmapHeader) - 14;
    BitmapHeader.Width = WidthInPixels;
    BitmapHeader.Height = -(i32)HeightInPixels;
    BitmapHeader.Planes = 1;
    BitmapHeader.BitsPerPixel = 32;
    BitmapHeader.Compression = 0;
    BitmapHeader.SizeOfBitmap = OutputPixelSize;
    BitmapHeader.HorizontalResolution = 0;
    BitmapHeader.VerticalResolution = 0;
    BitmapHeader.ColorsUsed = 0;
    BitmapHeader.ColorsImportant = 0;

    FILE *OutputFile = fopen(FileName, "wb");
    if (OutputFile)
    {
        fwrite(&BitmapHeader, sizeof(BitmapHeader), 1, OutputFile);
        fwrite((void *)Pixels, OutputPixelSize, 1, OutputFile);
        fclose(OutputFile);
    }
    else
    {
        printf("ERROR: unable to create the output bitmap file.\n");
    }
}

inline image_u32
CreateImage(u32 Width, u32 Height)
{
    image_u32 OutputImage = {};
    OutputImage.WidthInPixels = Width;
    OutputImage.HeightInPixels = Height;
    OutputImage.Pixels = (u32 *)malloc
    (
        OutputImage.WidthInPixels *
        OutputImage.HeightInPixels *
        sizeof(u32)
    );
    return OutputImage;
}

inline u32 *
GetPixelPointer(image_u32 *Image, u32 X, u32 Y)
{
    u32 *Result = Image->Pixels + Y * Image->WidthInPixels + X;
    return Result;
}

inline v3
RenderPixel
(
    work_order *WorkOrder, v3 PixelCenterOnFilm,
    u64 *BouncesComputedPerTile, u64 *LoopsComputedPerTile
)
{
    v3 PixelColor = {};

    world *World = WorkOrder->World;
    v3_lane CameraX = V3LaneFromV3(World->Camera.CoordinateSet.X);
    v3_lane CameraY = V3LaneFromV3(World->Camera.CoordinateSet.Y);
    f32_lane HalfPixelWidth = F32LaneFromF32(World->Film.HalfPixelWidth);
    f32_lane HalfPixelHeight = F32LaneFromF32(World->Film.HalfPixelHeight);
    random_series_lane *RandomSeries = &WorkOrder->Entropy;
    v3_lane CameraPosition = V3LaneFromV3(World->Camera.Position);
    f32_lane ToleranceToZero = F32LaneFromF32(WorkOrder->RenderingParameters->ToleranceToZero);
    f32_lane HitDistanceLowerLimit = F32LaneFromF32(WorkOrder->RenderingParameters->HitDistanceLowerLimit);

    for (u32 RayBatchIndex = 0; RayBatchIndex < RAY_BATCHES_PER_PIXEL; RayBatchIndex++)
    {
        v3_lane RayBatchColor = V3Lane(0, 0, 0);
        v3_lane RayBatchColorAttenuation = V3Lane(1, 1, 1);

        v3_lane RayBatchPositionOnFilm =
            V3LaneFromV3(PixelCenterOnFilm) +
            HalfPixelWidth * RandomBilateralLane(RandomSeries) * CameraX +
            HalfPixelHeight * RandomBilateralLane(RandomSeries) * CameraY;

        v3_lane BounceOrigin = CameraPosition;
        v3_lane BounceDirection = Normalize(RayBatchPositionOnFilm - BounceOrigin);

        u32_lane LaneMask = U32LaneFromU32(0xFFFFFFFF);

        for (u32 BounceIndex = 0; BounceIndex < BOUNCES_PER_RAY; BounceIndex++)
        {
            v3_lane BounceNormal = {};
            v3_lane BounceTangent = {};
            v3_lane BounceBiTangent = {};

            *BouncesComputedPerTile += HorizontalAdd(U32LaneFromU32(1) & LaneMask);
            *LoopsComputedPerTile += SIMD_NUMBEROF_LANES;

            f32_lane MinimumHitDistanceFound = F32LaneFromF32(FLT_MAX);
            u32_lane HitMaterialIndex = U32LaneFromU32(0);

            for (u32 PlaneIndex = 0; PlaneIndex < World->PlanesCount; PlaneIndex++)
            {
                plane *CurrentPlane = &World->Planes[PlaneIndex];
                v3_lane PlaneNormal = V3LaneFromV3(CurrentPlane->Normal);
                f32_lane PlaneDistance = F32LaneFromF32(CurrentPlane->Distance);

                f32_lane Denominator = InnerProduct(PlaneNormal, BounceDirection);
                u32_lane DenominatorMask = (Denominator > ToleranceToZero) | (Denominator < -ToleranceToZero);

                if (!MaskIsAllZeroes(DenominatorMask))
                {
                    f32_lane CurrentHitDistance =
                        (-PlaneDistance - InnerProduct(PlaneNormal, BounceOrigin)) /
                        Denominator;

                    u32_lane CurrentHitDistanceMask =
                        (CurrentHitDistance < MinimumHitDistanceFound) &
                        (CurrentHitDistance > HitDistanceLowerLimit);

                    u32_lane HitMask = CurrentHitDistanceMask & DenominatorMask;
                    if (!MaskIsAllZeroes(HitMask))
                    {
                        ConditionalAssign(&MinimumHitDistanceFound, CurrentHitDistance, HitMask);
                        ConditionalAssign(&HitMaterialIndex, U32LaneFromU32(CurrentPlane->MaterialIndex), HitMask);
                        ConditionalAssign(&BounceNormal, PlaneNormal, HitMask);
                        ConditionalAssign(&BounceTangent, V3LaneFromV3(CurrentPlane->Tangent), HitMask);
                        ConditionalAssign(&BounceBiTangent, V3LaneFromV3(CurrentPlane->BiTangent), HitMask);
                    }
                }
            }

            for (u32 SphereIndex = 0; SphereIndex < World->SpheresCount; SphereIndex++)
            {
                sphere *CurrentSphere = &World->Spheres[SphereIndex];
                v3_lane SpherePosition = V3LaneFromV3(CurrentSphere->Position);
                f32_lane SphereRadius = F32LaneFromF32(CurrentSphere->Radius);
                v3_lane SphereRelativeRayOrigin = BounceOrigin - SpherePosition;

                f32_lane A = InnerProduct(BounceDirection, BounceDirection);
                f32_lane B = 2.0f * InnerProduct(BounceDirection, SphereRelativeRayOrigin);
                f32_lane C = InnerProduct(SphereRelativeRayOrigin, SphereRelativeRayOrigin) - Square(SphereRadius);

                f32_lane RootTerm = SquareRoot((B * B) - (4 * A * C));
                u32_lane RootTermMask = RootTerm > ToleranceToZero;

                if (!MaskIsAllZeroes(RootTermMask))
                {
                    f32_lane QuadraticDenominator = 2 * A;
                    f32_lane PositiveSolution = (-B + RootTerm) / QuadraticDenominator;
                    f32_lane NegativeSolution = (-B - RootTerm) / QuadraticDenominator;

                    u32_lane NegativeSolutionMask = (NegativeSolution > HitDistanceLowerLimit) & (NegativeSolution < PositiveSolution);

                    f32_lane CurrentHitDistance = PositiveSolution;
                    ConditionalAssign(&CurrentHitDistance, NegativeSolution, NegativeSolutionMask);

                    u32_lane HitDistanceMask =
                        (CurrentHitDistance < MinimumHitDistanceFound) &
                        (CurrentHitDistance > HitDistanceLowerLimit);

                    if (!MaskIsAllZeroes(HitDistanceMask))
                    {
                        u32_lane HitMask = RootTermMask & HitDistanceMask;

                        ConditionalAssign(&MinimumHitDistanceFound, CurrentHitDistance, HitMask);
                        ConditionalAssign(&HitMaterialIndex, U32LaneFromU32(CurrentSphere->MaterialIndex), HitMask);
                        ConditionalAssign
                        (
                            &BounceNormal,
                            Normalize(BounceOrigin + (MinimumHitDistanceFound * BounceDirection) - SpherePosition),
                            HitMask
                        );

                        v3_lane SphereTangent = Normalize(CrossProduct(V3LaneFromV3(V3(0, 0, 1)), BounceNormal));
                        v3_lane SphereBiTangent = Normalize(CrossProduct(BounceNormal, SphereTangent));
                        ConditionalAssign(&BounceTangent, SphereTangent, HitMask);
                        ConditionalAssign(&BounceBiTangent, SphereBiTangent, HitMask);
                    }
                }
            }

            v3_lane HitMaterialEmmissionColor =
                LaneMask & GatherV3(World->Materials, EmmissionColor, HitMaterialIndex);

            RayBatchColor += HadamardProduct(RayBatchColorAttenuation, HitMaterialEmmissionColor);

            LaneMask = LaneMask & MaskFromBoolean(HitMaterialIndex != U32LaneFromU32(0));

            if (MaskIsAllZeroes(LaneMask))
            {
                break;
            }
            else
            {
                BounceOrigin += MinimumHitDistanceFound * BounceDirection;

                v3_lane PreviousBounceDirection = BounceDirection;

                v3_lane PureBounceDirection = Normalize
                (
                    BounceDirection - 2 * InnerProduct(BounceDirection, BounceNormal) * BounceNormal
                );

                v3_lane RandomBounceDirection = Normalize
                (
                    BounceNormal +
                    V3Lane
                    (
                        RandomBilateralLane(RandomSeries),
                        RandomBilateralLane(RandomSeries),
                        RandomBilateralLane(RandomSeries)
                    )
                );

                f32_lane HitMaterialSpecularity = StaticCastU32LaneToF32Lane
                (
                    LaneMask &
                    StaticCastF32LaneToU32Lane(GatherF32(World->Materials, Specularity, HitMaterialIndex))
                );

                BounceDirection = Normalize(Lerp(RandomBounceDirection, PureBounceDirection, HitMaterialSpecularity));

                v3_lane HitMaterialReflectionColor = GetMaterialReflectionColor
                (
                    World->Materials, HitMaterialIndex, LaneMask,
                    BounceTangent, BounceBiTangent, BounceNormal,
                    -PreviousBounceDirection, BounceDirection
                );
                RayBatchColorAttenuation = HadamardProduct(RayBatchColorAttenuation, HitMaterialReflectionColor);
            }
        }

        PixelColor += HorizontalAdd(RayBatchColor) / (f32)RAYS_PER_PIXEL;
    }

    return PixelColor;
}

inline b32
RenderTile(work_queue *WorkQueue)
{
    u64 WorkOrderIndex = InterlockedExchangeAdd64(&WorkQueue->NextWorkOrderIndex, 1);
    if (WorkOrderIndex >= WorkQueue->WorkOrderCount)
    {
        return FALSE;
    }

    work_order *WorkOrder = &WorkQueue->WorkOrders[WorkOrderIndex];

    image_u32 *Image = WorkOrder->Image;
    world *World = WorkOrder->World;
    film *Film = &WorkOrder->World->Film;
    rendering_parameters *RenderingParameters = WorkOrder->RenderingParameters;
    v3 CameraX = World->Camera.CoordinateSet.X;
    v3 CameraY = World->Camera.CoordinateSet.Y;

    u64 BouncesComputedPerTile = 0;
    u64 LoopsComputedPerTile = 0;

    for (u32 PixelY = WorkOrder->StartPixelY; PixelY < WorkOrder->EndPixelY; PixelY++)
    {
        f32 FilmPixelY = 2.0f * ((f32)PixelY / (f32)Image->HeightInPixels) - 1.0f;

        for (u32 PixelX = WorkOrder->StartPixelX; PixelX < WorkOrder->EndPixelX; PixelX++)
        {
            f32 FilmPixelX = 2.0f * ((f32)PixelX / (f32)Image->WidthInPixels) - 1.0f;

            u32 *PixelWritePointer = GetPixelPointer(Image, PixelX, PixelY);

            v3 PixelCenterOnFilm =
                Film->Center +
                (FilmPixelX * Film->HalfWidth + Film->HalfPixelWidth) * CameraX +
                (FilmPixelY * Film->HalfHeight + Film->HalfPixelHeight) * CameraY;

            v3 PixelColor = RenderPixel
            (
                WorkOrder, PixelCenterOnFilm, &BouncesComputedPerTile, &LoopsComputedPerTile
            );

            v4 BitmapColorRGBA =
            {
                255 * TranslateLinearTosRGB(PixelColor.Red),
                255 * TranslateLinearTosRGB(PixelColor.Green),
                255 * TranslateLinearTosRGB(PixelColor.Blue),
                255
            };

            *PixelWritePointer++ =
            (
                (RoundF32ToU32(BitmapColorRGBA.Alpha) << 24) |
                (RoundF32ToU32(BitmapColorRGBA.Red) << 16) |
                (RoundF32ToU32(BitmapColorRGBA.Green) << 8) |
                (RoundF32ToU32(BitmapColorRGBA.Blue) << 0)
            );
        }
    }

    InterlockedExchangeAdd64(&WorkQueue->TotalTilesDone, 1);
    InterlockedExchangeAdd64(&WorkQueue->TotalRayBouncesComputed, BouncesComputedPerTile);
    InterlockedExchangeAdd64(&WorkQueue->TotalLoopsComputed, LoopsComputedPerTile);
    return TRUE;
}

static DWORD WINAPI
WorkerThreadEntry(void *Parameter)
{
    work_queue *WorkQueue = (work_queue *)Parameter;
    while (RenderTile(WorkQueue)){}
    return 0;
}

i32
main(i32 argc, char **argv)
{
    printf("RayCasting...");

    image_u32 OutputImage = CreateImage(1280, 720);

    material MaterialsArray[7] = {};

    MaterialsArray[0].EmmissionColor = V3(0.3, 0.4, 0.5);

    MaterialsArray[1].ReflectionColor = V3(0.5, 0.5, 0.5);
    LoadReflectionDataForMaterial("data\\ray_tracer\\cayman.astm", &MaterialsArray[1]);

    MaterialsArray[2].ReflectionColor = V3(0.7, 0.5, 0.3);
    LoadReflectionDataForMaterial("data\\ray_tracer\\garnet_red.astm", &MaterialsArray[2]);

    MaterialsArray[3].EmmissionColor = V3(4, 0, 0);

    MaterialsArray[4].Specularity = 0.7;
    MaterialsArray[4].ReflectionColor = V3(0.2, 0.8, 0.2);

    MaterialsArray[5].Specularity = 0.85;
    MaterialsArray[5].ReflectionColor = V3(0.4, 0.8, 0.9);

    MaterialsArray[6].Specularity = 1;
    MaterialsArray[6].ReflectionColor = V3(0.95, 0.95, 0.95);

    plane PlanesArray[1] = {};

    PlanesArray[0].MaterialIndex = 1;
    PlanesArray[0].Normal = V3(0, 0, 1);
    PlanesArray[0].Tangent = V3(1, 0, 0);
    PlanesArray[0].BiTangent = V3(0, 1, 0);
    PlanesArray[0].Distance = 0;

    // PlanesArray[1].MaterialIndex = 1;
    // PlanesArray[1].Normal = V3(1, 0, 0);
    // PlanesArray[1].Distance = 2;

    sphere SpheresArray[5] = {};

    SpheresArray[0].MaterialIndex = 2;
    SpheresArray[0].Position = V3(0, 0, 0);
    SpheresArray[0].Radius = 1;

    SpheresArray[1].MaterialIndex = 3;
    SpheresArray[1].Position = V3(3, -2, 0);
    SpheresArray[1].Radius = 1;

    SpheresArray[2].MaterialIndex = 4;
    SpheresArray[2].Position = V3(-2, -1, 2);
    SpheresArray[2].Radius = 1;

    SpheresArray[3].MaterialIndex = 5;
    SpheresArray[3].Position = V3(1, -1, 3);
    SpheresArray[3].Radius = 1;

    SpheresArray[4].MaterialIndex = 6;
    SpheresArray[4].Position = V3(-2, 3, 0);
    SpheresArray[4].Radius = 2;

    world World = {};
    World.Materials = MaterialsArray;
    World.MaterialsCount = ArrayCount(MaterialsArray);

    World.Planes = PlanesArray;
    World.PlanesCount = ArrayCount(PlanesArray);

    World.Spheres = SpheresArray;
    World.SpheresCount = ArrayCount(SpheresArray);

    World.CoordinateSet.X = V3(1, 0, 0);
    World.CoordinateSet.Y = V3(0, 1, 0);
    World.CoordinateSet.Z = V3(0, 0, 1);

    World.Camera.Position = {0, -10, 1};
    World.Camera.CoordinateSet.Z = Normalize(World.Camera.Position);
    World.Camera.CoordinateSet.X = Normalize(CrossProduct(World.Camera.CoordinateSet.Z, World.CoordinateSet.Z));
    World.Camera.CoordinateSet.Y = Normalize(CrossProduct(World.Camera.CoordinateSet.Z, World.Camera.CoordinateSet.X));

    World.Film.DistanceFromCamera = 1.0f;
    World.Film.Center = World.Camera.Position + World.Film.DistanceFromCamera * (-World.Camera.CoordinateSet.Z);

    if (OutputImage.WidthInPixels >= OutputImage.HeightInPixels)
    {
        World.Film.HalfWidth = 0.5f;
        World.Film.HalfHeight =
            0.5f *
            (f32)OutputImage.HeightInPixels /
            (f32)OutputImage.WidthInPixels;
    }
    else
    {
        World.Film.HalfHeight = 1.0f;
        World.Film.HalfWidth =
            (f32)OutputImage.WidthInPixels /
            (f32)OutputImage.HeightInPixels;
    }

    World.Film.HalfPixelWidth = 0.5f / (f32)OutputImage.WidthInPixels;
    World.Film.HalfPixelHeight = 0.5f / (f32)OutputImage.HeightInPixels;

    rendering_parameters RenderingParameters = {};

    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    RenderingParameters.CoreCount = (u8)SystemInfo.dwNumberOfProcessors;

    RenderingParameters.HitDistanceLowerLimit = 0.001f;
    RenderingParameters.ToleranceToZero = 0.0001f;

    RenderingParameters.TileWidthInPixels = OutputImage.WidthInPixels / RenderingParameters.CoreCount;
    RenderingParameters.TileHeightInPixels = RenderingParameters.TileWidthInPixels;

    RenderingParameters.TileCountX =
        (OutputImage.WidthInPixels + RenderingParameters.TileWidthInPixels - 1) /
        RenderingParameters.TileWidthInPixels;

    RenderingParameters.TileCountY =
        (OutputImage.HeightInPixels + RenderingParameters.TileHeightInPixels - 1) /
        RenderingParameters.TileHeightInPixels;

    RenderingParameters.TotalTileCount =
        RenderingParameters.TileCountX * RenderingParameters.TileCountY;

    work_queue WorkQueue = {};
    WorkQueue.WorkOrders = (work_order *)
        malloc(RenderingParameters.TotalTileCount * sizeof(work_order));

    for (u32 TileY = 0; TileY < RenderingParameters.TileCountY; TileY++)
    {
        u32 StartPixelY = TileY * RenderingParameters.TileHeightInPixels;
        u32 EndPixelY = StartPixelY + RenderingParameters.TileHeightInPixels;
        EndPixelY = Clamp(EndPixelY, 0, OutputImage.HeightInPixels);

        for (u32 TileX = 0; TileX < RenderingParameters.TileCountX; TileX++)
        {
            u32 StartPixelX = TileX * RenderingParameters.TileWidthInPixels;
            u32 EndPixelX = StartPixelX + RenderingParameters.TileWidthInPixels;
            EndPixelX = Clamp(EndPixelX, 0, OutputImage.WidthInPixels);

            work_order *WorkOrder = WorkQueue.WorkOrders + WorkQueue.WorkOrderCount++;
            WorkOrder->World = &World;
            WorkOrder->Image = &OutputImage;
            WorkOrder->RenderingParameters = &RenderingParameters;
            WorkOrder->StartPixelX = StartPixelX;
            WorkOrder->StartPixelY = StartPixelY;
            WorkOrder->EndPixelX = EndPixelX;
            WorkOrder->EndPixelY = EndPixelY;

#if (SIMD_NUMBEROF_LANES == 1)
            WorkOrder->Entropy.State = U32LaneFromU32(TileX * 52350329 + TileY * 793083851 + 63274279);
#elif (SIMD_NUMBEROF_LANES == 4)
            WorkOrder->Entropy.State = U32LaneFromU32
            (
                TileX * 32542345 + TileY * 881712265 + 93073411,
                TileX * 98698641 + TileY * 640200962 + 24681141,
                TileX * 52350329 + TileY * 793083851 + 63274279,
                TileX * 39846279 + TileY * 505147656 + 12932640
            );
#elif (SIMD_NUMBEROF_LANES == 8)
            WorkOrder->Entropy.State = U32LaneFromU32
            (
                TileX * 32542345 + TileY * 881712265 + 93073411,
                TileX * 98698641 + TileY * 640200962 + 24681141,
                TileX * 52350329 + TileY * 793083851 + 63274279,
                TileX * 39846279 + TileY * 505147656 + 12932640,
                TileX * 23523623 + TileY * 907324654 + 29875642,
                TileX * 98732198 + TileY * 235267674 + 46541234,
                TileX * 22362367 + TileY * 876238957 + 49872463,
                TileX * 32968422 + TileY * 986851235 + 21335002
            );
#else
#endif // SIMD_NUMBEROF_LANES != 1
        }
    }

    MemoryBarrier();

    clock_t StartTime = clock();

    for (u32 CoreIndex = 1; CoreIndex < RenderingParameters.CoreCount; CoreIndex++)
    {
        DWORD ThreadId;
        HANDLE ThreadHandle = CreateThread(0, 0, WorkerThreadEntry, &WorkQueue, 0, &ThreadId);
        CloseHandle(ThreadHandle);
    }

    while (WorkQueue.TotalTilesDone < RenderingParameters.TotalTileCount)
    {
        if (RenderTile(&WorkQueue))
        {
            printf("\rRayCasting %d%%", 100 * (u32)WorkQueue.TotalTilesDone / RenderingParameters.TotalTileCount);
            fflush(stdout);
        }
    }

    clock_t TotalTimeElapsed = clock() - StartTime;

    WriteBitmapImage(OutputImage.Pixels, OutputImage.WidthInPixels, OutputImage.HeightInPixels, argv[1]);

    printf("\nRayCasting time: %ld ms\n", TotalTimeElapsed);
    printf("Core Count: %d\n", RenderingParameters.CoreCount);
    printf("SIMD width used: %d\n", SIMD_NUMBEROF_LANES);
    printf("Rays Per Pixel: %d\n", RAYS_PER_PIXEL);
    printf("Bounces Per Ray: %d\n", BOUNCES_PER_RAY);

    f32 KBytesPerTile = (f32)
    (
        RenderingParameters.TileWidthInPixels *
        RenderingParameters.TileHeightInPixels *
        sizeof(u32) /
        1024.0f
    );

    printf
    (
        "Using %d %dx%d tiles, %.2f KBytes/tile\n",
        RenderingParameters.TotalTileCount,
        RenderingParameters.TileWidthInPixels,
        RenderingParameters.TileHeightInPixels,
        KBytesPerTile
    );
    printf("Bounce loops performed: %lld\n", WorkQueue.TotalLoopsComputed);
    printf("Bounces Computed: %lld\n", WorkQueue.TotalRayBouncesComputed);
    u64 WastedBounces = WorkQueue.TotalLoopsComputed - WorkQueue.TotalRayBouncesComputed;
    printf("wasted SIMD capacity: %lld bounces == %.02f%%\n", WastedBounces, 100.0f * (f32)WastedBounces / (f32)WorkQueue.TotalLoopsComputed);
    printf
    (
        "performance metric: %f ns/bounce\n",
        (f64)TotalTimeElapsed / (f64)WorkQueue.TotalRayBouncesComputed * 1000000.0f
    );

    return 0;
}