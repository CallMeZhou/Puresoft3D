#include <math.h>
#include "mcemaths.h"
#include "samplrcube.h"

/*

major axis
direction		target		sc	tc	ma
-----------------------------------------------
+x				LAYER_XPOS	-z	-y	x
-x				LAYER_XNEG	+z	-y	x
+y				LAYER_YPOS	+x	+z	y
-y				LAYER_YNEG	+x	-z	y
+z				LAYER_ZPOS	+x	-y	z
-z				LAYER_ZNEG	-x	-y	z

s = (sc / abs(ma) + 1) / 2
t = (tc / abs(ma) + 1) / 2

                                  --- ogl spec
*/

#define X direction[0]
#define Y direction[1]
#define Z direction[2]
#define S texcoord[0]
#define T texcoord[1]

static PuresoftFBO::LAYER texcoordFromDirection(float* texcoord, const float* direction)
{
	if(fabs(X) > fabs(Y))
	{
		if(fabs(X) > fabs(Z)) // X wins
		{
			if(X > 0)
			{
				S = (-Z / fabs(X) + 1.0f) / 2.0f;
				T = (-Y / fabs(X) + 1.0f) / 2.0f;
				return PuresoftFBO::LAYER_XPOS;
			}
			else
			{
				S = ( Z / fabs(X) + 1.0f) / 2.0f;
				T = (-Y / fabs(X) + 1.0f) / 2.0f;
				return PuresoftFBO::LAYER_XNEG;
			}
		}
		else // Z wins
		{
			if(Z > 0)
			{
				S = ( X / fabs(Z) + 1.0f) / 2.0f;
				T = (-Y / fabs(Z) + 1.0f) / 2.0f;
				return PuresoftFBO::LAYER_ZPOS;
			}
			else
			{
				S = (-X / fabs(Z) + 1.0f) / 2.0f;
				T = (-Y / fabs(Z) + 1.0f) / 2.0f;
				return PuresoftFBO::LAYER_ZNEG;
			}
		}
	}
	else
	{
		if(fabs(Z) > fabs(Y)) // Z wins
		{
			if(Z > 0)
			{
				S = ( X / fabs(Z) + 1.0f) / 2.0f;
				T = (-Y / fabs(Z) + 1.0f) / 2.0f;
				return PuresoftFBO::LAYER_ZPOS;
			}
			else
			{
				S = (-X / fabs(Z) + 1.0f) / 2.0f;
				T = (-Y / fabs(Z) + 1.0f) / 2.0f;
				return PuresoftFBO::LAYER_ZNEG;
			}
		}
		else // Y wins
		{
			if(Z > 0)
			{
				S = ( X / fabs(Y) + 1.0f) / 2.0f;
				T = ( Z / fabs(Y) + 1.0f) / 2.0f;
				return PuresoftFBO::LAYER_YPOS;
			}
			else
			{
				S = ( X / fabs(Y) + 1.0f) / 2.0f;
				T = (-Z / fabs(Y) + 1.0f) / 2.0f;
				return PuresoftFBO::LAYER_YNEG;
			}
		}
	}
}

void PuresoftSamplerCube::get(const PuresoftFBO* imageBuffer, const float* direction, void* data, size_t len)
{
	float texcoord[2];
	PuresoftFBO::LAYER layer = texcoordFromDirection(texcoord, direction);
	imageBuffer->getExtraLayer(layer)->directRead(
		(unsigned int)((float)imageBuffer->getHeight() * S + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * T + 0.5f), 
		data, len);
}

void PuresoftSamplerCube::get1(const PuresoftFBO* imageBuffer, const float* direction, void* data)
{
	float texcoord[2];
	PuresoftFBO::LAYER layer = texcoordFromDirection(texcoord, direction);
	imageBuffer->getExtraLayer(layer)->directRead1(
		(unsigned int)((float)imageBuffer->getHeight() * S + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * T + 0.5f), 
		data);
}

void PuresoftSamplerCube::get4(const PuresoftFBO* imageBuffer, const float* direction, void* data)
{
	float texcoord[2];
	PuresoftFBO::LAYER layer = texcoordFromDirection(texcoord, direction);
	imageBuffer->getExtraLayer(layer)->directRead4(
		(unsigned int)((float)imageBuffer->getHeight() * S + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * T + 0.5f), 
		data);
}

void PuresoftSamplerCube::get16(const PuresoftFBO* imageBuffer, const float* direction, void* data)
{
	float texcoord[2];
	PuresoftFBO::LAYER layer = texcoordFromDirection(texcoord, direction);
	imageBuffer->getExtraLayer(layer)->directRead16(
		(unsigned int)((float)imageBuffer->getHeight() * S + 0.5f), 
		(unsigned int)((float)imageBuffer->getWidth()  * T + 0.5f), 
		data);
}