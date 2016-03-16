#include "stdafx.h"
#include "objxio.h"
#include "privhlpr.h"
#include "mcemaths.h"

#define OBJX_CURVERSION		0x010003
#define OBJX_FILENAMELEN	260

#pragma pack(1)
struct light
{
	float light_position[4];
	float light_direction[4];
};

struct objx_header
{
	unsigned int version;
	unsigned int num_meshes;
	unsigned int mesheader_size;
	unsigned int reserved;
	float camera_position[4];
	float camera_ypr[4];
	light lights[4];
};
struct mesh_header
{
	char name[OBJX_FILENAMELEN];
	unsigned int num_vertices;
	unsigned int num_indices;
	bool has_texcoords;
	bool has_normals;
	bool has_tangents;
	float ambient[4]; // bgra
	float diffuse[4]; // bgra
	float specular[4]; // bgra
	float specularExp;
	char diffuse_file[OBJX_FILENAMELEN];
	char bump_file[OBJX_FILENAMELEN];
	char spc_file[OBJX_FILENAMELEN];
	char spe_file[OBJX_FILENAMELEN];
	char programme[OBJX_FILENAMELEN];
	unsigned int next_offset; // offset of next mesh, for extension/compatibility purpose
};
#pragma pack()

struct OBJXO
{
	objx_header file_header;
	FILE* file;
	size_t cb;
};

struct OBJXI : public OBJXO
{
	mesh_header last_mesh_header;
};

static unsigned int calc_mesh_bytes(const mesh_header& header);
static void generate_tangents(mesh_info& mesh, const mesh_header& header);
static void generate_tangent(vec4& tangent, const vec4& a, const vec4& b, const vec4& c, const vec2& ta, const vec2& tb, const vec2& tc);
//////////////////////////////////////////////////////////////////////////

HOBJXIO _cdecl create_objxW(const wchar_t* filename, const scene_desc& scn)
{
	OBJXO* handle = (OBJXO*)_aligned_malloc(sizeof(OBJXO), 16);
	if(!handle)
	{
		return NULL;
	}
	handle->cb = sizeof(OBJXO);
	memset(&handle->file_header, 0, sizeof(objx_header));
	handle->file_header.version = OBJX_CURVERSION;
	handle->file_header.mesheader_size = sizeof(mesh_header);
	memcpy(handle->file_header.camera_position, scn.camera_pos, 4 * sizeof(float));
	memcpy(handle->file_header.camera_ypr, scn.camera_ypr, 4 * sizeof(float));
	memcpy(handle->file_header.lights[0].light_position,  scn.light_pos[0], 4 * sizeof(float));
	memcpy(handle->file_header.lights[0].light_direction, scn.light_dir[0], 4 * sizeof(float));
	memcpy(handle->file_header.lights[1].light_position,  scn.light_pos[1], 4 * sizeof(float));
	memcpy(handle->file_header.lights[1].light_direction, scn.light_dir[1], 4 * sizeof(float));
	memcpy(handle->file_header.lights[2].light_position,  scn.light_pos[2], 4 * sizeof(float));
	memcpy(handle->file_header.lights[2].light_direction, scn.light_dir[2], 4 * sizeof(float));
	memcpy(handle->file_header.lights[3].light_position,  scn.light_pos[3], 4 * sizeof(float));
	memcpy(handle->file_header.lights[3].light_direction, scn.light_dir[3], 4 * sizeof(float));

	if(0 != _wfopen_s(&handle->file, filename, _T("w+b")))
	{
		_aligned_free(handle);
		return NULL;
	}
	fwrite(handle, sizeof(objx_header), 1, handle->file);
	return (HOBJXIO)handle;
}

HOBJXIO _cdecl create_objxA(const char* filename, const scene_desc& scn)
{
	USES_CONVERSION;
	return create_objxW(A2CW(filename), scn);
}

