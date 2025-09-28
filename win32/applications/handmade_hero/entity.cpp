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

b32 IsEntityFlagSet(entity *Entity, u32 Flag)
{
    b32 Result = ((Entity->Flags & Flag) != 0);
    return Result;
}

b32 AreAnyEntityFlagsSet(entity *Entity, u32 Flags)
{
    b32 Result = ((Entity->Flags & Flags) != 0);
    return Result;
}

void SetEntityFlags(entity *Entity, u32 Flags)
{
    Entity->Flags |= Flags;
}

void ClearEntityFlags(entity *Entity, u32 Flags)
{
    Entity->Flags &= ~Flags;
}

void ClearAllEntityFlags(entity *Entity)
{
    Entity->Flags = 0;
}

void StoreEntityReference(entity_reference *EntityReference)
{
    if (EntityReference->Entity)
    {
        EntityReference->StorageIndex = EntityReference->Entity->StorageIndex;
    }
}

void SortEntityPointersByEntityTypes(entity **A, entity **B)
{
    if ((*A)->Type > (*B)->Type)
    {
        entity *SwappingTemporary = *A;
        *A = *B;
        *B = SwappingTemporary;
    }
}

b32 CanEntitiesOverlap(game_state *GameState, entity *MovingEntity, entity *TestEntity)
{
    b32 Result = FALSE;
    if
    (
        (MovingEntity != TestEntity) &&
        (TestEntity->Type == ET_STAIRS)
    )
    {
        Result = TRUE;
    }
    return Result;
}

void InitializeEntityHitPoints(storage_entity *StorageEntity, u32 HitpointsCount)
{
    Assert(HitpointsCount < ArrayCount(StorageEntity->Entity.HitPoints));

    StorageEntity->Entity.HitPointsMax = HitpointsCount;

    for (u32 HitPointIndex = 0; HitPointIndex < HitpointsCount; HitPointIndex++)
    {
        StorageEntity->Entity.HitPoints[HitPointIndex].FilledAmount = ENTITY_HIT_POINT_SUBCOUNT;
        StorageEntity->Entity.HitPoints[HitPointIndex].Flags = 0;
    }
}

entity_movement_parameters DefaultEntityMovementParameters()
{
    entity_movement_parameters Result;
    Result.NormalizeAcceleration = FALSE;
    Result.SpeedInXYPlane = 1.0f;
    Result.DragInXYPlane = 0;
    return Result;
}

void MakeEntityNonSpatial(entity *Entity)
{
    SetEntityFlags(Entity, EF_NON_SPATIAL);
    Entity->Position = INVALID_ENTITY_POSITION;
}

void MakeEntitySpatial(entity *Entity, v3 InitialPosition, v3 InitialVelocity)
{
    ClearEntityFlags(Entity, EF_NON_SPATIAL);
    Entity->Position = InitialPosition;
    Entity->Velocity = InitialVelocity;
}

v3 GetEntityGroundPoint(entity *Entity)
{
    v3 GroundPoint = Entity->Position;
    return GroundPoint;
}

f32 GetStairsEntityGroundLevel(entity *StairsEntity, v3 MeasurementPoint)
{
    Assert(StairsEntity->Type == ET_STAIRS);
    rectangle2 StairsWalkableRegion = RectCenterDiameter(StairsEntity->Position.XY, StairsEntity->WalkableDiameter);
    v2 BarycentricMeasurementPosition = Clamp01(GetbarycentricPoint(StairsWalkableRegion, MeasurementPoint.XY));
    f32 GroundLevel = StairsEntity->Position.Z + BarycentricMeasurementPosition.Y * StairsEntity->WalkableHeight;
    return GroundLevel;
}

void HandleEntityOverlapWithStairs(game_state *GameState, entity *MovingEntity, entity *TestEntity, f32 TimeDelta, f32 *CurrentGroundLevel)
{
    if (TestEntity->Type == ET_STAIRS)
    {
        *CurrentGroundLevel = GetStairsEntityGroundLevel(TestEntity, GetEntityGroundPoint(MovingEntity));
    }
}

