#include <stdexcept>
#include <string>
#include <map>
#include "defproc.h"
#include "testproc.h"
#include "procreator.h"
using namespace std;

class ProcCreator
{
	typedef PuresoftProcessor* (*funcProcCreator)(void);
	typedef map<string, funcProcCreator> ProcCreators;
	ProcCreators m_creators;
	static PuresoftProcessor* createVertDEF01(void)			{return new VertexProcesserDEF01;}
	static PuresoftProcessor* createInterpDEF01(void)		{return new InterpolationProcessorDEF01;}
	static PuresoftProcessor* createFragDEF01(void)			{return new FragmentProcessorDEF01;}
	static PuresoftProcessor* createVertDEF02(void)			{return new VertexProcesserDEF02;}
	static PuresoftProcessor* createInterpDEF02(void)		{return new InterpolationProcessorDEF02;}
	static PuresoftProcessor* createFragDEF02(void)			{return new FragmentProcessorDEF02;}
	static PuresoftProcessor* createVertDEF03(void)			{return new VertexProcesserDEF03;}
	static PuresoftProcessor* createInterpDEF03(void)		{return new InterpolationProcessorDEF03;}
	static PuresoftProcessor* createFragDEF03(void)			{return new FragmentProcessorDEF03;}
	static PuresoftProcessor* createVertDEF04(void)			{return new VertexProcesserDEF04;}
	static PuresoftProcessor* createInterpDEF04(void)		{return new InterpolationProcessorDEF04;}
	static PuresoftProcessor* createFragDEF04(void)			{return new FragmentProcessorDEF04;}
	static PuresoftProcessor* createVertPlanet(void)		{return new VP_Planet;}
	static PuresoftProcessor* createInterpPlanet(void)		{return new IP_Planet;}
	static PuresoftProcessor* createFragEarth(void)			{return new FP_Earth;}
	static PuresoftProcessor* createFragMoon(void)			{return new FP_Satellite;}
	static PuresoftProcessor* createVertCloud(void)			{return new VP_Cloud;}
	static PuresoftProcessor* createInterpCloud(void)		{return new IP_Cloud;}
	static PuresoftProcessor* createFragCloud(void)			{return new FP_Cloud;}
	static PuresoftProcessor* createVertCloudShadow(void)	{return new VP_CloudShadow;}
	static PuresoftProcessor* createInterpCloudShadow(void)	{return new IP_CloudShadow;}
	static PuresoftProcessor* createFragCloudShadow(void)	{return new FP_CloudShadow;}
public:
	ProcCreator(void)
	{
		m_creators[typeid(VertexProcesserDEF01).raw_name()]			= createVertDEF01;
		m_creators[typeid(InterpolationProcessorDEF01).raw_name()]	= createInterpDEF01;
		m_creators[typeid(FragmentProcessorDEF01).raw_name()]		= createFragDEF01;
		m_creators[typeid(VertexProcesserDEF02).raw_name()]			= createVertDEF02;
		m_creators[typeid(InterpolationProcessorDEF02).raw_name()]	= createInterpDEF02;
		m_creators[typeid(FragmentProcessorDEF02).raw_name()]		= createFragDEF02;
		m_creators[typeid(VertexProcesserDEF03).raw_name()]			= createVertDEF03;
		m_creators[typeid(InterpolationProcessorDEF03).raw_name()]	= createInterpDEF03;
		m_creators[typeid(FragmentProcessorDEF03).raw_name()]		= createFragDEF03;
		m_creators[typeid(VertexProcesserDEF04).raw_name()]			= createVertDEF04;
		m_creators[typeid(InterpolationProcessorDEF04).raw_name()]	= createInterpDEF04;
		m_creators[typeid(FragmentProcessorDEF04).raw_name()]		= createFragDEF04;
		m_creators[typeid(VP_Planet).raw_name()]					= createVertPlanet;
		m_creators[typeid(IP_Planet).raw_name()]					= createInterpPlanet;
		m_creators[typeid(FP_Earth).raw_name()]						= createFragEarth;
		m_creators[typeid(FP_Satellite).raw_name()]					= createFragMoon;
		m_creators[typeid(VP_Cloud).raw_name()]						= createVertCloud;
		m_creators[typeid(IP_Cloud).raw_name()]						= createInterpCloud;
		m_creators[typeid(FP_Cloud).raw_name()]						= createFragCloud;
		m_creators[typeid(VP_CloudShadow).raw_name()]				= createVertCloudShadow;
		m_creators[typeid(IP_CloudShadow).raw_name()]				= createInterpCloudShadow;
		m_creators[typeid(FP_CloudShadow).raw_name()]				= createFragCloudShadow;
	}
	PuresoftProcessor* createProcessor(const char* classname)
	{
		ProcCreators::iterator it = m_creators.find(classname);
		if(m_creators.end() == it)
		{
			throw invalid_argument(classname);
		}

		return it->second();
	}
} g_creator;

PuresoftProcessor* createProcessor(const char* classname)
{
	return g_creator.createProcessor(classname);
}