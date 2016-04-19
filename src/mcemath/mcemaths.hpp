#pragma once
#include "mcemaths.h"
#include <crtdefs.h>
/*****************************************************************************/
/*                                                                           */
/* This is an extended definition header for "ZHOU's Fast 3D Maths Library". */
/* You can see the above inclusion for details.                              */
/*                                                                           */
/* Some simple C++ wrappers and helpers are defined here to ease the work of */
/* those C++ compiler users.                                                 */
/*                                                                           */
/* This is not a substitute to the plane APIs, you have to use mcemaths_xxxx */
/* to do most of the calculation work. The main purpose of these classes is  */
/* to help you allocate memory both on heap and on stack.                    */
/*                                                                           */
/*****************************************************************************/

/* SPECIAL NOTICE: if you wanna hold 16-bytes aligned objects (such as the 'vec4') 
in STL container, the original 'vector' (the VC2010 version) can't be compiled due to 
its bug in resize() method. So I fixed it in fixvec.hpp (the name remains), please 
take this instead. Another point is that you can't use the default allocator due 
to alignment issue. A working allocator 'alloc16' is provided in alloc16.hpp. So this 
time your vector's declaration should be like this:

vector<vec4, alloc16<vec4> >, or vector<mat4, alloc16<mat4> >, etc...

Other containers are similar, such as:

map<KEY_TYPE, vec4, less<KEY_TYPE>, alloc16<KEY_TYPE, vec4> >, ...

Lastly, about the lovely 'deque': just a friendly reminder, many websites may urge 
you to replace vector by deque, but never forget doing '&deque[0]' is not very safe 
when the capacity is large, although deque is not list, it doesn't indeed guarantee a 
continuous linear buffer!*/
// include the following two files in your programme when STL is needed:
// #include "alloc16.hpp"
// #include "fixvec.hpp"

#pragma pack(4) // keep floats as floats

namespace mcemaths
{

class align_base_16
{
public:
	void* operator new(size_t bytes);
	void  operator delete(void* mem);
	void* operator new(size_t bytes, void* place);
	void  operator delete(void* mem, void* place);
	void* operator new[](size_t bytes);
	void  operator delete[](void* mem);
	void* operator new[](size_t bytes, void* place);
	void  operator delete[](void* mem, void* place);
};

class align_base_64
{
public:
	void* operator new(size_t bytes);
	void  operator delete(void* mem);
	void* operator new(size_t bytes, void* place);
	void  operator delete(void* mem, void* place);
	void* operator new[](size_t bytes);
	void  operator delete[](void* mem);
	void* operator new[](size_t bytes, void* place);
	void  operator delete[](void* mem, void* place);
};

class mat4;
class quaternion;
ALIGN16 class vec4 : public align_base_16
{
	// class mat4 has operator float*, which makes the constructor 
	// vec4::vec4(const float* src) accept mat4 but only pointlessly 
	// copy the first column of elements, so we reject it here.
	vec4(const mat4&){};
public:
	vec4(void);
	vec4(float src);
	vec4(float xyz, float w);
	vec4(float x, float y, float z, float w);
	vec4(const vec4& src);
	vec4(const float* src);
	const vec4& operator=  (const vec4& src);
	operator float* (void);
	operator const float* (void) const;
	const vec4& set(float xyz, float w = 0.0f);
	const vec4& set(float x, float y, float z, float w);
	const vec4& norm(void);
	const vec4& mul(float scalar);
	const vec4& mul(const vec4& scalars);
	const vec4& div(float scalar);
	const vec4& div(const vec4& scalars);
	const vec4& from_quaternion(const quaternion& q, float len = 1.0f);
	float len(void) const;
	float x;
	float y;
	float z;
	float w;
};

class quaternion;
ALIGN16 class mat4 : public align_base_16
{
	// for similar reason
	mat4(const vec4&){}
	mat4(const quaternion&){}
public:
	mat4(void);
	mat4(const mat4& src);
	mat4(const float* src);
	mat4(const float* tangent, const float* binormal, const float* normal);
	mat4(const float* col1, const float* col2, const float* col3, const float* col4);
	const mat4& operator= (const mat4& src);
	const mat4& operator= (const quaternion& q); // similar to mat4::rotation()
	operator float* (void);
	operator const float* (void) const;
	const mat4& zero(void);
	const mat4& indentity(void);
	const mat4& transpose(void);
	const mat4& inverse(void);
	const mat4& translation(float x, float y, float z);
	const mat4& translation(const vec4& xyz);
	const mat4& translation(const float* xyz3);
	const mat4& scaling(float x, float y, float z);
	const mat4& scaling(const vec4 & xyz);
	const mat4& scaling(const float* xyz3);
	const mat4& rotation(const vec4& axis, float radian);
	const mat4& perspective(float znear, float zfar, float aspect, float fov_rad);
	const mat4& orthographic(float znear, float zfar, float left, float right, float bottom, float top);
	const mat4& view(const vec4& from, const vec4& look_at, const vec4& up);
	const mat4& view(const vec4& from, const vec4& ypr); // ypr = yaw/pitch/roll in radian
	float* translation(void);
	const float* translation(void) const;

	float elem[16];
};

ALIGN16 class quaternion : public align_base_16
{
	// for similar reason
	quaternion(const mat4&){}
public:
	quaternion(void);
	quaternion(const quaternion& src);
	const quaternion& operator= (const quaternion& src);
	const quaternion& operator= (const mat4& rot);
	operator float* (void);
	operator const float* (void) const;
	const quaternion& conjugate(void);
	const quaternion& from_axis_angle(const vec4& axis, float angle);
	const quaternion& from_eular(const vec4& ypr);
	float x;
	float y;
	float z;
	float w;
};

class vec2
{
public:
	float x, y;
};

} // namespace mcemaths

#pragma pack()