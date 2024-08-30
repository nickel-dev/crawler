#define STB_SPRINTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define MINIAUDIO_IMPLEMENTATION

#include "third_party/stb_sprintf.h"
#include "third_party/stb_image.h"
#include "third_party/stb_truetype.h"
#include "third_party/miniaudio.h"

#include "platform.h"
#include "math.h"
#include "meta.h"
#include "io.c"
#include "opengl.c"
#include "collision.c"
#include "entity.c"

#define Seconds(x) ((f32)x / state.timeScale)
#define TilePos(t, x, y) t[(TILE_MAP_HEIGHT - y - 1) * TILE_MAP_WIDTH + x]

global State state;
global Tutorial tutorial;

global b8 pickedUpItemThisFrame;
global b8 loadNextLevel;
global b8 inTransition;

global u32 uiTransitionTexture;
global u32 uiMenuBackTexture;
global u32 uiInvTexture;
global u32 uiWhiteSpot;
global u32 uiOverlayTexture;

global u32 potionOutlineTexture;
global u32 potionFillTexture;
global u32 heartOutlineTexture;
global u32 heartFillTexture;
global u32 swordTexture;
global u32 maceTexture;

global u32 chestTexture;
global u32 dustTexture;
global u32 toombstoneTexture;
global u32 trapdoorTexture;

global u32 wolfTexture;
global u32 snowmanTexture;
global u32 mouseTexture;
global u32 crawlerTexture;
global u32 tortoiseTexture;

global u32 tileMapTextures[256];

global Entity* frog;
global Entity* trapdoor;

global char* tileMap;

global ma_engine soundEngine;
global ma_sound* hitSound;
global ma_sound* hitPlayerSound;
global ma_sound* potionSound;
global ma_sound* menuSound;
global ma_sound* selectSound;
global ma_sound* pickupSound;
global ma_sound* swingSound;
global ma_sound* footstepSound;

#include "maps.h"

ma_sound* LoadSound(const char* path)
{
	printf("-- Loading sound. path: %s --\n", path);
	ma_sound *sound = (ma_sound*)malloc(sizeof(ma_sound));
	ma_result result = ma_sound_init_from_file(&soundEngine, path, 0, NULL, NULL, sound);
	if (result != MA_SUCCESS)
		printf("loading sound failed! path: %s\n", path);
	return sound;
}

void LoadAssets()
{
	// Sound
	ma_result result = ma_engine_init(NULL, &soundEngine);
	if (result != MA_SUCCESS)
		printf("failed to init sound engine!\n");
	
	// Rendering
	StartRenderer(&state);
	srand(time(0));
	
	state.timeScale = 1.0f;
	
	state.defaultShader = LoadShader("shaders/default.glsl");
	state.uiShader = LoadShader("shaders/ui.glsl");
	state.fontShader = LoadShader("shaders/font.glsl");
	
	// Loading UI textures
	state.fontTexture = LoadTexture("textures/font.png");
	uiTransitionTexture = CreateTextureFromRGBA(V4(0, 0, 0, 255));
	uiMenuBackTexture = LoadTexture("textures/main_menu.png");
	uiInvTexture = LoadTexture("textures/inv_ring.png");
	uiWhiteSpot = LoadTexture("textures/white_spot.png");
	uiOverlayTexture = CreateTextureFromRGBA(V4(0, 0, 0, 155));
	
	potionOutlineTexture = LoadTexture("textures/potion_outline.png");
	potionFillTexture = LoadTexture("textures/potion_fill.png");
	heartOutlineTexture = LoadTexture("textures/heart_outline.png");
	heartFillTexture = LoadTexture("textures/heart_fill.png");
	swordTexture = LoadTexture("textures/sword.png");
	maceTexture = LoadTexture("textures/mace.png");
	
	// Loading Entity textures
	chestTexture = LoadTexture("textures/chest.png");
	dustTexture = LoadTexture("textures/dust.png");
	toombstoneTexture = LoadTexture("textures/toombstone.png");
	trapdoorTexture = LoadTexture("textures/trapdoor.png");
	
	wolfTexture = LoadTexture("textures/wolf_small.png");
	snowmanTexture = LoadTexture("textures/snowman.png");
	mouseTexture = LoadTexture("textures/mouse.png");
	crawlerTexture = LoadTexture("textures/crawler.png");
	tortoiseTexture = LoadTexture("textures/tortoise.png");
	
	// Loading tile map textures
	tileMapTextures[0] = CreateTextureFromRGBA(V4(50.0f, 27.0f, 39.0f, 255.0f));
	tileMapTextures[1] = LoadTexture("textures/brick_small.png");
	tileMapTextures[2] = LoadTexture("textures/stone_ground.png");
	
	// Loading sounds
	hitSound = LoadSound("sounds/hit.wav");
	hitPlayerSound = LoadSound("sounds/hit_player.wav");
	potionSound = LoadSound("sounds/potion.wav");
	menuSound = LoadSound("sounds/menu.wav");
	selectSound = LoadSound("sounds/select.wav");
	pickupSound = LoadSound("sounds/pickup.wav");
	swingSound = LoadSound("sounds/swing.wav");
	footstepSound = LoadSound("sounds/footstep.wav");
	
	ma_sound_set_volume(selectSound, 0.5f);
	ma_sound_set_volume(swingSound, 0.5f);
	
	CreateArenas();
}

