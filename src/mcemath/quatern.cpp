#include "stdafx.h"
#include "mcemaths.h"

#define USE_SSE2
#pragma warning(disable: 4305)
#include "sse_mathfun.h"
#pragma warning(default: 4305)

extern "C" MCEMATHAPI(void) mcemaths_quatcpy(float* q_d, const float* q_s)
{
	__asm{
		mov		eax,	q_s
		movaps	xmm0,	[eax]
		mov		eax,	q_d
		movaps	[eax],	xmm0
	}
}

extern "C" MCEMATHAPI(void) mcemaths_conjugate_q(float* q)
{
	__declspec(align(16)) const float FACTORS[4] = {-1.0f, -1.0f, -1.0f, 1.0f};
	__asm{
		lea		eax,	FACTORS
		movaps	xmm0,	[eax]
		mov		eax,	q
		movaps	xmm1,	[eax]
		mulps	xmm1,	xmm0
		movaps	[eax],	xmm1
	}
}

__declspec(align(16)) const float CROSS_FACTORS1[4] = { 1.0f,  1.0f,  1.0f, -1.0f};
__declspec(align(16)) const float CROSS_FACTORS2[4] = {-1.0f, -1.0f, -1.0f, -1.0f};
extern "C" MCEMATHAPI(void) mcemaths_cross_q(float* q, const float* q_l, const float* q_r)
{
	/* - base theory - 
	x = w_l * x_r + x_l * w_r + y_l * z_r - z_l * y_r
	y = w_l * y_r + y_l * w_r + z_l * x_r - x_l * z_r
	z = w_l * z_r + z_l * w_r + x_l * y_r - y_l * x_r
	w = w_l * w_r - x_l * x_r - y_l * y_r - z_l * z_r
	*/

	__asm{
		mov		eax,	q_l
		movaps	xmm6,	[eax]
		mov		eax,	q_r
		movaps	xmm7,	[eax]
		lea		eax,	CROSS_FACTORS1
		movaps	xmm5,	[eax]
		lea		eax,	CROSS_FACTORS2
		movaps	xmm4,	[eax]

		movaps	xmm1,	xmm6
		shufps	xmm1,	xmm6,	0xff	; w_l w_l w_l w_l
		mulps	xmm1,	xmm7			; xmm1 - the 1st mul-column
		movaps	xmm2,	xmm6
		shufps	xmm2,	xmm6,	0x24	; x_l y_l z_l x_l
		movaps	xmm3,	xmm7
		shufps	xmm3,	xmm7,	0x3f	; w_r w_r w_r x_r
		mulps	xmm2,	xmm3
		mulps	xmm2,	xmm5			; xmm2 - the 2nd mul-column
		addps	xmm1,	xmm2			; xmm1 += xmm2
		movaps	xmm2,	xmm6
		shufps	xmm2,	xmm6,	0x48	; y_l z_l x_l y_l
		movaps	xmm3,	xmm7
		shufps	xmm3,	xmm7,	0xa2	; z_r x_r y_r y_r
		mulps	xmm2,	xmm3
		mulps	xmm2,	xmm5			; xmm2 - the 3rd mul-column
		addps	xmm1,	xmm2			; xmm1 += xmm2

		movaps	xmm2,	xmm6
		shufps	xmm2,	xmm6,	0x92	; z_l x_l y_l z_l
		movaps	xmm3,	xmm7
		shufps	xmm3,	xmm7,	0xa2	; y_r z_r x_r z_r
		mulps	xmm2,	xmm3
		mulps	xmm2,	xmm4			; xmm2 - the 4th mul-column
		addps	xmm1,	xmm2			; xmm1 += xmm2

		mov		eax,	q
		movaps	[eax],	xmm1
	}
}