b32 SpeculativeCollision(entity *MovingEntity, entity *TestEntity)
{
    b32 WillEntitiesCollide = TRUE;
    if (TestEntity->Type == ET_STAIRS)
    {
        f32 StairStepHeight = 0.1f;

#if 0
        WillEntitiesCollide =
        (
            (AbsoluteValue(GetEntityGroundPoint(MovingEntity).Z - GroundLevel) > StairStepHeight) ||
            ((BarycentricMovingEntityPosition.Y > 0.1f) && (BarycentricMovingEntityPosition.Y < 0.9f))
        );
#endif

        v3 MovingEntityLowermostPoint = GetEntityGroundPoint(MovingEntity);
        f32 GroundLevel = GetStairsEntityGroundLevel(TestEntity, MovingEntityLowermostPoint);
        WillEntitiesCollide = AbsoluteValue(MovingEntityLowermostPoint.Z - GroundLevel) > StairStepHeight;
    }
    return WillEntitiesCollide;
}

void RawChangeStorageEntityLocationInWorld(world *World, memory_arena *MemoryArena, u32 StorageIndex, entity_world_position *OldPosition, entity_world_position *NewPosition)
{
    Assert(!OldPosition || IsWorldPositionValid(*OldPosition));
    Assert(!NewPosition || IsWorldPositionValid(*NewPosition));
    Assert(MemoryArena);

    if (OldPosition && NewPosition && AreInTheSameChunk(World, OldPosition, NewPosition))
    {
    }
    else
    {
        if (OldPosition)
        {
            chunk *OldLocationChunk = GetChunk(World, 0, OldPosition->ChunkX, OldPosition->ChunkY, OldPosition->ChunkZ);
            Assert(OldLocationChunk);

            storage_entity_indices_block *FirstBlockInChunk = &OldLocationChunk->FirstStorageEntitiesIndicesBlock;
            for
            (
                storage_entity_indices_block *CurrentIndicesBlock = FirstBlockInChunk;
                CurrentIndicesBlock;
                CurrentIndicesBlock = CurrentIndicesBlock->NextBlock
            )
            {
                b32 OuterBreakFlag = FALSE;
                for
                (
                    u32 StorageEntityIndexIndex = 0;
                    StorageEntityIndexIndex < CurrentIndicesBlock->StorageEntityIndicesCount;
                    StorageEntityIndexIndex++
                )
                {
                    if (CurrentIndicesBlock->StorageEntityIndices[StorageEntityIndexIndex] == StorageIndex)
                    {
                        Assert(FirstBlockInChunk->StorageEntityIndicesCount > 0);

                        CurrentIndicesBlock->StorageEntityIndices[StorageEntityIndexIndex] =
                            FirstBlockInChunk->StorageEntityIndices[--FirstBlockInChunk->StorageEntityIndicesCount];

                        if ((FirstBlockInChunk->StorageEntityIndicesCount == 0) && (FirstBlockInChunk->NextBlock))
                        {
                            storage_entity_indices_block *SecondBlock = FirstBlockInChunk->NextBlock;
                            *FirstBlockInChunk = *SecondBlock;

                            SecondBlock->NextBlock = World->StorageEntitiesIndicesBlocksFreeListHead;
                            World->StorageEntitiesIndicesBlocksFreeListHead = SecondBlock;
                        }

                        OuterBreakFlag = TRUE;
                        break;
                    }
                }

                if (OuterBreakFlag) break;
            }
        }

        if (NewPosition)
        {
            chunk *NewLocationChunk = GetChunk(World, MemoryArena, NewPosition->ChunkX, NewPosition->ChunkY, NewPosition->ChunkZ);
            Assert(NewLocationChunk);
            storage_entity_indices_block *FirstBlock = &NewLocationChunk->FirstStorageEntitiesIndicesBlock;

            if (FirstBlock->StorageEntityIndicesCount == ArrayCount(FirstBlock->StorageEntityIndices))
            {
                storage_entity_indices_block *NewBlock;
                if (World->StorageEntitiesIndicesBlocksFreeListHead)
                {
                    NewBlock = World->StorageEntitiesIndicesBlocksFreeListHead;
                    World->StorageEntitiesIndicesBlocksFreeListHead = World->StorageEntitiesIndicesBlocksFreeListHead->NextBlock;
                }
                else
                {
                    NewBlock = PushStruct(MemoryArena, storage_entity_indices_block);
                }

                *NewBlock = *FirstBlock;

                FirstBlock->NextBlock = NewBlock;
                FirstBlock->StorageEntityIndicesCount = 0;
            }

            Assert(FirstBlock->StorageEntityIndicesCount < ArrayCount(FirstBlock->StorageEntityIndices));
            FirstBlock->StorageEntityIndices[FirstBlock->StorageEntityIndicesCount++] = StorageIndex;
        }
    }
}

