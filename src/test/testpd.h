#pragma once

#pragma pack(16)
__declspec(align(16)) struct MYTESTPROCDATA
{
	float normal[4];
	float worldPos[4];
	float texcoord[2];
};
#pragma pack()