#pragma once
#include "alloc16.hpp"
#include "fixvec.hpp"
#include "proc.h"
#include "testpd.h"

class MyTestProcessorUserDataManager :
	public PuresoftProcessorUserDataManager
{
public:
	static void* _stdcall createInstance(void);

public:
	MyTestProcessorUserDataManager(void);
	~MyTestProcessorUserDataManager(void);

	void release(void);
	void alloc(size_t count);
	void* get(size_t idx);

private:
	typedef std::vector<MYTESTPROCDATA, alloc16<MYTESTPROCDATA> > coll;
	coll m_data;
};

