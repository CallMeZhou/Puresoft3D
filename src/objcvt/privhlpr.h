#pragma once

const float FP_MIN_DIFF = 0.00000001f;

template<typename FPTYPE>
static inline bool is_float_zero(FPTYPE val)
{
	FPTYPE temp = val - FP_MIN_DIFF;
	if(-FP_MIN_DIFF <= temp && temp <= FP_MIN_DIFF)
		return true;
	return false;
}

template<typename FPTYPE>
static inline bool is_float_equal(FPTYPE val1, FPTYPE val2)
{
	return is_float_zero(val1 - val2);
}

template<typename FPTYPE>
static inline FPTYPE max2(FPTYPE v1, FPTYPE v2)
{
	return v1 > v2 ? v1 : v2;
}

template<typename FPTYPE>
static inline FPTYPE max3(FPTYPE v1, FPTYPE v2, FPTYPE v3)
{
	return max2(max2(v1, v2), v3);
}

template<typename FPTYPE>
static inline FPTYPE min2(FPTYPE v1, FPTYPE v2)
{
	return v1 < v2 ? v1 : v2;
}

template<typename FPTYPE>
static inline FPTYPE min3(FPTYPE v1, FPTYPE v2, FPTYPE v3)
{
	return min2(min2(v1, v2), v3);
}

template<typename FPTYPE>
static inline FPTYPE clamp(FPTYPE v, FPTYPE min, FPTYPE max)
{
	return min2(max2(v, min), max);
}