extern "C" MCEMATHAPI(void) mcemaths_cross_qv3(float* v4, const float* q_l, const float* v4_r)
{
	/* - base theory - 
	vec' = q * vec * conjugate(q)
	*/
	__declspec(align(16)) float q_l_conj[4];
	__declspec(align(16)) float q_r[4];
	__declspec(align(16)) float temp[4];
	mcemaths_quatcpy(q_l_conj, q_l);
	mcemaths_quatcpy(q_r, v4_r);
	mcemaths_conjugate_q(q_l_conj);
	mcemaths_cross_q(temp, q_r, q_l_conj);
	mcemaths_cross_q(v4, q_l, temp);
}

extern "C" MCEMATHAPI(void) mcemaths_axis2q(float* q, const float* axis4, float angle_rad)
{
	angle_rad /= 2.0f;
	mcemaths_quatcpy(q, axis4);
	mcemaths_mul_3_4(q, sin(angle_rad));
	q[3] = cos(angle_rad);
}

__declspec(align(16)) const float EULAR2Q_FACTORS1[4] = {0.5f, 0.5f, 0.5f, 0.5f};
extern "C" MCEMATHAPI(void) mcemaths_eular2q(float* q, const float* ypr4)
{
	//////////////////////////////////////////////////////////////////////////
	// sincos_ps() was written by Julien Pommier, published at http://gruntthepeon.free.fr/ssemath/, 
	// under zlib's license. His function takes 4 floats to calculate 4 sines and 4cosines simultaneously.
	// His work is in file sse_mathfun.h. I guess it would be better if he used inline asm instead of intrinsics, 
	// because, if so, I wouldn't need a local float array (ypr_trigon) to connect his sse and mine, but 
	// that's not a big deal.
	//
	// Our Eular-to-quaternion function needs both sin and cos of 3 input floats (yaw/pitch/roll) to work,
	// so Julien's function just meets our needs. Doing 6 trigonal calculations by one function call, I 
	// believe we will get bloody benefits!
	//////////////////////////////////////////////////////////////////////////
	__declspec(align(16)) float ypr_trigon[16];
	ypr_trigon[12] = ypr_trigon[15] = 1.0f;
	ypr_trigon[13] = ypr_trigon[14] = -1.0f;
	
	__m128 sines, cosines;
	sincos_ps(_mm_mul_ps(_mm_load_ps(ypr4), _mm_load_ps(EULAR2Q_FACTORS1)), &sines, &cosines);
	__m128	y = _mm_shuffle_ps(cosines, sines, 0x00), 
			p = _mm_shuffle_ps(cosines, sines, 0x11), 
			r = _mm_shuffle_ps(cosines, sines, 0x22);
	_mm_store_ps(ypr_trigon,   y);
	_mm_store_ps(ypr_trigon+4, p);
	_mm_store_ps(ypr_trigon+8, r);

	//////////////////////////////////////////////////////////////////////////
	// now in ypr_trigon[16], data is arranged as:
	// cos(y), cos(y), sin(y), sin(y), cos(p), cos(p), sin(p), sin(p), cos(r), cos(r), sin(r), sin(r), 1, -1, -1, 1
	// the following algorithm was from Richard Davidson's work, in c it is like this:
	//
	// (simplified from q(yaw)*q(pitch)*q(roll))
	// q.x = cosr * sinp * cosy + sinr * cosp * siny;
	// q.y = cosr * cosp * siny - sinr * sinp * cosy;
	// q.z = sinr * cosp * cosy - cosr * sinp * siny;
	// q.w = cosr * cosp * cosy + sinr * sinp * siny;
	//
	// we now can easily shuffle each 'trigonal column' using ypr_trigon[0~9], and do
	// quadruple multiplication happily by sse. the only small pain is shuffling which
	// is inevitable.
	//////////////////////////////////////////////////////////////////////////
	__asm{
		lea		eax,	ypr_trigon
		prefetchNTA		[eax]
		movaps	xmm0,	[eax]
		movaps	xmm1,	[eax+16]
		movaps	xmm2,	[eax+32]
		movaps	xmm7,	[eax+48]

		movaps	xmm3,	xmm2
		shufps	xmm3,	xmm3,	0x20

		movaps	xmm6,	xmm1
		shufps	xmm6,	xmm6,	0x02

		mulps	xmm3,	xmm6

		movaps	xmm6,	xmm0
		shufps	xmm6,	xmm6,	0x08

		mulps	xmm3,	xmm6

		movaps	xmm4,	xmm2
		shufps	xmm4,	xmm4,	0x8a

		movaps	xmm6,	xmm1
		shufps	xmm6,	xmm6,	0xa8

		mulps	xmm4,	xmm6

		movaps	xmm6,	xmm0
		shufps	xmm6,	xmm6,	0xa2

		mulps	xmm4,	xmm6

		mulps	xmm4,	xmm7

		addps	xmm3,	xmm4

		mov		eax,	q
		movaps	[eax],	xmm3
	}
}

