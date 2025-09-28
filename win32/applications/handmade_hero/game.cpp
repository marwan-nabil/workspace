// TODO:
// - improve collision rule storage data structure to optimize for insertion and clearing.
// - fix buggy Z handling in entity.cpp AddGoundBasedStorageEntity().
// - improve velocity and postition update accuracy in MoveEntity().
// - reduce the number of tested against entities in the sim region
//   using spatial partition in MoveEntity().
// - bitmap facing direction should be determined by the Acceleration vector instead
//   of the velocity vector in MoveEntity().
// - win32_platform.cpp Win32DisplayBufferInWindow() center when in full screen.
// - win32_platform.cpp Win32FillGlobalSoundBuffer() assert that the returned region sizes
//   from Lock() are valid (multiples of 4 bytes).
// - in game.cpp in the simulation region entity processing loop,
//   ShadowBitMap alpha/position calculation is impercise, should be after postition update for all entities.
// - in win32_platform.cpp add logging for missed frames.
// - in world.cpp IsOffsetWithinInterval():
// - determine a good safety margin.
// - fix fp math so this should be < & >.
// - in world.cpp CanonicalizeIntervalIndexAndOffset() fix this shit (truncation party).
// - in world.cpp GetWorldPositionFromTilePosition() fix buggy Z handling.
// - determine inlinable functions and move them to headers
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

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert
    (
        (&GameInput->ControllerStates[0].Start - &GameInput->ControllerStates[0].Buttons[0])
        ==
        (ArrayCount(GameInput->ControllerStates[0].Buttons) - 1)
    );
    Assert(sizeof(game_state) <= GameMemory->PermanentStorageSize);

    game_state *GameState = (game_state *)GameMemory->PermanentStorage;

    if (!GameMemory->IsInitialized)
    {
        GameMemory->IsInitialized = TRUE;

        InitializeMemoryArena
        (
            &GameState->WorldArena,
            GameMemory->PermanentStorageSize - sizeof(game_state),
            (void *)((size_t)GameMemory->PermanentStorage + sizeof(game_state))
        );

        u32 TilesPerScreenWidth = 17;
        u32 TilesPerScreenHeight = 9;
        f32 TileSideInMeters = 1.4f;
        f32 TileDepthInMeters = 3.0f;
        i32 TileSideInPixels = 60;

        GameState->NullCollisionMeshGroupTemplate =
            MakeNullCollisionMeshTemplate(GameState);
        GameState->SwordCollisionMeshGroupTemplate =
            MakeSimpleCollisionMeshTemplate(GameState, V3(1.0f, 0.5f, 0.1f));
        GameState->WallCollisionMeshGroupTemplate =
            MakeSimpleCollisionMeshTemplate(GameState, V3(TileSideInMeters, TileSideInMeters, TileDepthInMeters));
        GameState->StandardRoomCollisionMeshGroupTemplate =
            MakeSimpleCollisionMeshTemplate(GameState, V3(TilesPerScreenWidth * TileSideInMeters, TilesPerScreenHeight * TileSideInMeters, 0.9f * TileDepthInMeters));
        GameState->StairsCollisionMeshGroupTemplate =
            MakeSimpleCollisionMeshTemplate(GameState, V3(TileSideInMeters, 2 * TileSideInMeters, 1.1f * TileDepthInMeters));
        GameState->PlayerCollisionMeshGroupTemplate =
            MakeSimpleCollisionMeshTemplate(GameState, V3(1.0f, 0.5f, 1.2f));
        GameState->FamiliarCollisionMeshGroupTemplate =
            MakeSimpleCollisionMeshTemplate(GameState, V3(1.0f, 0.5f, 0.5f));
        GameState->MonsterCollisionMeshGroupTemplate =
            MakeSimpleCollisionMeshTemplate(GameState, V3(1.0f, 0.5f, 0.5f));

        GameState->BackDropBitMap =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_background.bmp");
        GameState->ShadowBitMap =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_shadow.bmp");
        GameState->TreeBitMap =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test2/tree00.bmp");
        GameState->StairWellBitMap =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test2/rock02.bmp");
        GameState->SwordBitMap =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test2/rock03.bmp");

        hero_bitmap_group *HeroBitmapGroup = &GameState->HeroBitmapGroups[0];
        HeroBitmapGroup->Head =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_right_head.bmp");
        HeroBitmapGroup->Cape =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_right_cape.bmp");
        HeroBitmapGroup->Torso =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_right_torso.bmp");
        HeroBitmapGroup->Alignment = V2(72, 182);

        HeroBitmapGroup = &GameState->HeroBitmapGroups[1];
        HeroBitmapGroup->Head =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_back_head.bmp");
        HeroBitmapGroup->Cape =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_back_cape.bmp");
        HeroBitmapGroup->Torso =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_back_torso.bmp");
        HeroBitmapGroup->Alignment = V2(72, 182);

        HeroBitmapGroup = &GameState->HeroBitmapGroups[2];
        HeroBitmapGroup->Head =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_left_head.bmp");
        HeroBitmapGroup->Cape =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_left_cape.bmp");
        HeroBitmapGroup->Torso =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_left_torso.bmp");
        HeroBitmapGroup->Alignment = V2(72, 182);

        HeroBitmapGroup = &GameState->HeroBitmapGroups[3];
        HeroBitmapGroup->Head =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_front_head.bmp");
        HeroBitmapGroup->Cape =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_front_cape.bmp");
        HeroBitmapGroup->Torso =
            LoadBitmap(GameMemory->PlatformReadFile, ThreadContext, (char *)"data/handmade_hero/test/test_hero_front_torso.bmp");
        HeroBitmapGroup->Alignment = V2(72, 182);

        world *World = PushStruct(&GameState->WorldArena, world);
        GameState->World = World;
        InitializeWorld(World, TileSideInMeters, TileDepthInMeters);

        GameState->PixelsToMetersRatio = (f32)TileSideInPixels / TileSideInMeters;

        AddStorageEntity(GameState, ET_NULL, InvalidWorldPosition());

        u32 RandomNumbersIndex = 99;

        u32 StartScreenX = 0;// INT16_MAX / TilesPerScreenWidth / 2;
        u32 StartScreenY = 0;//INT16_MAX / TilesPerScreenHeight / 2;
        u32 StartScreenZ = 0;//INT16_MAX / 2;

        u32 PreviousScreenX = StartScreenX;
        u32 PreviousScreenY = StartScreenY;
        u32 PreviousScreenZ = StartScreenZ;

        u32 NextScreenX = PreviousScreenX;
        u32 NextScreenY = PreviousScreenY;
        u32 NextScreenZ = PreviousScreenZ;

        b32 PreviousDoorLeft = FALSE;
        b32 PreviousDoorRight = FALSE;
        b32 PreviousDoorTop = FALSE;
        b32 PreviousDoorBottom = FALSE;
        b32 PreviousDoorUpStairs = FALSE;
        b32 PreviousDoorDownStairs = FALSE;

        for (u32 ScreenIndex = 0; ScreenIndex < 2000; ScreenIndex++)
        {
            u32 CurrentScreenX = NextScreenX;
            u32 CurrentScreenY = NextScreenY;
            u32 CurrentScreenZ = NextScreenZ;
            b32 CurrentDoorLeft = FALSE;
            b32 CurrentDoorRight = FALSE;
            b32 CurrentDoorTop = FALSE;
            b32 CurrentDoorBottom = FALSE;
            b32 CurrentDoorUpStairs = FALSE;
            b32 CurrentDoorDownStairs = FALSE;

            Assert(RandomNumbersIndex < ArrayCount(RandomNumbersTable));
            u32 RandomChoice = 0;

            if
            (
                (PreviousDoorDownStairs && (CurrentScreenZ < PreviousScreenZ)) ||
                (PreviousDoorUpStairs && (CurrentScreenZ > PreviousScreenZ))
            )
            {
                // we can't have upstairs/downstairs movement,
                // previous room is directly above or below us
                RandomChoice = RandomNumbersTable[RandomNumbersIndex++] % 2;
            }
            else
            {
                RandomChoice = RandomNumbersTable[RandomNumbersIndex++] % 3;
            }

            if (RandomNumbersIndex == ArrayCount(RandomNumbersTable))
            {
                RandomNumbersIndex = 0;
            }

            if (RandomChoice == 0)
            {
                if (PreviousDoorLeft && (CurrentScreenX < PreviousScreenX))
                {
                    CurrentDoorTop = TRUE;
                    NextScreenY = CurrentScreenY + 1;
                }
                else
                {
                    CurrentDoorRight = TRUE;
                    NextScreenX = CurrentScreenX + 1;
                }
            }
            else if (RandomChoice == 1)
            {
                if (PreviousDoorBottom && (CurrentScreenY < PreviousScreenY))
                {
                    CurrentDoorRight = TRUE;
                    NextScreenX = CurrentScreenX + 1;
                }
                else
                {
                    CurrentDoorTop = TRUE;
                    NextScreenY = CurrentScreenY + 1;
                }
            }
            else if (RandomChoice == 2)
            {
                if (CurrentScreenZ == StartScreenZ)
                {
                    CurrentDoorUpStairs = TRUE;
                    NextScreenZ = StartScreenZ + 1;
                }
                else
                {
                    CurrentDoorDownStairs = TRUE;
                    NextScreenZ = StartScreenZ;
                }
            }

            if (CurrentScreenX < PreviousScreenX)
            {
                CurrentDoorRight = TRUE;
            }
            else if (CurrentScreenX > PreviousScreenX)
            {
                CurrentDoorLeft = TRUE;
            }
            else if (CurrentScreenY < PreviousScreenY)
            {
                CurrentDoorTop = TRUE;
            }
            else if (CurrentScreenY > PreviousScreenY)
            {
                CurrentDoorBottom = TRUE;
            }
            else if (CurrentScreenZ < PreviousScreenZ)
            {
                CurrentDoorUpStairs = TRUE;
            }
            else if (CurrentScreenZ > PreviousScreenZ)
            {
                CurrentDoorDownStairs = TRUE;
            }

            AddStandardRoom
            (
                GameState,
                (u32)(CurrentScreenX * TilesPerScreenWidth + TilesPerScreenWidth / 2.0f),
                (u32)(CurrentScreenY * TilesPerScreenHeight + TilesPerScreenHeight / 2.0f),
                CurrentScreenZ
            );

            for (u32 TileY = 0; TileY < TilesPerScreenHeight; TileY++)
            {
                for (u32 TileX = 0; TileX < TilesPerScreenWidth; TileX++)
                {
                    u32 AbsTileX = CurrentScreenX * TilesPerScreenWidth + TileX;
                    u32 AbsTileY = CurrentScreenY * TilesPerScreenHeight + TileY;
                    u32 AbsTileZ = CurrentScreenZ;
                    u32 TileValue = 1; // walkable

                    if (TileX == 0)
                    {
                        TileValue = 2; // wall
                        if (CurrentDoorLeft && (TileY == TilesPerScreenHeight / 2))
                        {
                            TileValue = 1; // door
                        }
                    }
                    if (TileX == (TilesPerScreenWidth - 1))
                    {
                        TileValue = 2; // wall
                        if (CurrentDoorRight && (TileY == TilesPerScreenHeight / 2))
                        {
                            TileValue = 1; // door
                        }
                    }
                    if (TileY == 0)
                    {
                        TileValue = 2; // wall
                        if (CurrentDoorBottom && (TileX == TilesPerScreenWidth / 2))
                        {
                            TileValue = 1; // door
                        }
                    }
                    if (TileY == (TilesPerScreenHeight - 1))
                    {
                        TileValue = 2; // wall
                        if (CurrentDoorTop && (TileX == TilesPerScreenWidth / 2))
                        {
                            TileValue = 1; // door
                        }
                    }

                    if (TileX == 2 && TileY == 5)
                    {
                        if (CurrentDoorUpStairs)
                        {
                            TileValue = 3; // upstairs door
                        }
                    }
                    else if (TileX == 2 && TileY == 2)
                    {
                        if (CurrentDoorDownStairs)
                        {
                            TileValue = 4; // downstairs door
                        }
                    }

                    if (TileValue == 2) // wall
                    {
                        AddWall(GameState, AbsTileX, AbsTileY, AbsTileZ);
                    }
                    else if (TileValue == 3)
                    {
                        AddStairs(GameState, AbsTileX, AbsTileY, AbsTileZ);
                    }
                    //else if (TileValue == 4)
                    //{
                    //    AddStairs(GameState, AbsTileX, AbsTileY, AbsTileZ);
                    //}
                }
            }

            PreviousScreenX = CurrentScreenX;
            PreviousScreenY = CurrentScreenY;
            PreviousScreenZ = CurrentScreenZ;
            PreviousDoorLeft = CurrentDoorLeft;
            PreviousDoorRight = CurrentDoorRight;
            PreviousDoorTop = CurrentDoorTop;
            PreviousDoorBottom = CurrentDoorBottom;
            PreviousDoorUpStairs = CurrentDoorUpStairs;
            PreviousDoorDownStairs = CurrentDoorDownStairs;
        }

#if 0
        // NOTE: Dummy filler low freq. entities
        while (GameState->World->StorageEntityCount < (ArrayCount(GameState->World->StorageEntities) - 16))
        {
            u32 Coordinate = 1024 + GameState->World->StorageEntityCount;
            AddWall(GameState, Coordinate, Coordinate, Coordinate);
        }
#endif

        u32 CameraTileX = StartScreenX * TilesPerScreenWidth + TilesPerScreenWidth / 2;
        u32 CameraTileY = StartScreenY * TilesPerScreenHeight + TilesPerScreenHeight / 2;
        u32 CameraTileZ = StartScreenZ;

        GameState->CameraPosition = GetWorldPositionFromTilePosition(World, CameraTileX, CameraTileY, CameraTileZ, V3(0, 0, 0));

        AddMonster(GameState, CameraTileX + 2, CameraTileY + 2, CameraTileZ);

        for (u32 FamiliarIndex = 0; FamiliarIndex < 1; FamiliarIndex++)
        {
            i32 FamiliarXOffset = (RandomNumbersTable[RandomNumbersIndex++] % 6) - 3;
            if (RandomNumbersIndex == ArrayCount(RandomNumbersTable))
            {
                RandomNumbersIndex = 0;
            }

            i32 FamiliarYOffset = (RandomNumbersTable[RandomNumbersIndex++] % 4) - 2;
            if (RandomNumbersIndex == ArrayCount(RandomNumbersTable))
            {
                RandomNumbersIndex = 0;
            }

            if ((FamiliarXOffset != 0) || (FamiliarYOffset != 0))
            {
                AddFamiliar(GameState, CameraTileX - FamiliarXOffset, CameraTileY - FamiliarYOffset, CameraTileZ);
            }
        }
    }

    world *World = GameState->World;

    for (u32 ControllerIndex = 0; ControllerIndex < ArrayCount(GameInput->ControllerStates); ControllerIndex++)
    {
        controller_state *ControllerInput = GetController(GameInput, ControllerIndex);
        controlled_hero_input *ControlledHeroInput = GameState->ControllerToHeroInputMap + ControllerIndex;

        if (ControlledHeroInput->HeroEntityStorageIndex == 0)
        {
            if (ControllerInput->Start.IsDown)
            {
                *ControlledHeroInput = {};
                ControlledHeroInput->HeroEntityStorageIndex = AddPlayer(GameState).StorageIndex;
            }
        }
        else
        {
            ControlledHeroInput->InputJumpVelocity = 0;
            ControlledHeroInput->InputAcceleration = V3(0.0f, 0.0f, 0.0f);
            ControlledHeroInput->InputSwordDirection = V2(0.0f, 0.0f);

            if (ControllerInput->MovementIsAnalog)
            {
                ControlledHeroInput->InputAcceleration =
                    V3(ControllerInput->StickAverageX, ControllerInput->StickAverageY, 0);
            }
            else
            {
                if (ControllerInput->MoveUp.IsDown)
                {
                    ControlledHeroInput->InputAcceleration.Y = 1.0f;
                }
                if (ControllerInput->MoveDown.IsDown)
                {
                    ControlledHeroInput->InputAcceleration.Y = -1.0f;
                }
                if (ControllerInput->MoveRight.IsDown)
                {
                    ControlledHeroInput->InputAcceleration.X = 1.0f;
                }
                if (ControllerInput->MoveLeft.IsDown)
                {
                    ControlledHeroInput->InputAcceleration.X = -1.0f;
                }
            }

            if (ControllerInput->Start.IsDown)
            {
                ControlledHeroInput->InputJumpVelocity = 2.0f;
            }

            if (ControllerInput->ActionUp.IsDown)
            {
                ControlledHeroInput->InputSwordDirection = V2(0, 1);
            }
            else if (ControllerInput->ActionDown.IsDown)
            {
                ControlledHeroInput->InputSwordDirection = V2(0, -1);
            }
            else if (ControllerInput->ActionRight.IsDown)
            {
                ControlledHeroInput->InputSwordDirection = V2(1, 0);
            }
            else if (ControllerInput->ActionLeft.IsDown)
            {
                ControlledHeroInput->InputSwordDirection = V2(-1, 0);
            }
        }
    }

    u32 CameraRegionTileSpanX = 17 * 3;
    u32 CameraRegionTileSpanY = 9 * 3;
    u32 CameraRegionTileSpanZ = 1;
    rectangle3 CameraRegionUpdateBounds = RectCenterDiameter
    (
        V3(0, 0, 0),
        GameState->World->TileSideInMeters * V3((f32)CameraRegionTileSpanX, (f32)CameraRegionTileSpanY, (f32)CameraRegionTileSpanZ)
    );

    memory_arena SimulationArena;
    InitializeMemoryArena(&SimulationArena, GameMemory->TransientStorageSize, GameMemory->TransientStorage);

    simulation_region *CameraSimulationRegion =
        BeginSimulation(GameState, World, &SimulationArena, GameState->CameraPosition, CameraRegionUpdateBounds, GameInput->TimeDeltaForFrame);

