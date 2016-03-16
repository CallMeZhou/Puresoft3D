#include <windows.h>
#include <atlbase.h>
#include <shlwapi.h>
#include <algorithm>
#include "fixvec.hpp"
#include "picldr.h"
#include "procreator.h"
#include "loadscene.h"

using namespace std;
using namespace mcemaths;

typedef struct
{
	vec4 translation;		// space translation
	vec4 ambientColour;		// material
	vec4 diffuseColour;		// material
	vec4 specularColour;	// material
	string diffuseFile;		// material
	string bumpFile;		// material
	string spcFile;			// material
	string speFile;			// material
	string programme;		// material
	float specularExponent;	// material
	unsigned int numVerts;	// mesh
	vec4* vertices;			// mesh
	vec4* normals;			// mesh
	vec4* tangents;			// mesh
	vec4* binormals;		// mesh
	vec2* texcoords;		// mesh
} MESH;

typedef struct
{
	vec4 translation;
	typedef map<string, MESH, less<string>, alloc16<MESH>> MESH_COLL;
	MESH_COLL meshes;
} OBJECT;

typedef map<string, OBJECT, less<string>, alloc16<OBJECT>> OBJ_COLL;
typedef map<string, int> MapStrToIndex;

const char* const ROOTNAME = "/";

static int findOrCreateTexture(const string& pictureFile, PuresoftPipeline* pipeline);
static int findOrCreateProgramme(const string& programme, PuresoftPipeline* pipeline);

//////////////////////////////////////////////////////////////////////////
bool SceneObject::m_usePrivateProgramme = true;

SceneObject::SceneObject(void)
{
	m_parent = NULL;
	m_vao = 
	m_prog = -1;
	m_diffuseTex = 
	m_bumpTex = 
	m_spcTex = 
	m_speTex = -2;
	m_specularExponent = 0;
}

SceneObject::SceneObject(const SceneObject& src)
{
	*this = src;
}

SceneObject::~SceneObject()
{
}

const SceneObject& SceneObject::operator= (const SceneObject& src)
{
	m_translation = src.m_translation;
	m_rotation = src.m_rotation;
	m_scale = src.m_scale;
	m_model = src.m_model;
	m_ambientColour = src.m_ambientColour;
	m_diffuseColour = src.m_diffuseColour;
	m_specularColour = src.m_specularColour;
	m_diffuseTex = src.m_diffuseTex;
	m_bumpTex = src.m_bumpTex;
	m_spcTex = src.m_spcTex;
	m_speTex = src.m_speTex;
	m_specularExponent = src.m_specularExponent;
	m_prog = src.m_prog;
	m_vao = src.m_vao;
	m_name = src.m_name;
	m_parent = src.m_parent;
	m_children = src.m_children;
	return *this;
}

void SceneObject::update(float timeSpanSec, const mat4& parent, const mat4& pv)
{
	// m_model = parent * m_translation * m_scale * m_rotation
	mcemaths_transform_m4m4(m_model, m_scale, m_rotation);
	mcemaths_transform_m4m4_r_ip(m_translation, m_model);
	mcemaths_transform_m4m4_r_ip(parent, m_model);

	// m_pvm = project * view * (parent * m_translation * m_scale * m_rotation)
	mcemaths_transform_m4m4(m_pvm, pv, m_model);

	for(size_t i = 0; i < m_children.size(); i++)
		m_children[i]->update(timeSpanSec, m_model, pv);
}

void SceneObject::draw(PuresoftPipeline& pipeline)
{
	pipeline.setUniform(0, m_model, sizeof(mat4));
	pipeline.setUniform(1, m_rotation, sizeof(mat4));
	pipeline.setUniform(5, m_pvm, sizeof(mat4));
	pipeline.setUniform(30, m_ambientColour, sizeof(vec4));
	pipeline.setUniform(31, m_diffuseColour, sizeof(vec4));
	pipeline.setUniform(32, m_specularColour, sizeof(vec4));
	pipeline.setUniform(33, &m_specularExponent, sizeof(float));
	pipeline.setUniform(40, &m_diffuseTex, sizeof(int));
	pipeline.setUniform(41, &m_bumpTex, sizeof(int));
	pipeline.setUniform(42, &m_spcTex, sizeof(int));
	pipeline.setUniform(43, &m_speTex, sizeof(int));

	if(m_vao < 0)
		return;

	if(m_usePrivateProgramme)
	{
		pipeline.useProgramme(m_prog);
	}

	pipeline.drawVAO(m_vao);
}

