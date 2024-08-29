#define STB_SPRINTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

#include "third_party/stb_sprintf.h"
#include "third_party/stb_image.h"
#include "third_party/stb_truetype.h"

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

global b8 pickedUpItemThisFrame;
global b8 loadNextLevel;

global u32 uiInvTexture;
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

#include "maps.h"

//~ NOTE(nickel): Function OnStart
extern GAME_ON_START(OnStart)
{
	StartRenderer(&state);
	srand(time(0));
	
	state.timeScale = 1.0f;
	
	state.defaultShader = LoadShader("shaders/default.glsl");
	state.uiShader = LoadShader("shaders/ui.glsl");
	state.fontShader = LoadShader("shaders/font.glsl");
	
	// Loading UI textures
	state.fontTexture = LoadTexture("textures/font.png");
	uiInvTexture = LoadTexture("textures/inv_ring.png");
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
	
	// Allocate entity arena
	state.entitiesSize = 0;
	state.entities = (Entity*)calloc(ENTITIES_MAX_SIZE, sizeof(Entity));
	
	// Items
	state.itemsSize = 0;
	state.items = (Item*)calloc(ITEMS_MAX_SIZE, sizeof(Item));
	
	state.playerItemId = 0;
	state.playerItems[0] = NULL;
	state.playerItems[1] = NULL;
	state.playerItems[2] = NULL;
	state.playerItems[3] = NULL;
	
	frog = NewEntity(&state);
	frog->texture = LoadTexture("textures/frog_small.png");
	frog->health = 5;
	frog->maxHealth = frog->health;
	
	LoadMapFiles();
	LoadMap(maps[0], TILE_MAP_WIDTH, TILE_MAP_HEIGHT);
	
	//state.cameraPos = frog->pos;
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
	
	if (windowWidth != state.windowSize.x || windowHeight != state.windowSize.y)
	{
		state.projection = M4OrthoMatrix(-(newWidth / 2), newWidth / 2, -(newHeight / 2), newHeight / 2, -1.0f, 1.0f);
		state.uiProjection = M4OrthoMatrix(0.0f, newWidth, newHeight, 0.0f, -1.0f, 1.0f);
	}
	state.windowSize = V2(windowWidth, windowHeight);
	
	dt *= state.timeScale;
	pickedUpItemThisFrame = false;
	
	// item switching
	if (input.keys['1']) state.playerItemId = 0;
	if (input.keys['2']) state.playerItemId = 1;
	if (input.keys['3']) state.playerItemId = 2;
	if (input.keys['4']) state.playerItemId = 3;
	
	if (mouseWheel > 0)
		state.playerItemId -= 1;
	else if (mouseWheel < 0)
		state.playerItemId += 1;
	
	if (state.playerItemId >= PLAYER_ITEMS_SIZE)
		state.playerItemId = 0;
	else if (state.playerItemId < 0)
		state.playerItemId = PLAYER_ITEMS_SIZE - 1;
	
	v2 dir = V2(input.keys['D'] - input.keys['A'], input.keys['W'] - input.keys['S']);
	MovePlayer(frog, dir);
	
	WobbleAnimation(frog, dt);
	
	for (u64 i = 0; i < state.enemiesSize; ++i)
	{
		Entity* e = state.enemies[i];
		if (e == NULL)
			break;
		
		if (V2Distance(e->pos, frog->pos) < 10.0f)
		{
			if (e->health <= 0 && e->destroyable)
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
					if (currentTime > frog->lastTimeHurt + Seconds(1.8f) && V2Distance(e->pos, frog->pos) <= e->scale.x - 0.1f)
					{
						u8 oldHealth = frog->health;
						if (e->item == NULL)
							frog->health -= e->strength;
						else
							frog->health -= e->item->damage + e->strength;
						if (frog->health > oldHealth)
							frog->health = 0;
						frog->lastTimeHurt = currentTime;
						frog->wobbleFrame = 40.0f;
						frog->angle = 10.0f * FastSin(40);
						frog->lastStunnedTime = currentTime;
						frog->stunned = true;
						frog->knockback = V2MulScalar(MoveTowards(e->pos, frog->pos), 5.0f);
						state.cameraPos = V2AddV2(state.cameraPos, V2DivScalar(frog->knockback, 50.0f));
					}
				}
			}
			else
			{
				if (CheckOverlap(EntityAABB(frog), EntityAABB(e)))
				{
					loadNextLevel = true;
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
				for (u64 i = 0; i < state.enemiesSize; ++i)
				{
					Entity* e = state.enemies[i];
					
					Item* playerItem = state.playerItems[state.playerItemId];
					//AABB hitbox = { V2SubV2(itemPos, V2(0.0f, 0.5f)), V2AddV2(itemPos, V2(1.5f, 1.0f)) };
					
					if (playerItem->isPotion)
					{
						switch (playerItem->effect)
						{
							case POTION_EFFECT_HEALTH:
							frog->health += 1;
							frog->maxHealth += 1;
							break;
							
							case POTION_EFFECT_STRENGTH:
							frog->strength += 1;
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
					else if (V2Distance(e->pos, itemPos) < 2.0f /*CheckOverlap(EntityAABB(e), hitbox)*/ && e->health > 0)
					{
						e->knockback = V2MulScalar(MoveTowards(frog->pos, e->pos), 3.0f);
						if (!e->stunned)
						{
							e->lastStunnedTime = currentTime;
							e->stunned = true;
							u8 oldHealth = e->health;
							e->health -= playerItem->damage + frog->strength;
							if (e->health > oldHealth)
								e->health = 0;
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
			DrawCharWorld(&state, i->pos, V2Scalar(0.5f), 'E', V4Scalar(1.0f));
			
			if (input.keys['E'] && !oldInput.keys['E'] && !pickedUpItemThisFrame)
			{
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
	
	// NOTE(nickel): Rendering the Ui
	DrawUiTexture(&state, uiInvTexture, V2Scalar(0.0f), V2Scalar(6.0f), 0.0f, V4Scalar(1.0f));
	
	// heart
	DrawUiTexture(&state, heartFillTexture, V2Scalar(0.7f), V2Scalar(1.0f), 0.0f, V4Scalar((f32)frog->health / frog->maxHealth + 0.25f));
	DrawUiTexture(&state, heartOutlineTexture, V2Scalar(0.7f), V2Scalar(1.0f), 0.0f, V4Scalar(1.0f));
	DrawChar(&state, V2(0.9725f, 0.725f), V2Scalar(0.9f), 0x30 + frog->health, V4Scalar(1.0f)); // ASCII '0' == 0x30
	
	for (u8 i = 0; i < PLAYER_ITEMS_SIZE; ++i)
	{
		Item* item = state.playerItems[i];
		
		if (item != NULL)
		{
			f32 tint = 0.5f;
			v2 scale = V2Scalar(0.4f);
			
			if (i == state.playerItemId)
			{
				tint = 1.0f;
				scale = V2Scalar(0.5f);
			}
			
			DrawUiTexture(&state, item->texture, V2((FastSin(90 + i * 24) * 2.5 - 0.25),
													(FastCos(i * 24 - 90) * 2.0 + 0.5)), scale, 0.0f, V4(tint, tint, tint, 1.0f));
			if (item->fill != 0)
				DrawUiTexture(&state, item->fill, V2((FastSin(90 + i * 24) * 2.5 - 0.25),
													 (FastCos(i * 24 - 90) * 2.0 + 0.5)), scale, 0.0f, V4MulV4(item->fillTint, V4(tint, tint, tint, 1.0f)));
		}
	}
	
	// NOTE(nickel): Death message
	if (frog->health == 0)
	{
		state.timeScale = 0.0f;
		DrawUiTexture(&state, uiOverlayTexture, V2Scalar(0.0f), V2Scalar(100.0f), 0.0f, V4Scalar(1.0f));
		DrawString(&state, V2(0.0f, 5.0f), V2Scalar(2.0f), "YOU DIED", V4(1.0f, 0.3f, 0.2f, 1.0f));
	}
	
	if (loadNextLevel)
	{
		UnloadMap();
		u64 i = 0;
		while (maps[i] != tileMap) ++i;
		LoadMap(maps[i + 1], TILE_MAP_WIDTH, TILE_MAP_HEIGHT);
		loadNextLevel = false;
	}
}