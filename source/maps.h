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
				e->health = 7;
				e->strength = 1;
				e->speed = 2.0f;
				
				if (spawnWithWeapon)
				{
					// Mace
					e->item = NewItem(&state);
					e->item->texture = maceTexture;
					e->item->damage = 2.0f;
					e->item->isHeld = true;
					e->item->cooldownTime = 1.0f;
				}
				break;
				
				case 'M':
				e = NewEnemy(&state);
				e->pos = V2(x, y);
				e->texture = mouseTexture;
				e->health = 2;
				e->strength = 2;
				e->speed = 2.0f;
				e->scale = V2Scalar(0.5f);
				break;
				
				case 'C':
				e = NewEnemy(&state);
				e->pos = V2(x, y);
				e->texture = crawlerTexture;
				e->health = 3;
				e->strength = 2;
				e->speed = 1.5f;
				e->scale = V2Scalar(0.5f);
				break;
				
				case 'T':
				e = NewEnemy(&state);
				e->pos = V2(x, y);
				e->texture = tortoiseTexture;
				e->health = 6;
				e->strength = 1;
				e->speed = 2.5f;
				e->scale = V2Scalar(1.0f);
				
				if (spawnWithWeapon)
				{
					// Axe
					e->item = NewItem(&state);
					e->item->texture = axeTexture;
					e->item->damage = 2.0f;
					e->item->isHeld = true;
					e->item->cooldownTime = 0.75f;
				}
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
				trapdoor = NewEnemy(&state);
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

void CreateArenas()
{
	state.entitiesSize = 0;
	state.itemsSize = 0;
	
	state.entities = (Entity*)calloc(ENTITIES_MAX_SIZE, sizeof(Entity));
	state.items = (Item*)calloc(ITEMS_MAX_SIZE, sizeof(Item));
	
	state.playerItemId = 0;
	for (u64 i = 0; i < PLAYER_ITEMS_SIZE; ++i)
		state.playerItems[i] = NULL;
	
	state.enemiesSize = 0;
	for (u64 i = 0; i < ENEMIES_MAX_SIZE; ++i)
		state.enemies[i] = NULL;
	
	trapdoor = NULL;
	frog = NULL;
}

void UnloadMap(b8 keepPlayer)
{
	Entity playerData;
	Item itemsData[PLAYER_ITEMS_SIZE];
	i32 itemIdData = 0;
	if (keepPlayer)
	{
		itemIdData = state.playerItemId;
		memcpy(&playerData, frog, sizeof(Entity));
		for (u64 i = 0; i < PLAYER_ITEMS_SIZE; ++i)
		{
			Item* item = state.playerItems[i];
			if (item != NULL)
			{
				memcpy(&itemsData[i], item, sizeof(Item));
				itemsData[i].isHeld = true;
			}
			else
				itemsData[i].isHeld = false;
		}
	}
	
	free(state.entities);
	free(state.items);
	
	CreateArenas();
	
	if (keepPlayer)
	{
		state.playerItemId = itemIdData;
		
		frog = NewEntity(&state);
		memcpy(frog, &playerData, sizeof(Entity));
		for (u64 i = 0; i < PLAYER_ITEMS_SIZE; ++i)
		{
			if (!itemsData[i].isHeld)
			{
				state.playerItems[i] = NULL;
				continue;
			}
			
			Item* item = NewItem(&state);
			memcpy(item, &itemsData[i], sizeof(Item));
			state.playerItems[i] = item;
		}
	}
}

#endif // __MAPS_H_