void SceneObject::loadScene(const char* objx, PuresoftPipeline* pipeline, SceneObjects& sceneObjectColl, scene_desc& sceneDesc)
{
	OBJ_COLL objects;

	// load objx file
	HOBJXIO hobjx = open_objxA(objx, sceneDesc);

	for(int i = 0; i < get_mesh_count(hobjx); i++)
	{
		// read object/mesh header
		mesh_info info;
		memset(&info, 0, sizeof(info));
		read_mesh_header(hobjx, info);

		// skip if name is in wrong form
		int delimit = (int)info.mesh_name.find_first_of('/');
		if(delimit < 0)
		{
			read_mesh(hobjx, info);
			continue;
		}

		// mesh_info::mesh_name is in form of 'object_name/mesh_name' (i.e., lamp/shade)
		string objectName = info.mesh_name.substr(0, delimit);
		string meshName = info.mesh_name.substr(delimit + 1);

		// allocate buffers
		info.vertices = new vec4[info.num_vertices];
		if(info.has_normals)
		{
			info.normals = new vec4[info.num_vertices];
		}

		if(info.has_texcoords)
		{
			info.texcoords = new vec2[info.num_vertices];
			info.tangents = new vec4[info.num_vertices];
		}

		// read mesh data
		read_mesh(hobjx, info);

		// generate binormal (better than doing it in shader)
		vec4* binormal;
		if(info.has_texcoords)
		{
			binormal = new vec4[info.num_vertices];
			for(unsigned int i = 0; i < info.num_vertices; i++)
			{
				mcemaths_cross_3(binormal[i], info.normals[i], info.tangents[i]);
				mcemaths_norm_3_4(binormal[i]);
			}
		}

		// store in temp
		OBJECT& newObj = objects[objectName];
		MESH& newMesh = newObj.meshes[meshName];

		newMesh.numVerts = info.num_vertices;
		newMesh.vertices =info.vertices;
		newMesh.normals = info.normals;
		newMesh.tangents = info.tangents;
		newMesh.binormals = binormal;
		newMesh.texcoords = info.texcoords;
		newMesh.ambientColour = info.ambient_colour;
		newMesh.diffuseColour = info.diffuse_colour;
		newMesh.specularColour = info.specular_colour;
		newMesh.diffuseFile = info.diffuse_file;
		newMesh.bumpFile = info.bump_file;
		newMesh.spcFile = info.spc_file;
		newMesh.speFile = info.spe_file;
		newMesh.programme = info.programme;
		newMesh.specularExponent = info.specular_exponent;
	}

	// loading objx file completed
	close_objx(hobjx);

	// positions in objx file are all in world space

	// for meshes, translate positions from world space to model space. will find out translations.
	for(OBJ_COLL::iterator obj = objects.begin(); obj != objects.end(); obj++)
	{
		for(OBJECT::MESH_COLL::iterator mesh = obj->second.meshes.begin(); mesh != obj->second.meshes.end(); mesh++)
		{
			// find bounding box
			vec4 minpos(FLT_MAX, 0), maxpos(-FLT_MAX, 0);
			for(unsigned int i = 0; i < mesh->second.numVerts; i++)
			{
				mcemaths_minpos_3_4_ip(minpos, mesh->second.vertices[i]);
				mcemaths_maxpos_3_4_ip(maxpos, mesh->second.vertices[i]);
			}

			// find centre of bounding box, the centre is translation
			mcemaths_line_centre(mesh->second.translation, NULL, minpos, maxpos);

			// translate W->M
			for(unsigned int i = 0; i < mesh->second.numVerts; i++)
			{
				mcemaths_sub_3_4_ip(mesh->second.vertices[i], mesh->second.translation);
				// by the way enable vertex translation (better than doing it in shader)
				mesh->second.vertices[i].w = 1.0f;
			}
		}
	}

	// for objects, calculate geometry centres, geo-centre in world space is also the object's translation from
	// model space to world space
	for(OBJ_COLL::iterator obj = objects.begin(); obj != objects.end(); obj++)
	{
		for(OBJECT::MESH_COLL::iterator mesh = obj->second.meshes.begin(); mesh != obj->second.meshes.end(); mesh++)
		{
			mcemaths_add_3_4_ip(obj->second.translation, mesh->second.translation);
		}

		mcemaths_div_3_4(obj->second.translation, (float)obj->second.meshes.size());
	}

	// recalculate mesh translations, we now regards mesh as sub-object
	for(OBJ_COLL::iterator obj = objects.begin(); obj != objects.end(); obj++)
	{
		for(OBJECT::MESH_COLL::iterator mesh = obj->second.meshes.begin(); mesh != obj->second.meshes.end(); mesh++)
		{
			mcemaths_sub_3_4_ip(mesh->second.translation, obj->second.translation);
		}
	}

	// now, a mesh in MESH_COLL is an object with mesh, an object in OBJ_COLL is an root object without mesh but sub-objects
	// we can proceed to build SceneObject tree
	SceneObject& sceneRoot = sceneObjectColl[ROOTNAME];
	for(OBJ_COLL::iterator obj = objects.begin(); obj != objects.end(); obj++)
	{
		// create new character and set translation
		SceneObject& character = sceneObjectColl[obj->first];
		character.m_translation.translation(obj->second.translation);

		// inter-connect with scene-root
		character.m_parent = &sceneRoot;
		sceneRoot.m_children.push_back(&character);

		// character name
		character.m_name = obj->first;

		for(OBJECT::MESH_COLL::iterator mesh = obj->second.meshes.begin(); mesh != obj->second.meshes.end(); mesh++)
		{
			string componentName = obj->first + "/" + mesh->first;

			// create new component for current character and set component's translation
			SceneObject& component = sceneObjectColl[componentName];
			component.m_translation.translation(mesh->second.translation);

			// inter-connect with character
			component.m_parent = &character;
			character.m_children.push_back(&component);

			// create vao-vbo for the component
			component.m_vao = pipeline->createVAO();

			pipeline->attachVBO(component.m_vao, 0, new PuresoftVBO(16, mesh->second.numVerts));
			pipeline->getVBO(component.m_vao, 0)->updateContent(mesh->second.vertices);

			if(mesh->second.texcoords)
			{
				pipeline->attachVBO(component.m_vao, 1, new PuresoftVBO(16, mesh->second.numVerts));
				pipeline->attachVBO(component.m_vao, 2, new PuresoftVBO(16, mesh->second.numVerts));
				pipeline->attachVBO(component.m_vao, 4, new PuresoftVBO(8, mesh->second.numVerts));

				pipeline->getVBO(component.m_vao, 1)->updateContent(mesh->second.tangents);
				pipeline->getVBO(component.m_vao, 2)->updateContent(mesh->second.binormals);
				pipeline->getVBO(component.m_vao, 4)->updateContent(mesh->second.texcoords);
			}

			if(mesh->second.normals)
			{
				pipeline->attachVBO(component.m_vao, 3, new PuresoftVBO(16, mesh->second.numVerts));
				pipeline->getVBO(component.m_vao, 3)->updateContent(mesh->second.normals);
			}

			// load texture for the component
			if(PathFileExistsA(mesh->second.diffuseFile.c_str()))
			{
				component.m_diffuseTex = findOrCreateTexture(mesh->second.diffuseFile, pipeline);
			}

			if(PathFileExistsA(mesh->second.bumpFile.c_str()))
			{
				component.m_bumpTex = findOrCreateTexture(mesh->second.bumpFile, pipeline);
			}

			if(PathFileExistsA(mesh->second.spcFile.c_str()))
			{
				component.m_spcTex = findOrCreateTexture(mesh->second.spcFile, pipeline);
			}

			if(PathFileExistsA(mesh->second.speFile.c_str()))
			{
				component.m_speTex = findOrCreateTexture(mesh->second.speFile, pipeline);
			}
	
			component.m_ambientColour = mesh->second.ambientColour;
			component.m_diffuseColour = mesh->second.diffuseColour;
			component.m_specularColour = mesh->second.specularColour;
			component.m_specularExponent = mesh->second.specularExponent;

			component.m_name = componentName;
			component.m_prog = findOrCreateProgramme(mesh->second.programme, pipeline);
		}
	}
}

