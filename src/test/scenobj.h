#pragma once

#include <string>
#include <set>
#include <map>
#include "mcemaths.hpp"
#include "pipeline.h"

using namespace std;
using namespace mcemaths;

#define PROCNAME(cls) (typeid(cls).raw_name())
typedef map<string, int> Str2Idx;

const int MAX_SHADOWMAPS = 16;

class MeshBuilder
{
public:
	virtual void build(PuresoftPipeline& pipeline, int vao) = 0;
};

class SceneObject
{
public:
	static bool m_useShadowProgramme;
	static int m_defaultShadowProgramme;
	static int m_shadowMaps[MAX_SHADOWMAPS];
	static mat4 m_shadowPVs[MAX_SHADOWMAPS];

public:
	SceneObject(PuresoftPipeline& pipeline, SceneObject* parent);
	virtual ~SceneObject();
	virtual void update(float timeSpanSec, const mat4& parent);
	virtual void draw(PuresoftPipeline& pipeline, bool extraThread = false);

	void chainSubUpdater(SceneObject* sub);
	void unchainSubUpdater(SceneObject* sub);

protected:
	mat4 m_model;
	mat4 m_rotation;
	mat4 m_scale;
	mat4 m_translation;
	int m_programme;
	int m_vao;

	PuresoftPipeline& m_pipeline;
	static Str2Idx m_meshes;
	static Str2Idx m_textures;
	static Str2Idx m_processers;
	static Str2Idx m_programmes;

	typedef set<SceneObject*> SubColl;
	SubColl m_subUpdaters;

protected:
	int findOrCreateVao(MeshBuilder* builder);
	int findOrCreateVao(const char* objx);
	int findOrCreateTexture(const char* pictureFile);
	int findOrCreateSkybox(const char* pictureFiles[6]);
	int findOrCreateProgramme(const char* vertProcName, const char* interpProcName, const char* fragProcName);
};
