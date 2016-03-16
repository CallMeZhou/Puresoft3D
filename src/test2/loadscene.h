#pragma once
#include <string>
#include <vector>
#include <map>
#include "mcemaths.hpp"
#include "alloc16.hpp"
#include "libobjx.h"
#include "pipeline.h"

using namespace std;
using namespace mcemaths;

class SceneObject
{
public:
	typedef map<string, SceneObject, less<string>, alloc16<SceneObject> > SceneObjects;
	static void loadScene(const char* objx, PuresoftPipeline* pipeline, SceneObjects& sceneObjectColl, scene_desc& sceneDesc);
	static SceneObject& getRoot(SceneObjects& sceneObjectColl);
	static void getLightSource1(SceneObjects& sceneObjectColl, vec4& from, vec4& to);
	static bool m_usePrivateProgramme;

public:
	SceneObject(void);
	SceneObject(const SceneObject& src);
	~SceneObject();
	const SceneObject& operator= (const SceneObject& src);

public:
	void update(float timeSpanSec, const mat4& parent, const mat4& pv);
	void draw(PuresoftPipeline& pipeline);

private:
	mat4 m_translation;
	mat4 m_rotation;
	mat4 m_scale;
	mat4 m_model;
	mat4 m_pvm;
	vec4 m_ambientColour;
	vec4 m_diffuseColour;
	vec4 m_specularColour;
	
	int m_diffuseTex;
	int m_bumpTex;
	int m_spcTex;
	int m_speTex;
	float m_specularExponent;

	int m_prog;
	int m_vao;

	string m_name;

	const SceneObject* m_parent;
	vector<SceneObject*> m_children;
};