void ChangeStorageEntityLocationInWorld
(
    world *World, memory_arena *MemoryArena, u32 StorageIndex,
    storage_entity *StorageEntity, entity_world_position *NewWorldPosition
)
{
    entity_world_position *OldPosition = 0;
    if
    (
        !IsEntityFlagSet(&StorageEntity->Entity, EF_NON_SPATIAL) &&
        IsWorldPositionValid(StorageEntity->WorldPosition)
    )
    {
        OldPosition = &StorageEntity->WorldPosition;
    }

    entity_world_position *NewPosition = 0;
    if (IsWorldPositionValid(*NewWorldPosition))
    {
        NewPosition = NewWorldPosition;
    }

    RawChangeStorageEntityLocationInWorld(World, MemoryArena, StorageIndex, OldPosition, NewPosition);

    if (NewPosition)
    {
        StorageEntity->WorldPosition = *NewPosition;
        ClearEntityFlags(&StorageEntity->Entity, EF_NON_SPATIAL);
    }
    else
    {
        StorageEntity->WorldPosition = InvalidWorldPosition();
        SetEntityFlags(&StorageEntity->Entity, EF_NON_SPATIAL);
    }
}

storage_entity *GetStorageEntity(game_state *GameState, u32 StorageIndex)
{
    storage_entity *Result = 0;

    if ((StorageIndex > 0) && (StorageIndex < GameState->World->StorageEntityCount))
    {
        Result = &GameState->World->StorageEntities[StorageIndex];
    }

    return Result;
}

add_storage_entity_result AddStorageEntity(game_state *GameState, entity_type Type, entity_world_position WorldPosition)
{
    Assert(GameState->World->StorageEntityCount < ArrayCount(GameState->World->StorageEntities));

    add_storage_entity_result Result;

    Result.StorageIndex = GameState->World->StorageEntityCount++;
    Result.StorageEntity = GameState->World->StorageEntities + Result.StorageIndex;

    *Result.StorageEntity = {};
    Result.StorageEntity->Entity.Type = Type;
    Result.StorageEntity->Entity.CollisionMeshGroup = GameState->NullCollisionMeshGroupTemplate;
    Result.StorageEntity->WorldPosition = InvalidWorldPosition();

    ChangeStorageEntityLocationInWorld(GameState->World, &GameState->WorldArena, Result.StorageIndex, Result.StorageEntity, &WorldPosition);

    return Result;
}

add_storage_entity_result AddGoundBasedStorageEntity
(
    game_state *GameState, entity_type Type, entity_world_position GroundPoint,
    entity_collision_mesh_group *EntityCollisionMeshGroup
)
{
    add_storage_entity_result Result = AddStorageEntity(GameState, Type, GroundPoint);
    Result.StorageEntity->Entity.CollisionMeshGroup = EntityCollisionMeshGroup;
    return Result;
}

add_storage_entity_result AddStandardRoom(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    entity_world_position RoomPosition = GetWorldPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ, V3(0, 0, 0));

    add_storage_entity_result RoomStorageEntityResult =
        AddGoundBasedStorageEntity(GameState, ET_SPACE, RoomPosition, GameState->StandardRoomCollisionMeshGroupTemplate);

    SetEntityFlags(&RoomStorageEntityResult.StorageEntity->Entity, EF_TRAVERSABLE);

    return RoomStorageEntityResult;
}

