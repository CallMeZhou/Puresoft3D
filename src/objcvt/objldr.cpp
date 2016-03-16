#include "stdafx.h"
#include "objldr.h"

using namespace std;
using namespace mcemaths;

#define OBJOBJECT	"object"	// new scene object
#define OBJO		"o"			// new scene object
#define OBJGROUP	"g"			// new mesh (g = facet [g]roup)
#define OBJMTLLIB	"mtllib"	// sets an active external material file
#define OBJUSEMTL	"usemtl"	// sets an active material definition in the active external material file
#define OBJCOMMENT	"#"			// comment
#define OBJVERT		"v"			// defines a vertex
#define OBJTEX		"vt"		// defines a texture coordinate
#define OBJNORM		"vn"		// defines a normal
#define OBJFACE		"f"			// defines a facet
#define MATNEW		"newmtl"	// defines a material
#define MATKA		"ka"		// ambient colour
#define MATKD		"kd"		// diffuse colour
#define MATKS		"ks"		// specular colour
#define MATNS		"ns"		// specular exponent
#define MATTEXFILE	"map_kd"	// diffuse texture file
#define MATBUMPFILE	"map_bump"	// bump texture file
#define MATSPCFILE	"map_ks"	// specular colour texture file
#define MATSPEFILE	"map_ns"	// specular exponent texture file
#define MATPROGNAME	"prog_name"	// shader programme name [non-standard]

#define OBJSCENE	"scene"		// include scene description file [non-standard]
#define SCNCAMERA	"camera"
#define SCNLIGHT1	"light1"
#define SCNLIGHT2	"light1"
#define SCNLIGHT3	"light1"
#define SCNLIGHT4	"light1"

#define WRONG_TEXTURE "wrong_texture"

static void trimspace(string& str);
const unsigned int FACE_HAS_TEXCOORD  = 0x00000001;
const unsigned int FACE_HAS_NORMAL    = 0x00000002;
static unsigned int facetype_from_indexcount(const string& test);

static string make_mtl_name(const char* mtlname, const char* mtllibfilename);
static bool load_matlib(const wchar_t* filename, mtllib& lib, bool removepath);
static bool load_scene_desc(const wchar_t* filename, scene& scn);

//////////////////////////////////////////////////////////////////////////
bool _cdecl load_obj_fileW(const wchar_t* filename, obj_mesh_data& coords, objects& objs, mtllib& mtl, scene& scn, bool amalgroups, bool removepath, lof_progress pfprog)
{
	ifstream file(filename);
	if(!file)
		return false;

	file.seekg(0, ios_base::end);
	float length = static_cast<float>(file.tellg());
	file.seekg(0, ios_base::beg);

	faces* current_faces = NULL;
	group* current_group = NULL;
	groups* current_object = NULL;

	string cur_mtl_file, cur_mtl_name, cur_grp_name;
	string header, body; // of a line
	string temp;
	while(file >> header)
	{
		transform(header.begin(), header.end(), header.begin(), tolower);

		body.clear();
		while(1)
		{
			getline(file, temp);
			if(temp.empty())
				break;
			body += temp;
			trimspace(body); // remove prefix and suffix white space char
			if('\\' == *body.rbegin())
				body.erase(body.length() - 1);
			else
				break;
		}

		if(OBJCOMMENT == header)
		{
		}
		if(OBJOBJECT == header || OBJO == header)
		{
			// setting stuffs below to NULL pushes 'if(OBJFACE == header)' to create new stuffs
			current_object = NULL;
			current_group = NULL;
		}
		if(OBJGROUP == header)
		{
			cur_grp_name = body;

			// setting stuffs below to NULL pushes 'if(OBJFACE == header)' to create new stuffs
			if(!amalgroups)
				current_group = NULL;
		}
		else if(OBJFACE == header)
		{
			if(!current_object)
			{
				objs.push_back(groups());
				current_object = &*objs.rbegin();
			}

			if(!current_group)
			{
				current_group = &((*current_object)[cur_grp_name]);
			}

			unsigned int facetype = facetype_from_indexcount(body);

			string mtlname;
			if(cur_mtl_name.empty())
				mtlname = "";
			else
				mtlname = make_mtl_name(cur_mtl_name.c_str(), cur_mtl_file.c_str());

			current_faces = &((*current_group)[mtlname]);

			facet* current_face;
			if(facetype & FACE_HAS_TEXCOORD)
			{
				current_faces->face_data[faces::HAS_TEXCOORD].push_back(facet());
				current_face = &*current_faces->face_data[faces::HAS_TEXCOORD].rbegin();
			}
			else
			{
				current_faces->face_data[faces::NO_TEXCOORD].push_back(facet());
				current_face = &*current_faces->face_data[faces::NO_TEXCOORD].rbegin();
			}

			replace(body.begin(), body.end(), '/', ' '); // "v1/t1/n1 v2/t2/n2" -> "v1 t1 n1 v2 t2 n2"
			stringstream sstream(body);

			int index;
			vector<int> indices;
			while(sstream >> index)
				indices.push_back(index - 1);

			for(vector<int>::iterator it = indices.begin(); it != indices.end();)
			{
				current_face->vertIndices.push_back(*it++);
				if(facetype & FACE_HAS_TEXCOORD)
					current_face->texIndices.push_back(*it++);
				if(facetype & FACE_HAS_NORMAL)
					current_face->normIndices.push_back(*it++);
			}
		} // else if(OBJFACE == header)...
		else if(OBJVERT == header)
		{
			stringstream sstream(body);
			vec4 v(0.0f);
			sstream >> v.x >> v.y >> v.z;
			coords.vertices.push_back(v);
		}
		else if(OBJTEX == header)
		{
			stringstream sstream(body);
			vec2 texcoord;
			sstream >> texcoord.x >> texcoord.y;
			coords.texcoords.push_back(texcoord);
		}
		else if(OBJNORM == header)
		{
			stringstream sstream(body);
			vec4 v(0.0f);
			sstream >> v.x >> v.y >> v.z;
			coords.normals.push_back(v);
		}
		else if(OBJMTLLIB == header)
		{
			USES_CONVERSION;
			WCHAR path[MAX_PATH];
			//_tcscpy_s(path, MAX_PATH, filename);
			GetFullPathNameW(filename, MAX_PATH, path, NULL);
			PathRemoveFileSpecW(path);
			cur_mtl_file = body;
			PathAppend(path, A2CW(cur_mtl_file.c_str()));
			load_matlib(path, mtl, removepath);
		}
		else if(OBJSCENE == header)
		{
			USES_CONVERSION;
			WCHAR path[MAX_PATH];
			//_tcscpy_s(path, MAX_PATH, filename);
			GetFullPathNameW(filename, MAX_PATH, path, NULL);
			PathRemoveFileSpecW(path);
			PathAppend(path, A2CW(body.c_str()));
			load_scene_desc(path, scn);
		}
		else if(OBJUSEMTL == header && cur_mtl_name != body)
		{
			cur_mtl_name = body;
			current_group = NULL; // we can't use multiple materials in one component
		}

		if(pfprog)
			pfprog(static_cast<float>(file.tellg()) / length);
	} // while(file.good())

	return true;
}