bool _cdecl write_mesh(HOBJXIO handle, const mesh_info& mesh)
{
	ATLASSERT(handle);
	if( (0 == mesh.num_vertices) || 
		(NULL == mesh.vertices) || 
		(0 != mesh.num_indices && NULL == mesh.indices) || 
		(0 == mesh.num_indices && NULL != mesh.indices))
	{
		ATLASSERT(0);
		return false;
	}

	OBJXO* hobjxio = reinterpret_cast<OBJXO*>(handle);
	hobjxio->file_header.num_meshes++;

	mesh_header header;
	memset(&header, 0, sizeof(header));
	header.num_vertices = mesh.num_vertices;
	header.num_indices = mesh.num_indices;
	header.has_texcoords = mesh.texcoords != NULL;
	header.has_normals = mesh.normals != NULL;
	header.has_tangents = mesh.tangents != NULL;
	header.ambient[0] = mesh.ambient_colour.z;
	header.ambient[1] = mesh.ambient_colour.y;
	header.ambient[2] = mesh.ambient_colour.x;
	header.ambient[3] = mesh.ambient_colour.w;
	header.diffuse[0] = mesh.diffuse_colour.z;
	header.diffuse[1] = mesh.diffuse_colour.y;
	header.diffuse[2] = mesh.diffuse_colour.x;
	header.diffuse[3] = mesh.diffuse_colour.w;
	header.specular[0] = mesh.specular_colour.z;
	header.specular[1] = mesh.specular_colour.y;
	header.specular[2] = mesh.specular_colour.x;
	header.specular[3] = mesh.specular_colour.w;
	header.specularExp = mesh.specular_exponent;
	strcpy_s(header.diffuse_file, OBJX_FILENAMELEN, mesh.diffuse_file.c_str());
	strcpy_s(header.bump_file, OBJX_FILENAMELEN, mesh.bump_file.c_str());
	strcpy_s(header.spc_file, OBJX_FILENAMELEN, mesh.spc_file.c_str());
	strcpy_s(header.spe_file, OBJX_FILENAMELEN, mesh.spe_file.c_str());
	strcpy_s(header.programme, OBJX_FILENAMELEN, mesh.programme.c_str());
	strcpy_s(header.name, OBJX_FILENAMELEN, mesh.mesh_name.c_str());
	header.next_offset = calc_mesh_bytes(header);

	size_t bytes = sizeof(header);
	if(1 != fwrite(&header, bytes, 1, hobjxio->file))
		return false;

	bytes = sizeof(vec4) * header.num_vertices;
	if(1 != fwrite(mesh.vertices, bytes, 1, hobjxio->file))
	{
		__asm int 3;
		return false;
	}

	if(mesh.normals)
	{
		if(1 != fwrite(mesh.normals, bytes, 1, hobjxio->file))
			return false;
	}

	if(mesh.tangents)
	{
		if(1 != fwrite(mesh.tangents, bytes, 1, hobjxio->file))
			return false;
	}

	if(mesh.texcoords)
	{
		if(1 != fwrite(mesh.texcoords, sizeof(float) * 2 * header.num_vertices, 1, hobjxio->file))
			return false;
	}

	if(mesh.indices)
	{
		bytes = sizeof(int) * header.num_indices;
		if(1 != fwrite(mesh.indices, bytes, 1, hobjxio->file))
			return false;
	}

	return true;
}