add_storage_entity_result AddSword(game_state *GameState)
{
    add_storage_entity_result SwordStorageEntityResult = AddStorageEntity(GameState, ET_SWORD, InvalidWorldPosition());

    SwordStorageEntityResult.StorageEntity->Entity.CollisionMeshGroup = GameState->SwordCollisionMeshGroupTemplate;

    SetEntityFlags(&SwordStorageEntityResult.StorageEntity->Entity, EF_NON_SPATIAL | EF_MOVEABLE);

    return SwordStorageEntityResult;
}

add_storage_entity_result AddPlayer(game_state *GameState)
{
    add_storage_entity_result PlayerStorageEntityResult =
        AddGoundBasedStorageEntity(GameState, ET_HERO, GameState->CameraPosition, GameState->PlayerCollisionMeshGroupTemplate);

    SetEntityFlags(&PlayerStorageEntityResult.StorageEntity->Entity, EF_COLLIDES | EF_MOVEABLE);
    InitializeEntityHitPoints(PlayerStorageEntityResult.StorageEntity, 3);

    add_storage_entity_result SwordStorageEntityResult = AddSword(GameState);
    PlayerStorageEntityResult.StorageEntity->Entity.SwordEntityReference.StorageIndex = SwordStorageEntityResult.StorageIndex;

    if (GameState->StorageIndexOfEntityThatCameraFollows == 0)
    {
        GameState->StorageIndexOfEntityThatCameraFollows = PlayerStorageEntityResult.StorageIndex;
    }

    return PlayerStorageEntityResult;
}

add_storage_entity_result AddMonster(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    entity_world_position MonsterPosition = GetWorldPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ, V3(0, 0, 0));

    add_storage_entity_result MonsterStorageEntityResult =
        AddGoundBasedStorageEntity(GameState, ET_MONSTER, MonsterPosition, GameState->MonsterCollisionMeshGroupTemplate);

    SetEntityFlags(&MonsterStorageEntityResult.StorageEntity->Entity, EF_COLLIDES | EF_MOVEABLE);
    InitializeEntityHitPoints(MonsterStorageEntityResult.StorageEntity, 3);

    return MonsterStorageEntityResult;
}

add_storage_entity_result AddFamiliar(game_state *GameState, i32 AbsTileX, i32 AbsTileY, i32 AbsTileZ)
{
    entity_world_position FamiliarPosition = GetWorldPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ, V3(0, 0, 0));

    add_storage_entity_result FamiliarStorageEntityResult =
        AddGoundBasedStorageEntity(GameState, ET_FAMILIAR, FamiliarPosition, GameState->FamiliarCollisionMeshGroupTemplate);

    SetEntityFlags(&FamiliarStorageEntityResult.StorageEntity->Entity, EF_COLLIDES | EF_MOVEABLE);

    return FamiliarStorageEntityResult;
}

add_storage_entity_result AddWall(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    entity_world_position WallGroundPosition =
        GetWorldPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ, V3(0, 0, 0));

    add_storage_entity_result WallStorageEntityResult =
        AddGoundBasedStorageEntity(GameState, ET_WALL, WallGroundPosition, GameState->WallCollisionMeshGroupTemplate);

    SetEntityFlags(&WallStorageEntityResult.StorageEntity->Entity, EF_COLLIDES);

    return WallStorageEntityResult;
}

add_storage_entity_result AddStairs(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    entity_world_position StairsGroundPosition = GetWorldPositionFromTilePosition
    (
        GameState->World, AbsTileX, AbsTileY, AbsTileZ,
        V3(0, 0, 0)
    );

    // NOTE: this is the correct thing
    //entity_world_position StairsGroundPosition = GetWorldPositionFromTilePosition
    //(
    //    GameState->World, AbsTileX, AbsTileY, AbsTileZ,
    //    V3(0, 0, -0.5f * StairDiameter.Z)
    //);

    add_storage_entity_result StairStorageEntityResult =
        AddGoundBasedStorageEntity(GameState, ET_STAIRS, StairsGroundPosition, GameState->StairsCollisionMeshGroupTemplate);

    SetEntityFlags(&StairStorageEntityResult.StorageEntity->Entity, EF_COLLIDES);

    StairStorageEntityResult.StorageEntity->Entity.WalkableHeight = GameState->World->TileDepthInMeters;
    StairStorageEntityResult.StorageEntity->Entity.WalkableDiameter =
        StairStorageEntityResult.StorageEntity->Entity.CollisionMeshGroup->TotalCollisionMesh.Diameter.XY;

    return StairStorageEntityResult;
}

