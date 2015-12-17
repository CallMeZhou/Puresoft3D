#include "stdafx.h"
#include "mcemaths.h"

#define USE_SSE2
#pragma warning(disable: 4305)
#include "sse_mathfun.h"
#pragma warning(default: 4305)

extern "C" MCEMATHAPI(void) mcemaths_make_proj_perspective(float* m44, float znear, float zfar, float aspect, float fov_rad)
{
	mcemaths_mat4ident(m44);

	float h = 1.0f / tan(fov_rad);
	float neg_depth = znear-zfar;

	m44[0] = h / aspect;
	m44[5] = h;
	m44[10]	= (zfar + znear) / neg_depth;
	m44[11]	= -1.0f;
	m44[14]	= 2.0f * (znear * zfar) / neg_depth;
	m44[15]	= 0.0f;
}

extern "C" MCEMATHAPI(void) mcemaths_make_proj_orthographic(float* m44, float znear, float zfar, float left, float right, float bottom, float top)
{
	mcemaths_mat4ident(m44);

	m44[0]	= 2.0f / (right-left);
	m44[5]	= 2.0f / (top-bottom);
	m44[10]	= -2.0f / (zfar-znear);
	m44[12] = -(right+left)/(right-left);
	m44[13] = -(top+bottom)/(top-bottom);
	m44[14] = -(zfar+znear)/(zfar-znear);
}

extern "C" MCEMATHAPI(void) mcemaths_make_view_traditional(float* m44, const float* from4, const float* look_at4, const float* up4)
{
	/* - base theory - 
	it's based on Richard's Matrix4::BuildViewMatrix(), read it for details
	*/
	__declspec(align(16)) float last_row[4];

	__asm{
		mov		eax,	from4
		movaps	xmm0,	[eax]
		mov		eax,	look_at4
		movaps	xmm1,	[eax]
		mov		eax,	up4
		movaps	xmm2,	[eax]
		mov		edx,	m44

		; MATRIX+32/xmm3 <-- rev_sight = from - lookat
		movaps	xmm3,	xmm0
		subps	xmm3,	xmm1
		; rev_sight normalization
		movaps	xmm4,	xmm3
		mulps	xmm4,	xmm4		; multiply four floats
		movhlps	xmm5,	xmm4
		addps	xmm4,	xmm5		; [0]=[0]+[2] and [1]=[1]+[3]
		movaps	xmm5,	xmm4
		shufps	xmm5,	xmm5,	0x1	; xmm1[0] = [1]
		addps	xmm4,	xmm5		; xmm0[0] = [0]+[1]
		rsqrtss	xmm4,	xmm4
		shufps	xmm4,	xmm4,	0x0	; make 4 copies of the sqrt
		mulps	xmm3,	xmm4

		movaps	[edx+32], xmm3

		; MATRIX+0/xmm6 <-- right = up x rev_sight = xmm2 x xmm3
		movaps	xmm4,	xmm2
		movaps	xmm5,	xmm3
		movaps	xmm6,	xmm2
		movaps	xmm7,	xmm3
		shufps	xmm6,	xmm6,	 0xc9	; a1 a2 a0
		shufps	xmm7,	xmm7,	 0xd2	; b2 b0 b1
		shufps	xmm4,	xmm4,	 0xd2	; a2 a0 a1
		shufps	xmm5,	xmm5,	 0xc9	; b1 b2 b0
		mulps	xmm6,	xmm7
		mulps	xmm4,	xmm5
		subps	xmm6,	xmm4
		movaps	[edx],	xmm6

		; MATRIX+16/xmm7 <-- ortho_up = rev_sight x right = xmm3 x xmm6
		movaps	xmm4,	xmm3
		movaps	xmm5,	xmm6
		movaps	xmm7,	xmm3
		movaps	xmm1,	xmm6
		shufps	xmm7,	xmm7,	 0xc9	; a1 a2 a0
		shufps	xmm1,	xmm1,	 0xd2	; b2 b0 b1
		shufps	xmm4,	xmm4,	 0xd2	; a2 a0 a1
		shufps	xmm5,	xmm5,	 0xc9	; b1 b2 b0
		mulps	xmm7,	xmm1
		mulps	xmm4,	xmm5
		subps	xmm7,	xmm4
		movaps	[edx+16],	xmm7

		; neg_from in xmm1
		xorps	xmm1,	xmm1
		subps	xmm1,	xmm0

		; free regs: xmm0/4/5

		; xmm0[0] <-- dot(neg_from, right) = dot(xmm1, xmm6)
		mulps	xmm6,	xmm1		; multiply four floats
		movhlps	xmm0,	xmm6
		addps	xmm6,	xmm0		; [0]=[0]+[2] and [1]=[1]+[3]
		movaps	xmm0,	xmm6
		shufps	xmm0,	xmm0,	0x1	; xmm0[0] = [1]
		addps	xmm0,	xmm6		; xmm0[0] = [0]+[1]

		; free regs: xmm4/5/6

		; xmm4[0] <-- dot(neg_from, ortho_up) = dot(xmm1, xmm7)
		mulps	xmm7,	xmm1		; multiply four floats
		movhlps	xmm4,	xmm7
		addps	xmm7,	xmm4		; [0]=[0]+[2] and [1]=[1]+[3]
		movaps	xmm4,	xmm7
		shufps	xmm4,	xmm4,	0x1	; xmm4[0] = [1]
		addps	xmm4,	xmm7		; xmm4[0] = [0]+[1]

		; free regs: xmm5/6/7

		; xmm5[0] <-- dot(neg_from, neg_sight) = dot(xmm1, xmm3)
		mulps	xmm3,	xmm1		; multiply four floats
		movhlps	xmm5,	xmm3
		addps	xmm3,	xmm5		; [0]=[0]+[2] and [1]=[1]+[3]
		movaps	xmm5,	xmm3
		shufps	xmm5,	xmm4,	0x1	; xmm5[0] = [1]
		addps	xmm5,	xmm3		; xmm5[0] = [0]+[1]

		; free regs: xmm1/2/3/6/7

		; make the last row: {xmm0[0], xmm4[0], xmm5[0], 1.0f}
		movlhps	xmm0,	xmm4
		shufps	xmm0,	xmm0,	0x8
		movlhps	xmm0,	xmm5
		; well, I think the last 1.0f can be set by normal code:)
		lea		eax,	last_row
		movaps	[eax],	xmm0
	}
	last_row[3] = 1.0f;
	memcpy(m44+12, IDENTITY4_LASTROW, sizeof(float)*4);
	mcemaths_mat4transpose(m44);
	memcpy(m44+12, last_row, sizeof(float)*4);
}

