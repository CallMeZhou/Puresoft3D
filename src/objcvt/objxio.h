#pragma once
#include <string>
#include "mcemaths.hpp"

struct mesh_info
{
	std::string mesh_name;
	unsigned int num_vertices;
	bool has_texcoords;
	bool has_normals;
	bool has_tangents;
	mcemaths::vec4* vertices;
	mcemaths::vec4* normals;
	mcemaths::vec4* tangents;
	mcemaths::vec2* texcoords;
	unsigned int num_indices;
	int* indices;
	mcemaths::vec4 ambient_colour;
	mcemaths::vec4 diffuse_colour;
	mcemaths::vec4 specular_colour;
	float specular_exponent;
	std::string diffuse_file;
	std::string bump_file;
	std::string spc_file;
	std::string spe_file;
	std::string programme;
};

const int OBJXMAXLIGHTS = 4;
enum LIGHT_TYPE {LT_OMNI, LT_DIR};
struct scene_desc
{
	mcemaths::vec4 camera_pos;
	mcemaths::vec4 camera_ypr;
	mcemaths::vec4 light_pos[OBJXMAXLIGHTS];
	mcemaths::vec4 light_dir[OBJXMAXLIGHTS];
	LIGHT_TYPE light_types[OBJXMAXLIGHTS];
};

typedef void* HOBJXIO;

HOBJXIO	_cdecl create_objxW(const wchar_t* filename, const scene_desc& scn);
HOBJXIO	_cdecl create_objxA(const char* filename, const scene_desc& scn);
bool	_cdecl write_mesh(HOBJXIO handle, const mesh_info& mesh);

HOBJXIO	_cdecl open_objxW(const wchar_t* filename, scene_desc& scn);
HOBJXIO	_cdecl open_objxA(const char* filename, scene_desc& scn);
int		_cdecl get_mesh_count(HOBJXIO handle);
bool	_cdecl read_mesh_header(HOBJXIO handle, mesh_info& mesh);
bool	_cdecl read_mesh(HOBJXIO handle, mesh_info& mesh);

void	_cdecl close_objx(HOBJXIO handle);

#ifdef _UNICODE
#define create_objx create_objxW
#define open_objx open_objxW
#else
#define create_objx create_objxA
#define open_objx open_objxA
#endif