#pragma once

#include "win32\shared\base_types.h"
#include "win32\shared\math\vector2.h"
#include "win32\shared\math\vector3.h"
#include "win32\handmade_hero\memory.h"
#include "win32\handmade_hero\bitmap.h"
#include "win32\handmade_hero\world.h"
#include "win32\handmade_hero\entity.h"
#include "win32\handmade_hero\collision.h"

struct controlled_hero_input
{
    u32 HeroEntityStorageIndex;
    v3 InputAcceleration;
    v2 InputSwordDirection;
    f32 InputJumpVelocity;
};

struct game_state
{
    memory_arena WorldArena;
    world *World;

    loaded_bitmap BackDropBitMap;
    loaded_bitmap ShadowBitMap;
    loaded_bitmap TreeBitMap;
    loaded_bitmap SwordBitMap;
    loaded_bitmap StairWellBitMap;
    hero_bitmap_group HeroBitmapGroups[4];

    f32 PixelsToMetersRatio;

    entity_world_position CameraPosition;
    u32 StorageIndexOfEntityThatCameraFollows;

    controlled_hero_input ControllerToHeroInputMap[ArrayCount(((game_input *)0)->ControllerStates)];

    entity_collision_rule *CollisionRulesTable[256];
    entity_collision_rule *FreeCollisionRulesListHead;

    entity_collision_mesh_group *NullCollisionMeshGroupTemplate;
    entity_collision_mesh_group *SwordCollisionMeshGroupTemplate;
    entity_collision_mesh_group *StairsCollisionMeshGroupTemplate;
    entity_collision_mesh_group *PlayerCollisionMeshGroupTemplate;
    entity_collision_mesh_group *MonsterCollisionMeshGroupTemplate;
    entity_collision_mesh_group *StandardRoomCollisionMeshGroupTemplate;
    entity_collision_mesh_group *WallCollisionMeshGroupTemplate;
    entity_collision_mesh_group *FamiliarCollisionMeshGroupTemplate;
};