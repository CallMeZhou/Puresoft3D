#pragma once
#include "scenobj.h"

class Earth : public SceneObject
{
public:
	Earth(PuresoftPipeline& pipeline, SceneObject* parent);
	~Earth();
	void update(float timeSpanSec, const mat4& parent);
	void draw(PuresoftPipeline& pipeline);

private:
	int m_diffuse;
	int m_bump;
	int m_specular;
	int m_night;
	int m_cloud;
	float m_meshRotateRad;
	float m_texTransX;
	mat4 m_texTrans;
};

class Moon : public SceneObject
{
public:
	Moon(PuresoftPipeline& pipeline, SceneObject* parent);
	~Moon();
	void update(float timeSpanSec, const mat4& parent);
	void draw(PuresoftPipeline& pipeline);

private:
	int m_diffuse;
	int m_bump;
	float m_meshRotateRad;
};

class Skybox : public SceneObject
{
public:
	Skybox(PuresoftPipeline& pipeline, SceneObject* parent);
	~Skybox();
	void update(float timeSpanSec, const mat4& parent);
	void draw(PuresoftPipeline& pipeline);

private:
	int m_texture;
};