#include "stdafx.h"
#include "objxio.h"
#include "privhlpr.h"

#define OBJX_CURVERSION		0x10001
#define OBJX_FILENAMELEN	260

#pragma pack(4)
struct objx_header
{
	unsigned int version;
	unsigned int num_meshes;
	unsigned int mesheader_size;
};
struct mesh_header
{
	unsigned int num_vertices;
	unsigned int num_indices;
	bool has_texcoords;
	bool has_normals;
	bool has_tangents;
	float single_col_r;
	float single_col_g;
	float single_col_b;
	float single_col_a;
	float specularity;
	char tex_file[OBJX_FILENAMELEN];
	char bump_file[OBJX_FILENAMELEN];
	unsigned int next_offset; // offset of next mesh, for extension/compatibility purpose
};
#pragma pack()

class OBJXO
{
public:
	virtual ~OBJXO(){}
	objx_header file_header;
	FILE* file;
};

class OBJXI : public OBJXO
{
public:
	mesh_header last_mesh_header;
};

static unsigned int calc_mesh_bytes(const mesh_header& header);
//////////////////////////////////////////////////////////////////////////

HOBJXIO _cdecl create_objxW(const wchar_t* filename)
{
	OBJXO* handle = new OBJXO;
	memset(&handle->file_header, 0, sizeof(objx_header));
	handle->file_header.version = OBJX_CURVERSION;
	handle->file_header.mesheader_size = sizeof(mesh_header);
	if(0 != _wfopen_s(&handle->file, filename, _T("w+b")))
	{
		delete handle;
		return NULL;
	}
	fwrite(handle, sizeof(objx_header), 1, handle->file);
	return dynamic_cast<HOBJXIO>(handle);
}

HOBJXIO _cdecl create_objxA(const char* filename)
{
	USES_CONVERSION;
	return create_objxW(A2CW(filename));
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
	header.single_col_r = mesh.single_colour.x;
	header.single_col_g = mesh.single_colour.y;
	header.single_col_b = mesh.single_colour.z;
	header.single_col_a = mesh.single_colour.w;
	header.specularity = mesh.specularity;
	strcpy_s(header.tex_file, OBJX_FILENAMELEN, mesh.tex_file.c_str());
	strcpy_s(header.bump_file, OBJX_FILENAMELEN, mesh.bump_file.c_str());
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

HOBJXIO	_cdecl open_objxW(const wchar_t* filename)
{
	OBJXI* handle = new OBJXI;
	if(0 != _tfopen_s(&handle->file, filename, _T("rb")))
	{
		delete handle;
		return NULL;
	}
	fread(&handle->file_header, sizeof(objx_header), 1, handle->file);
	if(handle->file_header.mesheader_size < sizeof(mesh_header))
	{
		fclose(handle->file);
		delete handle;
		return NULL;
	}
	memset(&handle->last_mesh_header, 0, sizeof(handle->last_mesh_header));
	return dynamic_cast<HOBJXIO>(handle);
}

HOBJXIO	_cdecl open_objxA(const char* filename)
{
	USES_CONVERSION;
	return open_objxW(A2CW(filename));
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

	fseek(hobjxio->file, hobjxio->file_header.mesheader_size - bytes, SEEK_CUR);

	mesh.num_vertices = hobjxio->last_mesh_header.num_vertices;
	mesh.num_indices = hobjxio->last_mesh_header.num_indices;
	mesh.has_normals = hobjxio->last_mesh_header.has_normals;
	mesh.has_tangents = hobjxio->last_mesh_header.has_tangents;
	mesh.has_texcoords = hobjxio->last_mesh_header.has_texcoords;
	mesh.single_colour.x = clamp(hobjxio->last_mesh_header.single_col_r, 0.0f, 1.0f);
	mesh.single_colour.y = clamp(hobjxio->last_mesh_header.single_col_g, 0.0f, 1.0f);
	mesh.single_colour.z = clamp(hobjxio->last_mesh_header.single_col_b, 0.0f, 1.0f);
	mesh.single_colour.w = 1.0f;
	mesh.specularity = hobjxio->last_mesh_header.specularity;
	mesh.tex_file = hobjxio->last_mesh_header.tex_file;
	mesh.bump_file = hobjxio->last_mesh_header.bump_file;
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
	OBJXO* hobjxo = static_cast<OBJXO*>(handle);
	OBJXI* hobjxi = dynamic_cast<OBJXI*>(hobjxo);
	if(!hobjxi) // not input mode (output mode)
	{
		rewind(hobjxo->file);
		fwrite(&hobjxo->file_header, sizeof(objx_header), 1, hobjxo->file);
	}
	fclose(hobjxo->file);
	// necessary? not sure...
	if(hobjxi) delete hobjxi; else delete hobjxo;
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