bool _cdecl load_obj_fileA(const char* filename, obj_mesh_data& coords, objects& objs, mtllib& mtl, scene& scn, bool amalgroups, bool removepath, lof_progress pfprog)
{
	USES_CONVERSION;
	return load_obj_fileW(A2CW(filename), coords, objs, mtl, scn, amalgroups, removepath, pfprog);
}

void _cdecl centralize(vec4_coll& vertices)
{
	vec4 minpos(FLT_MAX, FLT_MAX, FLT_MAX, 0), maxpos(-FLT_MAX, -FLT_MAX, -FLT_MAX, 0);
	for(int i = 0; i < (int)vertices.size(); i++)
	{
		vec4 v = vertices[i];
		if(v.x < minpos.x) minpos.x = v.x;
		if(v.y < minpos.y) minpos.y = v.y;
		if(v.z < minpos.z) minpos.z = v.z;
		if(v.x > maxpos.x) maxpos.x = v.x;
		if(v.y > maxpos.y) maxpos.y = v.y;
		if(v.z > maxpos.z) maxpos.z = v.z;
	}

	vec4 centre;
	centre.x = minpos.x + (maxpos.x - minpos.x) / 2.0f;
	centre.y = minpos.y + (maxpos.y - minpos.y) / 2.0f;
	centre.z = minpos.z + (maxpos.z - minpos.z) / 2.0f;
	centre.w = 0;
	vec4 offset;
	offset.x = -centre.x;
	offset.y = -centre.y;
	offset.z = -centre.z;
	offset.w = 0;
	for(int i = 0; i < (int)vertices.size(); i++)
	{
		vertices[i].x += offset.x;
		vertices[i].y += offset.y;
		vertices[i].z += offset.z;
	}
}

//////////////////////////////////////////////////////////////////////////
static void trimspace(string& str)
{
	int first = str.find_first_not_of(' ');
	int last = str.find_last_not_of(' ');
	if(first != str.npos && last != str.npos && last > first)
	{
		string temp = str.substr(first, last - first + 1);
		str = temp;
	}
}

static unsigned int facetype_from_indexcount(const string& test)
{
	string::size_type first = test.find_first_not_of(' ');
	string s = test.substr(first, test.find_first_of(' ', first));
	string::size_type pos1 = s.find_first_of('/');
	string::size_type pos2 = s.find_last_of('/');

	if(string::npos == pos1 && string::npos == pos1)
		return 0;
	else if(1 == pos2 - pos1)
		return FACE_HAS_NORMAL;
	else if(0 == pos2 - pos1) // pos2 == pos2 != string::npos
		return FACE_HAS_TEXCOORD;
	else // if(pos2 - pos1 > 1)
		return FACE_HAS_TEXCOORD | FACE_HAS_NORMAL;
}

