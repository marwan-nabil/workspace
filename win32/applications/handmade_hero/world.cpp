#include <stdint.h>
#include <math.h>
#include <intrin.h>
#include <string.h>

#include "win32\shared\base_types.h"
#include "win32\shared\basic_defines.h"
#include "win32\shared\math\constants.h"
#include "win32\shared\math\integers.h"
#include "win32\shared\math\bit_operations.h"
#include "win32\shared\math\floats.h"
#include "win32\shared\math\scalar_conversions.h"
#include "win32\shared\math\transcendentals.h"
#include "win32\shared\math\vector2.h"
#include "win32\shared\math\vector3.h"
#include "win32\shared\math\vector4.h"
#include "win32\shared\math\rectangle2.h"
#include "win32\shared\math\rectangle3.h"

#include "game_interface.h"
#include "memory.h"
#include "bitmap.h"
#include "renderer.h"
#include "random_numbers_table.h"

#include "entity.h"
#include "collision.h"
#include "world.h"
#include "simulation.h"
#include "game.h"

b32 IsOffsetWithinInterval(f32 IntervalLength, f32 OffsetFromIntervalCenter)
{
    f32 ToleranceEpsilon = 0.01f;
    f32 HalfIntervalLength = 0.5f * IntervalLength + ToleranceEpsilon;
    b32 Result =
    (
        (OffsetFromIntervalCenter >= -HalfIntervalLength) &&
        (OffsetFromIntervalCenter <= HalfIntervalLength)
    );
    return Result;
}

b32 IsChunkCenterOffsetCanonical(world *World, v3 OffsetFromChunkCenter)
{
    b32 Result =
    (
        IsOffsetWithinInterval(World->ChunkDiameterInMeters.X, OffsetFromChunkCenter.X) &&
        IsOffsetWithinInterval(World->ChunkDiameterInMeters.Y, OffsetFromChunkCenter.Y) &&
        IsOffsetWithinInterval(World->ChunkDiameterInMeters.Z, OffsetFromChunkCenter.Z)
    );
    return Result;
}

entity_world_position InvalidWorldPosition()
{
    entity_world_position Result = {};
    Result.ChunkX = CHUNK_POSITION_UNINITIALIZED_VALUE;
    return Result;
}

b32 IsWorldPositionValid(entity_world_position WorldPosition)
{
    b32 Result = (WorldPosition.ChunkX != CHUNK_POSITION_UNINITIALIZED_VALUE);
    return Result;
}

b32 AreInTheSameChunk(world *World, entity_world_position *A, entity_world_position *B)
{
    Assert(IsChunkCenterOffsetCanonical(World, A->OffsetFromChunkCenter));
    Assert(IsChunkCenterOffsetCanonical(World, B->OffsetFromChunkCenter));

    b32 Result =
    (
        (A->ChunkX == B->ChunkX) &&
        (A->ChunkY == B->ChunkY) &&
        (A->ChunkZ == B->ChunkZ)
    );

    return Result;
}

chunk *GetChunk(world *World, memory_arena *MemoryArena, i32 ChunkX, i32 ChunkY, i32 ChunkZ)
{
    Assert(ChunkX > -MAX_CHUNK_DISTANCE_FROM_CENTER);
    Assert(ChunkY > -MAX_CHUNK_DISTANCE_FROM_CENTER);
    Assert(ChunkZ > -MAX_CHUNK_DISTANCE_FROM_CENTER);

    Assert(ChunkX < MAX_CHUNK_DISTANCE_FROM_CENTER);
    Assert(ChunkY < MAX_CHUNK_DISTANCE_FROM_CENTER);
    Assert(ChunkZ < MAX_CHUNK_DISTANCE_FROM_CENTER);

    chunk *Result = 0;

    u32 HashTableIndex = (u32)(19 * ChunkX + 7 * ChunkY + 3 * ChunkZ) & (u32)(ArrayCount(World->ChunksTable) - 1);
    Assert(HashTableIndex < ArrayCount(World->ChunksTable));

    chunk *CurrentChunk = World->ChunksTable + HashTableIndex;
    while (CurrentChunk)
    {
        if
        (
            (ChunkX == CurrentChunk->ChunkX) &&
            (ChunkY == CurrentChunk->ChunkY) &&
            (ChunkZ == CurrentChunk->ChunkZ)
        )
        {
            Result = CurrentChunk;
            break;
        }
        else
        {
            if (CurrentChunk->ChunkX == CHUNK_POSITION_UNINITIALIZED_VALUE)
            {
                CurrentChunk->ChunkX = ChunkX;
                CurrentChunk->ChunkY = ChunkY;
                CurrentChunk->ChunkZ = ChunkZ;
                CurrentChunk->NextChunk = 0;
                Result = CurrentChunk;
                break;
            }
            else
            {
                if (CurrentChunk->NextChunk)
                {
                    CurrentChunk = CurrentChunk->NextChunk;
                }
                else
                {
                    if (MemoryArena)
                    {
                        chunk *NewChunk = PushStruct(MemoryArena, chunk);

                        NewChunk->ChunkX = ChunkX;
                        NewChunk->ChunkY = ChunkY;
                        NewChunk->ChunkZ = ChunkZ;
                        NewChunk->NextChunk = 0;

                        CurrentChunk->NextChunk = NewChunk;
                        Result = NewChunk;
                        break;
                    }
                    else
                    {
                        Result = 0;
                        break;
                    }
                }
            }
        }
    }

    return Result;
}

