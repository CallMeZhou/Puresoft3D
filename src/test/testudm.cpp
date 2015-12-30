#include "testudm.h"

void* _stdcall MyTestProcessorUserDataManager::createInstance(void)
{
	return new MyTestProcessorUserDataManager;
}

MyTestProcessorUserDataManager::MyTestProcessorUserDataManager(void)
{
}

MyTestProcessorUserDataManager::~MyTestProcessorUserDataManager(void)
{
}

void MyTestProcessorUserDataManager::release(void)
{
	delete this;
}

void MyTestProcessorUserDataManager::alloc(size_t count)
{
	m_data.resize(count);
}

void* MyTestProcessorUserDataManager::get(size_t idx)
{
	return &m_data[idx];
}