static string make_mtl_name(const char* mtlname, const char* mtllibfilename)
{
	if(0 == strlen(mtlname))
		return "";
	string s = mtllibfilename;
	s += "::";
	s += mtlname;
	return s;
}

static bool load_matlib(const wchar_t* filename, mtllib& lib, bool removepath)
{
	USES_CONVERSION;
	ifstream file(filename);
	if(!file)
		return false;

	string pure_name = W2CA(PathFindFileNameW(filename));

	string header, body;
	material* current_material = NULL;
	while(file >> header)
	{
		transform(header.begin(), header.end(), header.begin(), tolower);
		getline(file, body);
		trimspace(body);
		if(MATNEW == header)
		{
			current_material = &lib[make_mtl_name(body.c_str(), pure_name.c_str())];
			current_material->specularExponent = 0;
		}
		else if(MATKA == header && current_material)
			stringstream(body) >> current_material->ambientColour.z >> current_material->ambientColour.y >> current_material->ambientColour.x;
		else if(MATKD == header && current_material)
			stringstream(body) >> current_material->diffuseColour.z >> current_material->diffuseColour.y >> current_material->diffuseColour.x;
		else if(MATKS == header && current_material)
			stringstream (body) >> current_material->specularColour.z >> current_material->specularColour.y >> current_material->specularColour.x;
		else if(MATNS == header && current_material)
			stringstream(body) >> current_material->specularExponent;
		else if(MATTEXFILE == header && current_material)
		{
			stringstream ss(body);
			string str_frag;
			while(ss >> str_frag); // skip options, we don't support them
			trimspace(str_frag);
			current_material->diffuseFile = removepath ? PathFindFileNameA(str_frag.c_str()) : str_frag;
		}
		else if(MATBUMPFILE == header && current_material)
		{
			stringstream ss(body);
			string str_frag;
			while(ss >> str_frag); // skip options, we don't support them
			trimspace(str_frag);
			current_material->bumpFile = removepath ? PathFindFileNameA(str_frag.c_str()) : str_frag;
		}
		else if(MATSPCFILE == header && current_material)
		{
			stringstream ss(body);
			string str_frag;
			while(ss >> str_frag); // skip options, we don't support them
			trimspace(str_frag);
			current_material->spcFile = removepath ? PathFindFileNameA(str_frag.c_str()) : str_frag;
		}
		else if(MATSPEFILE == header && current_material)
		{
			stringstream ss(body);
			string str_frag;
			while(ss >> str_frag); // skip options, we don't support them
			trimspace(str_frag);
			current_material->speFile = removepath ? PathFindFileNameA(str_frag.c_str()) : str_frag;
		}
		else if(MATPROGNAME == header && current_material)
		{
			stringstream ss(body);
			string str_frag;
			while(ss >> str_frag); // skip options, we don't support them
			trimspace(str_frag);
			current_material->programmeName = str_frag;
		}
	}
	return true;
}

static bool load_scene_desc(const wchar_t* filename, scene& scn)
{
	USES_CONVERSION;
	ifstream file(filename);
	if(!file)
		return false;

	string pure_name = W2CA(PathFindFileNameW(filename));

	string header, body;
	material* current_material = NULL;
	while(file >> header)
	{
		transform(header.begin(), header.end(), header.begin(), tolower);
		getline(file, body);
		trimspace(body);
		if(SCNCAMERA == header)
			stringstream(body) >> scn.cameraPosition[0] >> scn.cameraPosition[1] >> scn.cameraPosition[2] >> scn.cameraYPR[0] >> scn.cameraYPR[1] >> scn.cameraYPR[2];
		else if(SCNLIGHT1 == header)
			stringstream(body) >> scn.lightPositions[0].x >> scn.lightPositions[0].y >> scn.lightPositions[0].z >> scn.lightDirections[0].x >> scn.lightDirections[0].y >> scn.lightDirections[0].z;
		else if(SCNLIGHT2 == header)
			stringstream(body) >> scn.lightPositions[1].x >> scn.lightPositions[1].y >> scn.lightPositions[1].z >> scn.lightDirections[1].x >> scn.lightDirections[1].y >> scn.lightDirections[1].z;
		else if(SCNLIGHT3 == header)
			stringstream(body) >> scn.lightPositions[2].x >> scn.lightPositions[2].y >> scn.lightPositions[2].z >> scn.lightDirections[2].x >> scn.lightDirections[2].y >> scn.lightDirections[2].z;
		else if(SCNLIGHT4 == header)
			stringstream(body) >> scn.lightPositions[3].x >> scn.lightPositions[3].y >> scn.lightPositions[3].z >> scn.lightDirections[3].x >> scn.lightDirections[3].y >> scn.lightDirections[3].z;
	}
	return true;
}