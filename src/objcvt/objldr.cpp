#include "stdafx.h"
#include "objldr.h"

#define OBJOBJECT	"object"	//the current line of the obj file defines the start of a new material
#define OBJO		"o"			//the current line of the obj file defines the start of a new material
#define OBJGROUP	"g"			//the current line of the obj file defines the start of a new face
#define OBJMTLLIB	"mtllib"	//the current line of the obj file defines the start of a new material
#define OBJUSEMTL	"usemtl"	//the current line of the obj file defines the start of a new material
#define OBJCOMMENT	"#"			//The current line of the obj file is a comment
#define OBJVERT		"v"			//the current line of the obj file defines a vertex
#define OBJTEX		"vt"		//the current line of the obj file defines texture coordinates
#define OBJNORM		"vn"		//the current line of the obj file defines a normal
#define OBJFACE		"f"			//the current line of the obj file defines a face
#define MATNEW		"newmtl"
#define MATKA		"ka"
#define MATKD		"kd"
#define MATKS		"ks"
#define MATNS		"ns"
#define MATTEXFILE	"map_kd"
#define MATBUMPFILE	"map_bump" // Extended by myself, you need to type these keyword by yourself into mtl file

#define WRONG_TEXTURE "wrong_texture"

static void trimspace(string& str);
const unsigned int FACE_HAS_TEXCOORD  = 0x00000001;
const unsigned int FACE_HAS_NORMAL    = 0x00000002;
static unsigned int facetype_from_indexcount(const string& test);

static string make_mtl_name(const char* mtlname, const char* mtllibfilename);
static bool load_matlib(const wchar_t* filename, mtllib& lib, bool removepath);

//////////////////////////////////////////////////////////////////////////
bool _cdecl load_obj_fileW(const wchar_t* filename, obj_mesh_data& coords, objects& objs, mtllib& mtl, bool amalgroups, bool removepath, lof_progress pfprog)
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

	string cur_mtl_file, cur_mtl_name;
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
				current_object->push_back(group());
				current_group = &*current_object->rbegin();
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

bool _cdecl load_obj_fileA(const char* filename, obj_mesh_data& coords, objects& objs, mtllib& mtl, bool amalgroups, bool removepath, lof_progress pfprog)
{
	USES_CONVERSION;
	return load_obj_fileW(A2CW(filename), coords, objs, mtl, amalgroups, removepath, pfprog);
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
			current_material->spec_expo = 0;
		}
		else if(MATKA == header && current_material)
			stringstream(body) >> current_material->ka.x >> current_material->ka.y >> current_material->ka.z;
		else if(MATKD == header && current_material)
			stringstream(body) >> current_material->kd.x >> current_material->kd.y >> current_material->kd.z;
		else if(MATKS == header && current_material)
			stringstream (body) >> current_material->ks.x >> current_material->ks.y >> current_material->ks.z;
		else if(MATNS == header && current_material)
			stringstream(body) >> current_material->spec_expo;
		else if(MATTEXFILE == header && current_material)
		{
			stringstream ss(body);
			string str_frag;
			while(ss >> str_frag); // we want the last fragment, god knows what are the first a few fragments for...
			trimspace(str_frag);
			current_material->texfile = removepath ? PathFindFileNameA(str_frag.c_str()) : str_frag;
		}
		else if(MATBUMPFILE == header && current_material)
		{
			stringstream ss(body);
			string str_frag;
			while(ss >> str_frag); // we want the last fragment, god knows what are the first a few fragments for...
			trimspace(str_frag);
			current_material->bumpfile = removepath ? PathFindFileNameA(str_frag.c_str()) : str_frag;
		}
	}
	return true;
}

