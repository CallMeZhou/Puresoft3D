/****************************************************************************/
/* To whom it may concern,                                                  */
/*                                                                          */
/* I got a lovely stuff the other day, it is called "ZHOU's Fast 3D Maths   */
/* Library". Never heard of it yet? Let me give you some ideas.             */
/*                                                                          */
/* This work is based on:                                                   */
/* (1) Richard Davidson's OpenGL framework;                                 */
/* (2) Julien Pommier's SSE2 trigonal procedures;                           */
/* (3) Groove's SSE matrix inverse procedure.                               */
/*                                                                          */
/* Richard wrote a set of 3d maths classes for teaching purpose, it is      */
/* clear and complete, however, not having good performance. I know it      */
/* is because fast code is hard to read. But it would be good to have       */
/* a set of really fast maths functions when you are working on something   */
/* commercial, so this library comes.                                       */
/*                                                                          */
/* All procedures in this library were written by SSE/SSE2 instructions,    */
/* and tested against control group written in common C statements.         */
/* Adjustment has been taken according to the test result, so the           */
/* performance is sure to be no worse than the C version and it can         */
/* completely replace it.                                                   */
/*                                                                          */
/* To use them, you just have to ensure three things:                       */
/* (1) length of vectors must be 4 even if you just care the former 3;      */
/*                                                                          */
/* (2) all buffers must be 16-bytes aligned. use __declspec(align(16)) for  */
/*     stack stuffs and _aligned_malloc() for heap allocation;              */
/*                                                                          */
/* (3) it is your responsibility to ensure the divisor to be none zero,     */
/*     notice that mcemaths_divvec_3_4() requires the place holder to be    */
/*     none zero too.                                                       */
/*                                                                          */
/* Some vector procedures named with suffix '_3_4' work for both 3d and 4d  */
/* vectors, but that is not a matter of maths, it is just because SSE       */
/* instructions calculate 4 floats at once.                                 */
/*                                                                          */
/* You should take all constant or by-ref arguments as inputs, and          */
/* non-constant pointer arguments as outputs. A function does IN-PLACE      */
/* MODIFICATION if it has only one non-constant pointer argument, or, it    */
/* is named with suffix "_ip". For those non-in-place functions, the output */
/* pointer CANNOT equal to any of the inputs. This will not be mentioned    */
/* again in following comments.                                             */
/*                                                                          */
/* One last thing, YOU WILL PROBABLY GET WRONG OUTPUT IF you skip those     */
/* 'NOTICE's in following comments.                                         */
/*                                                                          */
/* Ok, no more rigmarole, hope you enjoy it.                                */
/*                                                                          */
/* Sincerely yours,                                                         */
/*                                                                          */
/* Chong Zhou (agedboy@gmail.com)                                           */
/* B1022199, Computing School                                               */
/* University of Newcastle upon Tyne                                        */
/* Wednesday, 30 May 2012                                                   */
/*                                                                          */
/****************************************************************************/
#pragma once
#define ALIGN16 __declspec(align(16))

#ifndef _LIB
#ifdef _DEBUG
#pragma comment(lib, "mcemathsd")
#else
#pragma comment(lib, "mcemaths")
#endif
#endif

#define MCEMATHAPI(ret) ret _stdcall

