#include <stdexcept>
#include "defproc.h"
#include "testobjs.h"
#include "testproc.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////
Earth::Earth(PuresoftPipeline& pipeline, SceneObject* parent)
	: SceneObject(pipeline, parent)
{
	m_vao = findOrCreateVao("sphere.objx");

	m_diffuse = findOrCreateTexture("earth.jpg");
	m_bump = findOrCreateTexture("earth.dot3.png");
	m_specular = findOrCreateTexture("earth.spac.png");
	m_night = findOrCreateTexture("earth.night.png");
	m_cloud = findOrCreateTexture("earth.cloud.png");

	m_programme = findOrCreateProgramme(PROCNAME(VP_Planet), PROCNAME(IP_Planet), PROCNAME(FP_Earth));

	m_meshRotateRad = 0;
}

Earth::~Earth()
{

}

void Earth::update(float timeSpanSec, const mat4& parent)
{
	m_meshRotateRad += 0.07f * timeSpanSec;

	if(m_meshRotateRad > 6.2831854f)
	{
		m_meshRotateRad = 0;
	}

	m_rotation.rotation(vec4(0, 1.0f, 0, 0), m_meshRotateRad);
	mcemaths_transform_m4m4(m_model, m_translation, m_rotation);

	__super::update(timeSpanSec, parent);
}

void Earth::draw(PuresoftPipeline& pipeline)
{
	pipeline.setUniform(4, m_model, sizeof(m_model.elem));
	pipeline.setUniform(5, m_rotation, sizeof(m_rotation.elem));
	pipeline.setUniform(9, &m_diffuse, sizeof(int));
	pipeline.setUniform(10, &m_bump, sizeof(int));
	pipeline.setUniform(11, &m_specular, sizeof(int));
	pipeline.setUniform(12, &m_night, sizeof(int));
	pipeline.setUniform(13, &m_cloud, sizeof(int));
	pipeline.setUniform(15, &m_shadowMaps[0], sizeof(int));
	pipeline.setUniform(16, m_shadowPVs[0], sizeof(m_shadowPVs[0].elem));
	__super::draw(pipeline);
}

//////////////////////////////////////////////////////////////////////////
Cloud::Cloud(PuresoftPipeline& pipeline, SceneObject* parent)
	: SceneObject(pipeline, parent)
{
	m_vao = findOrCreateVao("sphere.objx");
	m_diffuse = findOrCreateTexture("earth.cloud.png");
	m_programme = findOrCreateProgramme(PROCNAME(VP_Cloud), PROCNAME(IP_Cloud), PROCNAME(FP_Cloud));
	m_myShadowProgramme = findOrCreateProgramme(PROCNAME(VP_CloudShadow), PROCNAME(IP_CloudShadow), PROCNAME(FP_CloudShadow));
	m_meshRotateRad = 0;

	m_scale.scaling(1.1f, 1.1f, 1.1f);
}

Cloud::~Cloud()
{

}

void Cloud::update(float timeSpanSec, const mat4& parent)
{
	m_meshRotateRad += 0.3f * timeSpanSec;

	if(m_meshRotateRad > 6.2831854f)
	{
		m_meshRotateRad = 0;
	}

	m_rotation.rotation(vec4(0, 1.0f, 0, 0), m_meshRotateRad);
	mcemaths_transform_m4m4(m_model, m_rotation, m_scale);
	mcemaths_transform_m4m4_r_ip(m_translation, m_model);

	__super::update(timeSpanSec, parent);
}

void Cloud::draw(PuresoftPipeline& pipeline)
{
	pipeline.setUniform(4, m_model, sizeof(m_model.elem));
	pipeline.setUniform(5, m_rotation, sizeof(m_rotation.elem));
	pipeline.setUniform(9, &m_diffuse, sizeof(int));
	pipeline.setUniform(15, &m_shadowMaps[0], sizeof(int));
	pipeline.setUniform(16, m_shadowPVs[0], sizeof(m_shadowPVs[0].elem));

	if(m_useShadowProgramme)
	{
		pipeline.useProgramme(m_myShadowProgramme);
	}
	else
	{
		pipeline.useProgramme(m_programme);
	}

	pipeline.drawVAO(m_vao);
}

