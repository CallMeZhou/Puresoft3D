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
	static PuresoftProcessor* createVP_PositionOnly() {return new VP_PositionOnly;}
	static PuresoftProcessor* createIP_Null() {return new IP_Null;}
	static PuresoftProcessor* createFP_SingleColourNoLighting() {return new FP_SingleColourNoLighting;}
	static PuresoftProcessor* createVP_SingleColour() {return new VP_SingleColour;}
	static PuresoftProcessor* createIP_SingleColour() {return new IP_SingleColour;}
	static PuresoftProcessor* createFP_SingleColour() {return new FP_SingleColour;}
	static PuresoftProcessor* createVP_DiffuseOnly() {return new VP_DiffuseOnly;}
	static PuresoftProcessor* createIP_DiffuseOnly() {return new IP_DiffuseOnly;}
	static PuresoftProcessor* createFP_DiffuseOnly() {return new FP_DiffuseOnly;}
public:
	ProcCreator(void)
	{
		m_creators[typeid(VP_PositionOnly).name()] = createVP_PositionOnly;
		m_creators[typeid(IP_Null).name()] = createIP_Null;
		m_creators[typeid(FP_SingleColourNoLighting).name()] = createFP_SingleColourNoLighting;
		m_creators[typeid(VP_SingleColour).name()] = createVP_SingleColour;
		m_creators[typeid(IP_SingleColour).name()] = createIP_SingleColour;
		m_creators[typeid(FP_SingleColour).name()] = createFP_SingleColour;
		m_creators[typeid(VP_DiffuseOnly).name()] = createVP_DiffuseOnly;
		m_creators[typeid(IP_DiffuseOnly).name()] = createIP_DiffuseOnly;
		m_creators[typeid(FP_DiffuseOnly).name()] = createFP_DiffuseOnly;
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

PuresoftProcessor* createProcessorClassPrefix(const char* classnameNoPrefix)
{
	string decorated = "class ";
	decorated += classnameNoPrefix;
	return createProcessor(decorated.c_str());
}