//~ NOTE(nickel): Function OnStart
extern GAME_ON_START(OnStart)
{
	local_persist b8 assetsLoaded = false;
	if (!assetsLoaded)
	{
		LoadAssets();
		assetsLoaded = true;
	}
	
	if (frog == NULL)
	{
		frog = NewEntity(&state);
		frog->texture = LoadTexture("textures/frog_small.png");
		frog->health = 5;
		frog->maxHealth = frog->health;
	}
	
	LoadMapFiles();
	LoadMap(maps[0], TILE_MAP_WIDTH, TILE_MAP_HEIGHT);
}

b8 TileCollsion(i32 x, i32 y, AABB aabb)
{
	if (TilePos(tileMap, x, y) != '#')
		return false;
	
	AABB h;
	h.min = V2(x, y);
	h.max = V2(x + 1, y + 1);
	
	return CheckOverlap(aabb, h);
}

b8 EntityPhysics(Entity* e)
{
	i32 x = (i32)e->pos.x;
	i32 y = (i32)e->pos.y;
	AABB h = EntityAABB(e);
	
	if      (TileCollsion(x + 0, y + 0, h)) return true;
	else if (TileCollsion(x + 1, y + 0, h)) return true;
	else if (TileCollsion(x + 1, y + 1, h)) return true;
	else if (TileCollsion(x + 0, y + 1, h)) return true;
	else if (TileCollsion(x + 1, y - 1, h)) return true;
	else if (TileCollsion(x - 1, y - 0, h)) return true;
	else if (TileCollsion(x - 1, y - 1, h)) return true;
	else if (TileCollsion(x - 0, y - 1, h)) return true;
	else if (TileCollsion(x - 1, y + 1, h)) return true;
	
	return false;
}

