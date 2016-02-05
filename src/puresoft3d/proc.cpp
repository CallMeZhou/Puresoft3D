#include <memory.h>
#include <stdexcept>
#include "proc.h"
#include "mcemaths.h"

using namespace std;

PuresoftProcessor::PuresoftProcessor(createProcessorInstance vpc, createProcessorInstance ipc, createProcessorInstance fpc, size_t userDataBytes)
{
	m_userDataBytes = userDataBytes;
	m_vertProc = (PuresoftVertexProcessor*)vpc();
	m_interpProc = (PuresoftInterpolationProcessor*)ipc();
	m_fragProc = (PuresoftFragmentProcessor*)fpc();
}

PuresoftProcessor::~PuresoftProcessor()
{
	m_vertProc->release();
	m_interpProc->release();
	m_fragProc->release();
}

size_t PuresoftProcessor::getUserDataBytes() const
{
	return m_userDataBytes;
}

PuresoftVertexProcessor* PuresoftProcessor::getVertProc(void)
{
	return m_vertProc;
}

PuresoftInterpolationProcessor* PuresoftProcessor::getInterpProc(void)
{
	return m_interpProc;
}

PuresoftFragmentProcessor* PuresoftProcessor::getFragProc(void)
{
	return m_fragProc;
}