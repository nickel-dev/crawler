//**********************************************************
// Date: August 19th 2024 3:23 pm
// Creator: Daniel Nickel
// Notice: Copyright (C) Daniel Nickel, All Rights Reserved.
// File: platform.h
//**********************************************************

#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t b8;
typedef int16_t b16;
typedef int32_t b32;
typedef int64_t b64;

typedef float f32;
typedef double f64;

#define internal static
#define local_persist static
#define global static

enum InputState
{
	INPUT_STATE_UP = 0, INPUT_STATE_DOWN
};

typedef struct
{
	u8 keys[256];
} InputProfile;

#define GAME_ON_START(n) void n(void)
#define GAME_ON_UPDATE(n) void n(InputProfile input, InputProfile oldInput, f64 dt, i32 windowWidth, i32 windowHeight, f64 currentTime, i64 mouseX, i64 mouseY, b8 mouseDown, i32 mouseWheel)

GAME_ON_START(OnStart);
GAME_ON_UPDATE(OnUpdate);

#endif // __PLATFORM_H_