b32 TestWall
(
    f32 SquareCenterRelativeWallX,
    f32 SquareCenterToMovingEntityOriginalPositionX, f32 SquareCenterToMovingEntityOriginalPositionY,
    f32 MovingEntityMovementVectorX, f32 MovingEntityMovementVectorY,
    f32 *OriginalMinimalTParameter, f32 SquareCenterRelativeMinimumOrthogonalWallY, f32 SquareCenterRelativeMaximumOrthogonalWallY
)
{
    b32 DidMovingEntityHitWall = FALSE;

    if (MovingEntityMovementVectorX != 0.0f)
    {
        f32 TParameter = (SquareCenterRelativeWallX - SquareCenterToMovingEntityOriginalPositionX) / MovingEntityMovementVectorX;
        f32 SquareCenterRelativeCollisionPointY = SquareCenterToMovingEntityOriginalPositionY + TParameter * MovingEntityMovementVectorY;
        if
        (
            (SquareCenterRelativeCollisionPointY >= SquareCenterRelativeMinimumOrthogonalWallY) &&
            (SquareCenterRelativeCollisionPointY <= SquareCenterRelativeMaximumOrthogonalWallY)
        )
        {
            if ((TParameter >= 0) && (TParameter < *OriginalMinimalTParameter))
            {
                f32 ToleranceEpsilon = 0.001f;
                *OriginalMinimalTParameter = Max(0.0f, TParameter - ToleranceEpsilon);
                DidMovingEntityHitWall = TRUE;
            }
        }
    }

    return DidMovingEntityHitWall;
}

