#include "stdafx.h"
#include "mcemaths.h"

extern "C" MCEMATHAPI(void) mcemaths_add_3_4(float* r4, const float* v4_1, const float* v4_2)
{
	__asm{
		mov		eax,	v4_1
		movaps	xmm0,	[eax]
		mov		eax,	v4_2
		movaps	xmm1,	[eax]
		addps	xmm0,	xmm1
		mov		eax,	r4
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_sub_3_4(float* r4, const float* v4_l, const float* v4_r)
{
	__asm{
		mov		eax,	v4_l
		movaps	xmm0,	[eax]
		mov		eax,	v4_r
		movaps	xmm1,	[eax]
		subps	xmm0,	xmm1
		mov		eax,	r4
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_add_3_4_ip(float* v4_1, const float* v4_2)
{
	__asm{
		mov		eax,	v4_2
		movaps	xmm1,	[eax]
		mov		eax,	v4_1
		movaps	xmm0,	[eax]
		addps	xmm0,	xmm1
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_sub_3_4_ip(float* v4_l, const float* v4_r)
{
	__asm{
		mov		eax,	v4_r
		movaps	xmm1,	[eax]
		mov		eax,	v4_l
		movaps	xmm0,	[eax]
		subps	xmm0,	xmm1
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(float) mcemaths_dot_3_4(const float* v4_1, const float* v4_2)
{
	__declspec(align(16)) float result[4];
	__asm{
		mov		eax,	v4_1
		movaps	xmm0,	[eax]
		mov		eax,	v4_2
		movaps	xmm1,	[eax]
		mulps	xmm0,	xmm1		; multiply four floats

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; for SS3 processor
		haddps	xmm0,	xmm0
		haddps	xmm0,	xmm0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; alternative version for non-SSE3 processor
;		movhlps	xmm1,	xmm0
;		addps	xmm0,	xmm1		; [0]=[0]+[2] and [1]=[1]+[3]
;		movaps	xmm1,	xmm0
;		shufps	xmm1,	xmm1,	0x1	; xmm1[0] = [1]
;		addps	xmm0,	xmm1		; xmm0[0] = [0]+[1]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		lea		eax,	result
		movaps	[eax],	xmm0
	}
	return result[0];
}

extern "C" MCEMATHAPI(void) mcemaths_cross_3(float* r4, const float* v4_l, const float* v4_r)
{
	/*
	r0 = a1 x b2 - a2 x b1
	r1 = a2 x b0 - a0 x b2
	r2 = a0 x b1 - a1 x b0
	r3 = a3 x b3 - a3 x b3 <= incorrect, but can output [3] as zero if the input [3]s are zero
	xmm0 : a[1 2 0 -]
	xmm1 : b[2 0 1 -]
	xmm2 : a[2 0 1 -]
	xmm3 : b[1 2 0 -]
	result = xmm0 x xmm1 - xmm2 x xmm3
	*/
	__asm{
		mov		eax,	v4_l
		movaps	xmm0,	[eax]
		mov		eax,	v4_r
		movaps	xmm1,	[eax]
		movaps	xmm2,	xmm0
		movaps	xmm3,	xmm1
		shufps	xmm0,	xmm0,	 0xc9	; a1 a2 a0 a3
		shufps	xmm1,	xmm1,	 0xd2	; b2 b0 b1 b3
		shufps	xmm2,	xmm2,	 0xd2	; a2 a0 a1 a3
		shufps	xmm3,	xmm3,	 0xc9	; b1 b2 b0 b3
		mulps	xmm0,	xmm1
		mulps	xmm2,	xmm3
		subps	xmm0,	xmm2
		mov		eax,	r4
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_mul_3_4(float* r4, float fac)
{
	__asm{
		mov		eax,	r4
		movaps	xmm0,	[eax]
		movss	xmm1,	fac
		shufps	xmm1,	xmm1,	0x00
		mulps	xmm0,	xmm1
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_div_3_4(float* r4, float fac)
{
 	__asm{
 		mov		eax,	r4
 		movaps	xmm0,	[eax]
 		movss	xmm1,	fac
 		shufps	xmm1,	xmm1,	0x00
 		rcpps	xmm1,	xmm1
 		mulps	xmm0,	xmm1
 		movaps	[eax],	xmm0
 	}
}

extern "C" MCEMATHAPI(void) mcemaths_mulvec_3_4(float* r4, const float* fac4)
{
	__asm{
		mov		eax,	r4
		movaps	xmm0,	[eax]
		mov		ecx,	fac4
		movaps	xmm1,	[ecx]
		mulps	xmm0,	xmm1
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_divvec_3_4(float* r4, const float* fac4)
{
	__asm{
		mov		eax,	r4
		movaps	xmm0,	[eax]
		mov		ecx,	fac4
		movaps	xmm1,	[ecx]
		rcpps	xmm1,	xmm1
		mulps	xmm0,	xmm1
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(float) mcemaths_len_3_4(const float* v4)
{
//	return sqrt(mcemaths_dot4(v4, v4));
	__declspec(align(16)) float result[4];
	__asm{
		mov		eax,	v4
		movaps	xmm0,	[eax]
		mulps	xmm0,	xmm0		; multiply four floats

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; for SS3 processor
		haddps	xmm0,	xmm0
		haddps	xmm0,	xmm0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; alternative version for non-SSE3 processor
;		movhlps	xmm1,	xmm0
;		addps	xmm0,	xmm1		; [0]=[0]+[2] and [1]=[1]+[3]
;		movaps	xmm1,	xmm0
;		shufps	xmm1,	xmm1,	0x1	; xmm1[0] = [1]
;		addps	xmm0,	xmm1		; xmm0[0] = [0]+[1]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

		sqrtss	xmm0,	xmm0
		lea		eax,	result
		movaps	[eax],	xmm0
	}
	return result[0];
}

extern "C" MCEMATHAPI(void) mcemaths_norm_3_4(float* v4)
{
	// rsqrtss really benefits us a lot
	__asm{
		mov		eax,	v4
		movaps	xmm0,	[eax]
		movaps	xmm2,	xmm0
		mulps	xmm0,	xmm0		; multiply four floats
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; for SS3 processor
		haddps	xmm0,	xmm0
		haddps	xmm0,	xmm0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; alternative version for non-SSE3 processor
;		movhlps	xmm1,	xmm0
;		addps	xmm0,	xmm1		; [0]=[0]+[2] and [1]=[1]+[3]
;		movaps	xmm1,	xmm0
;		shufps	xmm1,	xmm1,	0x1	; xmm1[0] = [1]
;		addps	xmm0,	xmm1		; xmm0[0] = [0]+[1]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		rsqrtss	xmm0,	xmm0
		shufps	xmm0,	xmm0,	0x0	; make 4 copies of the sqrt
		mulps	xmm2,	xmm0
		movaps	[eax],	xmm2
	}
}

extern "C" MCEMATHAPI(void) mcemaths_set_vec_ary(float* ary4, int count, const float* v4)
{
	if(1 == count)
	{
		__asm{
			mov		eax,	v4
			movaps	xmm0,	[eax]
			mov		eax,	ary4
			movaps	[eax],	xmm0
		}
	}
	else
	{
		__asm{
			mov		eax,	v4
			movaps	xmm0,	[eax]
			mov		eax,	count
			xor		ecx,	ecx
			mov		edx,	ary4
lup:
			movaps	[edx],	xmm0
			add		edx,	16
			inc		ecx
			cmp		ecx,	eax
			jl		lup
		}
	}
}

extern "C" MCEMATHAPI(void) mcemaths_zero_vec_ary(float* ary4, int count)
{
	if(1 == count)
	{
		__asm{
			xorps	xmm0,	xmm0
			mov		eax,	ary4
			movaps	[eax],	xmm0
		}
	}
	else
	{
		__asm{
			xorps	xmm0,	xmm0
			mov		eax,	count
			xor		ecx,	ecx
			mov		edx,	ary4
lup:
			movaps	[edx],	xmm0
			add		edx,	16
			inc		ecx
			cmp		ecx,	eax
			jl		lup
		}
	}
}

extern "C" MCEMATHAPI(void) mcemaths_minpos_3_4(float* r4, const float* v4_1, const float* v4_2)
{
	__asm{
		mov		eax,	v4_1
		movaps	xmm0,	[eax]
		mov		eax,	v4_2
		minps	xmm0,	[eax]
		mov		eax,	r4
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_minpos_3_4_ip(float* v4_1, const float* v4_2)
{
	__asm{
		mov		eax,	v4_1
		movaps	xmm0,	[eax]
		mov		ecx,	v4_2
		minps	xmm0,	[ecx]
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_maxpos_3_4(float* r4, const float* v4_1, const float* v4_2)
{
	__asm{
		mov		eax,	v4_1
		movaps	xmm0,	[eax]
		mov		eax,	v4_2
		maxps	xmm0,	[eax]
		mov		eax,	r4
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_maxpos_3_4_ip(float* v4_1, const float* v4_2)
{
	__asm{
		mov		eax,	v4_1
		movaps	xmm0,	[eax]
		mov		ecx,	v4_2
		maxps	xmm0,	[ecx]
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_minmax_3_4_ip(float* min4, float* max4, const float* ref4)
{
	__asm{
		mov		eax,	min4
		movaps	xmm0,	[eax]
		mov		ecx,	max4
		movaps	xmm1,	[ecx]
		mov		edx,	ref4
		movaps	xmm2,	[edx]
		minps	xmm0,	xmm2
		maxps	xmm1,	xmm2
		movaps	[eax],	xmm0
		movaps	[ecx],	xmm1
	}
}

extern "C" MCEMATHAPI(void) mcemaths_clamp_3_4(float* v4, float min, float max)
{
	__asm{
		mov		eax,	v4
		movaps	xmm0,	[eax]
		movss	xmm1,	min
		shufps	xmm1,	xmm1,	0x00
		maxps	xmm0,	xmm1
		movss	xmm1,	max
		shufps	xmm1,	xmm1,	0x00
		minps	xmm0,	xmm1
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_floor_3_4(float* v4, float min)
{
	__asm{
		mov		eax,	v4
		movaps	xmm0,	[eax]
		movss	xmm1,	min
		shufps	xmm1,	xmm1,	0x00
		maxps	xmm0,	xmm1
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_ceiling_3_4(float* v4, float max)
{
	__asm{
		mov		eax,	v4
		movaps	xmm0,	[eax]
		movss	xmm1,	max
		shufps	xmm1,	xmm1,	0x00
		minps	xmm0,	xmm1
		movaps	[eax],	xmm0
	}
}

ALIGN16 const float HALF_VECTOR[] = {0.5f, 0.5f, 0.5f, 0.5f};
extern "C" MCEMATHAPI(void) mcemaths_line_centre(float* c4, float* r, const float* e4_1, const float* e4_2)
{
	ALIGN16 float temp[4];
	__asm{
		mov		eax,	e4_1
		movaps	xmm0,	[eax]
		mov		eax,	e4_2
		movaps	xmm1,	[eax]
		lea		eax,	HALF_VECTOR
		movaps	xmm2,	[eax]
		subps	xmm0,	xmm1
		mulps	xmm0,	xmm2
		addps	xmm1,	xmm0
		mov		eax,	c4
		movaps	[eax],	xmm1
		mov		eax,	r
		sub		eax,	0
		jz		quit
		; calc squared length
		mulps	xmm0,	xmm0
		movhlps	xmm1,	xmm0
		addps	xmm0,	xmm1
		movaps	xmm1,	xmm0
		shufps	xmm1,	xmm1,	0x1	; xmm1[0] = [1]
		addps	xmm0,	xmm1		; xmm0[0] = [0]+[1]
		lea		eax,	temp
		movaps	[eax],	xmm0
	}
	*r = sqrt(temp[0]);
quit:;
}

extern "C" MCEMATHAPI(bool) mcemaths_equalvec_3(const float* v4_1, const float* v4_2)
{
// 	ALIGN16 long r[4];
// 	__asm{
// 		mov		eax,	v4_1
// 		movaps	xmm0,	[eax]
// 		mov		eax,	v4_2
// 		cmpeqps	xmm0,	[eax]
// 		lea		eax,	[r]
// 		movaps	[eax],	xmm0
// 	}
// 	return ((0xFFFFFFFF == r[0]) && (0xFFFFFFFF == r[1]) && (0xFFFFFFFF == r[2]));
	return ((*((__int64*)v4_1) == *((__int64*)v4_2)) && (*((long*)(v4_1 + 2)) == *((long*)(v4_2 + 2))));
}

extern "C" MCEMATHAPI(bool) mcemaths_equalvec_4(const float* v4_1, const float* v4_2)
{
// 	ALIGN16 __int64 r[4];
// 	__asm{
// 		mov		eax,	v4_1
// 		movaps	xmm0,	[eax]
// 		mov		eax,	v4_2
// 		cmpeqps	xmm0,	[eax]
// 		lea		eax,	[r]
// 		movaps	[eax],	xmm0
// 	}
// 	return ((0xFFFFFFFFFFFFFFFF == r[0]) && (0xFFFFFFFFFFFFFFFF == r[1]));
	return ((*((__int64*)v4_1) == *((__int64*)v4_2)) && (*((__int64*)(v4_1 + 2)) == *((__int64*)(v4_2 + 2))));
}

extern "C" MCEMATHAPI(void) mcemaths_add_3(float* r4, const float* v4_1, const float* v4_2)
{
	r4[0] = v4_1[0] + v4_2[0];
	r4[1] = v4_1[1] + v4_2[1];
	r4[2] = v4_1[2] + v4_2[2];
// 	__asm{
// 		mov		eax,	v4_1
// 		movaps	xmm0,	[eax]
// 		mov		eax,	v4_2
// 		movaps	xmm1,	[eax]
// 		addps	xmm0,	xmm1
// 		mov		eax,	r4
// 		movaps	[eax],	xmm0
// 	}
}

extern "C" MCEMATHAPI(void) mcemaths_sub_3(float* r4, const float* v4_l, const float* v4_r)
{
	r4[0] = v4_l[0] - v4_r[0];
	r4[1] = v4_l[1] - v4_r[1];
	r4[2] = v4_l[2] - v4_r[2];
// 	__asm{
// 		mov		eax,	v4_l
// 		movaps	xmm0,	[eax]
// 		mov		eax,	v4_r
// 		movaps	xmm1,	[eax]
// 		subps	xmm0,	xmm1
// 		mov		eax,	r4
// 		movaps	[eax],	xmm0
// 	}
}

extern "C" MCEMATHAPI(void) mcemaths_add_3_ip(float* v4_1, const float* v4_2)
{
	v4_1[0] += v4_2[0];
	v4_1[1] += v4_2[1];
	v4_1[2] += v4_2[2];
}

extern "C" MCEMATHAPI(void) mcemaths_sub_3_ip(float* v4_l, const float* v4_r)
{
	v4_l[0] -= v4_r[0];
	v4_l[1] -= v4_r[1];
	v4_l[2] -= v4_r[2];
}

extern "C" MCEMATHAPI(void) mcemaths_mul_3(float* r4, float fac)
{
// 	float for_w = 1.0f;
// 	__asm{
// 		mov		eax,	r4
// 		movaps	xmm0,	[eax]
// 		movss	xmm1,	fac
// 		shufps	xmm1,	xmm1,	0x00	; fac fac fac fac
// 		movss	xmm2,	for_w
// 		movss	xmm1,	xmm2
// 		shufps	xmm1,	xmm1,	0x2a	; fac fac fac 1.0
// 		mulps	xmm0,	xmm1			;  x   y   z   w
// 		movaps	[eax],	xmm0
// 	}
	r4[0] *= fac;
	r4[1] *= fac;
	r4[2] *= fac;
}

extern "C" MCEMATHAPI(void) mcemaths_div_3(float* r4, float fac)
{
// 	float for_w = 1.0f;
// 	__asm{
// 		mov		eax,	r4
// 		movaps	xmm0,	[eax]
// 		movss	xmm1,	fac
// 		shufps	xmm1,	xmm1,	0x00	; fac fac fac fac
// 		movss	xmm2,	for_w
// 		movss	xmm1,	xmm2
// 		shufps	xmm1,	xmm1,	0x2a	; fac fac fac 1.0
// 		rcpps	xmm1,	xmm1			; reciprocate them
// 		mulps	xmm0,	xmm1			;  x   y   z   w
// 		movaps	[eax],	xmm0
// 	}
	fac = 1.0f / fac;
	r4[0] *= fac;
	r4[1] *= fac;
	r4[2] *= fac;
}

extern "C" MCEMATHAPI(void) mcemaths_add_1to4(float* r4, float a)
{
	__asm{
		mov		eax,	r4
		movaps	xmm0,	[eax]
		movss	xmm1,	a
		shufps	xmm1,	xmm1,	0x00
		addps	xmm0,	xmm1
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_sub_4by1(float* r4, float a)
{
	__asm{
		mov		eax,	r4
		movaps	xmm0,	[eax]
		movss	xmm1,	a
		shufps	xmm1,	xmm1,	0x00
		subps	xmm0,	xmm1
		movaps	[eax],	xmm0
	}
}