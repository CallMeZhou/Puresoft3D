#include "stdafx.h"
#include "mcemaths.hpp"

namespace mcemaths
{

void* align_base_16::operator new(size_t bytes)
{
	return _aligned_malloc(bytes, 16);
}

void align_base_16::operator delete(void* mem)
{
	_aligned_free(mem);
}

void* align_base_16::operator new(size_t bytes, void* place)
{
	assert(0 == (unsigned __int64)place % 16);
	return place;
}

void align_base_16::operator delete(void* mem, void* place)
{
}

void* align_base_16::operator new[](size_t bytes)
{
	return _aligned_malloc(bytes, 16);
}

void align_base_16::operator delete[](void* mem)
{
	_aligned_free(mem);
}

void* align_base_16::operator new[](size_t bytes, void* place)
{
	return place;
}

void align_base_16::operator delete[](void* mem, void* place)
{
}

void* align_base_64::operator new(size_t bytes)
{
	return _aligned_malloc(bytes, 64);
}

void align_base_64::operator delete(void* mem)
{
	_aligned_free(mem);
}

void* align_base_64::operator new(size_t bytes, void* place)
{
	assert(0 == (unsigned __int64)place % 64);
	return place;
}

void align_base_64::operator delete(void* mem, void* place)
{
}

void* align_base_64::operator new[](size_t bytes)
{
	return _aligned_malloc(bytes, 64);
}

void align_base_64::operator delete[](void* mem)
{
	_aligned_free(mem);
}

void* align_base_64::operator new[](size_t bytes, void* place)
{
	return place;
}

void align_base_64::operator delete[](void* mem, void* place)
{
}

vec4::vec4(void)
{
// setting w to 0 instead of 1 by default makes it incorrect in maths since its name is vec4, 
// but it really benefits us because on most cases, we use Cartesian coordinate and treat vec4 
// as vec3 and sometimes get wrong result due to forgetting to set w to 0.
//	set(0, 1.0f);
	set(0, 0);
}

vec4::vec4(float src)
{
	set(src, src);
}

vec4::vec4(float xyz, float w)
{
	x = xyz;
	y = xyz;
	z = xyz;
	this->w = w;
}

vec4::vec4(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

vec4::vec4(const vec4& src)
{
	*this = src;
}

vec4::vec4(const float* src)
{
	x = src[0];
	y = src[1];
	z = src[2];
	w = src[3];
}

const vec4& vec4::operator=  (const vec4& src)
{
	mcemaths_quatcpy(&x, src);
	return *this;
}

const vec4& vec4::mul(float scalar)
{
	mcemaths_mul_3_4(&x, scalar);
	return *this;
}

const vec4& vec4::mul(const vec4& scalars)
{
	mcemaths_mulvec_3_4(&x, scalars);
	return *this;
}

const vec4& vec4::div(float scalar)
{
	mcemaths_div_3_4(&x, scalar);
	return *this;
}

const vec4& vec4::div(const vec4& scalars)
{
	mcemaths_divvec_3_4(&x, scalars);
	return *this;
}

vec4::operator float* (void)
{
	return &x;
}

vec4::operator const float* (void) const
{
	return &x;
}

const vec4& vec4::set(float xyz, float w /* = 0.0f */)
{
	x = y = z = xyz;
	this->w = w;
	return *this;
}

const vec4& vec4::set(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
	return *this;
}

const vec4& vec4::norm(void)
{
	mcemaths_norm_3_4(*this);
	return *this;
}

const vec4& vec4::from_quaternion(const quaternion& q, float len /* = 1.0f */)
{
	mcemaths_dir_from_quat(&x, &q.x, len);
	return *this;
}

float vec4::len(void) const
{
	return mcemaths_len_3_4(*this);
}

mat4::mat4(void)
{
	indentity();
}

mat4::mat4(const mat4& src)
{
	*this = src;
}

mat4::mat4(const float* src)
{
	__asm{
		mov		eax,	this
		lea		edi,	[eax].elem
		mov		esi,	src
		mov		ecx,	16
		cld
		rep		movsd
	}
}

mat4::mat4(const float* tangent, const float* binormal, const float* normal)
{
	mcemaths_make_tbn(elem, tangent, binormal, normal);
}

mat4::mat4(const float* col1, const float* col2, const float* col3, const float* col4)
{
	__asm{
		mov		eax,	col1
		movaps	xmm0,	[eax]
		mov		eax,	col2
		movaps	xmm1,	[eax]
		mov		eax,	col3
		movaps	xmm2,	[eax]
		mov		eax,	col4
		movaps	xmm3,	[eax]

		movaps	xmm4,	xmm0
		shufps	xmm0,	xmm1,	0x44	; xmm0: tmp0
		shufps	xmm4,	xmm1,	0xee	; xmm4: tmp2
		movaps	xmm5,	xmm2
		shufps	xmm2,	xmm3,	0x44	; xmm2: tmp1
		shufps	xmm5,	xmm3,	0xee	; xmm5: tmp3

		movaps	xmm6,	xmm0
		shufps	xmm0,	xmm2,	0x88	; xmm0: row0
		shufps	xmm6,	xmm2,	0xdd	; xmm6: row1
		movaps	xmm7,	xmm4
		shufps	xmm4,	xmm5,	0x88	; xmm4: row2
		shufps	xmm7,	xmm5,	0xdd	; xmm7: row3

		mov		eax,	this
		add		eax,	elem
		movaps [eax],	xmm0
		movaps [eax+16],xmm6
		movaps [eax+32],xmm4
		movaps [eax+48],xmm7
	}
}

const mat4& mat4::operator= (const mat4& src)
{
	mcemaths_mat4cpy(elem, src.elem);
	return *this;
}

const mat4& mat4::operator= (const quaternion& q)
{
	mcemaths_q2matrix(elem, &q.x);
	return *this;
}

mat4::operator float* (void)
{
	return elem;
}

mat4::operator const float* (void) const
{
	return elem;
}

ALIGN16 float ZERO_MAT4[] = {0};
const mat4& mat4::zero(void)
{
	mcemaths_mat4cpy(*this, ZERO_MAT4);
	return *this;
}

const mat4& mat4::indentity(void)
{
	mcemaths_mat4ident(*this);
	return *this;
}

const mat4& mat4::transpose(void)
{
	mcemaths_mat4transpose(*this);
	return *this;
}

const mat4& mat4::inverse(void)
{
	mcemaths_mat4inverse(*this);
	return *this;
}

const mat4& mat4::translation(float x, float y, float z)
{
	mcemaths_make_translation(elem, x, y, z);
	return *this;
}

const mat4& mat4::translation(const vec4 & xyz)
{
	mcemaths_make_translation(elem, xyz.x, xyz.y, xyz.z);
	return *this;
}

const mat4& mat4::translation(const float* xyz3)
{
	mcemaths_make_translation(elem, xyz3[0], xyz3[1], xyz3[2]);
	return *this;
}

const mat4& mat4::scaling(float x, float y, float z)
{
	mcemaths_make_scaling(elem, x, y, z);
	return *this;
}

const mat4& mat4::scaling(const vec4 & xyz)
{
	mcemaths_make_scaling(elem, xyz.x, xyz.y, xyz.z);
	return *this;
}

const mat4& mat4::scaling(const float* xyz3)
{
	mcemaths_make_scaling(elem, xyz3[0], xyz3[1], xyz3[2]);
	return *this;
}

const mat4& mat4::rotation(const vec4& axis, float radian)
{
	mcemaths_make_rotation(elem, &axis.x, radian);
	return *this;
}

const mat4& mat4::perspective(float znear, float zfar, float aspect, float fov_rad)
{
	mcemaths_make_proj_perspective(elem, znear, zfar, aspect, fov_rad);
	return *this;
}

const mat4& mat4::orthographic(float znear, float zfar, float left, float right, float bottom, float top)
{
	mcemaths_make_proj_orthographic(elem, znear, zfar, left, right, bottom, top);
	return *this;
}

const mat4& mat4::view(const vec4& from, const vec4& look_at, const vec4& up)
{
	mcemaths_make_view_traditional(elem, &from.x, &look_at.x, &up.x);
	return *this;
}

const mat4& mat4::view(const vec4& from, const vec4& ypr)
{
	mcemaths_make_view_camera(elem, &from.x, &ypr.x);
	return *this;
}

float* mat4::translation(void)
{
	return elem + 12;
}

const float* mat4::translation(void) const
{
	return elem + 12;
}

quaternion::quaternion(void)
{
	x = y = z = 0;
	w = 1.0f;
}

quaternion::quaternion(const quaternion& src)
{
	*this = src;
}

const quaternion& quaternion::operator= (const quaternion& src)
{
	mcemaths_quatcpy(&x, &src.x);
	return *this;
}

const quaternion& quaternion::operator= (const mat4& rot)
{
	mcemaths_matrix2q(&x, rot.elem);
	return *this;
}

quaternion::operator float* (void)
{
	return &x;
}

quaternion::operator const float* (void) const
{
	return &x;
}

const quaternion& quaternion::conjugate(void)
{
	mcemaths_conjugate_q(&x);
	return *this;
}

const quaternion& quaternion::from_axis_angle(const vec4& axis, float angle)
{
	mcemaths_axis2q(&x, &axis.x, angle);
	return *this;
}

const quaternion& quaternion::from_eular(const vec4& ypr)
{
	mcemaths_eular2q(&x, &ypr.x);
	return *this;
}

}