#if 1
    DrawRectangle(PixelBuffer, V2(0.0f, 0.0f), V2((f32)PixelBuffer->WidthInPixels, (f32)PixelBuffer->HeightInPixels), 0.5f, 0.5f, 0.5f, 1.0f);
#else
    DrawBitmap(&GameState->BackDropBitMap, PixelBuffer, 0, 0, 0, 0, 1);
#endif

    render_peice_group EntityRenderPeiceGroup = {};

    entity *CurrentEntity = CameraSimulationRegion->Entities;
    for
    (
        u32 EntityIndex = 0;
        EntityIndex < CameraSimulationRegion->CurrentEntityCount;
        EntityIndex++, CurrentEntity++
    )
    {
        if (CurrentEntity->CanUpdate)
        {
            EntityRenderPeiceGroup.Count = 0;
            hero_bitmap_group *HeroBitmapGroup = &GameState->HeroBitmapGroups[CurrentEntity->BitmapFacingDirection];

            f32 ShadowAlphaFactor = 1.0f - 0.5f * CurrentEntity->Position.Z;
            if (ShadowAlphaFactor < 0)
            {
                ShadowAlphaFactor = 0;
            }

            entity_movement_parameters EntityMoveSpec = DefaultEntityMovementParameters();
            v3 EntityAcceleration = V3(0, 0, 0);

            switch (CurrentEntity->Type)
            {
                case ET_HERO:
                {
                    for (u32 ControlledHeroInputIndex = 0; ControlledHeroInputIndex < ArrayCount(GameState->ControllerToHeroInputMap); ControlledHeroInputIndex++)
                    {
                        controlled_hero_input *ControlledHeroInput = GameState->ControllerToHeroInputMap + ControlledHeroInputIndex;
                        if (ControlledHeroInput->HeroEntityStorageIndex == CurrentEntity->StorageIndex)
                        {
                            if (ControlledHeroInput->InputJumpVelocity != 0)
                            {
                                CurrentEntity->Velocity.Z = ControlledHeroInput->InputJumpVelocity;
                            }

                            EntityMoveSpec.NormalizeAcceleration = TRUE;
                            EntityMoveSpec.SpeedInXYPlane = 50.0f;
                            EntityMoveSpec.DragInXYPlane = 4.0f;

                            EntityAcceleration = ControlledHeroInput->InputAcceleration;

                            if ((ControlledHeroInput->InputSwordDirection.X != 0) || (ControlledHeroInput->InputSwordDirection.Y != 0))
                            {
                                entity *SwordEntity = CurrentEntity->SwordEntityReference.Entity;
                                if (SwordEntity && IsEntityFlagSet(SwordEntity, EF_NON_SPATIAL))
                                {
                                    MakeEntitySpatial
                                    (
                                        SwordEntity,
                                        CurrentEntity->Position,
                                        CurrentEntity->Velocity + 5.0f * V3(ControlledHeroInput->InputSwordDirection, 0)
                                    );
                                    SwordEntity->MovementDistanceLimit = 5.0f;

                                    AddEntityCollisionRule(GameState, SwordEntity->StorageIndex, CurrentEntity->StorageIndex, FALSE);
                                }
                            }
                        }
                    }

                    PushBitmapRenderPiece(GameState, &EntityRenderPeiceGroup, &GameState->ShadowBitMap, V3(0, 0, 0), HeroBitmapGroup->Alignment, ShadowAlphaFactor, 0.0f);
                    PushBitmapRenderPiece(GameState, &EntityRenderPeiceGroup, &HeroBitmapGroup->Torso, V3(0, 0, 0), HeroBitmapGroup->Alignment, 1, 1.0f);
                    PushBitmapRenderPiece(GameState, &EntityRenderPeiceGroup, &HeroBitmapGroup->Cape, V3(0, 0, 0), HeroBitmapGroup->Alignment, 1, 1.0f);
                    PushBitmapRenderPiece(GameState, &EntityRenderPeiceGroup, &HeroBitmapGroup->Head, V3(0, 0, 0), HeroBitmapGroup->Alignment, 1, 1.0f);
                    DrawHitpoints(GameState, &EntityRenderPeiceGroup, CurrentEntity);
                } break;

                case ET_WALL:
                {
                    PushBitmapRenderPiece(GameState, &EntityRenderPeiceGroup, &GameState->TreeBitMap, V3(0, 0, 0), V2(40, 80), 1, 1.0f);
                } break;

                case ET_STAIRS:
                {
                    PushRectangleRenderPiece(GameState, &EntityRenderPeiceGroup, V3(0, 0, 0), CurrentEntity->WalkableDiameter, V4(1, 0.5f, 0, 1), 0);
                    PushRectangleRenderPiece(GameState, &EntityRenderPeiceGroup, V3(0, 0, CurrentEntity->WalkableHeight), CurrentEntity->WalkableDiameter, V4(1, 1, 0, 1), 0);
                } break;

                case ET_SWORD:
                {
                    EntityMoveSpec.NormalizeAcceleration = FALSE;
                    EntityMoveSpec.SpeedInXYPlane = 0;
                    EntityMoveSpec.DragInXYPlane = 0;

                    EntityAcceleration = V3(0, 0, 0);

                    if (CurrentEntity->MovementDistanceLimit == 0)
                    {
                        ClearAllEntityCollisionRules(GameState, CurrentEntity->StorageIndex);
                        MakeEntityNonSpatial(CurrentEntity);
                    }

                    PushBitmapRenderPiece(GameState, &EntityRenderPeiceGroup, &GameState->ShadowBitMap, V3(0, 0, 0), HeroBitmapGroup->Alignment, ShadowAlphaFactor, 0.0f);
                    PushBitmapRenderPiece(GameState, &EntityRenderPeiceGroup, &GameState->SwordBitMap, V3(0, 0, 0), V2(29, 10), 1, 1.0f);
                } break;

                case ET_MONSTER:
                {
                    PushBitmapRenderPiece(GameState, &EntityRenderPeiceGroup, &HeroBitmapGroup->Torso, V3(0, 0, 0), HeroBitmapGroup->Alignment, 1, 1.0f);
                    PushBitmapRenderPiece(GameState, &EntityRenderPeiceGroup, &GameState->ShadowBitMap, V3(0, 0, 0), HeroBitmapGroup->Alignment, ShadowAlphaFactor, 0.0f);
                    DrawHitpoints(GameState, &EntityRenderPeiceGroup, CurrentEntity);
                } break;

                case ET_FAMILIAR:
                {
                    EntityAcceleration = V3(0, 0, 0);

                    EntityMoveSpec.NormalizeAcceleration = TRUE;
                    EntityMoveSpec.SpeedInXYPlane = 50.0f;
                    EntityMoveSpec.DragInXYPlane = 4.0f;

    #if 0
                    entity *ClosestHeroEntity = 0;
                    f32 ClosestHeroDistanceSquared = 100.0f;

                    entity *TestEntity = CameraSimulationRegion->Entities;
                    for (u32 TestEntityIndex = 0; TestEntityIndex < CameraSimulationRegion->CurrentEntityCount; TestEntityIndex++, TestEntity++)
                    {
                        if (TestEntity->Type == ET_HERO)
                        {
                            f32 ThisHeroDistanceSquared = LengthSquared(TestEntity->GroundPoint - CurrentEntity->GroundPoint);
                            if (ThisHeroDistanceSquared < ClosestHeroDistanceSquared)
                            {
                                ClosestHeroDistanceSquared = ThisHeroDistanceSquared;
                                ClosestHeroEntity = TestEntity;
                            }
                        }
                    }

                    if (ClosestHeroEntity && (ClosestHeroDistanceSquared > Square(3.0f)))
                    {
                        f32 AccelerationMultiplier = 0.5f;
                        f32 OneOverClosestHeroDistance = 1.0f / SquareRoot(ClosestHeroDistanceSquared);

                        EntityAcceleration =
                            AccelerationMultiplier *
                            OneOverClosestHeroDistance *
                            (ClosestHeroEntity->GroundPoint - CurrentEntity->GroundPoint);
                    }
    #endif
                    CurrentEntity->BobbingSinParameter += 5.0f * GameInput->TimeDeltaForFrame;
                    if (CurrentEntity->BobbingSinParameter > 2.0f * PI32)
                    {
                        CurrentEntity->BobbingSinParameter -= 2.0f * PI32;
                    }
                    f32 BobbingSin = Sin(CurrentEntity->BobbingSinParameter);
                    ShadowAlphaFactor = 0.5f * ShadowAlphaFactor + 0.2f * BobbingSin;

                    PushBitmapRenderPiece(GameState, &EntityRenderPeiceGroup, &GameState->ShadowBitMap, V3(0, 0, 0), HeroBitmapGroup->Alignment, ShadowAlphaFactor, 0.0f);
                    PushBitmapRenderPiece(GameState, &EntityRenderPeiceGroup, &HeroBitmapGroup->Head, V3(0, 0, 0.25f * BobbingSin), HeroBitmapGroup->Alignment, 1, 1.0f);
                } break;

                case ET_SPACE:
                {
                    for (u32 MeshIndex = 0; MeshIndex < CurrentEntity->CollisionMeshGroup->MeshCount; MeshIndex++)
                    {
                        entity_collision_mesh *CurrnetCollisionMesh =  CurrentEntity->CollisionMeshGroup->Meshes + MeshIndex;

                        PushRectangleOutlineRenderPieces(GameState, &EntityRenderPeiceGroup, V3(CurrnetCollisionMesh->Offset.XY, 0),
                                                        CurrnetCollisionMesh->Diameter.XY, V4(0, 0.5f, 1.0f, 1.0f), 0);
                    }
                } break;

                default:
                {
                    InvalidCodepath;
                } break;
            }

            if
            (
                !IsEntityFlagSet(CurrentEntity, EF_NON_SPATIAL) &&
                IsEntityFlagSet(CurrentEntity, EF_MOVEABLE)
            )
            {
                MoveEntity(GameState, CameraSimulationRegion, CurrentEntity, EntityAcceleration, GameInput->TimeDeltaForFrame, &EntityMoveSpec);
            }

            render_piece *RenderPiece = EntityRenderPeiceGroup.Peices;
            for (u32 PieceIndex = 0; PieceIndex < EntityRenderPeiceGroup.Count; PieceIndex++, RenderPiece++)
            {
                v3 EntityBasePoint = GetEntityGroundPoint(CurrentEntity);

                f32 ZFudge = 1 + 0.1f * (EntityBasePoint.Z + RenderPiece->Offset.Z);

                v2 ScreenCenterOffset = 0.5f * V2((f32)PixelBuffer->WidthInPixels, (f32)PixelBuffer->HeightInPixels);

                v2 EntityGroundPoint =
                {
                    ScreenCenterOffset.X + EntityBasePoint.X * ZFudge * GameState->PixelsToMetersRatio,
                    ScreenCenterOffset.Y - EntityBasePoint.Y * ZFudge * GameState->PixelsToMetersRatio
                };
                f32 EntityZ = -EntityBasePoint.Z * GameState->PixelsToMetersRatio;

                v2 Center = V2
                (
                    EntityGroundPoint.X + RenderPiece->Offset.X,
                    EntityGroundPoint.Y + RenderPiece->Offset.Y + RenderPiece->EntityJumpZCoefficient * EntityZ
                );

                if (RenderPiece->Bitmap)
                {
                    DrawBitmap(RenderPiece->Bitmap, PixelBuffer, Center.X, Center.Y, RenderPiece->Color.Alpha);
                }
                else
                {
                    v2 DimensionInPixels = RenderPiece->Dimensions * GameState->PixelsToMetersRatio;
                    DrawRectangle
                    (
                        PixelBuffer,
                        Center - 0.5f * DimensionInPixels, Center + 0.5f * DimensionInPixels,
                        RenderPiece->Color.Red, RenderPiece->Color.Green, RenderPiece->Color.Blue, RenderPiece->Color.Alpha
                    );
                }
            }
        }
    }

    entity_world_position OriginWorldPosition = {};
    v3 Diff = SubtractPositions(CameraSimulationRegion->World, &OriginWorldPosition, &CameraSimulationRegion->Origin);
    v2 Origin = V2(Diff.X, Diff.Y);
    DrawRectangle(PixelBuffer, Origin, Origin + V2(10.0f, 10.0f), 1.0f, 1.0f, 0.0f, 1.0f);

    EndSimulation(CameraSimulationRegion, GameState);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)GameMemory->PermanentStorage;

    u32 ToneFrequency = 400;
    i16 ToneVolume = 200;
    u32 SamplesPerTonePeriod = SoundRequest->SamplesPerSecond / ToneFrequency;

    i16 *OutputSamples = SoundRequest->OutputSamples;

    for
    (
        u32 SampleIndex = 0;
        SampleIndex < SoundRequest->OutputSamplesCount;
        SampleIndex++
    )
    {
#if 0
        i16 SampleValue = (i16)(ToneVolume * Sin(GameState->SinParameterInRadians));

        GameState->SinParameterInRadians += 2.0f * PI32 / (f32)SamplesPerTonePeriod;
        if (GameState->SinParameterInRadians > 2.0f * PI32)
        {
            GameState->SinParameterInRadians -= 2.0f * PI32;
        }
#else
        i16 SampleValue = 0;
#endif
        *OutputSamples++ = SampleValue;
        *OutputSamples++ = SampleValue;
    }
}