__declspec(align(16)) const float UP[4] = {0, 1.0f, 0, 0};
__declspec(align(16)) const float NEG_X[4] = {-1.0f, 0, 0, 0};
__declspec(align(16)) const float NEG_Y[4] = {0, -1.0f, 0, 0};
extern "C" MCEMATHAPI(void) mcemaths_make_view_camera(float* m44, const float* from4, const float* ypr4)
{
	/* - base theory - 
	it's based on Richard's Matrix4::BuildViewMatrix(), read it for details
	*/

// 	enum {SIN_Y, SIN_P, SIN_R, NULL1, COS_Y, COS_P, COS_R, NULL2};
// 	__declspec(align(16)) float ypr_trigon[8];
// 	__m128 sines, cosines;
// 	sincos_ps(_mm_load_ps(ypr4), &sines, &cosines);
// 	_mm_store_ps(ypr_trigon,   sines);
// 	_mm_store_ps(ypr_trigon+4, cosines);
// 	__declspec(align(16)) float sight[4], last_row[4];
// 	sight[0] = ypr_trigon[COS_P] * ypr_trigon[SIN_Y];
// 	sight[1] = -ypr_trigon[SIN_P];
// 	sight[2] = -ypr_trigon[COS_P] * ypr_trigon[COS_Y];
// 	sight[3] = 0;
// 
// 	__asm{
// 		mov		eax,	from4
// 		movaps	xmm0,	[eax]
// 		;mov	eax,	look_at4
// 		;movaps	xmm1,	[eax]
// 		lea		eax,	UP
// 		movaps	xmm2,	[eax]
// 		mov		edx,	m44
// 
// 		lea		eax,	sight
// 		movaps	xmm1,	[eax]
// 		xorps	xmm3,	xmm3
// 		subps	xmm3,	xmm1
// 
// 		movaps	[edx+32], xmm3
// 
// 		; MATRIX+0/xmm6 <-- right = up x rev_sight = xmm2 x xmm3
// 		movaps	xmm4,	xmm2
// 		movaps	xmm5,	xmm3
// 		movaps	xmm6,	xmm2
// 		movaps	xmm7,	xmm3
// 		shufps	xmm6,	xmm6,	 0xc9	; a1 a2 a0
// 		shufps	xmm7,	xmm7,	 0xd2	; b2 b0 b1
// 		shufps	xmm4,	xmm4,	 0xd2	; a2 a0 a1
// 		shufps	xmm5,	xmm5,	 0xc9	; b1 b2 b0
// 		mulps	xmm6,	xmm7
// 		mulps	xmm4,	xmm5
// 		subps	xmm6,	xmm4
// 		movaps	[edx],	xmm6
// 
// 		; MATRIX+16/xmm7 <-- ortho_up = rev_sight x right = xmm3 x xmm6
// 		movaps	xmm4,	xmm3
// 		movaps	xmm5,	xmm6
// 		movaps	xmm7,	xmm3
// 		movaps	xmm1,	xmm6
// 		shufps	xmm7,	xmm7,	 0xc9	; a1 a2 a0
// 		shufps	xmm1,	xmm1,	 0xd2	; b2 b0 b1
// 		shufps	xmm4,	xmm4,	 0xd2	; a2 a0 a1
// 		shufps	xmm5,	xmm5,	 0xc9	; b1 b2 b0
// 		mulps	xmm7,	xmm1
// 		mulps	xmm4,	xmm5
// 		subps	xmm7,	xmm4
// 		movaps	[edx+16],	xmm7
// 
// 		; neg_from in xmm1
// 		xorps	xmm1,	xmm1
// 		subps	xmm1,	xmm0
// 
// 		; free regs: xmm0/4/5
// 
// 		; xmm0[0] <-- dot(neg_from, right) = dot(xmm1, xmm6)
// 		mulps	xmm6,	xmm1		; multiply four floats
// 		movhlps	xmm0,	xmm6
// 		addps	xmm6,	xmm0		; [0]=[0]+[2] and [1]=[1]+[3]
// 		movaps	xmm0,	xmm6
// 		shufps	xmm0,	xmm0,	0x1	; xmm0[0] = [1]
// 		addps	xmm0,	xmm6		; xmm0[0] = [0]+[1]
// 
// 		; free regs: xmm4/5/6
// 
// 		; xmm4[0] <-- dot(neg_from, ortho_up) = dot(xmm1, xmm7)
// 		mulps	xmm7,	xmm1		; multiply four floats
// 		movhlps	xmm4,	xmm7
// 		addps	xmm7,	xmm4		; [0]=[0]+[2] and [1]=[1]+[3]
// 		movaps	xmm4,	xmm7
// 		shufps	xmm4,	xmm4,	0x1	; xmm4[0] = [1]
// 		addps	xmm4,	xmm7		; xmm4[0] = [0]+[1]
// 
// 		; free regs: xmm5/6/7
// 
// 		; xmm5[0] <-- dot(neg_from, neg_sight) = dot(xmm1, xmm3)
// 		mulps	xmm3,	xmm1		; multiply four floats
// 		movhlps	xmm5,	xmm3
// 		addps	xmm3,	xmm5		; [0]=[0]+[2] and [1]=[1]+[3]
// 		movaps	xmm5,	xmm3
// 		shufps	xmm5,	xmm4,	0x1	; xmm5[0] = [1]
// 		addps	xmm5,	xmm3		; xmm5[0] = [0]+[1]
// 
// 		; free regs: xmm1/2/3/6/7
// 
// 		; make the last row: {xmm0[0], xmm4[0], xmm5[0], 1.0f}
// 		movlhps	xmm0,	xmm4
// 		shufps	xmm0,	xmm0,	0x8
// 		movlhps	xmm0,	xmm5
// 		; well, I think the last 1.0f can be set by normal code:)
// 		lea		eax,	last_row
// 		movaps	[eax],	xmm0
// 	}
// 	last_row[3] = 1.0f;
// 	memcpy(m44+12, IDENTITY4_LASTROW, sizeof(float)*4);
// 	mcemaths_mat4transpose(m44);
// 	memcpy(m44+12, last_row, sizeof(float)*4);
	mcemaths_make_rotation(m44, NEG_X, -ypr4[1]);
	ALIGN16 float temp[16];
	mcemaths_make_rotation(temp, NEG_Y, -ypr4[0]);
	mcemaths_transform_m4m4_l_ip(m44, temp);
	mcemaths_make_translation(temp, -from4[0], -from4[1], -from4[2]);
	mcemaths_transform_m4m4_l_ip(m44, temp);
}