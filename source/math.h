//**********************************************************
// Date: August 20th 2024 0:20 pm
// Creator: Daniel Nickel
// Notice: Copyright (C) Daniel Nickel, All Rights Reserved.
// File: math.h
//**********************************************************

#ifndef __MATH_H_
#define __MATH_H_

#include <math.h>

#define PI 3.1415926f
#define DegToRad(d) ((f32)d * (PI / 180.0f))
#define RadToDeg(r) (180.0 * (f32)r / PI)

//~ NOTE(nickel): Vectors
typedef union
{
	struct { f32 a, b; };
	struct { f32 x, y; };
	struct { f32 u, v; };
	struct { f32 s, t; };
	
	f32 elements[2];
} v2;

typedef union
{
	struct { f32 x, y, z; };
	struct { f32 r, g, b; };
	struct { v2 xy; f32 _z1; };
	
	f32 elements[3];
} v3;

typedef union
{
	struct { f32 x, y, z, w; };
	struct { f32 r, g, b, a; };
	struct { v2 xy, zw; };
	struct { v3 xyz; f32 _w1; };
	
	f32 elements[4];
} v4;

inline internal v2 V2(f32 x, f32 y) { return (v2){ x, y }; }
inline internal v3 V3(f32 x, f32 y, f32 z) { return (v3){ x, y, z }; }
inline internal v4 V4(f32 x, f32 y, f32 z, f32 w) { return (v4){ x, y, z, w }; }

inline internal v2 V2Scalar(f32 s) { return (v2){ s, s }; }
inline internal v3 V3Scalar(f32 s) { return (v3){ s, s, s }; }
inline internal v4 V4Scalar(f32 s) { return (v4){ s, s, s, s }; }

inline internal v2 V2Normalize(v2 v)
{
	f32 length = sqrt(v.x * v.x + v.y * v.y);
    if (length != 0)
	{
        v.x /= length;
        v.y /= length;
    }
    return v;
}

inline internal v3 V3Normalize(v3 v)
{
	f32 length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (length != 0)
	{
        v.x /= length;
        v.y /= length;
		v.z /= length;
    }
    return v;
}

inline internal v4 V4Normalize(v4 v)
{
	f32 length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    if (length != 0)
	{
        v.x /= length;
        v.y /= length;
		v.z /= length;
		v.w /= length;
    }
    return v;
}

inline internal f32 V2Distance(v2 a, v2 b)
{ return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y)); }

inline internal f32 V3Distance(v3 a, v3 b)
{ return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y) + (b.z - a.z) * (b.z - a.z)); }

inline internal f32 V4Distance(v4 a, v4 b)
{ return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y) + (b.z - a.z) * (b.z - a.z) + (b.w - a.w) * (b.w - a.w)); }

// NOTE(nickel): Generates math functions for vectors
#define __VECTOR_MATH_GEN(m)\
inline internal m(Add, +)\
inline internal m(Sub, -)\
inline internal m(Mul, *)\
inline internal m(Div, /)

#define __VEC2_V_MATH_STUB(n, o) v2 V2##n##V2(v2 a, v2 b) { return V2(a.x o b.x, a.y o b.y); }
#define __VEC3_V_MATH_STUB(n, o) v3 V3##n##V3(v3 a, v3 b) { return V3(a.x o b.x, a.y o b.y, a.z o b.z); }
#define __VEC4_V_MATH_STUB(n, o) v4 V4##n##V4(v4 a, v4 b) { return V4(a.x o b.x, a.y o b.y, a.z o b.z, a.w o b.w); }

#define __VEC2_S_MATH_STUB(n, o) v2 V2##n##Scalar(v2 v, f32 x) { return V2(v.x o x, v.y o x); }
#define __VEC3_S_MATH_STUB(n, o) v3 V3##n##Scalar(v3 v, f32 x) { return V3(v.x o x, v.y o x, v.z o x); }
#define __VEC4_S_MATH_STUB(n, o) v4 V4##n##Scalar(v4 v, f32 x) { return V4(v.x o x, v.y o x, v.z o x, v.w o x); }

__VECTOR_MATH_GEN(__VEC2_V_MATH_STUB)
__VECTOR_MATH_GEN(__VEC3_V_MATH_STUB)
__VECTOR_MATH_GEN(__VEC4_V_MATH_STUB)

__VECTOR_MATH_GEN(__VEC2_S_MATH_STUB)
__VECTOR_MATH_GEN(__VEC3_S_MATH_STUB)
__VECTOR_MATH_GEN(__VEC4_S_MATH_STUB)

