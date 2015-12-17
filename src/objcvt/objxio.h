#pragma once

struct mesh_info
{
	unsigned int num_vertices;
	bool has_texcoords;
	bool has_normals;
	bool has_tangents;
	vec4* vertices;
	vec4* normals;
	vec4* tangents;
	vec2* texcoords;
	unsigned int num_indices;
	int* indices;
	vec4 single_colour;
	float specularity; // exponent of specularity
	std::string tex_file;
	std::string bump_file;
};

typedef void* HOBJXIO;

HOBJXIO	_cdecl create_objxW(const wchar_t* filename);
HOBJXIO	_cdecl create_objxA(const char* filename);
bool	_cdecl write_mesh(HOBJXIO handle, const mesh_info& mesh);

HOBJXIO	_cdecl open_objxW(const wchar_t* filename);
HOBJXIO	_cdecl open_objxA(const char* filename);
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