#include <stdint.h>
#include <math.h>
#include <intrin.h>
#include <string.h>

#include "portable\shared\base_types.h"
#include "portable\shared\basic_defines.h"
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

entity_table_entry *GetEntityHashtableEntry(simulation_region *SimulationRegion, u32 StorageIndex)
{
    Assert(StorageIndex);

    entity_table_entry *Result = 0;
    u32 InitialHashTableIndex = StorageIndex;
    for
    (
        u32 ProbingOffset = 0;
        ProbingOffset < ArrayCount(SimulationRegion->EntityTable);
        ProbingOffset++
    )
    {
        u32 HashTableIndex = (InitialHashTableIndex + ProbingOffset) & (ArrayCount(SimulationRegion->EntityTable) - 1);
        entity_table_entry *HashtableEntry = SimulationRegion->EntityTable + HashTableIndex;

        if ((HashtableEntry->StorageIndex == 0) || (HashtableEntry->StorageIndex == StorageIndex))
        {
            Result = HashtableEntry;
            break;
        }
    }

    return Result;
}

v3 GetSimulationRegionRelativePosition(simulation_region *SimulationRegion, storage_entity *StorageEntity)
{
    v3 Result;
    if (!IsEntityFlagSet(&StorageEntity->Entity, EF_NON_SPATIAL))
    {
        Result = SubtractPositions(SimulationRegion->World, &StorageEntity->WorldPosition, &SimulationRegion->Origin);
    }
    else
    {
        Result = INVALID_ENTITY_POSITION;
    }
    return Result;
}

b32 DoesEntityCollisionMeshOverlapRectangle(v3 EntityPosition, entity_collision_mesh CollisionMesh, rectangle3 TestingRectangle)
{
    rectangle3 ExpandedTestingRectangle = ExpandRectangle(TestingRectangle, CollisionMesh.Diameter);
    b32 Result = IsInRectangle(ExpandedTestingRectangle, EntityPosition + CollisionMesh.Offset);
    return Result;
}

entity *RawAddEntityToSimulation(game_state *GameState, simulation_region *SimulationRegion, u32 StorageIndex, storage_entity *SourceStorageEntity)
{
    Assert(StorageIndex);

    entity *Result = 0;
    entity_table_entry *HashtableEntry = GetEntityHashtableEntry(SimulationRegion, StorageIndex);

    if (HashtableEntry->Entity == 0)
    {
        if (SimulationRegion->CurrentEntityCount < SimulationRegion->MaxEntityCount)
        {
            Result = SimulationRegion->Entities + SimulationRegion->CurrentEntityCount++;

            HashtableEntry->StorageIndex = StorageIndex;
            HashtableEntry->Entity = Result;

            if (SourceStorageEntity)
            {
                *Result = SourceStorageEntity->Entity;
                LoadEntityReference(GameState, SimulationRegion, &Result->SwordEntityReference);

                Assert(!IsEntityFlagSet(&SourceStorageEntity->Entity, EF_SIMULATING));
                SetEntityFlags(&SourceStorageEntity->Entity, EF_SIMULATING);
            }

            Result->StorageIndex = StorageIndex;
            Result->CanUpdate = FALSE;
        }
        else
        {
            InvalidCodepath;
        }
    }

    return Result;
}

entity *AddEntityToSimulation
(
    game_state *GameState, simulation_region *SimulationRegion,
    u32 StorageIndex, storage_entity *SourceStorageEntity,
    v3 *InitialPositionInSimulationRegion
)
{
    entity *Result = RawAddEntityToSimulation(GameState, SimulationRegion, StorageIndex, SourceStorageEntity);
    if (Result)
    {
        if (InitialPositionInSimulationRegion)
        {
            Result->Position = *InitialPositionInSimulationRegion;
            Result->CanUpdate = DoesEntityCollisionMeshOverlapRectangle
            (
                Result->Position,
                Result->CollisionMeshGroup->TotalCollisionMesh,
                SimulationRegion->UpdateBounds
            );
        }
        else
        {
            Result->Position =
                GetSimulationRegionRelativePosition(SimulationRegion, SourceStorageEntity);
        }
    }
    return Result;
}