HOBJXIO	_cdecl open_objxW(const wchar_t* filename, scene_desc& scn)
{
	OBJXI* handle = (OBJXI*)_aligned_malloc(sizeof(OBJXI), 16);
	if(!handle)
	{
		return NULL;
	}
	handle->cb = sizeof(OBJXI);
	if(0 != _tfopen_s(&handle->file, filename, _T("rb")))
	{
		_aligned_free(handle);
		return NULL;
	}
	fread(&handle->file_header, sizeof(objx_header), 1, handle->file);
	if(handle->file_header.version != OBJX_CURVERSION || 
		handle->file_header.mesheader_size < sizeof(mesh_header))
	{
		fclose(handle->file);
		_aligned_free(handle);
		return NULL;
	}
	memset(&handle->last_mesh_header, 0, sizeof(handle->last_mesh_header));
	memcpy(scn.camera_pos, handle->file_header.camera_position, 4 * sizeof(float));
	memcpy(scn.camera_ypr, handle->file_header.camera_ypr, 4 * sizeof(float));
	memcpy(scn.light_pos[0], handle->file_header.lights[0].light_position,  4 * sizeof(float));
	memcpy(scn.light_dir[0], handle->file_header.lights[0].light_direction, 4 * sizeof(float));
	memcpy(scn.light_pos[1], handle->file_header.lights[1].light_position,  4 * sizeof(float));
	memcpy(scn.light_dir[1], handle->file_header.lights[1].light_direction, 4 * sizeof(float));
	memcpy(scn.light_pos[2], handle->file_header.lights[2].light_position,  4 * sizeof(float));
	memcpy(scn.light_dir[2], handle->file_header.lights[2].light_direction, 4 * sizeof(float));
	memcpy(scn.light_pos[3], handle->file_header.lights[3].light_position,  4 * sizeof(float));
	memcpy(scn.light_dir[3], handle->file_header.lights[3].light_direction, 4 * sizeof(float));

	for(int i = 0; i < MAXLIGHTS; i++)
	{
		if(mcemaths_len_3_4(handle->file_header.lights[i].light_direction) < 0.0000001f)
			scn.light_types[i] = LT_OMNI;
		else
			scn.light_types[i] = LT_DIR;
	}

	return (HOBJXIO)handle;
}

HOBJXIO	_cdecl open_objxA(const char* filename, scene_desc& scn)
{
	USES_CONVERSION;
	return open_objxW(A2CW(filename), scn);
}

int _cdecl get_mesh_count(HOBJXIO handle)
{
	ATLASSERT(handle);
	OBJXI* hobjxio = reinterpret_cast<OBJXI*>(handle);
	return hobjxio->file_header.num_meshes;
}

bool _cdecl read_mesh_header(HOBJXIO handle, mesh_info& mesh)
{
	ATLASSERT(handle);
	OBJXI* hobjxio = reinterpret_cast<OBJXI*>(handle);

	size_t bytes = sizeof(mesh_header);
	if(1 != fread(&hobjxio->last_mesh_header, bytes, 1, hobjxio->file))
		return false;

	mesh.mesh_name = "";
	mesh.num_vertices = 0;
	mesh.has_texcoords = false;
	mesh.has_normals = false;
	mesh.has_tangents = false;
	mesh.vertices = NULL;
	mesh.normals = NULL;
	mesh.tangents = NULL;
	mesh.texcoords = NULL;
	mesh.num_indices = 0;
	mesh.indices = NULL;
	mesh.ambient_colour.set(0);
	mesh.diffuse_colour.set(0);
	mesh.specular_colour.set(0);
	mesh.specular_exponent = 0;
	mesh.diffuse_file = "";
	mesh.bump_file = "";
	mesh.spc_file = "";
	mesh.spe_file = "";
	mesh.programme = "";

	fseek(hobjxio->file, hobjxio->file_header.mesheader_size - bytes, SEEK_CUR);

	mesh.num_vertices = hobjxio->last_mesh_header.num_vertices;
	mesh.num_indices = hobjxio->last_mesh_header.num_indices;
	mesh.has_normals = hobjxio->last_mesh_header.has_normals;
	mesh.has_tangents = hobjxio->last_mesh_header.has_tangents;
	mesh.has_texcoords = hobjxio->last_mesh_header.has_texcoords;
	mesh.ambient_colour.z = clamp(hobjxio->last_mesh_header.ambient[0], 0.0f, 1.0f);
	mesh.ambient_colour.y = clamp(hobjxio->last_mesh_header.ambient[1], 0.0f, 1.0f);
	mesh.ambient_colour.x = clamp(hobjxio->last_mesh_header.ambient[2], 0.0f, 1.0f);
	mesh.ambient_colour.w = clamp(hobjxio->last_mesh_header.ambient[3], 0.0f, 1.0f);
	mesh.diffuse_colour.z = clamp(hobjxio->last_mesh_header.diffuse[0], 0.0f, 1.0f);
	mesh.diffuse_colour.y = clamp(hobjxio->last_mesh_header.diffuse[1], 0.0f, 1.0f);
	mesh.diffuse_colour.x = clamp(hobjxio->last_mesh_header.diffuse[2], 0.0f, 1.0f);
	mesh.diffuse_colour.w = clamp(hobjxio->last_mesh_header.diffuse[3], 0.0f, 1.0f);
	mesh.specular_colour.z = clamp(hobjxio->last_mesh_header.specular[0], 0.0f, 1.0f);
	mesh.specular_colour.y = clamp(hobjxio->last_mesh_header.specular[1], 0.0f, 1.0f);
	mesh.specular_colour.x = clamp(hobjxio->last_mesh_header.specular[2], 0.0f, 1.0f);
	mesh.specular_colour.w = clamp(hobjxio->last_mesh_header.specular[3], 0.0f, 1.0f);
	mesh.specular_exponent = hobjxio->last_mesh_header.specularExp;
	mesh.diffuse_file = hobjxio->last_mesh_header.diffuse_file;
	mesh.bump_file = hobjxio->last_mesh_header.bump_file;
	mesh.spc_file = hobjxio->last_mesh_header.spc_file;
	mesh.spe_file = hobjxio->last_mesh_header.spe_file;
	mesh.programme = hobjxio->last_mesh_header.programme;
	mesh.mesh_name = hobjxio->last_mesh_header.name;
	return true;
}

