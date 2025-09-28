#pragma once

#include "win32\shared\base_types.h"
#include "win32\handmade_hero\entity.h"
#include "win32\shared\math\vector3.h"

#define TILES_PER_CHUNK 16
#define MAX_CHUNK_DISTANCE_FROM_CENTER (INT32_MAX / 64)
#define CHUNK_POSITION_UNINITIALIZED_VALUE INT32_MAX

struct storage_entity_indices_block
{
    u32 StorageEntityIndicesCount;
    u32 StorageEntityIndices[16];
    storage_entity_indices_block *NextBlock;
};

struct chunk
{
    i32 ChunkX;
    i32 ChunkY;
    i32 ChunkZ;
    storage_entity_indices_block FirstStorageEntitiesIndicesBlock;
    chunk *NextChunk;
};

struct world
{
    f32 TileSideInMeters;
    f32 TileDepthInMeters;
    v3 ChunkDiameterInMeters;

    chunk ChunksTable[4096];

    storage_entity_indices_block *StorageEntitiesIndicesBlocksFreeListHead;

    u32 StorageEntityCount;
    storage_entity StorageEntities[100000];
};

b32 IsOffsetWithinInterval(f32 IntervalLength, f32 OffsetFromIntervalCenter);
b32 IsChunkCenterOffsetCanonical(world *World, v3 OffsetFromChunkCenter);
entity_world_position InvalidWorldPosition();
b32 IsWorldPositionValid(entity_world_position WorldPosition);
b32 AreInTheSameChunk(world *World, entity_world_position *A, entity_world_position *B);
chunk *GetChunk(world *World, memory_arena *MemoryArena, i32 ChunkX, i32 ChunkY, i32 ChunkZ);
void CanonicalizeIntervalIndexAndOffset(f32 IntervalLength, i32 *IntervalIndex, f32 *OffsetFromIntervalCenter);
entity_world_position MapIntoWorldPosition(world *World, entity_world_position BasePosition, v3 OffsetFromBase);
v3 SubtractPositions(world *World, entity_world_position *A, entity_world_position *B);
void InitializeWorld(world *World, f32 TileSideInMeters, f32 TileDepthInMeters);
entity_world_position GetWorldPositionFromTilePosition(world *World, i32 AbsTileX, i32 AbsTileY, i32 AbsTileZ, v3 OffsetFromTileCenter);