void MoveEntity
(
    game_state *GameState, simulation_region *SimulationRegion, entity *MovingEntity,
    v3 Acceleration, f32 TimeDelta, entity_movement_parameters *MoveSpec
)
{
    Assert(!IsEntityFlagSet(MovingEntity, EF_NON_SPATIAL));

    if (MoveSpec->NormalizeAcceleration)
    {
        if (LengthSquared(Acceleration) > 1.0f)
        {
            Acceleration = Normalize(Acceleration);
        }
    }
    Acceleration.XY *= MoveSpec->SpeedInXYPlane;
    Acceleration.XY -= MoveSpec->DragInXYPlane * MovingEntity->Velocity.XY;

    if (!IsEntityFlagSet(MovingEntity, EF_Z_SUPPORTED))
    {
        Acceleration.Z += -9.8f;
    }

    MovingEntity->Velocity = Acceleration * TimeDelta + MovingEntity->Velocity;
    Assert(Length(MovingEntity->Velocity) < SimulationRegion->MaxEntityVelocity);

    v3 MovementOffset =
        0.5f * Acceleration * Square(TimeDelta) +
        MovingEntity->Velocity * TimeDelta;

    if (MovingEntity->Position.Z < 0)
    {
        MovingEntity->Position.Z = 0;
    }

    f32 RemainingMovementDistance = MovingEntity->MovementDistanceLimit;
    if (RemainingMovementDistance == 0)
    {
        RemainingMovementDistance = ENTITY_INIFINTE_MOVEMENT_DISTANCE;
    }

    for (u32 CollisionDetectionIteration = 0; CollisionDetectionIteration < 4; CollisionDetectionIteration++)
    {
        f32 MovementDistance = Length(MovementOffset);
        if (MovementDistance > 0)
        {
            entity *HitEntity = 0;
            v3 CollisionNormal = V3(0, 0, 0);

            f32 TMin = 1.0f;
            if (RemainingMovementDistance < MovementDistance)
            {
                TMin = RemainingMovementDistance / MovementDistance;
            }

            v3 TargetPosition = MovingEntity->Position + MovementOffset;

            if (!IsEntityFlagSet(MovingEntity, EF_NON_SPATIAL))
            {
                entity *TestEntity = SimulationRegion->Entities;
                for
                (
                    u32 TestEntityIndex = 0;
                    TestEntityIndex < SimulationRegion->CurrentEntityCount;
                    TestEntityIndex++, TestEntity++
                )
                {
                    if (CanEntitiesCollide(GameState, TestEntity, MovingEntity))
                    {
                        b32 TestEntityWasHit = FALSE;
                        v3 CollisionPointNormal = V3(0, 0, 0);
                        f32 TestTMin = TMin;

                        for
                        (
                            u32 MovingEntityCollisionMeshIndex = 0;
                            MovingEntityCollisionMeshIndex < MovingEntity->CollisionMeshGroup->MeshCount;
                            MovingEntityCollisionMeshIndex++
                        )
                        {
                            entity_collision_mesh *MovingEntityCollisionMesh =
                                &MovingEntity->CollisionMeshGroup->Meshes[MovingEntityCollisionMeshIndex];

                            for
                            (
                                u32 TestEntityCollisionMeshIndex = 0;
                                TestEntityCollisionMeshIndex < TestEntity->CollisionMeshGroup->MeshCount;
                                TestEntityCollisionMeshIndex++
                            )
                            {
                                entity_collision_mesh *TestEntityCollisionMesh =
                                    &TestEntity->CollisionMeshGroup->Meshes[TestEntityCollisionMeshIndex];

                                v3 MinkowskiCollisionDiameter =
                                    TestEntityCollisionMesh->Diameter + MovingEntityCollisionMesh->Diameter;

                                v3 CollisionAreaMinCorner = -0.5f * MinkowskiCollisionDiameter;
                                v3 CollisionAreaMaxCorner = CollisionAreaMinCorner + MinkowskiCollisionDiameter;

                                v3 TestMeshToMovingMesh =
                                    (MovingEntity->Position + MovingEntityCollisionMesh->Offset) -
                                    (TestEntity->Position + TestEntityCollisionMesh->Offset);

                                b32 EntitiesOverlapInZ =
                                (
                                    (TestMeshToMovingMesh.Z >= CollisionAreaMinCorner.Z) &&
                                    (TestMeshToMovingMesh.Z < CollisionAreaMaxCorner.Z)
                                );

                                if (EntitiesOverlapInZ)
                                {
                                    if
                                    (
                                        TestWall
                                        (
                                            CollisionAreaMinCorner.X, TestMeshToMovingMesh.X, TestMeshToMovingMesh.Y,
                                            MovementOffset.X, MovementOffset.Y,
                                            &TestTMin, CollisionAreaMinCorner.Y, CollisionAreaMaxCorner.Y
                                        )
                                    )
                                    {
                                        CollisionPointNormal = V3(-1, 0, 0);
                                        TestEntityWasHit = TRUE;
                                    }

                                    if
                                    (
                                        TestWall
                                        (
                                            CollisionAreaMaxCorner.X, TestMeshToMovingMesh.X, TestMeshToMovingMesh.Y,
                                            MovementOffset.X, MovementOffset.Y,
                                            &TestTMin, CollisionAreaMinCorner.Y, CollisionAreaMaxCorner.Y
                                        )
                                    )
                                    {
                                        CollisionPointNormal = V3(1, 0, 0);
                                        TestEntityWasHit = TRUE;
                                    }

                                    if
                                    (
                                        TestWall
                                        (
                                            CollisionAreaMinCorner.Y, TestMeshToMovingMesh.Y, TestMeshToMovingMesh.X,
                                            MovementOffset.Y, MovementOffset.X,
                                            &TestTMin, CollisionAreaMinCorner.X, CollisionAreaMaxCorner.X
                                        )
                                    )
                                    {
                                        CollisionPointNormal = V3(0, -1, 0);
                                        TestEntityWasHit = TRUE;
                                    }

                                    if
                                    (
                                        TestWall
                                        (
                                            CollisionAreaMaxCorner.Y, TestMeshToMovingMesh.Y, TestMeshToMovingMesh.X,
                                            MovementOffset.Y, MovementOffset.X,
                                            &TestTMin, CollisionAreaMinCorner.X, CollisionAreaMaxCorner.X
                                        )
                                    )
                                    {
                                        CollisionPointNormal = V3(0, 1, 0);
                                        TestEntityWasHit = TRUE;
                                    }

                                    if (TestEntityWasHit)
                                    {
                                        if (SpeculativeCollision(MovingEntity, TestEntity))
                                        {
                                            HitEntity = TestEntity;
                                            TMin = TestTMin;
                                            CollisionNormal = CollisionPointNormal;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            MovingEntity->Position += TMin * MovementOffset;
            MovementOffset = TargetPosition - MovingEntity->Position;
            RemainingMovementDistance -= TMin * MovementDistance;

            if (HitEntity)
            {
                b32 EntityStoppedOnCollision = ProcessEntityCollision(GameState, MovingEntity, HitEntity);
                if (EntityStoppedOnCollision)
                {
                    MovementOffset = MovementOffset - 1 * InnerProduct(MovementOffset, CollisionNormal) * CollisionNormal;
                    MovingEntity->Velocity =
                        MovingEntity->Velocity - 1 * InnerProduct(MovingEntity->Velocity, CollisionNormal) * CollisionNormal;
                }
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    f32 GroundLevel = 0;
    rectangle3 MovingEntityVolume = RectCenterDiameter
    (
        MovingEntity->Position + MovingEntity->CollisionMeshGroup->TotalCollisionMesh.Offset,
        MovingEntity->CollisionMeshGroup->TotalCollisionMesh.Diameter
    );

    entity *TestEntity = SimulationRegion->Entities;
    for
    (
        u32 TestEntityIndex = 0;
        TestEntityIndex < SimulationRegion->CurrentEntityCount;
        TestEntityIndex++, TestEntity++
    )
    {
        rectangle3 TestEntityVolume = RectCenterDiameter
        (
            TestEntity->Position + TestEntity->CollisionMeshGroup->TotalCollisionMesh.Offset,
            TestEntity->CollisionMeshGroup->TotalCollisionMesh.Diameter
        );

        if
        (
            CanEntitiesOverlap(GameState, MovingEntity, TestEntity) &&
            DoRectanglesOverlap(MovingEntityVolume, TestEntityVolume)
        )
        {
            HandleEntityOverlapWithStairs(GameState, MovingEntity, TestEntity, TimeDelta, &GroundLevel);
        }
    }

    f32 MovingEntityTargetZLevel = GroundLevel + (MovingEntity->Position.Z - GetEntityGroundPoint(MovingEntity).Z);
    if
    (
        (MovingEntity->Position.Z <= MovingEntityTargetZLevel) ||
        (
            IsEntityFlagSet(MovingEntity, EF_Z_SUPPORTED) &&
            (MovingEntity->Velocity.Z == 0)
        )
    )
    {
        MovingEntity->Position.Z = MovingEntityTargetZLevel;
        MovingEntity->Velocity.Z = 0;
        SetEntityFlags(MovingEntity, EF_Z_SUPPORTED);
    }
    else
    {
        ClearEntityFlags(MovingEntity, EF_Z_SUPPORTED);
    }

    if (MovingEntity->MovementDistanceLimit != 0)
    {
        MovingEntity->MovementDistanceLimit = RemainingMovementDistance;
    }

    if ((MovingEntity->Velocity.X == 0) && (MovingEntity->Velocity.Y == 0))
    {
    }
    else if (AbsoluteValue(MovingEntity->Velocity.X) > AbsoluteValue(MovingEntity->Velocity.Y))
    {
        if (MovingEntity->Velocity.X > 0)
        {
            MovingEntity->BitmapFacingDirection = 0;
        }
        else
        {
            MovingEntity->BitmapFacingDirection = 2;
        }
    }
    else
    {
        if (MovingEntity->Velocity.Y > 0)
        {
            MovingEntity->BitmapFacingDirection = 1;
        }
        else
        {
            MovingEntity->BitmapFacingDirection = 3;
        }
    }
}