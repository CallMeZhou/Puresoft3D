#include "stdafx.h"

const material DEF_MATERIAL = 
{
	vec4(0.8f, 0.8f, 0.8f, 0), 
	vec4(0.8f, 0.8f, 0.8f, 0), 
	vec4(0.8f, 0.8f, 0.8f, 0), 
	1.0f, "", ""
};

bool write_face(HOBJXIO hobjxio, const string& name, const faces& fcs, const obj_mesh_data& dat, const material& mtl)
{
	static vec4_coll vertices;
	static vec4_coll normals;
	static vec4_coll tangents;
	static vec2_coll texcoords;

	mesh_info mesh;
	mesh.num_indices = 0;
	mesh.indices = NULL;

	for(int i = 0; i < faces::MAX; i++)
	{
		const face& fc = fcs.face_data[i];
		// stat vertices
		mesh.num_vertices = 0;
		for(int i = 0; i < (int)fc.size(); i++)
			// 3 indices for 1 triangle, 4 for 2, 5 for 3, ... n indices for n-2 triangles make (n-2)*3 vertices
			mesh.num_vertices += (fc[i].vertIndices.size() - 2) * 3;

		if(0 == mesh.num_vertices)
			continue;

		// prepare container
		vertices.clear();
		normals.clear();
		tangents.clear();
		texcoords.clear();

		// for each facet
		for(int i = 0; i < (int)fc.size(); i++)
		{
			const facet& f = fc[i];
			// facet -> triangles
			for(int j = 1; j < (int)f.vertIndices.size() - 1; j++)
			{
				vertices.push_back(dat.vertices[f.vertIndices[    0]]);
				vertices.push_back(dat.vertices[f.vertIndices[    j]]);
				vertices.push_back(dat.vertices[f.vertIndices[j + 1]]);
			}
		}
		ATLASSERT(vertices.size() > 0);
		mesh.vertices = &vertices[0];

		// for each facet
		for(int i = 0; i < (int)fc.size(); i++)
		{
			const facet& f = fc[i];
			// facet -> triangles
			for(int j = 1; j < (int)f.normIndices.size() - 1; j++)
			{
				normals.push_back(dat.normals[f.normIndices[    0]]);
				normals.push_back(dat.normals[f.normIndices[    j]]);
				normals.push_back(dat.normals[f.normIndices[j + 1]]);
			}
		}

		// unlike vertices, normal data can be absent
		if(0 == normals.size()) // if normal not exists, we generate
		{
			for(int i = 0; i < (int)vertices.size(); i += 3)
			{
				const vec4& a = vertices[    i];
				const vec4& b = vertices[i + 1];
				const vec4& c = vertices[i + 2];
				vec4 edge1, edge2, n;
				mcemaths_sub_3_4(edge1, c, b);
				mcemaths_sub_3_4(edge2, a, b);
				mcemaths_cross_3(n, edge1, edge2);
				mcemaths_norm_3_4(n);
				normals.push_back(n);
				normals.push_back(n);
				normals.push_back(n);
			}
		}
		mesh.normals = &normals[0];
		mesh.has_normals = true;

		// for each facet
		for(int i = 0; i < (int)fc.size(); i++)
		{
			const facet& f = fc[i];
			// facet -> triangles
			for(int j = 1; j < (int)f.texIndices.size() - 1; j++)
			{
				texcoords.push_back(dat.texcoords[f.texIndices[    0]]);
				texcoords.push_back(dat.texcoords[f.texIndices[    j]]);
				texcoords.push_back(dat.texcoords[f.texIndices[j + 1]]);
			}
		}

 		mesh.texcoords = NULL;
 		mesh.tangents = NULL;
 		mesh.has_texcoords = mesh.has_tangents = false;
		if(texcoords.size() > 0) // unlike vertices, texture coordinate can also be absent
		{
			mesh.texcoords = &texcoords[0];
			mesh.has_texcoords = true;

			// if texcoords exist, generate tangents (save time for game initialization as much as possible)
 			for(int i = 0; i < (int)vertices.size(); i += 3)
 			{
 				const vec4&  a = vertices [    i];
 				const vec4&  b = vertices [i + 1];
 				const vec4&  c = vertices [i + 2];
				const vec2& ta2 = texcoords[    i];
				const vec2& tb2 = texcoords[i + 1];
				const vec2& tc2 = texcoords[i + 2];
				vec4 ta(ta2.x, ta2.y, 0, 0);
				vec4 tb(tb2.x, tb2.y, 0, 0);
				vec4 tc(tc2.x, tc2.y, 0, 0);
 				vec4 modvec1(0.0f), modvec4(0.0f), texvec1(0.0f), texvec4(0.0f);
 				mcemaths_sub_3_4(modvec1, c , a);
 				mcemaths_sub_3_4(modvec4, b , a);
 				mcemaths_sub_3_4(texvec1, tc, ta);
 				mcemaths_sub_3_4(texvec4, tb, ta);
 				mcemaths_mul_3_4(modvec1, texvec4.y);
 				mcemaths_mul_3_4(modvec4, texvec1.y);
 				mcemaths_sub_3_4_ip(modvec1, modvec4);
 				mcemaths_div_3_4(modvec1, texvec1.x * texvec4.y - texvec4.x * texvec1.y);
 				mcemaths_norm_3_4(modvec1);
 				tangents.push_back(modvec1);
 				tangents.push_back(modvec1);
 				tangents.push_back(modvec1);
 			}
			mesh.tangents = &tangents[0];
			mesh.has_tangents = true;
		}

		mesh.mesh_name = name;
		mesh.ambient_colour = vec4(mtl.ambientColour.x, mtl.ambientColour.y, mtl.ambientColour.z, 1.0f);
		mesh.diffuse_colour = vec4(mtl.diffuseColour.x, mtl.diffuseColour.y, mtl.diffuseColour.z, 1.0f);
		mesh.specular_colour = vec4(mtl.specularColour.x, mtl.specularColour.y, mtl.specularColour.z, 1.0f);
		mesh.specular_exponent = mtl.specularExponent;
		mesh.diffuse_file = mtl.diffuseFile;
		mesh.bump_file = mtl.bumpFile;
		mesh.spc_file = mtl.spcFile;
		mesh.spe_file = mtl.speFile;
		mesh.programme = mtl.programmeName;

		if(!write_mesh(hobjxio, mesh))
			return false;
	}

	return true;
}