_declspec(align(16)) const float Q2M_FACTORS1[4] = { 1.0f,     0,     0,     0};
_declspec(align(16)) const float Q2M_FACTORS2[4] = {    0,  1.0f,     0,     0};
_declspec(align(16)) const float Q2M_FACTORS3[4] = {    0,     0,  1.0f,     0};
_declspec(align(16)) const float Q2M_FACTORS4[4] = {-2.0f,  2.0f,  2.0f,     0};
_declspec(align(16)) const float Q2M_FACTORS5[4] = { 2.0f, -2.0f,  2.0f,     0};
_declspec(align(16)) const float Q2M_FACTORS6[4] = { 2.0f,  2.0f, -2.0f,     0};
_declspec(align(16)) const float Q2M_FACTORS7[4] = {-2.0f,  2.0f, -2.0f,     0};
_declspec(align(16)) const float Q2M_FACTORS8[4] = {-2.0f, -2.0f,  2.0f,     0};
_declspec(align(16)) const float Q2M_FACTORS9[4] = { 2.0f, -2.0f, -2.0f,     0};
extern "C" MCEMATHAPI(void) mcemaths_q2matrix(float* m44, const float* q)
{
	/*
	float yy = y*y;
	float zz = z*z;
	float xy = x*y;
	float zw = z*w;
	float xz = x*z;
	float yw = y*w;
	float xx = x*x;
	float yz = y*z;
	float xw = x*w;

	mat.values[0] =  1 - 2*yy - 2*zz;
	mat.values[1] =      2*xy + 2*zw;
	mat.values[2] =      2*xz - 2*yw;	

	mat.values[4] =      2*xy - 2*zw;
	mat.values[5] =  1 - 2*xx - 2*zz;
	mat.values[6] =      2*yz + 2*xw;

	mat.values[8] =      2*xz + 2*yw;
	mat.values[9] =      2*yz - 2*xw;
	mat.values[10] = 1 - 2*xx - 2*yy;
	*/

	__asm{
		mov		eax,	q
		movaps	xmm0,	[eax]
		mov		edx,	m44

		movaps	xmm1,	xmm0
		movaps	xmm7,	xmm0
		shufps	xmm1,	xmm1,	0x01	; y x x
		shufps	xmm7,	xmm7,	0x25	; y y z
		mulps	xmm1,	xmm7
		lea		eax,	Q2M_FACTORS4	; -2 2 2
		movaps	xmm7,	[eax]
		mulps	xmm1,	xmm7

		movaps	xmm2,	xmm0
		movaps	xmm7,	xmm0
		shufps	xmm2,	xmm2,	0x1a	; z z y
		shufps	xmm7,	xmm7,	0x3e	; z w w
		mulps	xmm2,	xmm7
		lea		eax,	Q2M_FACTORS7	; -2 2 -2
		movaps	xmm7,	[eax]
		mulps	xmm2,	xmm7

		addps	xmm1,	xmm2
		lea		eax,	Q2M_FACTORS1	; 1 0 0
		movaps	xmm2,	[eax]
		addps	xmm1,	xmm2

		movaps	[edx],	xmm1

		movaps	xmm1,	xmm0
		movaps	xmm7,	xmm0
		shufps	xmm1,	xmm1,	0x10	; x x y
		shufps	xmm7,	xmm7,	0x21	; y x z
		mulps	xmm1,	xmm7
		lea		eax,	Q2M_FACTORS5	; 2 -2 2
		movaps	xmm7,	[eax]
		mulps	xmm1,	xmm7

		movaps	xmm2,	xmm0
		movaps	xmm7,	xmm0
		shufps	xmm2,	xmm2,	0x0a	; z z x
		shufps	xmm7,	xmm7,	0x3b	; w z w
		mulps	xmm2,	xmm7
		lea		eax,	Q2M_FACTORS8	; -2 -2 2
		movaps	xmm7,	[eax]
		mulps	xmm2,	xmm7

		addps	xmm1,	xmm2
		lea		eax,	Q2M_FACTORS2	; 0 1 0
		movaps	xmm2,	[eax]
		addps	xmm1,	xmm2

		movaps	[edx+16],	xmm1

		movaps	xmm1,	xmm0
		movaps	xmm7,	xmm0
		shufps	xmm1,	xmm1,	0x04	; x y x
		shufps	xmm7,	xmm7,	0x0a	; z z x
		mulps	xmm1,	xmm7
		lea		eax,	Q2M_FACTORS6	; 2 2 -2
		movaps	xmm7,	[eax]
		mulps	xmm1,	xmm7

		movaps	xmm2,	xmm0
		movaps	xmm7,	xmm0
		shufps	xmm2,	xmm2,	0x11	; y x y
		shufps	xmm7,	xmm7,	0x1f	; w w y
		mulps	xmm2,	xmm7
		lea		eax,	Q2M_FACTORS9	; 2 -2 -2
		movaps	xmm7,	[eax]
		mulps	xmm2,	xmm7

		addps	xmm1,	xmm2
		lea		eax,	Q2M_FACTORS3	; 0 0 1
		movaps	xmm2,	[eax]
		addps	xmm1,	xmm2

		movaps	[edx+32],	xmm1

		mov		eax,	IDENTITY4_LASTROW
		movaps	xmm0,	[eax]
		movaps	[edx+48],	xmm0
	}
}

