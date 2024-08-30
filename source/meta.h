//**********************************************************
// Date: August 20th 2024 2:45 pm
// Creator: Daniel Nickel
// Notice: Copyright (C) Daniel Nickel, All Rights Reserved.
// File: meta.h
//**********************************************************

#ifndef __META_H_
#define __META_H_

#define ENTITIES_MAX_SIZE 256
#define ENEMIES_MAX_SIZE 64
#define ITEMS_MAX_SIZE 256
#define PLAYER_ITEMS_SIZE 4

typedef struct State State;
typedef struct Vertex Vertex;
typedef struct AABB AABB;
typedef struct Item Item;
typedef struct Entity Entity;
typedef struct Tutorial Tutorial;

enum PotionEffect
{
	POTION_EFFECT_HEALTH = 0, POTION_EFFECT_STRENGTH, POTION_EFFECT_SPEED
};

enum GameMode
{
	GAME_MODE_MAIN_MENU = 0, GAME_MODE_GAME
};

struct State
{
	v2 cameraPos;
	v2 windowSize;
	
	f64 timeScale;
	
	// Shaders
	u32 defaultShader;
	m4 projection;
	
	u32 uiShader;
	m4 uiProjection;
	
	u32 fontShader;
	u32 fontTexture;
	
	// Entities
	u64 entitiesSize;
	Entity* entities;
	
	u64 enemiesSize;
	Entity* enemies[ENEMIES_MAX_SIZE];
	
	Entity* player;
	
	// Items
	u64 itemsSize;
	Item* items;
	
	i32 playerItemId;
	Item* playerItems[PLAYER_ITEMS_SIZE];
	
	// Rendering
	u32 errorTexture;
	u32 vao, vbo, ebo;
	
	// Game mode
	u32 gameMode;
};

struct Vertex
{
	v3 pos;
	v2 texCoords;
};

struct AABB
{
	v2 min;
	v2 max;
};

struct Item
{
	v2 pos;
	u32 texture, fill;
	v4 fillTint;
	u8 damage;
	b8 isHeld, isPotion;
	u32 effect;
	f64 lastHit;
	f32 cooldownTime;
	f32 angle;
};

struct Entity
{
	v2 pos, scale, vel;
	f32 angle;
	u32 texture;
	Item* item;
	f32 health, maxHealth;
	u64 lastTimeHurt;
	f64 wobbleFrame;
	f64 lastStunnedTime;
	b8 stunned;
	f32 strength;
	f32 speed;
	b8 useAi;
	b8 holdItem;
	v2 knockback;
	b8 destroyable;
};

struct Tutorial
{
	b8 wasd, attack;
	f64 wasdTime, attackTime;
};

#endif // __META_H_