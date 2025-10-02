#pragma once

#include "portable\shared\base_types.h"
#include "win32\shared\math\vector2.h"
#include "win32\shared\math\vector3.h"
#include "win32\handmade_hero\entity.h"

struct entity_collision_mesh
{
    v3 Diameter;
    v3 Offset;
};

struct entity_collision_mesh_group
{
    u32 MeshCount;
    entity_collision_mesh *Meshes;
    entity_collision_mesh TotalCollisionMesh;
};

struct entity_collision_rule
{
    u32 EntityAStorageIndex;
    u32 EntityBStorageIndex;
    b32 CanCollide;
    entity_collision_rule *Next;
};

b32 CanEntitiesCollide(game_state *GameState, entity *A, entity *B);
b32 ProcessEntityCollision(game_state *GameState, entity *MovingEntity, entity *StaticEntity);
entity_collision_mesh_group *MakeNullCollisionMeshTemplate(game_state *GameState);
entity_collision_mesh_group *MakeSimpleCollisionMeshTemplate(game_state *GameState, v3 CollisionMeshDiameter);
void AddEntityCollisionRule(game_state *GameState, u32 FirstEntityStorageIndex, u32 SecondEntityStorageIndex, b32 CanCollide);
b32 RemoveEntityCollisionRule(game_state *GameState, u32 FirstEntityStorageIndex, u32 SecondEntityStorageIndex);
void ClearAllEntityCollisionRules(game_state *GameState, u32 StorageIndex);
b32 CanEntitiesCollide(game_state *GameState, entity *A, entity *B);
b32 ProcessEntityCollision(game_state *GameState, entity *MovingEntity, entity *StaticEntity);