//~ NOTE(nickel): Matrices
typedef union
{
	f32 elements[4][4];
    v4 columns[4];
} m4;

inline internal m4 M4OrthoMatrix(f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar)
{
    m4 result = { 0 };
	
    result.elements[0][0] = 2.0f / (right - left);
    result.elements[1][1] = 2.0f / (top - bottom);
    result.elements[2][2] = 2.0f / (zNear - zFar);
    result.elements[3][3] = 1.0f;
	
    result.elements[3][0] = (left + right) / (left - right);
    result.elements[3][1] = (bottom + top) / (bottom - top);
    result.elements[3][2] = (zNear + zFar) / (zNear - zFar);
	
    return result;
}

inline internal m4 M4PerspectiveMatrix(f32 fov, f32 aspectRatio, f32 zNear, f32 zFar)
{
	m4 result = {0};
	
    // See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
	
    f32 cotangent = 1.0f / tanf(fov / 2.0f);
    result.elements[0][0] = cotangent / aspectRatio;
    result.elements[1][1] = cotangent;
    result.elements[2][3] = -1.0f;
	
    result.elements[2][2] = (zNear + zFar) / (zNear - zFar);
    result.elements[3][2] = (2.0f * zNear * zFar) / (zNear - zFar);
	
    return result;
}


//**************************************************
// Sin/Cosine lookup table
//**************************************************
global const f64 SIN_LOOKUP_TABLE[180] =
{
	0.000000, 0.017452, 0.034899, 0.052336, 0.069756, 0.087156, 0.104528, 0.121869, 0.139173, 0.156434, 0.173648, 0.190809, 0.207912, 0.224951, 0.241922, 0.258819, 0.275637, 0.292372, 0.309017, 0.325568, 0.342020, 0.358368, 0.374607, 0.390731, 0.406737, 0.422618, 0.438371, 0.453990, 0.469472, 0.484810, 0.500000, 0.515038, 0.529919, 0.544639, 0.559193, 0.573576, 0.587785, 0.601815, 0.615661, 0.629320, 0.642788, 0.656059, 0.669131, 0.681998, 0.694658, 0.707107, 0.719340, 0.731354, 0.743145, 0.754710, 0.766044, 0.777146, 0.788011, 0.798636, 0.809017, 0.819152, 0.829038, 0.838671, 0.848048, 0.857167, 0.866025, 0.874620, 0.882948, 0.891007, 0.898794, 0.906308, 0.913545, 0.920505, 0.927184, 0.933580, 0.939693, 0.945519, 0.951057, 0.956305, 0.961262, 0.965926, 0.970296, 0.974370, 0.978148, 0.981627, 0.984808, 0.987688, 0.990268, 0.992546, 0.994522, 0.996195, 0.997564, 0.998630, 0.999391, 0.999848, 1.000000, 0.999848, 0.999391, 0.998630, 0.997564, 0.996195, 0.994522, 0.992546, 0.990268, 0.987688, 0.984808, 0.981627, 0.978148, 0.974370, 0.970296, 0.965926, 0.961262, 0.956305, 0.951057, 0.945519, 0.939693, 0.933580, 0.927184, 0.920505, 0.913545, 0.906308, 0.898794, 0.891007, 0.882948, 0.874620, 0.866025, 0.857167, 0.848048, 0.838671, 0.829038, 0.819152, 0.809017, 0.798636, 0.788011, 0.777146, 0.766044, 0.754710, 0.743145, 0.731354, 0.719340, 0.707107, 0.694658, 0.681998, 0.669131, 0.656059, 0.642788, 0.629320, 0.615661, 0.601815, 0.587785, 0.573576, 0.559193, 0.544639, 0.529919, 0.515038, 0.500000, 0.484810, 0.469472, 0.453990, 0.438371, 0.422618, 0.406737, 0.390731, 0.374607, 0.358368, 0.342020, 0.325568, 0.309017, 0.292372, 0.275637, 0.258819, 0.241922, 0.224951, 0.207912, 0.190809, 0.173648, 0.156434, 0.139173, 0.121869, 0.104528, 0.087156, 0.069756, 0.052336, 0.034899, 0.017452
};

inline internal f64 FastSin(i32 d)
{
	// normalize
	d = d % 360;
	
	if (d < 0)
		d += 360;
	
	// sin(-x) = -sin(x)
	if (d >= 180)
	{
		d -= 180;
		return -SIN_LOOKUP_TABLE[d];
	}
	return SIN_LOOKUP_TABLE[d];
}

inline internal f64 FastCos(i32 d)
{
	return FastSin(d + 90);
}

#endif // __MATH_H_