//////////////////////////////////////////////////////////////////////////
Moon::Moon(PuresoftPipeline& pipeline, SceneObject* parent)
	: SceneObject(pipeline, parent)
{
	m_vao = findOrCreateVao("sphere.objx");

	m_diffuse = findOrCreateTexture("moon.jpg");
	m_bump = findOrCreateTexture("moon.dot3.png");

	m_programme = findOrCreateProgramme(PROCNAME(VP_Planet), PROCNAME(IP_Planet), PROCNAME(FP_Satellite));
	m_translation.translation(0.7f, 0, 0);
	m_scale.scaling(0.2f, 0.2f, 0.2f);

	m_meshRotateRad = 3.1415927f;
}

Moon::~Moon()
{

}

void Moon::update(float timeSpanSec, const mat4& parent)
{
	m_meshRotateRad += 0.1f * timeSpanSec;

	if(m_meshRotateRad > 6.2831854f)
	{
		m_meshRotateRad = 0;
	}

	m_rotation.rotation(vec4(0, 1.0f, 0, 0), m_meshRotateRad);
	mcemaths_transform_m4m4(m_model, m_translation, m_scale);
	mcemaths_transform_m4m4_r_ip(m_rotation, m_model);

	__super::update(timeSpanSec, parent);
}

void Moon::draw(PuresoftPipeline& pipeline)
{
	pipeline.setUniform(4, m_model, sizeof(m_model.elem));
	pipeline.setUniform(5, m_rotation, sizeof(m_rotation.elem));
	pipeline.setUniform(9, &m_diffuse, sizeof(int));
	pipeline.setUniform(10, &m_bump, sizeof(int));
	__super::draw(pipeline);
}

//////////////////////////////////////////////////////////////////////////
class FullViewBuilder : public MeshBuilder
{
public:
	void build(PuresoftPipeline& pipeline, int vao)
	{
		__declspec(align(16)) float fullsquare[] = 
		{
			-1.0f,  1.0f,  0,  1.0f, 
			-1.0f, -1.0f,  0,  1.0f, 
			 1.0f, -1.0f,  0,  1.0f, 
			 1.0f, -1.0f,  0,  1.0f, 
			 1.0f,  1.0f,  0,  1.0f, 
			-1.0f,  1.0f,  0,  1.0f, 
		};

		pipeline.attachVBO(vao, 0, new PuresoftVBO(16, 6));
		pipeline.getVBO(vao, 0)->updateContent(fullsquare);
	}
};

Skybox::Skybox(PuresoftPipeline& pipeline, SceneObject* parent)
	: SceneObject(pipeline, parent)
{
	m_vao = findOrCreateVao(&FullViewBuilder());
	m_programme = findOrCreateProgramme(PROCNAME(VertexProcesserDEF04), PROCNAME(InterpolationProcessorDEF04), PROCNAME(FragmentProcessorDEF04));

	const char* skyboxFiles[] = 
	{
		  "purplenebula_rt.png" // xpos
		, "purplenebula_lf.png" // xneg
		, "purplenebula_up.png" // ypos
		, "purplenebula_dn.png" // yneg
		, "purplenebula_ft.png" // zpos
		, "purplenebula_bk.png" // zneg
	};

	m_texture = findOrCreateSkybox(skyboxFiles);
}

Skybox::~Skybox()
{

}

void Skybox::update(float timeSpanSec, const mat4& parent)
{
	__super::update(timeSpanSec, parent);
}

void Skybox::draw(PuresoftPipeline& pipeline)
{
	if(m_useShadowProgramme)
		return;

	pipeline.setUniform(2, &m_texture, sizeof(int));
	pipeline.disable(BEHAVIOR_UPDATE_DEPTH | BEHAVIOR_TEST_DEPTH);
	__super::draw(pipeline);
	pipeline.enable(BEHAVIOR_UPDATE_DEPTH | BEHAVIOR_TEST_DEPTH);
}