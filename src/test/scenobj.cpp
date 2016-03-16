#include <atlbase.h>
#include "mcemaths.hpp"
#include "libobjx.h"
#include "picldr.h"
#include "procreator.h"
#include "scenobj.h"

using namespace mcemaths;

bool SceneObject::m_useShadowProgramme = true;
int SceneObject::m_defaultShadowProgramme = -1;
Str2Idx SceneObject::m_meshes;
Str2Idx SceneObject::m_textures;
Str2Idx SceneObject::m_processers;
Str2Idx SceneObject::m_programmes;

int SceneObject::m_shadowMaps[MAX_SHADOWMAPS];
mat4 SceneObject::m_shadowPVs[MAX_SHADOWMAPS];

SceneObject::SceneObject(PuresoftPipeline& pipeline, SceneObject* parent)
	: m_pipeline(pipeline)
	, m_vao(-1)
	, m_programme(-1)
{
	if(parent)
	{
		parent->chainSubUpdater(this);
	}
}

SceneObject::~SceneObject()
{
}

void SceneObject::update(float timeSpanSec, const mat4& parent /* = mat4 */)
{
	mcemaths_transform_m4m4_r_ip(parent, m_model);

	for(SubColl::iterator it = m_subUpdaters.begin(); it != m_subUpdaters.end(); it++)
	{
		(*it)->update(timeSpanSec, m_model);
	}
}

void SceneObject::draw(PuresoftPipeline& pipeline)
{
	if(m_useShadowProgramme)
	{
		pipeline.useProgramme(m_defaultShadowProgramme);
	}
	else
	{
		pipeline.useProgramme(m_programme);
	}

	pipeline.drawVAO(m_vao);
}

void SceneObject::chainSubUpdater(SceneObject* sub)
{
	m_subUpdaters.insert(sub);
}

void SceneObject::unchainSubUpdater(SceneObject* sub)
{
	SubColl::iterator it = m_subUpdaters.find(sub);
	if(m_subUpdaters.end() != it)
	{
		m_subUpdaters.erase(it);
	}
}

int SceneObject::findOrCreateVao(MeshBuilder* builder)
{
	const char* name = typeid(*builder).raw_name();

	Str2Idx::iterator it = m_meshes.find(name);
	if(m_meshes.end() != it)
	{
		return it->second;
	}

	int vao = m_pipeline.createVAO();
	builder->build(m_pipeline, vao);
	return m_meshes[name] = vao;
}

int SceneObject::findOrCreateVao(const char* objx)
{
	Str2Idx::iterator it = m_meshes.find(objx);
	if(m_meshes.end() != it)
	{
		return it->second;
	}

	scene_desc unused;
	HOBJXIO hobjx = open_objxA(objx, unused);

	mesh_info mi;
	read_mesh_header(hobjx, mi);
	mi.vertices = new vec4[mi.num_vertices];
	
	if(mi.has_normals)
	{
		mi.normals = new vec4[mi.num_vertices];
	}

	if(mi.has_texcoords)
	{
		mi.texcoords = new vec2[mi.num_vertices];
		mi.tangents = new vec4[mi.num_vertices];
	}

	read_mesh(hobjx, mi);
	close_objx(hobjx);

	for(unsigned int i = 0; i < mi.num_vertices; i++)
	{
		mi.vertices[i].w = 1.0f;
	}

	vec4* binormal;
	if(mi.has_texcoords)
	{
		binormal = new vec4[mi.num_vertices];
		for(unsigned int i = 0; i < mi.num_vertices; i++)
		{
			mcemaths_cross_3(binormal[i], mi.normals[i], mi.tangents[i]);
			mcemaths_norm_3_4(binormal[i]);
		}
	}

	int vao = m_pipeline.createVAO();

	m_pipeline.attachVBO(vao, 0, new PuresoftVBO(16, mi.num_vertices));
	m_pipeline.getVBO(vao, 0)->updateContent(mi.vertices);
	delete[] mi.vertices;

	if(mi.has_texcoords)
	{
		m_pipeline.attachVBO(vao, 1, new PuresoftVBO(16, mi.num_vertices));
		m_pipeline.attachVBO(vao, 2, new PuresoftVBO(16, mi.num_vertices));
		m_pipeline.attachVBO(vao, 4, new PuresoftVBO(8, mi.num_vertices));

		m_pipeline.getVBO(vao, 1)->updateContent(mi.tangents);
		m_pipeline.getVBO(vao, 2)->updateContent(binormal);
		m_pipeline.getVBO(vao, 4)->updateContent(mi.texcoords);

		delete[] mi.tangents;
		delete[] binormal;
		delete[] mi.texcoords;
	}

	if(mi.normals)
	{
		m_pipeline.attachVBO(vao, 3, new PuresoftVBO(16, mi.num_vertices));
		m_pipeline.getVBO(vao, 3)->updateContent(mi.normals);

		delete[] mi.normals;
	}

	return m_meshes[objx] = vao;
}

