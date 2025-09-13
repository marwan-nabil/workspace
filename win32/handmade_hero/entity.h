#pragma once

#include "win32\shared\base_types.h"
#include "win32\handmade_hero\memory.h"
#include "win32\shared\math\vector2.h"
#include "win32\shared\math\vector3.h"

#define ENTITY_INIFINTE_MOVEMENT_DISTANCE 10000.0f
#define INVALID_ENTITY_POSITION V3(100000.0f, 100000.0f, 100000.0f)
#define ENTITY_HIT_POINT_SUBCOUNT 4

enum entity_type
{
    ET_NULL,
    ET_SPACE,
    ET_WALL,
    ET_HERO,
    ET_FAMILIAR,
    ET_MONSTER,
    ET_SWORD,
    ET_STAIRS
};

enum entity_flag
{
    EF_COLLIDES = (1 << 0u),
    EF_NON_SPATIAL = (1 << 1u),
    EF_MOVEABLE = (1 << 2u),
    EF_Z_SUPPORTED = (1 << 3u),
    EF_TRAVERSABLE = (1 << 4u),

    EF_SIMULATING = (1 << 30u)
};

struct entity_movement_parameters
{
    b32 NormalizeAcceleration;
    f32 DragInXYPlane;
    f32 SpeedInXYPlane;
};

struct entity_world_position
{
    i32 ChunkX, ChunkY, ChunkZ;
    v3 OffsetFromChunkCenter;
};

struct entity_hit_point
{
    u8 Flags;
    u8 FilledAmount;
};

struct entity;

union entity_reference
{
    entity *Entity;
    u32 StorageIndex;
};

struct entity_collision_mesh_group;

struct entity
{
    entity_type Type;
    u32 Flags;
    b32 CanUpdate;

    v3 Position;
    v3 Velocity;

    f32 BobbingSinParameter;
    f32 MovementDistanceLimit;

    u32 StorageIndex;

    entity_collision_mesh_group *CollisionMeshGroup;

    i32 DiffAbsTileZ; // unused

    u32 HitPointsMax;
    entity_hit_point HitPoints[16];

    u32 BitmapFacingDirection;

    entity_reference SwordEntityReference;

    // NOTE: only for the stairwell
    f32 WalkableHeight;
    v2 WalkableDiameter;
};

struct storage_entity
{
    entity Entity;
    entity_world_position WorldPosition;
};

struct add_storage_entity_result
{
    u32 StorageIndex;
    storage_entity *StorageEntity;
};

struct world;
struct game_state;
struct simulation_region;

b32 IsEntityFlagSet(entity *Entity, u32 Flag);
b32 AreAnyEntityFlagsSet(entity *Entity, u32 Flags);
void SetEntityFlags(entity *Entity, u32 Flags);
void ClearEntityFlags(entity *Entity, u32 Flags);
void ClearAllEntityFlags(entity *Entity);
void StoreEntityReference(entity_reference *EntityReference);
void SortEntityPointersByEntityTypes(entity **A, entity **B);
b32 CanEntitiesOverlap(game_state *GameState, entity *MovingEntity, entity *TestEntity);
void InitializeEntityHitPoints(storage_entity *StorageEntity, u32 HitpointsCount);
entity_movement_parameters DefaultEntityMovementParameters();
void MakeEntityNonSpatial(entity *Entity);
void MakeEntitySpatial(entity *Entity, v3 InitialPosition, v3 InitialVelocity);
v3 GetEntityGroundPoint(entity *Entity);
f32 GetStairsEntityGroundLevel(entity *StairsEntity, v3 MeasurementPoint);
void HandleEntityOverlapWithStairs(game_state *GameState, entity *MovingEntity, entity *TestEntity, f32 TimeDelta, f32 *CurrentGroundLevel);
b32 SpeculativeCollision(entity *MovingEntity, entity *TestEntity);
void RawChangeStorageEntityLocationInWorld(world *World, memory_arena *MemoryArena, u32 StorageIndex, entity_world_position *OldPosition, entity_world_position *NewPosition);
void ChangeStorageEntityLocationInWorld
(
    world *World, memory_arena *MemoryArena, u32 StorageIndex,
    storage_entity *StorageEntity, entity_world_position *NewWorldPosition
);
storage_entity *GetStorageEntity(game_state *GameState, u32 StorageIndex);
add_storage_entity_result AddStorageEntity(game_state *GameState, entity_type Type, entity_world_position WorldPosition);
add_storage_entity_result AddGoundBasedStorageEntity
(
    game_state *GameState, entity_type Type, entity_world_position GroundPoint,
    entity_collision_mesh_group *EntityCollisionMeshGroup
);
add_storage_entity_result AddStandardRoom(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ);
add_storage_entity_result AddSword(game_state *GameState);
add_storage_entity_result AddPlayer(game_state *GameState);
add_storage_entity_result AddMonster(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ);
add_storage_entity_result AddFamiliar(game_state *GameState, i32 AbsTileX, i32 AbsTileY, i32 AbsTileZ);
add_storage_entity_result AddWall(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ);
add_storage_entity_result AddStairs(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ);
b32 TestWall
(
    f32 SquareCenterRelativeWallX,
    f32 SquareCenterToMovingEntityOriginalPositionX, f32 SquareCenterToMovingEntityOriginalPositionY,
    f32 MovingEntityMovementVectorX, f32 MovingEntityMovementVectorY,
    f32 *OriginalMinimalTParameter, f32 SquareCenterRelativeMinimumOrthogonalWallY, f32 SquareCenterRelativeMaximumOrthogonalWallY
);
void MoveEntity
(
    game_state *GameState, simulation_region *SimulationRegion, entity *MovingEntity,
    v3 Acceleration, f32 TimeDelta, entity_movement_parameters *MoveSpec
);