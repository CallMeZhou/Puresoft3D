#pragma once

/*
		OBJ file structure

OBJECT 1
{
	GROUP 1
	{
		FACE 1
		FACE 2
		...
		FACE n
	}
	...
	GROUP n
	{
		...
	}
}
...
OBJECT n
{
	...
}

So, as our first thought, one GROUP can be loaded as one Mesh object, because those FACE 
sections just contain primitives, like triangle / triangle-fan, that can be folded into 
one Mesh.

However, the "usemtl" keyword may appear ANYWHERE in a file, and as we know, one Mesh can 
only be set one texture, or no texture just colour, so within a GROUP, we have to separate 
faces into different categories due to different MTLs they are using.

So, oh my, you may probably think faces in one MTL category should now be put in one Mesh object. 
You are not quite right, because face sections under the same MTL mark still could vary -- 
some face sections may have no texture coordinates while others have. Those not having texture 
coordinates should make a separate Mesh, which uses the colour parameters in the current MTL 
but do not use texture. Chaos? Yes. In software developing area, I have seen many designs, 
protocols, libraries, that have unbelievable flexibilities. For instance, a "media sample" 
structure in Microsoft's DirectShow has a lot of "optional" fields that may or may not contain 
valid values. But for many years, I have been hearing people asking whether or not should they 
use an optional field, just because it's optional, God knows when it is available? God also 
needs a lot of if-else!

Ok, to clear the chaos, in turns of, in an OBJ file, what on earth should make Mesh object, I say 
it clearly in one sentence: inside one GROUP section, FACE sections under one MTL mark can make 
ONE or TWO Mesh objects. Look at the example: (not in real OBJ's syntax)

GROUP 
{
	MTL 1	<-- change material settings

	FACE 1
	FACE 2

	MTL 2	<-- change material settings again

	FACE 3
	FACE 4
	FACE 5 -NO-TEXTURE	<-- Although it uses MTL 2, but skips texture
	FACE 6 -NO-TEXTURE 
}

This GROUP makes 3 Mesh objects, they are {FACE1, FACE2}, {FACE3, FACE4}, {FACE5, FACE6}.

But, "load_obj_file"'s users don't need to struggle too much for the above chaos.
When you get a "objects" through the parameter, you iterate the nested containers, and 
you will eventually get to "faces" object. You should simply know one "face" object 
makes one Mesh object. That's it.

*/
#include "alloc16.hpp"
#if(_MSC_VER <= 1600) // VS2010 or ealier
#include "fixvec.hpp"
#else
#include <vector>
#endif
#include <string>
#include <map>
#include "mcemaths.hpp"

typedef std::vector<mcemaths::vec4, alloc16<mcemaths::vec4> > vec4_coll;
typedef std::vector<mcemaths::vec2> vec2_coll;
// coordinate data in OBJ file
struct obj_mesh_data
{
	vec4_coll vertices;
	vec4_coll normals;
	vec2_coll texcoords;
};

// MTL file content
struct material
{
	mcemaths::vec4 ambientColour;
	mcemaths::vec4 diffuseColour;
	mcemaths::vec4 specularColour;
	float specularExponent;
	std::string diffuseFile;
	std::string bumpFile;
	std::string spcFile;
	std::string speFile;
	std::string programmeName;
};

// data of a FACE, consisted of indices
struct facet
{
	std::vector<int> texIndices;
	std::vector<int> vertIndices;
	std::vector<int> normIndices;
};
typedef std::vector<facet> face;

// FACEs defined in one GROUP
struct faces
{
	enum {HAS_TEXCOORD = 0, NO_TEXCOORD, MAX};
	face face_data[MAX];
};

// scene description
const int MAXLIGHTS = 4;
struct scene
{
	mcemaths::vec4 cameraPosition;
	mcemaths::vec4 cameraYPR;
	mcemaths::vec4 lightPositions[MAXLIGHTS];
	mcemaths::vec4 lightDirections[MAXLIGHTS];
};

// GROUP, the std::map's key is material name
typedef std::map<std::string, faces> group;
// GROUPs defined in one OBJECT
typedef std::map<std::string, group> groups;
// OBJECT
typedef std::vector<groups> objects;
// MATERIAL std::map, the std::map's key is MATERIAL NAME too
typedef std::map<std::string, material, std::less<std::string>, alloc16<std::pair<const std::string, material> > > mtllib; // "std::string" key of "mtllib" coincides with the "group"'s key

typedef void (_stdcall *lof_progress)(float percent);
bool _cdecl load_obj_fileA(const char* filename, obj_mesh_data& coords, objects& objs, mtllib& mtl, scene& scn, bool amalgroups, bool removepath, lof_progress pfprog);
bool _cdecl load_obj_fileW(const wchar_t* filename, obj_mesh_data& coords, objects& objs, mtllib& mtl, scene& scn, bool amalgroups, bool removepath, lof_progress pfprog);
void _cdecl centralize(vec4_coll& vertices);

#ifdef _UNICODE
#define load_obj_file load_obj_fileW
#else
#define load_obj_file load_obj_fileA
#endif