#ifdef __cplusplus
extern "C"{
#endif

/************************************************************************/
/*   vector 3d/4d                                                       */
/************************************************************************/

/* Add / subtract two 3d vectors, or 4d vectors, or quaternions. */
MCEMATHAPI(void) mcemaths_add_3_4(float* r4, const float* v4_1, const float* v4_2);
MCEMATHAPI(void) mcemaths_sub_3_4(float* r4, const float* v4_l, const float* v4_r);
MCEMATHAPI(void) mcemaths_add_3_4_ip(float* v4_1, const float* v4_2);
MCEMATHAPI(void) mcemaths_sub_3_4_ip(float* v4_l, const float* v4_r);

/* Calculate dot product of two 3d vectors, or 4d vectors, or quaternions.
   NOTICE: The forth elements of the inputs MUST be zero for 3d vectors.*/
MCEMATHAPI(float) mcemaths_dot_3_4(const float* v4_1, const float* v4_2);

/* Calculate cross product of two 3d vectors. It does not work for 4d vectors. 'v4_l' is the left one, 'v4_r' is the right one. 
   NOTICE: The forth elements of the inputs are nice to be zero, unless you ignore it in the output.*/
MCEMATHAPI(void) mcemaths_cross_3(float* r4, const float* v4_l, const float* v4_r);

/* Multiply / divide 3d vector, or 4d vector, or quaternion by a scalar. fac should not be zero for division. */
MCEMATHAPI(void) mcemaths_mul_3_4(float* r4, float fac);
MCEMATHAPI(void) mcemaths_div_3_4(float* r4, float fac);

/* Multiply / divide 3d vector, or 4d vector, or quaternion by four scalars. None of fac4 elements should be zero. 
   NOTICE: For 3d vectors, even the forth place-holding element in fac4 must not be zero. Check this if your 
   programme runs at an weird low speed. */
MCEMATHAPI(void) mcemaths_mulvec_3_4(float* r4, const float* fac4);
MCEMATHAPI(void) mcemaths_divvec_3_4(float* r4, const float* fac4);

/* Calculate length of 3d vectors, or 4d vectors.
   NOTICE: The forth elements of the inputs must be zero for 3d vectors.*/
MCEMATHAPI(float) mcemaths_len_3_4(const float* v4);

/* Normalize 3d vector, or 4d vector, or quaternion.
   NOTICE: The forth elements of the inputs must be zero for 3d vectors.*/
MCEMATHAPI(void) mcemaths_norm_3_4(float* v4);

/* I guess large array of vectors must be very popular in your programme, isn't it? */
MCEMATHAPI(void) mcemaths_set_vec_ary(float* ary4, int count, const float* v4);
MCEMATHAPI(void) mcemaths_zero_vec_ary(float* ary4, int count);

/* Find minimum / maximum values for all four elements of two vectors, useful for finding
   bounding box. */
MCEMATHAPI(void) mcemaths_minpos_3_4(float* r4, const float* v4_1, const float* v4_2);
MCEMATHAPI(void) mcemaths_minpos_3_4_ip(float* v4_1, const float* v4_2);
MCEMATHAPI(void) mcemaths_maxpos_3_4(float* r4, const float* v4_1, const float* v4_2);
MCEMATHAPI(void) mcemaths_maxpos_3_4_ip(float* v4_1, const float* v4_2);
MCEMATHAPI(void) mcemaths_minmax_3_4_ip(float* min4, float* max4, const float* ref4);

/* Find centre point of line segment. c4 is centre point, r is radius scalar. r can be NULL if not wanted */
MCEMATHAPI(void) mcemaths_line_centre(float* c4, float* r, const float* e4_1, const float* e4_2);

/* Check if two vectors are identical */
MCEMATHAPI(bool) mcemaths_equalvec_3(const float* v4_1, const float* v4_2);
MCEMATHAPI(bool) mcemaths_equalvec_4(const float* v4_1, const float* v4_2);

/* DEPRECATED. The following four have the similar function as those of _3_4 version, but work only for 3d vectors, 
   so all the forth elements are completely ignored. Use them only when you find taking care of the forth elements 
   really wasting time. */
MCEMATHAPI(void) mcemaths_add_3(float* r4, const float* v4_1, const float* v4_2);
MCEMATHAPI(void) mcemaths_sub_3(float* r4, const float* v4_l, const float* v4_r);
MCEMATHAPI(void) mcemaths_add_3_ip(float* v4_1, const float* v4_2);
MCEMATHAPI(void) mcemaths_sub_3_ip(float* v4_l, const float* v4_r);
MCEMATHAPI(void) mcemaths_mul_3(float* r4, float fac);
MCEMATHAPI(void) mcemaths_div_3(float* r4, float fac);

/* Clamp */
MCEMATHAPI(void) mcemaths_clamp_3_4(float* v4, float min, float max);
MCEMATHAPI(void) mcemaths_floor_3_4(float* v4, float min);
MCEMATHAPI(void) mcemaths_ceiling_3_4(float* v4, float max);

/* Misc. */
MCEMATHAPI(void) mcemaths_add_1to4(float* r4, float a);
MCEMATHAPI(void) mcemaths_sub_4by1(float* r4, float a);

/************************************************************************/
/*   matrix 4x4 (matrix are stored in column-major order)               */
/************************************************************************/

/* Reset matrix to identity. */
MCEMATHAPI(void) mcemaths_mat4ident(float* m44);

/* Transpose matrix. */
MCEMATHAPI(void) mcemaths_mat4transpose(float* m44);

/* Inverse matrix. */
MCEMATHAPI(void) mcemaths_mat4inverse(float* m44);

/* Copy matrix. Faster than memcpy() for sure :-) */
MCEMATHAPI(void) mcemaths_mat4cpy(float* dest44, const float* src44);

/* Check if two matrices are identical */
MCEMATHAPI(bool) mcemaths_mat3equal(const float* m4_1, const float* m4_2);
MCEMATHAPI(bool) mcemaths_mat4equal(const float* m4_1, const float* m4_2);

/* Transform 3d vector by 3x3 matrix. It can also be 4x4 matrix only if the forth row and column is (0, 0, 0, 1). 
   If you are sure of which, use this instead of _m4v3 to get slightly faster speed. */
MCEMATHAPI(void) mcemaths_transform_m3v3(float* r4, const float* trans43, const float* v4);
MCEMATHAPI(void) mcemaths_transform_m3v3_ip(float* r4, const float* trans43);

/* Transform 3d vector by 4x4 matrix. */
MCEMATHAPI(void) mcemaths_transform_m4v3(float* r4, const float* trans44, const float* v4);
MCEMATHAPI(void) mcemaths_transform_m4v3_ip(float* r4, const float* trans44);

/* Transform 4d vector (in homogeneous coordinate) by 4x4 matrix. */
MCEMATHAPI(void) mcemaths_transform_m4v4(float* r4, const float* trans44, const float* v4);
MCEMATHAPI(void) mcemaths_transform_m4v4_ip(float* r4, const float* trans44);

/* Multiply two matrices. 'l44' is the left one, 'r44' is the right one. */
MCEMATHAPI(void) mcemaths_transform_m4m4(float* d44, const float* l44, const float* r44);
MCEMATHAPI(void) mcemaths_transform_m4m4_r_ip(const float* l44, float* r44);
MCEMATHAPI(void) mcemaths_transform_m4m4_l_ip(float* l44, const float* r44);

/* Build 4x4 transformation matrix*/
MCEMATHAPI(void) mcemaths_make_translation(float* m44, float x, float y, float z);
MCEMATHAPI(void) mcemaths_make_scaling(float* m44, float x, float y, float z);
MCEMATHAPI(void) mcemaths_make_rotation(float* m44, const float* axis4, float radian);
MCEMATHAPI(void) mcemaths_make_tbn(float* m44, const float* tangent, const float* binormal, const float* normal);

/* Build 4x4 OpenGL projection / view matrix*/
MCEMATHAPI(void) mcemaths_make_proj_perspective(float* m44, float znear, float zfar, float aspect, float fov_rad);
MCEMATHAPI(void) mcemaths_make_proj_orthographic(float* m44, float znear, float zfar, float left, float right, float bottom, float top);
MCEMATHAPI(void) mcemaths_make_view_traditional(float* m44, const float* from4, const float* look_at4, const float* up4);
MCEMATHAPI(void) mcemaths_make_view_camera(float* m44, const float* from4, const float* ypr4); // ypr = yaw/pitch/roll in radian

/************************************************************************/
/*   quaternion                                                         */
/************************************************************************/

/* Copy quaternion. No slower than memcpy() for sure :-| */
MCEMATHAPI(void) mcemaths_quatcpy(float* q_d, const float* q_s);

/* Calculate conjugate of quaternion. */
MCEMATHAPI(void) mcemaths_conjugate_q(float* q);

/* Calculate cross product of two quaternions. 'q_l' is the left one, 'r_r' is the right one. */
MCEMATHAPI(void) mcemaths_cross_q(float* q, const float* q_l, const float* q_r);

/* Rotate 3d vector by quaternion. See mcemaths_dir_from_quat() for more information. */
MCEMATHAPI(void) mcemaths_cross_qv3(float* v4, const float* q_l, const float* v4_r);

/* Build rotation quaternion by 3d axis vector and angle. The axis should be normalized. */
MCEMATHAPI(void) mcemaths_axis2q(float* q, const float* axis4, float angle_rad);

/* Build rotation quaternion by Eular angles. 'ypr4' is float[4] containing yaw (ypr4[0]), 
   pitch (ypr4[1]), and roll (ypr4[2]) in right order. */
MCEMATHAPI(void) mcemaths_eular2q(float* q, const float* ypr4);

/* Build equivalent rotation matrix out of quaternion. */
MCEMATHAPI(void) mcemaths_q2matrix(float* m44, const float* q);

/* Build equivalent quaternion by rotation matrix.
   NOTICE: The matrix MUST NOT contain scaling. Instead of inversing matrix, it is recommended that you 
   always store scaling matrix apart from rotation/translation matrix. Uniforming them separately into
   your GLSL will benefit you a lot when TBN is involved :-) */
MCEMATHAPI(void) mcemaths_matrix2q(float* q, const float* m44);

/* Build an orientation vector by quaternion. It is actually rotating the base vector (0, 0, -1) by the quaternion.
   Although you can use mcemaths_cross_qv3(), but due to the special base vector (those lovely zeros), this function 
   is surely much faster. This function is useful for OpenGL programmes. */
MCEMATHAPI(void) mcemaths_dir_from_quat(float* dir4, const float* quat, float len);

#ifdef __cplusplus
}
#endif
