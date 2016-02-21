#include "pipeline.h"

int PuresoftPipeline::addProcessor(PuresoftProcessor* proc)
{
	size_t vacant = 0;
	for(; vacant < m_processors.size(); vacant++)
	{
		if(!m_processors[vacant])
			break;
	}

	if(m_processors.size() == vacant)
	{
		m_processors.push_back(NULL);
	}

	m_processors[vacant] = proc;

	return (int)vacant;
}

void PuresoftPipeline::destroyProcessor(int idx)
{
	if(idx < 0 || idx >= (int)m_processors.size())
	{
		throw std::out_of_range("PuresoftPipeline::destroyProcessor");
	}

	PuresoftProcessor* proc = m_processors[idx];
	if(proc)
	{
		if((uintptr_t)m_vp == (uintptr_t)proc)
			m_vp = NULL;
		if((uintptr_t)m_ip == (uintptr_t)proc)
			m_ip = NULL;
		if((uintptr_t)m_fp == (uintptr_t)proc)
			m_fp = NULL;
		delete proc;
		m_processors[idx] = NULL;
	}
}

int PuresoftPipeline::createProgramme(int vid, int iid, int fid)
{
	if(vid < 0 || vid >= (int)m_processors.size())
	{
		throw std::out_of_range("PuresoftPipeline::createProgramme, vid");
	}

	if(iid < 0 || iid >= (int)m_processors.size())
	{
		throw std::out_of_range("PuresoftPipeline::createProgramme, iid");
	}

	if(fid < 0 || fid >= (int)m_processors.size())
	{
		throw std::out_of_range("PuresoftPipeline::createProgramme, fid");
	}

	size_t vacant = 0;
	for(; vacant < m_programmes.size(); vacant++)
	{
		if(-1 == m_programmes[vacant].vp)
			break;
	}

	PURESOFTPROGRAMME prog = {-1, -1, -1};
	if(m_programmes.size() == vacant)
	{
		m_programmes.push_back(prog);
	}

	setUserDataBytes(dynamic_cast<PuresoftInterpolationProcessor*>(m_processors[iid])->userDataBytes());

	class prep
	{
		void* m_qbuff;
		size_t m_bbytes;
	public:
		prep(void* qbuff, size_t bbytes) : m_qbuff(qbuff), m_bbytes(bbytes) {}
		void operator()(size_t idx, FRAGTHREADTASK* item)
		{
			item->userDataStart = (void*)((uintptr_t)m_qbuff + idx * 2 * m_bbytes);
			item->userDataStep = (void*)((uintptr_t)item->userDataStart + m_bbytes);
		}
	};

	// reinitialize user data buffers in task queue
	uintptr_t user = (uintptr_t)m_userDataBuffers.verts;
	for(size_t i = 0; i < 3; i++)
	{
		m_vertOutput[i].user = (void*)user;
		user += m_userDataBuffers.unitBytes;
	}

	for(int i = 0; i < m_numberOfThreads; i++)
	{
		prep _prep(m_userDataBuffers.taskQueues[i], m_userDataBuffers.unitBytes);
		m_fragTaskQueues[i].prepare(_prep);
	}

	prog.vp = vid;
	prog.ip = iid;
	prog.fp = fid;
	m_programmes[vacant] = prog;

	return (int)vacant;
}

void PuresoftPipeline::destroyProgramme(int idx)
{
	if(idx < 0 || idx >= (int)m_programmes.size())
	{
		throw std::out_of_range("PuresoftPipeline::destroyProgramme");
	}

	PURESOFTPROGRAMME prog = {-1, -1, -1};
	m_programmes[idx] = prog;
}

void PuresoftPipeline::useProgramme(int idx)
{
	if(idx < 0 || idx >= (int)m_programmes.size())
	{
		throw std::out_of_range("PuresoftPipeline::useProgramme");
	}

	const PURESOFTPROGRAMME& prog = m_programmes[idx];
	m_vp = dynamic_cast<PuresoftVertexProcessor*>(m_processors[prog.vp]);
	m_ip = dynamic_cast<PuresoftInterpolationProcessor*>(m_processors[prog.ip]);
	m_fp = dynamic_cast<PuresoftFragmentProcessor*>(m_processors[prog.fp]);
}