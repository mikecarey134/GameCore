#ifndef _IrrConsole_includes_h_
#define _IrrConsole_includes_h_

#include <iostream>
#include <map>
#include <irrlicht.h>
#include <wchar.h>

#ifdef WIN32
        #ifdef _MSC_VER
            #pragma comment(lib, "irrlicht")
        #endif
	
        #ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#endif

#ifndef _WIN32
	inline double _wtof(const wchar_t* str) { return wcstod(str, 0); }
#endif

using irr::u8;
using irr::u16;
using irr::u32;
using irr::c8;
using irr::s16;
using irr::s32;
using irr::f32;

using irr::core::array;
using irr::core::vector3df;
using irr::core::vector2df;
using irr::core::rect;
using irr::core::dimension2d;


using std::cout;
using std::cerr;
using std::endl;
using std::ostream;

typedef irr::core::stringw WideString;
typedef irr::core::stringc String;


#endif