//~ NOTE(nickel): Function OnUpdate
extern GAME_ON_UPDATE(OnUpdate)
{
	f32 newWidth = (f32)windowWidth / 100;
	f32 newHeight = (f32)windowHeight / 100;
	state.projection = M4OrthoMatrix(-(newWidth / 2), newWidth / 2, -(newHeight / 2), newHeight / 2, -1.0f, 1.0f);
	state.uiProjection = M4OrthoMatrix(0.0f, newWidth, newHeight, 0.0f, -1.0f, 1.0f);
	state.windowSize = V2(windowWidth, windowHeight);
	
	f64 realDeltaTime = dt;
	dt *= state.timeScale;
	pickedUpItemThisFrame = false;
	
	////////////////////
	// Main Menu Mode //
	////////////////////
	
	if (state.gameMode == GAME_MODE_MAIN_MENU)
	{
		DrawUiTexture(&state, uiMenuBackTexture, V2(newWidth / 2, newHeight / 2), V2(19, 10.8), 0.0f, V4Scalar(1.0f));
		DrawString(&state, V2(newWidth / 2 + 3.0f, newHeight / 2 - 3.0f), V2Scalar(2.0f), "CRAWLER", V4Scalar(1.0f));
		
		DrawString(&state, V2(newWidth / 2 + 1.5f, newHeight / 2 + 4.0f), V2Scalar(0.5f), "PRESS SPACE TO START", V4Scalar(1.0f));
		
		if (input.keys[' '] && !oldInput.keys[' '])
		{
			ma_sound_start(menuSound);
			inTransition = true;
		}
		
		local_persist f64 transitionTimer = 0.0f;
		local_persist f32 transitionOpacity = 0.0f;
		if (inTransition)
		{
			DrawUiTexture(&state, uiTransitionTexture, V2Scalar(0.0f), V2(windowWidth / 2, windowHeight / 2), 0.0f, V4Scalar(transitionOpacity));
			transitionOpacity += 1.5f * realDeltaTime;
			state.timeScale = 0.0f;
			
			if (transitionTimer + 1.0f < currentTime)
			{
				state.gameMode = GAME_MODE_GAME;
				transitionTimer = 0.0f;
				inTransition = false;
				state.timeScale = 1.0f;
				transitionOpacity = 0.0f;
			}
		}
		else
			transitionTimer = currentTime;
		
		return;
	}
	
	///////////////////
	// Gameplay mode //
	///////////////////
	
	// item switching
	if (input.keys['1']) { state.playerItemId = 0; ma_sound_stop(selectSound); ma_sound_start(selectSound); }
	if (input.keys['2']) { state.playerItemId = 1; ma_sound_stop(selectSound); ma_sound_start(selectSound); }
	if (input.keys['3']) { state.playerItemId = 2; ma_sound_stop(selectSound); ma_sound_start(selectSound); }
	if (input.keys['4']) { state.playerItemId = 3; ma_sound_stop(selectSound); ma_sound_start(selectSound); }
	
	if (mouseWheel > 0)
	{
		state.playerItemId -= 1;
		ma_sound_stop(selectSound); 
		ma_sound_start(selectSound);
	}
	else if (mouseWheel < 0)
	{
		state.playerItemId += 1;
		ma_sound_stop(selectSound); 
		ma_sound_start(selectSound);
	}
	
	if (state.playerItemId >= PLAYER_ITEMS_SIZE)
		state.playerItemId = 0;
	else if (state.playerItemId < 0)
		state.playerItemId = PLAYER_ITEMS_SIZE - 1;
	
	v2 dir = V2(input.keys['D'] - input.keys['A'], input.keys['W'] - input.keys['S']);
	MovePlayer(frog, dir);
	
	//if (dir.x != 0.0f || dir.y != 0.0f)
	//ma_sound_start(footstepSound);
	
	WobbleAnimation(frog, dt);
	
	for (u64 i = 0; i < state.enemiesSize; ++i)
	{
		Entity* e = state.enemies[i];
		if (e == NULL)
			break;
		
		if (V2Distance(e->pos, frog->pos) < 10.0f)
		{
			if (e->health < 0.5f && e->destroyable)
			{
				if (e->useAi)
					e->texture = toombstoneTexture;
				else
					e->texture = dustTexture;
				
				e->angle = 0;
				if (e->item != NULL)
				{
					e->item->isHeld = false;
					e->item->pos = e->pos;
					e->item = NULL;
				}
				continue;
			}
			
			if (e->useAi)
			{
				if (V2Distance(e->pos, frog->pos) > e->scale.x - 0.2f)
					e->vel = V2MulScalar(MoveTowards(e->pos, frog->pos), e->speed);
				WobbleAnimation(e, dt);
			}
			
			if (e != trapdoor)
			{
				if (CheckOverlap(EntityAABB(frog), EntityAABB(e)) && e->useAi)
				{
					if (currentTime > frog->lastTimeHurt + Seconds(1.5f) && V2Distance(e->pos, frog->pos) <= e->scale.x - 0.1f)
					{
						u8 oldHealth = frog->health;
						i32 damage = e->strength;
						if (e->item != NULL)
							damage += e->strength;
						ma_sound_start(hitPlayerSound);
						frog->health -= damage;
						if (frog->health > oldHealth)
							frog->health = 0.0f;
						frog->lastTimeHurt = currentTime;
						frog->wobbleFrame = 40.0f;
						frog->angle = 10.0f * FastSin(40);
						frog->lastStunnedTime = currentTime;
						frog->stunned = true;
						frog->knockback = V2MulScalar(MoveTowards(e->pos, frog->pos), 5.0f);
						// Camera shake
						state.cameraPos = V2AddV2(state.cameraPos,
												  V2DivScalar(frog->knockback, 5.0f / damage * 10.0f));
					}
				}
			}
		}
	}
	
	state.cameraPos = SmoothFollow(state.cameraPos, frog->pos, 0.0f, 5.0f * dt);
	
	// NOTE(nickel): Rendering the map
	DrawTexture(&state, tileMapTextures[0], V2Scalar(00.0f), V2Scalar(5000.0f), 0.0f);
	for (u64 y = 0; y < TILE_MAP_HEIGHT; ++y)
	{
		for (u64 x = 0; x < TILE_MAP_WIDTH; ++x)
		{
			u8 tile = TilePos(tileMap, x, y);
			u32 texture = 0;
			v4 tint = V4Scalar(1.0f);
			
			switch (tile)
			{
				case '#':
				if
				(TilePos(tileMap, x + 2, y) == '#' &&
				 TilePos(tileMap, x - 2, y) == '#' &&
				 TilePos(tileMap, x, y + 2) == '#' &&
				 TilePos(tileMap, x, y - 2) == '#' &&
				 
				 TilePos(tileMap, x - 2, y - 2) == '#' &&
				 TilePos(tileMap, x + 2, y + 2) == '#' &&
				 TilePos(tileMap, x - 2, y + 2) == '#' &&
				 TilePos(tileMap, x + 2, y - 2) == '#')
					tint.xyz = V3Scalar(0.0f);
				
				else if
				(TilePos(tileMap, x + 1, y) == '#' &&
				 TilePos(tileMap, x - 1, y) == '#' &&
				 TilePos(tileMap, x, y + 1) == '#' &&
				 TilePos(tileMap, x, y - 1) == '#' &&
				 
				 TilePos(tileMap, x - 1, y - 1) == '#' &&
				 TilePos(tileMap, x + 1, y + 1) == '#' &&
				 TilePos(tileMap, x - 1, y + 1) == '#' &&
				 TilePos(tileMap, x + 1, y - 1) == '#')
					tint.xyz = V3Scalar(0.5f);
				
				texture = 1;
				break;
				
				case '.':
				texture = 2; // Stone
				break;
				
				default:
				continue;
			}
			
			DrawTextureTinted(&state, tileMapTextures[texture], V2(x, y), V2Scalar(1.0f), 0.0f, tint);
		}
	}
	
	// NOTE(nickel): Rendering the entities
	// NOTE(nickel): Rendering back to front so the player is always in front
	for (i64 i = state.entitiesSize; i >= 0; --i)
	{
		Entity* e = state.entities + i;
		
		{
			if (e->stunned)
				e->vel = V2Scalar(0.0f);
			else
				e->knockback = V2Scalar(0.0f);
			
			e->vel = V2AddV2(e->vel, e->knockback);
			
			e->pos.x += e->vel.x * dt;
			if (EntityPhysics(e)) e->pos.x -= e->vel.x * dt;
			
			e->pos.y += e->vel.y * dt;
			if (EntityPhysics(e)) e->pos.y -= e->vel.y * dt;
		}
		
		if (e->lastStunnedTime + Seconds(0.1f) < currentTime)
			e->stunned = false;
		
		// Shadow
		DrawTextureTinted(&state, uiInvTexture, V2SubV2(e->pos, V2(0.0f, 0.075f)), V2(0.5f, 0.125f), 0.0f, V4(1.0f, 1.0f, 1.0f, 0.25f));
		
		// Entity texture
		DrawTexture(&state, e->texture, e->pos, e->scale, e->angle);
		
		/*
		// NOTE(nickel): Debug
		AABB hitbox = EntityAABB(e);
		DrawAABB(&state, hitbox);
		*/
		
		if (e == frog)
		{
			Item* playerItem = state.playerItems[state.playerItemId];
			if (playerItem == NULL)
				continue;
			
			v2 itemPos = PositionTowards(0.3f, V2AddV2(e->pos, V2(0.0f, 0.5f)), V2(mouseX, mouseY));
			f32 a = RotateTowards(e->pos, V2(mouseX, mouseY));
			
			if (mouseDown && (playerItem->lastHit + playerItem->cooldownTime - 0.05f < currentTime))
			{
				ma_sound_start(swingSound);
				for (u64 i = 0; i < state.enemiesSize; ++i)
				{
					Entity* e = state.enemies[i];
					
					Item* playerItem = state.playerItems[state.playerItemId];
					//AABB hitbox = { V2SubV2(itemPos, V2(0.0f, 0.5f)), V2AddV2(itemPos, V2(1.5f, 1.0f)) };
					
					if (playerItem->isPotion)
					{
						ma_sound_start(potionSound);
						
						switch (playerItem->effect)
						{
							case POTION_EFFECT_HEALTH:
							frog->health += 1;
							frog->maxHealth += 1;
							break;
							
							case POTION_EFFECT_STRENGTH:
							frog->strength += 0.5f;
							break;
							
							case POTION_EFFECT_SPEED:
							frog->speed += 0.1f;
							break;
							
							default:
							printf("unknown potion effect: %u\n", playerItem->effect);
							break;
						}
						
						playerItem->fillTint.w = 0.0f; // make fill invisible
						playerItem->isPotion = false;
					}
					else if (V2Distance(e->pos, V2SubV2(itemPos, V2(0.0f, 0.75))) < 1.75f /*CheckOverlap(EntityAABB(e), hitbox)*/ && e->health > 0)
					{
						e->knockback = V2MulScalar(MoveTowards(frog->pos, e->pos), 3.0f);
						if (!e->stunned)
						{
							e->lastStunnedTime = currentTime;
							e->stunned = true;
							u8 oldHealth = e->health;
							i32 damage = playerItem->damage + frog->strength;
							ma_sound_stop(hitSound);
							ma_sound_start(hitSound);
							e->health -= damage;
							if (e->health > oldHealth)
								e->health = 0.0f;
						}
					}
					playerItem->lastHit = currentTime;
				}
			}
			else if (playerItem->lastHit + playerItem->cooldownTime > currentTime)
				playerItem->angle -= (180.0f * dt) / (10 * playerItem->cooldownTime) * 20;
			else
				playerItem->angle = 0.0f;
			
			playerItem->pos = V2(itemPos.x, frog->pos.y + 0.2f);
			DrawTexture(&state, playerItem->texture, playerItem->pos, V2Scalar(0.5f), playerItem->angle + a);
			
			if (playerItem->fill != 0)
				DrawTextureTinted(&state, playerItem->fill, playerItem->pos, V2Scalar(0.5f), playerItem->angle + a, playerItem->fillTint);
		}
		else if (e->holdItem && e->item != NULL)
		{
			e->item->pos = V2AddV2(e->pos, V2(0.35f, 0.15f));
			DrawTexture(&state, e->item->texture, e->item->pos, V2Scalar(0.5f), e->item->angle - 30.0f);
		}
		
		e->vel = V2Scalar(0.0f);
	}
	
	// NOTE(nickel): Rendering the items
	for (u64 n = 0; n < state.itemsSize; ++n)
	{
		Item* i = state.items + n;
		if (i->isHeld)
			continue;
		
		DrawTexture(&state, i->texture, i->pos, V2Scalar(0.5f), 0.0f);
		if (i->fill != 0)
			DrawTextureTinted(&state, i->fill, i->pos, V2Scalar(0.5f), 0.0f, i->fillTint);
		
		// Item pickup
		if (V2Distance(i->pos, frog->pos) < 1.5f)
		{
			DrawCharWorld(&state, V2AddV2(i->pos, V2(0.125f, -0.25f)), V2Scalar(0.5f), 'E', V4Scalar(1.0f));
			
			if (i->isPotion)
			{
				switch (i->effect)
				{
					case POTION_EFFECT_HEALTH:
					DrawString(&state, V2(newWidth / 2 + 0.75f, newHeight / 2 + 4.0f), V2Scalar(0.35f), "HEALTH POTION", V4Scalar(1.0f));
					break;
					
					case POTION_EFFECT_STRENGTH:
					DrawString(&state, V2(newWidth / 2 + 0.75f, newHeight / 2 + 4.0f), V2Scalar(0.35f), "STRENGTH POTION", V4Scalar(1.0f));
					break;
					
					case POTION_EFFECT_SPEED:
					DrawString(&state, V2(newWidth / 2 + 0.75f, newHeight / 2 + 4.0f), V2Scalar(0.35f), "SPEED POTION", V4Scalar(1.0f));
					break;
				}
			}
			
			if (input.keys['E'] && !oldInput.keys['E'] && !pickedUpItemThisFrame)
			{
				ma_sound_start(pickupSound);
				
				u8 c;
				b8 hasEmpty = false;
				for (c = 0; c < PLAYER_ITEMS_SIZE; ++c)
				{
					if (state.playerItems[c] == NULL)
					{
						hasEmpty = true;
						break;
					}
				}
				
				if (!hasEmpty)
				{
					c = state.playerItemId;
					
					Item* p = state.playerItems[c];
					p->pos = i->pos;
					p->isHeld = false;
				}
				
				state.playerItems[c] = i;
				i->isHeld = true;
				pickedUpItemThisFrame = true;
			}
		}
	}
	
	// Item throw
	if (input.keys['Q'] && !oldInput.keys['Q'] && !pickedUpItemThisFrame)
	{
		Item* i = state.playerItems[state.playerItemId];
		if (state.playerItems[state.playerItemId] != NULL)
		{
			state.playerItems[state.playerItemId] = NULL;
			i->pos = frog->pos;
			i->isHeld = false;
		}
	}
	
	// NOTE(nickel): Rendering the Ui
	DrawUiTexture(&state, uiInvTexture, V2Scalar(0.0f), V2Scalar(6.0f), 0.0f, V4Scalar(1.0f));
	
	// heart
	DrawUiTexture(&state, heartFillTexture, V2Scalar(0.7f), V2Scalar(1.0f), 0.0f, V4Scalar((f32)frog->health / frog->maxHealth + 0.25f));
	DrawUiTexture(&state, heartOutlineTexture, V2Scalar(0.7f), V2Scalar(1.0f), 0.0f, V4Scalar(1.0f));
	DrawChar(&state, V2(0.9725f, 0.725f), V2Scalar(0.9f), 0x30 + frog->health, V4Scalar(1.0f)); // ASCII '0' == 0x30
	
	for (u8 i = 0; i < PLAYER_ITEMS_SIZE; ++i)
	{
		Item* item = state.playerItems[i];
		
		f32 tint = 0.5f;
		v2 scale = V2Scalar(0.4f);
		
		if (i == state.playerItemId)
		{
			tint = 1.0f;
			scale = V2Scalar(0.5f);
		}
		
		if (item != NULL)
		{
			DrawUiTexture(&state, item->texture, V2((FastSin(90 + i * 24) * 2.5 - 0.25),
													(FastCos(i * 24 - 90) * 2.0 + 0.5)), scale, 0.0f, V4(tint, tint, tint, 1.0f));
			
			if (item->fill != 0)
				DrawUiTexture(&state, item->fill, V2((FastSin(90 + i * 24) * 2.5 - 0.25),
													 (FastCos(i * 24 - 90) * 2.0 + 0.5)), scale, 0.0f, V4MulV4(item->fillTint, V4(tint, tint, tint, 1.0f)));
		}
		else
		{
			DrawUiTexture(&state, uiWhiteSpot, V2((FastSin(90 + i * 24) * 2.5 - 0.25),
												  (FastCos(i * 24 - 90) * 2.0 + 0.5)), scale, 0.0f, V4(tint, tint, tint, 1.0f));
		}
	}
	
	// NOTE(nickel): Tutorial
	if (!tutorial.wasd)
	{
		DrawString(&state, V2(newWidth / 2 + 1, newHeight / 2 + 4.0f), V2Scalar(0.5f), "WASD TO MOVE", V4Scalar(1.0f));
		if (tutorial.wasdTime == 0.0f)
			tutorial.wasdTime = currentTime;
		else if (tutorial.wasdTime + 2.0f < currentTime)
			tutorial.wasd = true;
	}
	else if (!tutorial.attack)
	{
		DrawString(&state, V2(newWidth / 2 + 1, newHeight / 2 + 4.0f), V2Scalar(0.5f), "LEFT CLICK TO ATTACK", V4Scalar(1.0f));
		if (tutorial.attackTime == 0.0f)
			tutorial.attackTime = currentTime;
		else if (tutorial.attackTime + 2.0f < currentTime)
			tutorial.attack = true;
	}
	
	// NOTE(nickel): Trapdoor
	{
		b8 allEnemiesDead = true;
		for (u64 i = 0; i < state.enemiesSize; ++i)
		{
			if (state.enemies[i]->health >= 0.5f)
				allEnemiesDead = false;
		}
		
		if (CheckOverlap(EntityAABB(frog), EntityAABB(trapdoor)))
		{
			if (allEnemiesDead)
				loadNextLevel = true;
			else
				DrawString(&state, V2(newWidth / 2 + 1.5, newHeight / 2 + 4.0f), V2Scalar(0.35f), "YOU NEED TO KILL ALL ENEMIES FIRST", V4Scalar(1.0f));
		}
	}
	
	// NOTE(nickel): Death message
	if (frog->health <= 0.5f)
	{
		state.timeScale = 0.0f;
		DrawUiTexture(&state, uiOverlayTexture, V2Scalar(0.0f), V2Scalar(100.0f), 0.0f, V4Scalar(1.0f));
		DrawString(&state, V2(newWidth / 2 + 3, newHeight / 2), V2Scalar(2.0f), "YOU DIED",
				   V4(1.0f, 0.3f, 0.2f, 1.0f));
		DrawString(&state, V2(newWidth / 2 + 1, newHeight / 2 + 4.0f), V2Scalar(0.5f), "PRESS SPACE", V4Scalar(1.0f));
		
		if (input.keys[' '] && !oldInput.keys[' '])
		{
			ma_sound_start(menuSound);
			UnloadMap(false);
			state.gameMode = GAME_MODE_MAIN_MENU;
			OnStart();
			return;
		}
	}
	
	if (loadNextLevel)
	{
		UnloadMap(true);
		u64 i = 0;
		while (maps[i] != tileMap) ++i;
		LoadMap(maps[i + 1], TILE_MAP_WIDTH, TILE_MAP_HEIGHT);
		loadNextLevel = false;
		return;
	}
}