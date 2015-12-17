#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <assert.h>
#include <memory.h>
#include <math.h>
#include <xmmintrin.h>

__declspec(align(16)) static const float IDENTITY4[16] = {1.0f, 0, 0, 0, 0, 1.0f, 0, 0, 0, 0, 1.0f, 0, 0, 0, 0, 1.0f};
static const float* IDENTITY4_LASTROW = IDENTITY4 + 12;