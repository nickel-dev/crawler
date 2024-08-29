//**********************************************************
// Date: August 25th 2024 10:16 am
// Creator: Daniel Nickel
// Notice: Copyright (C) Daniel Nickel, All Rights Reserved.
// File: maps.h
//**********************************************************

#ifndef __MAPS_H_
#define __MAPS_H_

#define TILE_MAP_WIDTH 65
#define TILE_MAP_HEIGHT 64

/*
S - Snowman
W - Wolf
M - Mouse
C - Crawler
T - Tortoise
* - Chest
| - Sword
' - potion
*/

global char* maps[11];

void LoadMapFiles()
{
	maps[0x00] = ReadEntireFile("maps/0.map");
	maps[0x01] = ReadEntireFile("maps/1.map");
	maps[0x02] = ReadEntireFile("maps/2.map");
	maps[0x03] = ReadEntireFile("maps/3.map");
	maps[0x04] = ReadEntireFile("maps/4.map");
	maps[0x05] = ReadEntireFile("maps/5.map");
	maps[0x06] = ReadEntireFile("maps/6.map");
	maps[0x07] = ReadEntireFile("maps/7.map");
	maps[0x08] = ReadEntireFile("maps/8.map");
	maps[0x09] = ReadEntireFile("maps/9.map");
	maps[0x0A] = ReadEntireFile("maps/10.map");
}

void LoadMap(char* mTileMap, u64 tileMapWidth, u64 tileMapHeight)
{
	tileMap = mTileMap;
	Entity* e;
	Item* i;
	
	b8 spawnWithWeapon = false;
	
	for (u64 y = 0; y < tileMapHeight; ++y)
	{
		for (u64 x = 0; x < tileMapWidth; ++x)
		{
			char tile = TilePos(mTileMap, x, y);
			if (tile > 96 && tile < 123)
			{
				tile -= 32;
				spawnWithWeapon = true;
			}
			
			switch (tile)
			{
				case '@':
				frog->pos = V2(x, y);
				state.cameraPos = frog->pos;
				break;
				
				case 'W':
				e = NewEnemy(&state);
				e->pos = V2(x, y);
				e->texture = wolfTexture;
				e->health = 3;
				e->strength = 1;
				e->speed = 1.5f;
				
				if (spawnWithWeapon)
				{
					e->item = NewItem(&state);
					e->item->texture = swordTexture;
					e->item->isHeld = true;
				}
				break;
				
				case 'S':
				e = NewEnemy(&state);
				e->pos = V2(x, y);
				e->texture = snowmanTexture;
				e->health = 6;
				e->strength = 1;
				e->speed = 1.9f;
				
				if (spawnWithWeapon)
				{
					// Mace
					e->item = NewItem(&state);
					e->item->texture = maceTexture;
					e->item->damage = 2;
					e->item->isHeld = true;
					e->item->cooldownTime = 0.75f;
				}
				break;
				
				case 'M':
				e = NewEnemy(&state);
				e->pos = V2(x, y);
				e->texture = mouseTexture;
				e->health = 2;
				e->strength = 1;
				e->speed = 2.0f;
				e->scale = V2Scalar(0.5f);
				
				e->item = NewItem(&state);
				e->item->isHeld = true;
				e->item->texture = swordTexture;
				break;
				
				case 'C':
				e = NewEnemy(&state);
				e->pos = V2(x, y);
				e->texture = crawlerTexture;
				e->health = 3;
				e->strength = 2;
				e->speed = 1.5f;
				e->scale = V2Scalar(0.5f);
				
				e->item = NewItem(&state);
				break;
				
				case 'T':
				e = NewEnemy(&state);
				e->pos = V2(x, y);
				e->texture = tortoiseTexture;
				e->health = 6;
				e->strength = 2;
				e->speed = 0.5f;
				e->scale = V2Scalar(1.0f);
				
				e->item = NewItem(&state);
				break;
				
				case '*':
				e = NewEnemy(&state);
				e->pos = V2(x, y);
				e->texture = chestTexture;
				e->useAi = false;
				e->item = NewPotion(&state, potionOutlineTexture, potionFillTexture);
				e->scale = V2Scalar(0.75f);
				e->holdItem = false;
				break;
				
				case '|':
				i = NewItem(&state);
				i->pos = V2(x, y);
				i->texture = swordTexture;
				break;
				
				case '\'':
				i = NewPotion(&state, potionOutlineTexture, potionFillTexture);
				i->pos = V2(x, y);
				break;
				
				case '!':
				trapdoor= NewEnemy(&state);
				trapdoor->pos = V2(x, y);
				trapdoor->texture = trapdoorTexture;
				trapdoor->useAi = false;
				trapdoor->scale = V2Scalar(1.0f);
				trapdoor->destroyable = false;
				trapdoor->health = 0;
				break;
				
				default:
				break;
			}
		}
	}
	
	if (trapdoor == NULL)
		printf("Level exit missing!\n");
}

void UnloadMap()
{
	for (u64 i = 1; i < state.entitiesSize; ++i)
	{
		Entity* e = state.entities + i;
		if (e->item != NULL)
			e->item->isHeld = false;
		e->item = NULL;
	}
	
	Entity frogData = *state.entities;
	// TODO(nickel): FIX ITEMS
	/*// NOTE(nickel): Item playerItemsData[PLAYER_ITEMS_SIZE];
	for (u64 i = 0; i < PLAYER_ITEMS_SIZE; ++i)
	{
		Item* t = state.playerItems[i];
		memcpy(&playerItemsData[i], t, sizeof(Item));
	}
	
	free(state.items);
	state.itemsSize = 0;
	state.items = (Item*)calloc(ITEMS_MAX_SIZE, sizeof(Item));*/
	
	free(state.entities);
	state.entitiesSize = 0;
	state.entities = (Entity*)calloc(ENTITIES_MAX_SIZE, sizeof(Entity));
	
	frog = NewEntity(&state);
	*frog = frogData;
	
	/*for (u64 i = 0; i < PLAYER_ITEMS_SIZE; ++i)
	{
		Item* t = NewItem(&state);
		memcpy(t, &playerItemsData[i], sizeof(Item));
	}*/
}

#endif // __MAPS_H_