void LoadEntityReference(game_state *GameState, simulation_region *SimulationRegion, entity_reference *EntityReference)
{
    if (EntityReference->StorageIndex)
    {
        entity_table_entry *EntityHashTableEntry =
            GetEntityHashtableEntry(SimulationRegion, EntityReference->StorageIndex);

        if (EntityHashTableEntry->Entity == 0)
        {
            storage_entity *ReferencedStorageEntity =
                GetStorageEntity(GameState, EntityReference->StorageIndex);

            v3 SimulationRegionRelativePosition =
                GetSimulationRegionRelativePosition(SimulationRegion, ReferencedStorageEntity);

            EntityHashTableEntry->StorageIndex = EntityReference->StorageIndex;
            EntityHashTableEntry->Entity = AddEntityToSimulation
            (
                GameState, SimulationRegion, EntityReference->StorageIndex,
                ReferencedStorageEntity, &SimulationRegionRelativePosition
            );
        }

        EntityReference->Entity = EntityHashTableEntry->Entity;
    }
}

simulation_region *BeginSimulation
(
    game_state *GameState, world *World, memory_arena *SimulationArena,
    entity_world_position RegionOrigin, rectangle3 UpdateBounds, f32 TimeDelta
)
{
    simulation_region *SimulationRegion = PushStruct(SimulationArena, simulation_region);
    ZeroStruct(SimulationRegion->EntityTable);

    SimulationRegion->World = World;
    SimulationRegion->Origin = RegionOrigin;

    SimulationRegion->MaxEntityRadius = 5.0f;
    SimulationRegion->MaxEntityVelocity = 30.0f;
    f32 SimulationBoundsSafetyMargin = SimulationRegion->MaxEntityRadius + SimulationRegion->MaxEntityVelocity * TimeDelta;

    SimulationRegion->UpdateBounds =
        ExpandRectangle(UpdateBounds,
                        2 * V3(SimulationRegion->MaxEntityRadius, SimulationRegion->MaxEntityRadius, SimulationRegion->MaxEntityRadius));

    SimulationRegion->SimulationBounds =
        ExpandRectangle(SimulationRegion->UpdateBounds,  // NOTE: or just the given UpdateBounds directly ?
                        2 * V3(SimulationBoundsSafetyMargin, SimulationBoundsSafetyMargin, SimulationBoundsSafetyMargin));

    SimulationRegion->MaxEntityCount = 4096;
    SimulationRegion->CurrentEntityCount = 0;
    SimulationRegion->Entities = PushArray(SimulationArena, SimulationRegion->MaxEntityCount, entity);

    entity_world_position SimulationAreaMinChunkPosition = MapIntoWorldPosition(World, SimulationRegion->Origin, GetMinCorner(SimulationRegion->SimulationBounds));
    entity_world_position SimulationAreaMaxChunkPosition = MapIntoWorldPosition(World, SimulationRegion->Origin, GetMaxCorner(SimulationRegion->SimulationBounds));

    for (i32 ChunkZ = SimulationAreaMinChunkPosition.ChunkZ; ChunkZ <= SimulationAreaMaxChunkPosition.ChunkZ; ChunkZ++)
    {
        for (i32 ChunkY = SimulationAreaMinChunkPosition.ChunkY; ChunkY <= SimulationAreaMaxChunkPosition.ChunkY; ChunkY++)
        {
            for (i32 ChunkX = SimulationAreaMinChunkPosition.ChunkX; ChunkX <= SimulationAreaMaxChunkPosition.ChunkX; ChunkX++)
            {
                chunk *CurrentChunk = GetChunk(World, 0, ChunkX, ChunkY, ChunkZ);
                if (CurrentChunk)
                {
                    for
                    (
                        storage_entity_indices_block *Block = &CurrentChunk->FirstStorageEntitiesIndicesBlock;
                        Block;
                        Block = Block->NextBlock
                    )
                    {
                        for (u32 StorageEntityIndexIndex = 0; StorageEntityIndexIndex < Block->StorageEntityIndicesCount; StorageEntityIndexIndex++)
                        {
                            u32 StorageIndex = Block->StorageEntityIndices[StorageEntityIndexIndex];
                            storage_entity *StorageEntity = GetStorageEntity(GameState, StorageIndex);

                            if (!IsEntityFlagSet(&StorageEntity->Entity, EF_NON_SPATIAL))
                            {
                                v3 Position = GetSimulationRegionRelativePosition(SimulationRegion, StorageEntity);
                                if
                                (
                                    DoesEntityCollisionMeshOverlapRectangle
                                    (
                                        Position,
                                        StorageEntity->Entity.CollisionMeshGroup->TotalCollisionMesh,
                                        SimulationRegion->SimulationBounds
                                    )
                                )
                                {
                                    AddEntityToSimulation(GameState, SimulationRegion, StorageIndex, StorageEntity, &Position);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return SimulationRegion;
}

void EndSimulation(simulation_region *SimulationRegion, game_state *GameState)
{
    entity *Entity = SimulationRegion->Entities;
    for
    (
        u32 EntityIndex = 0;
        EntityIndex < SimulationRegion->CurrentEntityCount;
        EntityIndex++, Entity++
    )
    {
        storage_entity *StorageEntity = GetStorageEntity(GameState, Entity->StorageIndex);

        Assert(IsEntityFlagSet(&StorageEntity->Entity, EF_SIMULATING));
        StorageEntity->Entity = *Entity;
        Assert(!IsEntityFlagSet(&StorageEntity->Entity, EF_SIMULATING));

        StoreEntityReference(&StorageEntity->Entity.SwordEntityReference);

        entity_world_position NewWorldPosition;
        if (IsEntityFlagSet(Entity, EF_NON_SPATIAL))
        {
            NewWorldPosition = InvalidWorldPosition();
        }
        else
        {
            NewWorldPosition =
                MapIntoWorldPosition(GameState->World, SimulationRegion->Origin, Entity->Position);
        }

        ChangeStorageEntityLocationInWorld(GameState->World, &GameState->WorldArena, Entity->StorageIndex,
                                           StorageEntity, &NewWorldPosition);

        if (Entity->StorageIndex == GameState->StorageIndexOfEntityThatCameraFollows)
        {
            entity_world_position NewCameraPosition;
#if 0

            if (MovingEntity->Position.X > (0.5f * TilesPerScreenWidth * TileSideInMeters))
            {
                // new camera position = old camera position +
                NewCameraPosition.OffsetFromChunkCenter += V2((f32)TileSideInMeters * TilesPerScreenWidth, 0.0f);
            }
            if (StorageEntityThatCameraFollows.High->PositionRelativeToCamera.X < -(0.5f * TilesPerScreenWidth * TileSideInMeters))
            {
                NewCameraPosition.OffsetFromChunkCenter -= V2((f32)TileSideInMeters * TilesPerScreenWidth, 0.0f);
            }
            if (StorageEntityThatCameraFollows.High->PositionRelativeToCamera.SquareCenterRelativeCollisionPointY > (0.5f * TilesPerScreenHeight * TileSideInMeters))
            {
                NewCameraPosition.OffsetFromChunkCenter += V2(0.0f, (f32)TileSideInMeters * TilesPerScreenHeight);
            }
            if (StorageEntityThatCameraFollows.High->PositionRelativeToCamera.SquareCenterRelativeCollisionPointY < -(0.5f * TilesPerScreenHeight * TileSideInMeters))
            {
                NewCameraPosition.OffsetFromChunkCenter -= V2(0.0f, (f32)TileSideInMeters * TilesPerScreenHeight);
            }
#else
            NewCameraPosition = StorageEntity->WorldPosition;
            NewCameraPosition.OffsetFromChunkCenter.Z = GameState->CameraPosition.OffsetFromChunkCenter.Z;
#endif
            GameState->CameraPosition = NewCameraPosition;
        }
    }
}