_declspec(align(16)) const float M2Q_FACTORS1[4] = { 1.0f,  2.0f,     0,     0};
_declspec(align(16)) const float M2Q_FACTORS2[4] = { 1.0f, -1.0f, -1.0f,  1.0f};
_declspec(align(16)) const float M2Q_FACTORS3[4] = {-1.0f,  1.0f, -1.0f,  1.0f};
_declspec(align(16)) const float M2Q_FACTORS4[4] = {-1.0f, -1.0f,  1.0f,  1.0f};
extern "C" MCEMATHAPI(void) mcemaths_matrix2q(float* q, const float* m44)
{
	/*
	q.x = sqrt( max( 0.0f, (1.0f + m.values[0] - m.values[5] - m.values[10]) ) ) / 2;
	q.y = sqrt( max( 0.0f, (1.0f - m.values[0] + m.values[5] - m.values[10]) ) ) / 2;
	q.z = sqrt( max( 0.0f, (1.0f - m.values[0] - m.values[5] + m.values[10]) ) ) / 2;
	q.w = sqrt( max( 0.0f, (1.0f + m.values[0] + m.values[5] + m.values[10]) ) ) / 2;

	q.x = (float)_copysign( q.x, m.values[9] - m.values[6] );
	q.y = (float)_copysign( q.y, m.values[2] - m.values[8] );
	q.z = (float)_copysign( q.z, m.values[4] - m.values[1] );
	*/

	__asm{
		lea		eax,	M2Q_FACTORS1
		movaps	xmm0,	[eax]
		movaps	xmm1,	xmm0
		shufps	xmm0,	xmm0,	0x00	; 1 1 1 1
		shufps	xmm1,	xmm1,	0x55	; 2 2 2 2

		mov		eax,	m44

		movaps	xmm2,	[eax]
		shufps	xmm2,	xmm2,	0x00	; m[0] m[0] m[0] m[0]
		lea		ecx,	M2Q_FACTORS2
		movaps	xmm7,	[ecx]
		mulps	xmm2,	xmm7

		addps	xmm0,	xmm2

		movaps	xmm2,	[eax+16]
		shufps	xmm2,	xmm2,	0x55	; m[1] m[1] m[1] m[1]
		lea		ecx,	M2Q_FACTORS3
		movaps	xmm7,	[ecx]
		mulps	xmm2,	xmm7

		addps	xmm0,	xmm2

		movaps	xmm2,	[eax+32]
		shufps	xmm2,	xmm2,	0xaa	; m[2] m[2] m[2] m[2]
		lea		ecx,	M2Q_FACTORS4
		movaps	xmm7,	[ecx]
		mulps	xmm2,	xmm7

		addps	xmm0,	xmm2

		xorps	xmm2,	xmm2
		maxps	xmm0,	xmm2
		sqrtps	xmm0,	xmm0
		divps	xmm0,	xmm1

		mov		eax,	q
		movaps	[eax],	xmm0
	}

	q[0] = (float)_copysign(q[0], m44[9] - m44[6]);
	q[1] = (float)_copysign(q[1], m44[2] - m44[8]);
	q[2] = (float)_copysign(q[2], m44[4] - m44[1]);
}

