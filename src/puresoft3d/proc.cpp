#include <memory.h>
#include <stdexcept>
#include "proc.h"
#include "mcemaths.h"

using namespace std;

PuresoftProcessor::PuresoftProcessor(createProcessorInstance vpc, createProcessorInstance ipc, createProcessorInstance fpc, size_t userDataBytes)
{
	m_userDataBytes = userDataBytes;
	m_vertProc = (PuresoftVertexProcessor*)vpc();

	for(size_t i = 0; i < MAX_FRAG_PIPES; i++)
	{
		m_interpProc[i] = (PuresoftInterpolationProcessor*)ipc();
		m_fragProc[i] = (PuresoftFragmentProcessor*)fpc();
	}
}

PuresoftProcessor::~PuresoftProcessor()
{
	m_vertProc->release();

	for(size_t i = 0; i < MAX_FRAG_PIPES; i++)
	{
		m_interpProc[i]->release();
		m_fragProc[i]->release();
	}
}

size_t PuresoftProcessor::getUserDataBytes() const
{
	return m_userDataBytes;
}

PuresoftVertexProcessor* PuresoftProcessor::getVertProc(void)
{
	return m_vertProc;
}

PuresoftInterpolationProcessor* PuresoftProcessor::getInterpProc(int idx)
{
	if(idx < 0 || idx >= MAX_FRAG_PIPES)
	{
		throw out_of_range("PuresoftProcessor::getInterpProc");
	}

	return m_interpProc[idx];
}

PuresoftFragmentProcessor* PuresoftProcessor::getFragProc(int idx)
{
	if(idx < 0 || idx >= MAX_FRAG_PIPES)
	{
		throw out_of_range("PuresoftProcessor::getInterpProc");
	}

	return m_fragProc[idx];
}