static int findOrCreateTexture(const string& pictureFile, PuresoftPipeline* pipeline)
{
	static MapStrToIndex textures;

	MapStrToIndex::iterator it = textures.find(pictureFile);
	if(textures.end() != it)
	{
		return it->second;
	}

	USES_CONVERSION;

	PURESOFTIMGBUFF32 image;
	PuresoftDefaultPictureLoader picLoader;
	picLoader.loadFromFile(CA2W(pictureFile.c_str()), &image);
	int tex = pipeline->createTexture(&image);
	pipeline->getTexture(tex, &image);
	picLoader.retrievePixel(&image);
	picLoader.close();
	return textures[pictureFile] = tex;
}

static int findOrCreateProgramme(const string& programme, PuresoftPipeline* pipeline)
{
	static MapStrToIndex processors, programmes;

	MapStrToIndex::iterator prog = programmes.find(programme);
	if(programmes.end() != prog)
	{
		return prog->second;
	}

	int colon1 = programme.find_first_of(':'), colon2 = programme.find_last_of(':');

	if(colon1 < 0 || colon2 < 0 || colon1 == colon2)
	{
		return -1;
	}

	string	vertProcName = programme.substr(0, colon1),
			interpProcName = programme.substr(colon1 + 1, colon2 - colon1 - 1), 
			fragProcName = programme.substr(colon2 + 1);

	MapStrToIndex::iterator	v = processors.find(vertProcName), 
							i = processors.find(interpProcName), 
							f = processors.find(fragProcName);

	if(processors.end() == v)
	{
		processors[vertProcName] = pipeline->addProcessor(createProcessorClassPrefix(vertProcName.c_str()));
		v = processors.find(vertProcName);
	}

	if(processors.end() == i)
	{
		processors[interpProcName] = pipeline->addProcessor(createProcessorClassPrefix(interpProcName.c_str()));
		i = processors.find(interpProcName);
	}

	if(processors.end() == f)
	{
		processors[fragProcName] = pipeline->addProcessor(createProcessorClassPrefix(fragProcName.c_str()));
		f = processors.find(fragProcName);
	}

	return programmes[programme] = pipeline->createProgramme(v->second, i->second, f->second);
}

SceneObject& SceneObject::getRoot(SceneObjects& sceneObjectColl)
{
	return sceneObjectColl[ROOTNAME];
}

void SceneObject::getLightSource1(SceneObjects& sceneObjectColl, vec4& from, vec4& to)
{
	from.set(0);
	to.set(0);

	// let them update model matrix, we'll take translation as mesh centre
	getRoot(sceneObjectColl).update(0, mat4(), mat4());

	// these are mark meshes, not for display, but for light source
	SceneObjects::iterator _from = sceneObjectColl.find("light1/from"), _to = sceneObjectColl.find("light1/to"), light1 = sceneObjectColl.find("light1");
	if(sceneObjectColl.end() == _from || sceneObjectColl.end() == _to || sceneObjectColl.end() == light1)
		return;

	mcemaths_quatcpy(from, &_from->second.m_model[12]);
	mcemaths_quatcpy(to, &_to->second.m_model[12]);
	from.w = to.w = 0;

	light1->second.m_children.clear();
	sceneObjectColl.erase(_from);
	sceneObjectColl.erase(_to);
}
