#include "stdafx.h"
#include "mcemaths.h"

extern "C" MCEMATHAPI(void) mcemaths_mat4ident(float* m44)
{
	__asm{
		lea		eax,		IDENTITY4
		mov		edx,		m44
		movaps	xmm0,		[eax]
		movaps	[edx],		xmm0
		movaps	xmm0,		[eax + 16]
		movaps	[edx + 16],	xmm0
		movaps	xmm0,		[eax + 32]
		movaps	[edx + 32],	xmm0
		movaps	xmm0,		[eax + 48]
		movaps	[edx + 48],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_mat4transpose(float* m44)
{
	/* - base theory -
	source: mmintrin.h line 102
	_MM_TRANSPOSE4_PS(row0, row1, row2, row3)
	{
		__m128 tmp3, tmp2, tmp1, tmp0;                 

		tmp0   = _mm_shuffle_ps((row0), (row1), 0x44); 
		tmp2   = _mm_shuffle_ps((row0), (row1), 0xEE); 
		tmp1   = _mm_shuffle_ps((row2), (row3), 0x44); 
		tmp3   = _mm_shuffle_ps((row2), (row3), 0xEE); 

		(row0) = _mm_shuffle_ps(tmp0, tmp1, 0x88);
		(row1) = _mm_shuffle_ps(tmp0, tmp1, 0xDD);
		(row2) = _mm_shuffle_ps(tmp2, tmp3, 0x88);
		(row3) = _mm_shuffle_ps(tmp2, tmp3, 0xDD);
	}*/
	__asm{
		mov		eax,	m44
		movaps	xmm0,	[eax]
		movaps	xmm1,	[eax+16]
		movaps	xmm2,	[eax+32]
		movaps	xmm3,	[eax+48]

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

		movaps [eax],	xmm0
		movaps [eax+16],xmm6
		movaps [eax+32],xmm4
		movaps [eax+48],xmm7
	}
}

// matrix inversion is so troublesome, allow me to be lazy ok?
// reference: by Groove, published at http://devmaster.net/forums/topic/11799-sse-mat4-inverse/
extern "C" MCEMATHAPI(void) mcemaths_mat4inverse(float* m44)
{
	__m128 in[4];
	in[0] = _mm_load_ps(m44);
	in[1] = _mm_load_ps(m44 + 4);
	in[2] = _mm_load_ps(m44 + 8);
	in[3] = _mm_load_ps(m44 + 12);

	__m128 Fac0;
	{
		//      valType SubFactor00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		//      valType SubFactor00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		//      valType SubFactor06 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		//      valType SubFactor13 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

		__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(3, 3, 3, 3));
		__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(2, 2, 2, 2));

		__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(2, 2, 2, 2));
		__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(3, 3, 3, 3));

		__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
		__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
		Fac0 = _mm_sub_ps(Mul00, Mul01);
	}

	__m128 Fac1;
	{
		//      valType SubFactor01 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		//      valType SubFactor01 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		//      valType SubFactor07 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		//      valType SubFactor14 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

		__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(3, 3, 3, 3));
		__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(1, 1, 1, 1));

		__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(1, 1, 1, 1));
		__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(3, 3, 3, 3));

		__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
		__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
		Fac1 = _mm_sub_ps(Mul00, Mul01);
	}


	__m128 Fac2;
	{
		//      valType SubFactor02 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		//      valType SubFactor02 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		//      valType SubFactor08 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
		//      valType SubFactor15 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

		__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(2, 2, 2, 2));
		__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(1, 1, 1, 1));

		__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(1, 1, 1, 1));
		__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(2, 2, 2, 2));

		__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
		__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
		Fac2 = _mm_sub_ps(Mul00, Mul01);
	}

	__m128 Fac3;
	{
		//      valType SubFactor03 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		//      valType SubFactor03 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		//      valType SubFactor09 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
		//      valType SubFactor16 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

		__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(3, 3, 3, 3));
		__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(0, 0, 0, 0));

		__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(3, 3, 3, 3));

		__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
		__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
		Fac3 = _mm_sub_ps(Mul00, Mul01);
	}

	__m128 Fac4;
	{
		//      valType SubFactor04 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		//      valType SubFactor04 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		//      valType SubFactor10 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
		//      valType SubFactor17 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

		__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(2, 2, 2, 2));
		__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(0, 0, 0, 0));

		__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(2, 2, 2, 2));

		__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
		__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
		Fac4 = _mm_sub_ps(Mul00, Mul01);
	}

	__m128 Fac5;
	{
		//      valType SubFactor05 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		//      valType SubFactor05 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		//      valType SubFactor12 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
		//      valType SubFactor18 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

		__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(1, 1, 1, 1));
		__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(0, 0, 0, 0));

		__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(1, 1, 1, 1));

		__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
		__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
		Fac5 = _mm_sub_ps(Mul00, Mul01);
	}

	__m128 SignA = _mm_set_ps( 1.0f,-1.0f, 1.0f,-1.0f);
	__m128 SignB = _mm_set_ps(-1.0f, 1.0f,-1.0f, 1.0f);

	// m[1][0]
	// m[0][0]
	// m[0][0]
	// m[0][0]
	__m128 Temp0 = _mm_shuffle_ps(in[1], in[0], _MM_SHUFFLE(0, 0, 0, 0));
	__m128 Vec0 = _mm_shuffle_ps(Temp0, Temp0, _MM_SHUFFLE(2, 2, 2, 0));

	// m[1][1]
	// m[0][1]
	// m[0][1]
	// m[0][1]
	__m128 Temp1 = _mm_shuffle_ps(in[1], in[0], _MM_SHUFFLE(1, 1, 1, 1));
	__m128 Vec1 = _mm_shuffle_ps(Temp1, Temp1, _MM_SHUFFLE(2, 2, 2, 0));

	// m[1][2]
	// m[0][2]
	// m[0][2]
	// m[0][2]
	__m128 Temp2 = _mm_shuffle_ps(in[1], in[0], _MM_SHUFFLE(2, 2, 2, 2));
	__m128 Vec2 = _mm_shuffle_ps(Temp2, Temp2, _MM_SHUFFLE(2, 2, 2, 0));

	// m[1][3]
	// m[0][3]
	// m[0][3]
	// m[0][3]
	__m128 Temp3 = _mm_shuffle_ps(in[1], in[0], _MM_SHUFFLE(3, 3, 3, 3));
	__m128 Vec3 = _mm_shuffle_ps(Temp3, Temp3, _MM_SHUFFLE(2, 2, 2, 0));

	// col0
	// + (Vec1[0] * Fac0[0] - Vec2[0] * Fac1[0] + Vec3[0] * Fac2[0]),
	// - (Vec1[1] * Fac0[1] - Vec2[1] * Fac1[1] + Vec3[1] * Fac2[1]),
	// + (Vec1[2] * Fac0[2] - Vec2[2] * Fac1[2] + Vec3[2] * Fac2[2]),
	// - (Vec1[3] * Fac0[3] - Vec2[3] * Fac1[3] + Vec3[3] * Fac2[3]),
	__m128 Mul00 = _mm_mul_ps(Vec1, Fac0);
	__m128 Mul01 = _mm_mul_ps(Vec2, Fac1);
	__m128 Mul02 = _mm_mul_ps(Vec3, Fac2);
	__m128 Sub00 = _mm_sub_ps(Mul00, Mul01);
	__m128 Add00 = _mm_add_ps(Sub00, Mul02);
	__m128 Inv0 = _mm_mul_ps(SignB, Add00);

	// col1
	// - (Vec0[0] * Fac0[0] - Vec2[0] * Fac3[0] + Vec3[0] * Fac4[0]),
	// + (Vec0[0] * Fac0[1] - Vec2[1] * Fac3[1] + Vec3[1] * Fac4[1]),
	// - (Vec0[0] * Fac0[2] - Vec2[2] * Fac3[2] + Vec3[2] * Fac4[2]),
	// + (Vec0[0] * Fac0[3] - Vec2[3] * Fac3[3] + Vec3[3] * Fac4[3]),
	__m128 Mul03 = _mm_mul_ps(Vec0, Fac0);
	__m128 Mul04 = _mm_mul_ps(Vec2, Fac3);
	__m128 Mul05 = _mm_mul_ps(Vec3, Fac4);
	__m128 Sub01 = _mm_sub_ps(Mul03, Mul04);
	__m128 Add01 = _mm_add_ps(Sub01, Mul05);
	__m128 Inv1 = _mm_mul_ps(SignA, Add01);

	// col2
	// + (Vec0[0] * Fac1[0] - Vec1[0] * Fac3[0] + Vec3[0] * Fac5[0]),
	// - (Vec0[0] * Fac1[1] - Vec1[1] * Fac3[1] + Vec3[1] * Fac5[1]),
	// + (Vec0[0] * Fac1[2] - Vec1[2] * Fac3[2] + Vec3[2] * Fac5[2]),
	// - (Vec0[0] * Fac1[3] - Vec1[3] * Fac3[3] + Vec3[3] * Fac5[3]),
	__m128 Mul06 = _mm_mul_ps(Vec0, Fac1);
	__m128 Mul07 = _mm_mul_ps(Vec1, Fac3);
	__m128 Mul08 = _mm_mul_ps(Vec3, Fac5);
	__m128 Sub02 = _mm_sub_ps(Mul06, Mul07);
	__m128 Add02 = _mm_add_ps(Sub02, Mul08);
	__m128 Inv2 = _mm_mul_ps(SignB, Add02);

	// col3
	// - (Vec1[0] * Fac2[0] - Vec1[0] * Fac4[0] + Vec2[0] * Fac5[0]),
	// + (Vec1[0] * Fac2[1] - Vec1[1] * Fac4[1] + Vec2[1] * Fac5[1]),
	// - (Vec1[0] * Fac2[2] - Vec1[2] * Fac4[2] + Vec2[2] * Fac5[2]),
	// + (Vec1[0] * Fac2[3] - Vec1[3] * Fac4[3] + Vec2[3] * Fac5[3]));
	__m128 Mul09 = _mm_mul_ps(Vec0, Fac2);
	__m128 Mul10 = _mm_mul_ps(Vec1, Fac4);
	__m128 Mul11 = _mm_mul_ps(Vec2, Fac5);
	__m128 Sub03 = _mm_sub_ps(Mul09, Mul10);
	__m128 Add03 = _mm_add_ps(Sub03, Mul11);
	__m128 Inv3 = _mm_mul_ps(SignA, Add03);

	__m128 Row0 = _mm_shuffle_ps(Inv0, Inv1, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 Row1 = _mm_shuffle_ps(Inv2, Inv3, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 Row2 = _mm_shuffle_ps(Row0, Row1, _MM_SHUFFLE(2, 0, 2, 0));

	//      valType Determinant = m[0][0] * Inverse[0][0] 
	//                          + m[0][1] * Inverse[1][0] 
	//                          + m[0][2] * Inverse[2][0] 
	//                          + m[0][3] * Inverse[3][0];

	__m128 mul0 = _mm_mul_ps(in[0], Row2);
	__m128 swp0 = _mm_shuffle_ps(mul0, mul0, _MM_SHUFFLE(2, 3, 0, 1));
	__m128 add0 = _mm_add_ps(mul0, swp0);
	__m128 swp1 = _mm_shuffle_ps(add0, add0, _MM_SHUFFLE(0, 1, 2, 3));
	__m128 Det0 = _mm_add_ps(add0, swp1);
	__m128 Rcp0 = _mm_rcp_ps(Det0);

	//      Inverse /= Determinant;
	in[0] = _mm_mul_ps(Inv0, Rcp0);
	in[1] = _mm_mul_ps(Inv1, Rcp0);
	in[2] = _mm_mul_ps(Inv2, Rcp0);
	in[3] = _mm_mul_ps(Inv3, Rcp0);

	_mm_store_ps(m44,    in[0]);
	_mm_store_ps(m44+4,  in[1]);
	_mm_store_ps(m44+8,  in[2]);
	_mm_store_ps(m44+12, in[3]);
}

extern "C" MCEMATHAPI(void) mcemaths_mat4cpy(float* dest44, const float* src44)
{
	__asm{
		mov		eax,		src44
		mov		edx,		dest44
		movaps	xmm0,		[eax]
		movaps	[edx],		xmm0
		movaps	xmm0,		[eax + 16]
		movaps	[edx + 16],	xmm0
		movaps	xmm0,		[eax + 32]
		movaps	[edx + 32],	xmm0
		movaps	xmm0,		[eax + 48]
		movaps	[edx + 48],	xmm0
	}
}

extern "C" MCEMATHAPI(bool) mcemaths_mat3equal(const float* m4_1, const float* m4_2)
{
	ALIGN16 long r[4];
	__asm{
		mov		eax,	m4_1
		mov		ecx,	m4_2
		movaps	xmm0,	[eax]
		cmpeqps	xmm0,	[ecx]
		movaps	xmm1,	[eax+16]
		cmpeqps	xmm1,	[ecx+16]
		andps	xmm0,	xmm1
		movaps	xmm1,	[eax+32]
		cmpeqps	xmm1,	[ecx+32]
		andps	xmm0,	xmm1
		movaps	xmm1,	[eax+48]
		cmpeqps	xmm1,	[ecx+48]
		andps	xmm0,	xmm1
		lea		eax,	[r]
		movaps	[eax],	xmm0
	}
	return ((0xFFFFFFFF == r[0]) && (0xFFFFFFFF == r[1]) && (0xFFFFFFFF == r[2]));
}

extern "C" MCEMATHAPI(bool) mcemaths_mat4equal(const float* m4_1, const float* m4_2)
{
	ALIGN16 __int64 r[4];
	__asm{
		mov		eax,	m4_1
		mov		ecx,	m4_2
		movaps	xmm0,	[eax]
		cmpeqps	xmm0,	[ecx]
		movaps	xmm1,	[eax+16]
		cmpeqps	xmm1,	[ecx+16]
		andps	xmm0,	xmm1
		movaps	xmm1,	[eax+32]
		cmpeqps	xmm1,	[ecx+32]
		andps	xmm0,	xmm1
		movaps	xmm1,	[eax+48]
		cmpeqps	xmm1,	[ecx+48]
		andps	xmm0,	xmm1
		lea		eax,	[r]
		movaps	[eax],	xmm0
	}
	return ((0xFFFFFFFFFFFFFFFF == r[0]) && (0xFFFFFFFFFFFFFFFF == r[1]));
}

// v' = M x v, M is a 3x3 matrix stored in 4x3 space in column-major order, v is 3d column vector
extern "C" MCEMATHAPI(void) mcemaths_transform_m3v3(float* r4, const float* trans43, const float* v4)
{
	/* - base theory - 
	column-major matrix:
	M[0] M[4] M[8] M[-]
	M[1] M[5] M[9] M[-]
	M[2] M[6] M[A] M[-]
	M[3] M[7] M[B] M[-]
	pre-multiply:
	x' = x * M[0] + y * M[4] + z * M[8]
	y' = x * M[1] + y * M[5] + z * M[9]
	z' = x * M[2] + y * M[6] + z * M[A]
	*/
	__asm{
		; input data
		mov		eax,	trans43
		movaps	xmm4,	[eax]			; matrix: one column in one sse register
		movaps	xmm5,	[eax+16]
		movaps	xmm6,	[eax+32]
		;movaps	xmm7,	[eax+48]
		mov		eax,	v4
		movaps	xmm0,	[eax]			; vector: one component in one register
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; x x x x
		shufps	xmm1,	xmm1,	0x55	; y y y y
		shufps	xmm2,	xmm2,	0xaa	; z z z z
		;shufps	xmm3,	xmm3,	0xff	; w w w w
		; vec-component * mat-column
		mulps	xmm0,	xmm4
		mulps	xmm1,	xmm5
		mulps	xmm2,	xmm6
		;mulps	xmm3,	xmm7
		; finally, just add them together and output ;-)
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		;addps	xmm0,	xmm3
		mov		eax,	r4
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_transform_m3v3_ip(float* r4, const float* trans43)
{
	__asm{
		mov		eax,	trans43
		movaps	xmm4,	[eax]
		movaps	xmm5,	[eax+16]
		movaps	xmm6,	[eax+32]
		mov		eax,	r4
		movaps	xmm0,	[eax]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00
		shufps	xmm1,	xmm1,	0x55
		shufps	xmm2,	xmm2,	0xaa
		mulps	xmm0,	xmm4
		mulps	xmm1,	xmm5
		mulps	xmm2,	xmm6
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		movaps	[eax],	xmm0
	}
}

// v' = M x v, M is a 4x4 matrix stored in column-major order, v is 3d column vector
extern "C" MCEMATHAPI(void) mcemaths_transform_m4v3(float* r4, const float* trans44, const float* v4)
{
	/* - base theory - 
	column-major matrix:
	M[0] M[4] M[8] M[C]
	M[1] M[5] M[9] M[D]
	M[2] M[6] M[A] M[E]
	M[3] M[7] M[B] M[F]
	pre-multiply:
	x' = x * M[0] + y * M[4] + z * M[8] + M[C]
	y' = x * M[1] + y * M[5] + z * M[9] + M[D]
	z' = x * M[2] + y * M[6] + z * M[A] + M[E]
	w  = x * M[3] + y * M[7] + z * M[B] + M[F]
	(x', y', z') /= w;
	*/
	__asm{
		; input data
		mov		eax,	trans44
		movaps	xmm4,	[eax]			; matrix: one column in one sse register
		movaps	xmm5,	[eax+16]
		movaps	xmm6,	[eax+32]
		movaps	xmm7,	[eax+48]
		mov		eax,	v4
		movaps	xmm0,	[eax]			; vector: one component in one register
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		shufps	xmm0,	xmm0,	0x00	; x x x x
		shufps	xmm1,	xmm1,	0x55	; y y y y
		shufps	xmm2,	xmm2,	0xaa	; z z z z
		; vec-component * mat-column
		mulps	xmm0,	xmm4
		mulps	xmm1,	xmm5
		mulps	xmm2,	xmm6
		; add 4 columns together
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm7
		; divide xmm0[0~2] by xmm0[3]
		movaps	xmm1,	xmm0
		shufps	xmm1,	xmm1,	0xff	; w w w w
		rcpps	xmm1,	xmm1
		mulps	xmm0,	xmm1
		mov		eax,	r4
		movaps	[eax],	xmm0
	}
	// eliminate w in target data
	r4[3] = 0;
}

extern "C" MCEMATHAPI(void) mcemaths_transform_m4v3_ip(float* r4, const float* trans44)
{
	__asm{
		mov		eax,	trans44
		movaps	xmm4,	[eax]
		movaps	xmm5,	[eax+16]
		movaps	xmm6,	[eax+32]
		movaps	xmm7,	[eax+48]
		mov		eax,	r4
		movaps	xmm0,	[eax]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		shufps	xmm0,	xmm0,	0x00
		shufps	xmm1,	xmm1,	0x55
		shufps	xmm2,	xmm2,	0xaa
		mulps	xmm0,	xmm4
		mulps	xmm1,	xmm5
		mulps	xmm2,	xmm6
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm7
		movaps	xmm1,	xmm0
		shufps	xmm1,	xmm1,	0xff
		rcpps	xmm1,	xmm1
		mulps	xmm0,	xmm1
		movaps	[eax],	xmm0
	}
	r4[3] = 0;
}

// v' = M x v, M is a 4x4 matrix stored in column-major order, v is 4d column vector
extern "C" MCEMATHAPI(void) mcemaths_transform_m4v4(float* r4, const float* trans44, const float* v4)
{
	/* - base theory - 
	column-major matrix:
	M[0] M[4] M[8] M[C]
	M[1] M[5] M[9] M[D]
	M[2] M[6] M[A] M[E]
	M[3] M[7] M[B] M[F]
	pre-multiply:
	x' = x * M[0] + y * M[4] + z * M[8] + w  *  M[C]
	y' = x * M[1] + y * M[5] + z * M[9] + w  *  M[D]
	z' = x * M[2] + y * M[6] + z * M[A] + w  *  M[E]
	w' = x * M[3] + y * M[7] + z * M[B] + w  *  M[F]
	*/
	__asm{
		; input data
		mov		eax,	trans44
		movaps	xmm4,	[eax]			; matrix: one column in one sse register
		movaps	xmm5,	[eax+16]
		movaps	xmm6,	[eax+32]
		movaps	xmm7,	[eax+48]
		mov		eax,	v4
		movaps	xmm0,	[eax]			; vector: one component in one register
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; x x x x
		shufps	xmm1,	xmm1,	0x55	; y y y y
		shufps	xmm2,	xmm2,	0xaa	; z z z z
		shufps	xmm3,	xmm3,	0xff	; w w w w
		; vec-component * mat-column
		mulps	xmm0,	xmm4
		mulps	xmm1,	xmm5
		mulps	xmm2,	xmm6
		mulps	xmm3,	xmm7
		; finally, just add them together and output ;-)
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3
		mov		eax,	r4
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_transform_m4v4_ip(float* r4, const float* trans44)
{
	__asm{
		mov		eax,	trans44
		movaps	xmm4,	[eax]
		movaps	xmm5,	[eax+16]
		movaps	xmm6,	[eax+32]
		movaps	xmm7,	[eax+48]
		mov		eax,	r4
		movaps	xmm0,	[eax]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00
		shufps	xmm1,	xmm1,	0x55
		shufps	xmm2,	xmm2,	0xaa
		shufps	xmm3,	xmm3,	0xff
		mulps	xmm0,	xmm4
		mulps	xmm1,	xmm5
		mulps	xmm2,	xmm6
		mulps	xmm3,	xmm7
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3
		movaps	[eax],	xmm0
	}
}

// d = l * r, where all inputs are 4x4 matrix
extern "C" MCEMATHAPI(void) mcemaths_transform_m4m4(float* d44, const float* l44, const float* r44)
{
	/* - base theory - 

	d = l * r

	   \  r | 0 4 8 C
	    \   | 1 5 9 D
	     \  | 2 6 A E
	  l   \ | 3 7 B F
	--------+--------
	0 4 8 C | 0 4 8 C
	1 5 9 D | 1 5 9 D
	2 6 A E | 2 6 A E
	3 7 B F | 3 7 B F

	calculate the 1st row in d:

	d0 = l0r0 + l4r1 + l8r2 + lCr3
	d1 = l1r0 + l5r1 + l9r2 + lDr3
	d2 = l2r0 + l6r1 + lAr2 + lEr3
	d3 = l3r0 + l7r1 + lBr2 + lFr3

	the other 3 rows in d are made in the similar way

	loop's been flattened in the following code
	*/
	__asm{
		mov		eax,	r44
		mov		edx,	l44
		xor		ecx,	ecx
		movaps	xmm4,	[edx]			; l0 l1 l2 l3
		movaps	xmm5,	[edx+16]		; l4 l5 l6 l7
		movaps	xmm6,	[edx+32]		; l8 l9 lA lB
		movaps	xmm7,	[edx+48]		; lC lD lE lF
		mov		edx,	d44

		movaps	xmm0,	[eax]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; r0 r0 r0 r0
		shufps	xmm1,	xmm1,	0x55	; r1 r1 r1 r1
		shufps	xmm2,	xmm2,	0xaa	; r2 r2 r2 r2
		shufps	xmm3,	xmm3,	0xff	; r3 r3 r3 r3

		mulps	xmm0,	xmm4			; l0*r0 l1*r0 l2*r0 l3*r0
		mulps	xmm1,	xmm5			; l4*r1 l5*r1 l6*r1 l7*r1
		mulps	xmm2,	xmm6			; l8*r2 l9*r2 lA*r2 lB*r2
		mulps	xmm3,	xmm7			; lC*r3 lD*r3 lE*r3 lF*r3
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3

		movaps	[edx],	xmm0

		movaps	xmm0,	[eax+16]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; r4 r4 r4 r4
		shufps	xmm1,	xmm1,	0x55	; r5 r5 r5 r5
		shufps	xmm2,	xmm2,	0xaa	; r6 r6 r6 r6
		shufps	xmm3,	xmm3,	0xff	; r7 r7 r7 r7

		mulps	xmm0,	xmm4			; l0*r4 l1*r4 l2*r4 l3*r4
		mulps	xmm1,	xmm5			; l4*r5 l5*r5 l6*r5 l7*r5
		mulps	xmm2,	xmm6			; l8*r6 l9*r6 lA*r6 lB*r6
		mulps	xmm3,	xmm7			; lC*r7 lD*r7 lE*r7 lF*r7
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3

		movaps	[edx+16],	xmm0

		movaps	xmm0,	[eax+32]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; r8  r8  r8  r8
		shufps	xmm1,	xmm1,	0x55	; r9  r9  r9  r9
		shufps	xmm2,	xmm2,	0xaa	; r10 r10 r10 r10
		shufps	xmm3,	xmm3,	0xff	; r11 r11 r11 r11

		mulps	xmm0,	xmm4			; l0*r8  l1*r8  l2*r8  l3*r8
		mulps	xmm1,	xmm5			; l4*r9  l5*r9  l6*r9  l7*r9
		mulps	xmm2,	xmm6			; l8*r10 l9*r10 lA*r10 lB*r10
		mulps	xmm3,	xmm7			; lC*r11 lD*r11 lE*r11 lF*r11
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3

		movaps	[edx+32],	xmm0

		movaps	xmm0,	[eax+48]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; r12 r12 r12 r12
		shufps	xmm1,	xmm1,	0x55	; r13 r13 r13 r13
		shufps	xmm2,	xmm2,	0xaa	; r14 r14 r14 r14
		shufps	xmm3,	xmm3,	0xff	; r15 r15 r15 r15

		mulps	xmm0,	xmm4			; l0*r12 l1*r12 l2*r12 l3*r12
		mulps	xmm1,	xmm5			; l4*r13 l5*r13 l6*r13 l7*r13
		mulps	xmm2,	xmm6			; l8*r14 l9*r14 lA*r14 lB*r14
		mulps	xmm3,	xmm7			; lC*r15 lD*r15 lE*r15 lF*r15
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3

		movaps	[edx+48],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_transform_m4m4_r_ip(const float* l44, float* r44)
{
	__asm{
		mov		eax,	r44
		mov		edx,	l44
		xor		ecx,	ecx
		movaps	xmm4,	[edx]			; l0 l1 l2 l3
		movaps	xmm5,	[edx+16]		; l4 l5 l6 l7
		movaps	xmm6,	[edx+32]		; l8 l9 lA lB
		movaps	xmm7,	[edx+48]		; lC lD lE lF

		movaps	xmm0,	[eax]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; r0 r0 r0 r0
		shufps	xmm1,	xmm1,	0x55	; r1 r1 r1 r1
		shufps	xmm2,	xmm2,	0xaa	; r2 r2 r2 r2
		shufps	xmm3,	xmm3,	0xff	; r3 r3 r3 r3

		mulps	xmm0,	xmm4			; l0*r0 l1*r0 l2*r0 l3*r0
		mulps	xmm1,	xmm5			; l4*r1 l5*r1 l6*r1 l7*r1
		mulps	xmm2,	xmm6			; l8*r2 l9*r2 lA*r2 lB*r2
		mulps	xmm3,	xmm7			; lC*r3 lD*r3 lE*r3 lF*r3
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3

		movaps	[eax],	xmm0

		movaps	xmm0,	[eax+16]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; r4 r4 r4 r4
		shufps	xmm1,	xmm1,	0x55	; r5 r5 r5 r5
		shufps	xmm2,	xmm2,	0xaa	; r6 r6 r6 r6
		shufps	xmm3,	xmm3,	0xff	; r7 r7 r7 r7

		mulps	xmm0,	xmm4			; l0*r4 l1*r4 l2*r4 l3*r4
		mulps	xmm1,	xmm5			; l4*r5 l5*r5 l6*r5 l7*r5
		mulps	xmm2,	xmm6			; l8*r6 l9*r6 lA*r6 lB*r6
		mulps	xmm3,	xmm7			; lC*r7 lD*r7 lE*r7 lF*r7
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3

		movaps	[eax+16],	xmm0

		movaps	xmm0,	[eax+32]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; r8  r8  r8  r8
		shufps	xmm1,	xmm1,	0x55	; r9  r9  r9  r9
		shufps	xmm2,	xmm2,	0xaa	; r10 r10 r10 r10
		shufps	xmm3,	xmm3,	0xff	; r11 r11 r11 r11

		mulps	xmm0,	xmm4			; l0*r8  l1*r8  l2*r8  l3*r8
		mulps	xmm1,	xmm5			; l4*r9  l5*r9  l6*r9  l7*r9
		mulps	xmm2,	xmm6			; l8*r10 l9*r10 lA*r10 lB*r10
		mulps	xmm3,	xmm7			; lC*r11 lD*r11 lE*r11 lF*r11
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3

		movaps	[eax+32],	xmm0

		movaps	xmm0,	[eax+48]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; r12 r12 r12 r12
		shufps	xmm1,	xmm1,	0x55	; r13 r13 r13 r13
		shufps	xmm2,	xmm2,	0xaa	; r14 r14 r14 r14
		shufps	xmm3,	xmm3,	0xff	; r15 r15 r15 r15

		mulps	xmm0,	xmm4			; l0*r12 l1*r12 l2*r12 l3*r12
		mulps	xmm1,	xmm5			; l4*r13 l5*r13 l6*r13 l7*r13
		mulps	xmm2,	xmm6			; l8*r14 l9*r14 lA*r14 lB*r14
		mulps	xmm3,	xmm7			; lC*r15 lD*r15 lE*r15 lF*r15
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3

		movaps	[eax+48],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_transform_m4m4_l_ip(float* l44, const float* r44)
{
	__asm{
		mov		eax,	r44
		mov		edx,	l44
		xor		ecx,	ecx
		movaps	xmm4,	[edx]			; l0 l1 l2 l3
		movaps	xmm5,	[edx+16]		; l4 l5 l6 l7
		movaps	xmm6,	[edx+32]		; l8 l9 lA lB
		movaps	xmm7,	[edx+48]		; lC lD lE lF

		movaps	xmm0,	[eax]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; r0 r0 r0 r0
		shufps	xmm1,	xmm1,	0x55	; r1 r1 r1 r1
		shufps	xmm2,	xmm2,	0xaa	; r2 r2 r2 r2
		shufps	xmm3,	xmm3,	0xff	; r3 r3 r3 r3

		mulps	xmm0,	xmm4			; l0*r0 l1*r0 l2*r0 l3*r0
		mulps	xmm1,	xmm5			; l4*r1 l5*r1 l6*r1 l7*r1
		mulps	xmm2,	xmm6			; l8*r2 l9*r2 lA*r2 lB*r2
		mulps	xmm3,	xmm7			; lC*r3 lD*r3 lE*r3 lF*r3
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3

		movaps	[edx],	xmm0

		movaps	xmm0,	[eax+16]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; r4 r4 r4 r4
		shufps	xmm1,	xmm1,	0x55	; r5 r5 r5 r5
		shufps	xmm2,	xmm2,	0xaa	; r6 r6 r6 r6
		shufps	xmm3,	xmm3,	0xff	; r7 r7 r7 r7

		mulps	xmm0,	xmm4			; l0*r4 l1*r4 l2*r4 l3*r4
		mulps	xmm1,	xmm5			; l4*r5 l5*r5 l6*r5 l7*r5
		mulps	xmm2,	xmm6			; l8*r6 l9*r6 lA*r6 lB*r6
		mulps	xmm3,	xmm7			; lC*r7 lD*r7 lE*r7 lF*r7
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3

		movaps	[edx+16],	xmm0

		movaps	xmm0,	[eax+32]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; r8  r8  r8  r8
		shufps	xmm1,	xmm1,	0x55	; r9  r9  r9  r9
		shufps	xmm2,	xmm2,	0xaa	; r10 r10 r10 r10
		shufps	xmm3,	xmm3,	0xff	; r11 r11 r11 r11

		mulps	xmm0,	xmm4			; l0*r8  l1*r8  l2*r8  l3*r8
		mulps	xmm1,	xmm5			; l4*r9  l5*r9  l6*r9  l7*r9
		mulps	xmm2,	xmm6			; l8*r10 l9*r10 lA*r10 lB*r10
		mulps	xmm3,	xmm7			; lC*r11 lD*r11 lE*r11 lF*r11
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3

		movaps	[edx+32],	xmm0

		movaps	xmm0,	[eax+48]
		movaps	xmm1,	xmm0
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm0
		shufps	xmm0,	xmm0,	0x00	; r12 r12 r12 r12
		shufps	xmm1,	xmm1,	0x55	; r13 r13 r13 r13
		shufps	xmm2,	xmm2,	0xaa	; r14 r14 r14 r14
		shufps	xmm3,	xmm3,	0xff	; r15 r15 r15 r15

		mulps	xmm0,	xmm4			; l0*r12 l1*r12 l2*r12 l3*r12
		mulps	xmm1,	xmm5			; l4*r13 l5*r13 l6*r13 l7*r13
		mulps	xmm2,	xmm6			; l8*r14 l9*r14 lA*r14 lB*r14
		mulps	xmm3,	xmm7			; lC*r15 lD*r15 lE*r15 lF*r15
		addps	xmm0,	xmm1
		addps	xmm0,	xmm2
		addps	xmm0,	xmm3

		movaps	[edx+48],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_make_translation(float* m44, float x, float y, float z)
{
	mcemaths_mat4ident(m44);
	m44[12] = x;
	m44[13] = y;
	m44[14] = z;
}

extern "C" MCEMATHAPI(void) mcemaths_make_scaling(float* m44, float x, float y, float z)
{
	mcemaths_mat4ident(m44);
	m44[0] = x;
	m44[5] = y;
	m44[10] = z;
}

// shite, do you know how painful it was to write make_rotation in sse? but now it is proved
// by bench-marking that the c-version is much faster than the sse-version. I have to give it up!
//
// through which I got a principle:
// to an algorithm, SIMD is probably not suitable as long as massive data-rearrangement is required.
// moving and shuffling (and ...) should never outnumber calculation instructions (like addps, mulps, ...)
//
// __declspec(align(16)) const float SIGNS[12] = {1.0f, 1.0f, -1.0f, 0, -1.0f, 1.0f, 1.0f, 0, 1.0f, -1.0f, 1.0f, 0};
// 
// extern "C" MCEMATHAPI(void) mcemaths_make_rotation(float* m44, const float* axis4, float radian)
// {
// 	float c = cos(radian);
// 	float ic = 1.0f - c;
// 	float s = sin(radian);
// 
// 	const float* last_row = IDENTITY4+12;
// 
// 	__asm{
// 		mov		eax,	axis4
// 		movaps	xmm0,	[eax]			; x y z
// 		movaps	xmm1,	xmm0
// 		movaps	xmm2,	xmm0
// 		movaps	xmm3,	xmm0
// 		shufps	xmm1,	xmm1,	0x0		; x x x
// 		shufps	xmm2,	xmm2,	0x55	; y y y
// 		shufps	xmm3,	xmm3,	0xaa	; z z z
// 		movss	xmm4,	ic
// 		shufps	xmm4,	xmm4,	0x0		; ic ic ic
// 		movss	xmm5,	s
// 		shufps	xmm5,	xmm5,	0x0		; s s s
// 		mulps	xmm5,	xmm0			; x*s y*s z*s
// 		movss	xmm6,	c				; c 0 0 0
// 		shufps	xmm6,	xmm5,	0x24	; c 0 z*s 0
// 		shufps	xmm5,	xmm6,	0x24	; x*s y*s z*s c
// 		
// 		mov		eax,	m44
// 		lea		ecx,	SIGNS
// 
// 		movaps	xmm7,	xmm0
// 		mulps	xmm7,	xmm1			; x*x y*x z*x
// 		mulps	xmm7,	xmm4			; x*x*ic y*x*ic z*x*ic
// 		movaps	xmm6,	xmm5
// 		shufps	xmm6,	xmm6,	0x1b	; c z*s y*s
// 		mulps	xmm6,	[ecx]
// 		addps	xmm7,	xmm6			; x*x*ic+c y*x*ic+z*s z*x*ic-y*s
// 		movaps	[eax],	xmm7
// 
// 		movaps	xmm7,	xmm0
// 		mulps	xmm7,	xmm2			; x*y y*y z*y
// 		mulps	xmm7,	xmm4			; x*y*ic y*y*ic z*y*ic
// 		movaps	xmm6,	xmm5
// 		shufps	xmm6,	xmm6,	0x0e	; z*s c x*s
// 		mulps	xmm6,	[ecx+16]
// 		addps	xmm7,	xmm6			; x*y*ic-z*s y*y*ic+c z*y*ic+x*s
// 		movaps	[eax+16],	xmm7
// 
// 		movaps	xmm7,	xmm0
// 		mulps	xmm7,	xmm3			; x*z y*z z*z
// 		mulps	xmm7,	xmm4			; x*z*ic y*z*ic z*z*ic
// 		movaps	xmm6,	xmm5
// 		shufps	xmm6,	xmm6,	0x31	; y*s x*s c
// 		mulps	xmm6,	[ecx+32]
// 		addps	xmm7,	xmm6			; x*y*ic+y*s y*y*ic-x*s z*y*ic+c
// 		movaps	[eax+32],	xmm7
// 
// 		mov		ecx,	last_row
// 		movaps	xmm7,	[ecx]
// 		movaps	[eax+48],	xmm7
// 	}
// }

// the following was written by Richard Davidson
extern "C" MCEMATHAPI(void) mcemaths_make_rotation(float* m44, const float* axis4, float radian)
{
	float c = cos(radian), ic = 1.0f - c;
	float s = sin(radian);

	float sin_axis_x = axis4[0] * s;
	float sin_axis_y = axis4[1] * s;
	float sin_axis_z = axis4[2] * s;

	m44[0]  = (axis4[0] * axis4[0]) * ic + c;
	m44[1]  = (axis4[1] * axis4[0]) * ic + sin_axis_z;
	m44[2]  = (axis4[2] * axis4[0]) * ic - sin_axis_y;
	m44[4]  = (axis4[0] * axis4[1]) * ic - sin_axis_z;
	m44[5]  = (axis4[1] * axis4[1]) * ic + c;
	m44[6]  = (axis4[2] * axis4[1]) * ic + sin_axis_x;
	m44[8]  = (axis4[0] * axis4[2]) * ic + sin_axis_y;
	m44[9]  = (axis4[1] * axis4[2]) * ic - sin_axis_x;
	m44[10] = (axis4[2] * axis4[2]) * ic + c;
	m44[3] = m44[7] = m44[11] = 0;

	memcpy(m44+12, IDENTITY4+12, sizeof(float) * 4);
}

