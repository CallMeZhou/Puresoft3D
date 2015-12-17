#pragma once

#include "targetver.h"

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
// Windows Header Files:
#include <windowsx.h>
#include <guiddef.h>
#include <WTypes.h>
#include <MMSystem.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <crtdbg.h>

// C++ RunTime Header Files
#include "fixvec.hpp"
#include <set>
#include <map>
#include <string>
#include <sstream>
#include <exception>
#include <memory>
using namespace std;

// some CString constructors will be explicit
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <atlbase.h>
#include <atlstr.h>

// app-spec headers
#include <GdiPlus.h>
#include "mcemaths.hpp"
using namespace mcemaths;
using namespace Gdiplus;