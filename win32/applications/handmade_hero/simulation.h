#pragma once

#include "portable\shared\base_types.h"
#include "win32\handmade_hero\entity.h"
#include "win32\handmade_hero\world.h"
#include "win32\handmade_hero\game.h"
#include "win32\shared\math\rectangle3.h"

struct entity_table_entry
{
    entity *Entity;
    u32 StorageIndex;
};

struct simulation_region
{
    world *World;

    entity_world_position Origin;
    rectangle3 SimulationBounds;
    rectangle3 UpdateBounds;

    f32 MaxEntityRadius;
    f32 MaxEntityVelocity;

    u32 MaxEntityCount;
    u32 CurrentEntityCount;
    entity *Entities;

    entity_table_entry EntityTable[4096];
};

void LoadEntityReference(game_state *GameState, simulation_region *SimulationRegion, entity_reference *EntityReference);
entity_table_entry *GetEntityHashtableEntry(simulation_region *SimulationRegion, u32 StorageIndex);
v3 GetSimulationRegionRelativePosition(simulation_region *SimulationRegion, storage_entity *StorageEntity);
b32 DoesEntityCollisionMeshOverlapRectangle(v3 EntityPosition, entity_collision_mesh CollisionMesh, rectangle3 TestingRectangle);
entity *RawAddEntityToSimulation(game_state *GameState, simulation_region *SimulationRegion, u32 StorageIndex, storage_entity *SourceStorageEntity);
entity *AddEntityToSimulation
(
    game_state *GameState, simulation_region *SimulationRegion,
    u32 StorageIndex, storage_entity *SourceStorageEntity,
    v3 *InitialPositionInSimulationRegion
);
void LoadEntityReference(game_state *GameState, simulation_region *SimulationRegion, entity_reference *EntityReference);
simulation_region *BeginSimulation
(
    game_state *GameState, world *World, memory_arena *SimulationArena,
    entity_world_position RegionOrigin, rectangle3 UpdateBounds, f32 TimeDelta
);
void EndSimulation(simulation_region *SimulationRegion, game_state *GameState);