string disp_rgb(const vec4& c)
{
	char disp_rgb_buffer[1024];
	sprintf_s(disp_rgb_buffer, 1024, "(%.1f, %.1f, %.1f)", c.x, c.y, c.z);
	return disp_rgb_buffer;
}

const char* disp_mtl_name(string name)
{
	static char disp_mtl_buffer[1024];
	size_t pos = name.find_last_of(':');
	if(name.npos != pos)
		strcpy_s(disp_mtl_buffer, 1024, name.substr(pos + 1).c_str());
	else
		strcpy_s(disp_mtl_buffer, 1024, name.c_str());
	return disp_mtl_buffer;
}

void disp_statistics(const obj_mesh_data& coords, const objects& objs, const mtllib& mtl)
{
	cout << endl 
		 << "=============== Statistics ===============" 
		 << endl;

	for(int i = 0; i < (int)objs.size(); i++)
	{
		cout << "Object" << i << endl;
		const groups& grps = objs[i];
		for(groups::const_iterator g = grps.begin(); g != grps.end(); g++)
		{
			cout << "  Group \"" << g->first << "\"" << endl;
			for(group::const_iterator it = g->second.begin(); it != g->second.end(); it++)
			{
				int tri_tex = it->second.face_data[faces::HAS_TEXCOORD].size() - 2;
				tri_tex = max(tri_tex, 0);
				int tri_notex = it->second.face_data[faces::NO_TEXCOORD].size() - 2;
				tri_notex = max(tri_notex, 0);

				cout << "    Material \"" << disp_mtl_name(it->first) << "\" : " << tri_tex + tri_notex << " triangles ( " << tri_tex << " textured, " << tri_notex << " non-textured )" << endl;
			}
		}
	}

	cout << endl;
	cout << "Total Vertices:  " << coords.vertices.size() << endl
		 << "Total Normals:   " << coords.normals.size() << endl
		 << "Total Texcoords: " << coords.texcoords.size() << endl << endl;

	cout << "Materials (.mtl file) :" << endl;
	for(mtllib::const_iterator it = mtl.begin(); it != mtl.end(); it++)
	{
		cout << "  \"" << disp_mtl_name(it->first) << "\"" << endl
			 << "    Ambient  colour        = " << disp_rgb(it->second.ambientColour) << endl
			 << "    Diffuse  colour        = " << disp_rgb(it->second.diffuseColour) << endl
			 << "    Specular colour        = " << disp_rgb(it->second.specularColour) << endl
			 << "    Specularity exponent   = " << it->second.specularExponent << endl
			 << "    Texture file           > \"" << it->second.diffuseFile << "\""<< endl
			 << "    Bump file              > \"" << it->second.bumpFile << "\"" << endl
			 << "    Specular colour file   > \"" << it->second.spcFile << "\"" << endl
			 << "    Specular exponent file > \"" << it->second.speFile << "\"" << endl
			 << "    Shader programme       > \"" << it->second.programmeName << "\"" << endl;
	}

	cout << endl << "==========================================" << endl;
}