int SceneObject::findOrCreateTexture(const char* pictureFile)
{
	Str2Idx::iterator it = m_textures.find(pictureFile);
	if(m_textures.end() != it)
	{
		return it->second;
	}

	USES_CONVERSION;

	PURESOFTIMGBUFF32 image;
	PuresoftDefaultPictureLoader picLoader;
	picLoader.loadFromFile(CA2W(pictureFile), &image);
	int tex = m_pipeline.createTexture(&image, 0, PuresoftFBO::WRAP);
	m_pipeline.getTexture(tex, &image);
	picLoader.retrievePixel(&image);
	picLoader.close();
	return m_textures[pictureFile] = tex;
}

int SceneObject::findOrCreateSkybox(const char* pictureFiles[6])
{
	string name;
	for(int i = 0; i < 6; i++)
	{
		name += pictureFiles[i];
		name += ";";
	}

	Str2Idx::iterator it = m_textures.find(name);
	if(m_textures.end() != it)
	{
		return it->second;
	}

	USES_CONVERSION;

	PURESOFTIMGBUFF32 image;
	PuresoftDefaultPictureLoader picLoader;
	picLoader.loadFromFile(CA2W(pictureFiles[0]), &image);
	image.pixels = NULL;
	int tex = m_pipeline.createTexture(&image, 5);
	m_pipeline.getTexture(tex, &image, PuresoftFBO::LAYER_XPOS);
	picLoader.retrievePixel(&image);
	picLoader.close();
	for(int i = 1; i < 6; i++)
	{
		picLoader.loadFromFile(CA2W(pictureFiles[i]), &image);
		m_pipeline.getTexture(tex, &image, (PuresoftFBO::LAYER)i);
		picLoader.retrievePixel(&image);
		picLoader.close();
	}

	return m_textures[name] = tex;
}

int SceneObject::findOrCreateProgramme(const char* vertProcName, const char* interpProcName, const char* fragProcName)
{
	string progName = string(vertProcName) + ";" + string(interpProcName) + ";" + string(fragProcName);

	Str2Idx::iterator prog = m_programmes.find(progName);
	if(m_programmes.end() != prog)
	{
		return prog->second;
	}

	Str2Idx::iterator v = m_processers.find(vertProcName), i = m_processers.find(interpProcName), f = m_processers.find(fragProcName);

	if(m_processers.end() == v)
	{
		m_processers[vertProcName] = m_pipeline.addProcessor(createProcessor(vertProcName));
		v = m_processers.find(vertProcName);
	}

	if(m_processers.end() == i)
	{
		m_processers[interpProcName] = m_pipeline.addProcessor(createProcessor(interpProcName));
		i = m_processers.find(interpProcName);
	}

	if(m_processers.end() == f)
	{
		m_processers[fragProcName] = m_pipeline.addProcessor(createProcessor(fragProcName));
		f = m_processers.find(fragProcName);
	}

	return m_programmes[progName.c_str()] = m_pipeline.createProgramme(v->second, i->second, f->second);
}