bool _cdecl read_mesh(HOBJXIO handle, mesh_info& mesh)
{
	ATLASSERT(handle);
	OBJXI* hobjxio = reinterpret_cast<OBJXI*>(handle);

	long start_pos = ftell(hobjxio->file);

	size_t bytes = sizeof(vec4) * mesh.num_vertices;
	if(mesh.vertices)
	{
		if(1 != fread(mesh.vertices, bytes, 1, hobjxio->file))
			return false;
	}
	else
	{
		fseek(hobjxio->file, bytes, SEEK_CUR);
	}

	bytes = sizeof(vec4) * mesh.num_vertices;
	if(mesh.normals && hobjxio->last_mesh_header.has_normals)
	{
		if(1 != fread(mesh.normals, bytes, 1, hobjxio->file))
			return false;
	}
	else if(hobjxio->last_mesh_header.has_normals)
	{
		fseek(hobjxio->file, bytes, SEEK_CUR);
	}

	if(mesh.tangents && hobjxio->last_mesh_header.has_tangents)
	{
		if(1 != fread(mesh.tangents, bytes, 1, hobjxio->file))
			return false;
	}
	else if(hobjxio->last_mesh_header.has_tangents)
	{
		fseek(hobjxio->file, bytes, SEEK_CUR);
	}

	bytes = sizeof(float) * 2 * mesh.num_vertices;
	if(mesh.texcoords && hobjxio->last_mesh_header.has_texcoords)
	{
		if(1 != fread(mesh.texcoords, bytes, 1, hobjxio->file))
			return false;

		// if source has no tangents but texcoords, we can generate them from vertices and texcoords
		if(mesh.tangents && !hobjxio->last_mesh_header.has_tangents)
		{
			generate_tangents(mesh, hobjxio->last_mesh_header);
		}
	}
	else if(hobjxio->last_mesh_header.has_texcoords)
	{
		fseek(hobjxio->file, bytes, SEEK_CUR);
	}

	bytes = sizeof(int) * mesh.num_indices;
	if(mesh.indices && hobjxio->last_mesh_header.num_indices > 0)
	{
		if(1 != fread(mesh.indices, bytes, 1, hobjxio->file))
			return false;
	}
	else if(hobjxio->last_mesh_header.num_indices > 0)
	{
		fseek(hobjxio->file, bytes, SEEK_CUR);
	}

	fseek(hobjxio->file, start_pos, SEEK_SET);	
	fseek(hobjxio->file, hobjxio->last_mesh_header.next_offset, SEEK_CUR);

	return true;
}