void CanonicalizeIntervalIndexAndOffset(f32 IntervalLength, i32 *IntervalIndex, f32 *OffsetFromIntervalCenter)
{
    i32 IntervalIndexOffset = RoundF32ToI32(*OffsetFromIntervalCenter / IntervalLength);

    *IntervalIndex += IntervalIndexOffset;
    *OffsetFromIntervalCenter -= IntervalIndexOffset * IntervalLength;

    Assert(IsOffsetWithinInterval(IntervalLength, *OffsetFromIntervalCenter));
}

entity_world_position MapIntoWorldPosition(world *World, entity_world_position BasePosition, v3 OffsetFromBase)
{
    entity_world_position Result = BasePosition;
    Result.OffsetFromChunkCenter += OffsetFromBase;
    CanonicalizeIntervalIndexAndOffset(World->ChunkDiameterInMeters.X, &Result.ChunkX, &Result.OffsetFromChunkCenter.X);
    CanonicalizeIntervalIndexAndOffset(World->ChunkDiameterInMeters.Y, &Result.ChunkY, &Result.OffsetFromChunkCenter.Y);
    CanonicalizeIntervalIndexAndOffset(World->ChunkDiameterInMeters.Z, &Result.ChunkZ, &Result.OffsetFromChunkCenter.Z);
    return Result;
}

v3 SubtractPositions(world *World, entity_world_position *A, entity_world_position *B)
{
    v3 Result = V3
    (
        (f32)A->ChunkX - (f32)B->ChunkX,
        (f32)A->ChunkY - (f32)B->ChunkY,
        (f32)A->ChunkZ - (f32)B->ChunkZ
    );

    Result = HadamardProduct(World->ChunkDiameterInMeters, Result) + (A->OffsetFromChunkCenter - B->OffsetFromChunkCenter);

    return Result;
}

void InitializeWorld(world *World, f32 TileSideInMeters, f32 TileDepthInMeters)
{
    World->TileSideInMeters = TileSideInMeters;
    World->TileDepthInMeters = TileDepthInMeters;
    World->ChunkDiameterInMeters.X = (f32)TILES_PER_CHUNK * TileSideInMeters;
    World->ChunkDiameterInMeters.Y = (f32)TILES_PER_CHUNK * TileSideInMeters;
    World->ChunkDiameterInMeters.Z = TileDepthInMeters;
    World->StorageEntitiesIndicesBlocksFreeListHead = 0;

    for (u32 Index = 0; Index < ArrayCount(World->ChunksTable); Index++)
    {
        World->ChunksTable[Index].ChunkX = CHUNK_POSITION_UNINITIALIZED_VALUE;
        World->ChunksTable[Index].FirstStorageEntitiesIndicesBlock.StorageEntityIndicesCount = 0;
        World->ChunksTable[Index].FirstStorageEntitiesIndicesBlock.NextBlock = 0;
    }
}

entity_world_position GetWorldPositionFromTilePosition(world *World, i32 AbsTileX, i32 AbsTileY, i32 AbsTileZ, v3 OffsetFromTileCenter)
{
    entity_world_position BasePosition = {};

    v3 TileDiameter = V3(World->TileSideInMeters, World->TileSideInMeters, World->TileDepthInMeters);
    v3 AbsoluteWorldOffset =
        HadamardProduct(TileDiameter, V3((f32)AbsTileX, (f32)AbsTileY, (f32)AbsTileZ)) +
        OffsetFromTileCenter;
    //v3 AbsoluteWorldOffset = HadamardProduct(TileDiameter, V3((f32)AbsTileX, (f32)AbsTileY, (f32)AbsTileZ)) + OffsetFromTileCenter + 0.5f * TileDiameter;

    entity_world_position Result = MapIntoWorldPosition(World, BasePosition, AbsoluteWorldOffset);

    Assert(IsChunkCenterOffsetCanonical(World, Result.OffsetFromChunkCenter));

    return Result;
}