void _stdcall disp_propress(float percent)
{
	printf("Processing ... %.2f%%\r", percent * 100.0f);
}

int _tmain(int argc, _TCHAR* argv[])
{
	cout << "Zhou's obj -> objx Converter, Mar.2016." << endl << endl;
	if(argc < 2)
	{
		cout << "Usage:" << endl << endl 
			 << "objcvt.exe [-a] [-c] [-p] \"input file path name\"" << endl<< endl 
			 << "-a: amalgamate groups as much as possible. Groups using different materials, or belonging to different objects, cannot be merged together." << endl 
			 << "-c: move geometry centre to the origin. Use it only when the source file contains one mesh" << endl 
			 << "-p: remove path from texture file name." << endl << endl;
		system("pause");
		return E_INVALIDARG;
	}

	TCHAR infile[MAX_PATH], outfile[MAX_PATH];
	bool amalgamation = false, centralization = false, removepath = false;
	for(int i = 1; i < argc; i++)
	{
		if(0 == StrCmpI(_T("-a"), argv[i]))
			amalgamation = true;
		else if(0 == StrCmpI(_T("-c"), argv[i]))
			centralization = true;
		else if(0 == StrCmpI(_T("-p"), argv[i]))
			removepath = true;
		else
			_tcscpy_s(infile, MAX_PATH, argv[i]);
	}
	_tcscpy_s(outfile, MAX_PATH, infile);
	PathRenameExtension(outfile, _T(".objx"));

	obj_mesh_data data;
	objects objs;
	mtllib mtlib;
	scene scn;
	if(!load_obj_file(infile, data, objs, mtlib, scn, amalgamation, removepath, disp_propress))
	{
		int err = GetLastError();
		printf("Failed to open input file, error code: %X.\n\n", err);
		system("pause");
		return err;
	}

	if(centralization)
		centralize(data.vertices);

	disp_statistics(data, objs, mtlib);

	scene_desc sd;
	sd.camera_pos = scn.cameraPosition;
	sd.camera_ypr = scn.cameraYPR;
	for(int i = 0; i < MAXLIGHTS; i++)
	{
		sd.light_pos[i] = scn.lightPositions[i];
		sd.light_dir[i] = scn.lightDirections[i];
	}
	HOBJXIO hobjxio = create_objx(outfile, sd);
	if(!hobjxio)
	{
		int err = GetLastError();
		printf("Failed to open input file, error code: %X.\n\n", err);
		system("pause");
		return err;
	}

	mesh_info mesh;
	for(int o = 0; o < (int)objs.size(); o++)
	{
		groups& grps = objs[o];
		for(groups::iterator g = grps.begin(); g != grps.end(); g++)
		{
			group& grp = g->second;
			for(group::iterator it = grp.begin(); it != grp.end(); it++)
			{
				material mtl;
				mtllib::iterator it_mtl = mtlib.find(it->first);
				if(mtlib.end() == it_mtl)
					mtl = DEF_MATERIAL;
				else
					mtl = it_mtl->second;

				faces& fs = it->second;
				if(!write_face(hobjxio, g->first, fs, data, mtl))
				{
					int err = GetLastError();
					printf("Failed to write output file, error code: %X.\n\n", err);
					break;
				}
			}
		}
	}

	close_objx(hobjxio);
	cout << endl << endl << "Succeeded." << endl << "Output file: \"" << CT2CA(outfile) << "\"" << endl << endl;
	system("pause");
	return 0;
}