void _cdecl close_objx(HOBJXIO handle)
{
	OBJXO* hobjxo = (OBJXO*)handle;
	OBJXI* hobjxi = (OBJXI*)handle;
	if(sizeof(OBJXO) == hobjxo->cb) // not input mode (output mode)
	{
		rewind(hobjxo->file);
		fwrite(&hobjxo->file_header, sizeof(objx_header), 1, hobjxo->file);
	}
	fclose(hobjxo->file);
	_aligned_free(handle);
}

static unsigned int calc_mesh_bytes(const mesh_header& header)
{
	unsigned int bytes = header.num_vertices * sizeof(vec4);
	if(header.has_texcoords)
		bytes += header.num_vertices * sizeof(float) * 2;
	if(header.has_normals)
		bytes += header.num_vertices * sizeof(vec4);
	if(header.has_tangents)
		bytes += header.num_vertices * sizeof(vec4);
	if(header.num_indices > 0)
		bytes += header.num_indices * sizeof(unsigned int);
	return bytes;
}

static void generate_tangents(mesh_info& mesh, const mesh_header& header)
{
	vec4 tangent;

	if(header.num_indices > 0)
	{
		void* p = mesh.vertices;
		unsigned int n = mesh.num_vertices;
		__asm
		{
			mov		eax,	p
			mov		ecx,	n
			xorps	xmm0,	xmm0
	lup:	movaps	[eax],	xmm0
			add		eax,	16
			loop	lup
		}

		for(unsigned int i = 0; i < header.num_indices; i+=3)
		{
			int a = mesh.indices[i];
			int b = mesh.indices[i + 1];
			int c = mesh.indices[i + 2];

			generate_tangent(tangent, mesh.vertices[a], mesh.vertices[b], mesh.vertices[c], mesh.texcoords[a], mesh.texcoords[b], mesh.texcoords[c]);

			mcemaths_add_3_4_ip(mesh.tangents[a], tangent);
			mcemaths_add_3_4_ip(mesh.tangents[b], tangent);
			mcemaths_add_3_4_ip(mesh.tangents[c], tangent);
		}

		for(unsigned int i = 0; i < header.num_vertices; i++)
		{
			mcemaths_norm_3_4(mesh.tangents[i]);
		}
	}
	else
	{
		for(unsigned int i = 0; i < header.num_vertices; i+=3)
		{
			generate_tangent(tangent, mesh.vertices[i], mesh.vertices[i + 1], mesh.vertices[i + 2], mesh.texcoords[i], mesh.texcoords[i + 1], mesh.texcoords[i + 2]);
			mcemaths_norm_3_4(tangent);

			mcemaths_quatcpy(mesh.tangents[i], tangent);
			mcemaths_quatcpy(mesh.tangents[i + 1], tangent);
			mcemaths_quatcpy(mesh.tangents[i + 2], tangent);
		}
	}
}

static void generate_tangent(vec4& tangent, const vec4& a, const vec4& b, const vec4& c, const vec2& ta, const vec2& tb, const vec2& tc)
{
	vec2 coord1;
	coord1.x = tc.x - ta.x;
	coord1.y = tc.y - ta.y;

	vec2 coord2;
	coord2.x = tb.x - ta.x;
	coord2.y = tb.y - ta.y;

	vec4 vector1;
	mcemaths_sub_3_4(vector1, c, a);

	vec4 vector2;
	mcemaths_sub_3_4(vector1, b, a);

	mcemaths_mul_3_4(vector1, coord2.y);
	mcemaths_mul_3_4(vector2, coord1.y);
	mcemaths_sub_3_4(tangent, vector1, vector2);

	float factor = 1.0f / (coord1.x * coord2.y - coord1.y * coord2.x);
	mcemaths_mul_3_4(tangent, factor);
}