/*
make direction vector from a rotation quaternion based on z-negative:

dir = transform(q, (0, 0, -1))

because:	dir = (-m[2], -m[6], -m[10]) where m is a matrix containing only rotation,
and the conversion from quaternion to matrix is:
			-m[2]  = -2xz -2yw +0
			-m[6]  = -2yz +2xw +0
			-m[10] =  2xx +2yy -1
so:
		|-2| * | x| * | z| + |-2| * | y| * | w| + | 0|
dir =	|-2| * | y| * | z| + | 2| * | x| * | w| + | 0|
		| 2| * | x| * | x| + | 2| * | y| * | y| + |-1|
		xmm4 * xmm0 * xmm1 + xmm5 * xmm2 * xmm3 + xmm6
*/
extern "C" MCEMATHAPI(void) mcemaths_dir_from_quat(float* dir4, const float* quat, float len)
{
	ALIGN16 const float FACTORS1[4] = {-2.0f, -2.0f,  2.0f, 0};
	ALIGN16 const float FACTORS2[4] = {-2.0f,  2.0f,  2.0f, 0};
	ALIGN16 const float FACTORS3[4] = {    0,     0, -1.0f, 0};
	const float* factors1 = FACTORS1;
	const float* factors2 = FACTORS2;
	const float* factors3 = FACTORS3;

	__asm{
		; load quaternion
		mov		eax,	quat
		movaps	xmm7,	[eax]
		; xmm0 = x y x
		movaps	xmm0,	xmm7
		shufps	xmm0,	xmm0,	0x04
		; xmm1 = z z x
		movaps	xmm1,	xmm7
		shufps	xmm1,	xmm1,	0x0A
		; xmm2 = y x y
		movaps	xmm2,	xmm7
		shufps	xmm2,	xmm2,	0x11
		; xmm3 = w w y
		movaps	xmm3,	xmm7
		shufps	xmm3,	xmm3,	0x1F
		; xmm4 = -2 -2  2
		lea		eax,	FACTORS1
		movaps	xmm4,	[eax]
		; xmm5 = -2  2  2
		lea		eax,	FACTORS2
		movaps	xmm5,	[eax]
		; xmm6 =  0  0 -1
		lea		eax,	FACTORS3
		movaps	xmm6,	[eax]
		; xmm7 = xmm4 * xmm0 * xmm1 + xmm5 * xmm2 * xmm3 + xmm6   (^_^)
		mulps	xmm4,	xmm0
		mulps	xmm4,	xmm1
		mulps	xmm5,	xmm2
		mulps	xmm5,	xmm3
		movaps	xmm7,	xmm4
		addps	xmm7,	xmm5
		addps	xmm7,	xmm6
		; apply length
		movss	xmm0,	len
		shufps	xmm0,	xmm0,	0x0
		mulps	xmm7,	xmm0
		; output
		mov		eax,	dir4
		movaps	[eax],	xmm7
	}
}

