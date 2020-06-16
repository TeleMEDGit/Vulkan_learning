#pragma once

#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable:4328) // nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable:4324) // structure was padded due to __declspec(align())

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif


//++++++++++++++++++++++++++++++++++++++
//DON'T change this code 
//It is here to use as template for Hook to the Core lib
//And it is template for 3D application on Windows
//
//By Ahmed Omer
//April 2th 2018



//++++++++++++++++++++++++++++++++++++++

// WINDOWS
#if defined( _WIN32 )
// this is always defined on windows platform
//
#define VK_USE_PLATFORM_WIN32_KHR 1
#define PLATFORM_SURFACE_EXTENTION_NAME VK_KHR_WIN32_SURFACE_EXTENTION_NAME

//#define ID_USE_AMD_ALLOCATOR
#include <windows.h>

#include <wrl.h>

#define CACHE_LINE_SIZE						128


#endif

//#define _NO_CRT_STDIO_INLINE

static const unsigned int NUM_FRAME_DATA = 2;
static const int MAX_DESC_SETS = 16384;
static const int MAX_DESC_UNIFORM_BUFFERS = 8192;
static const int MAX_DESC_IMAGE_SAMPLERS = 12384;
static const int MAX_DESC_SET_WRITES = 32;
static const int MAX_DESC_SET_UNIFORMS = 48;
static const int MAX_IMAGE_PARMS = 16;
static const int MAX_UBO_PARMS = 2;
static const int NUM_TIMESTAMP_QUERIES = 16;

//namespace  Math {
//const float	PI = 3.14159265358979323846f;
//const float	M_DEG2RAD = PI / 180.0f;
//const float	M_RAD2DEG = 180.0f / PI;
//}



#define STENCIL_SHADOW_TEST_VALUE		128

#include <iostream>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <ostream>
#include <iostream>
#include <sstream>
#include <fstream>
#include <functional>
#include <vector>
#include <array>
#include <list>
#include <cstring>
#include <thread>
#include <heapapi.h>
#include <locale>
#include <assert.h>




#include <xmmintrin.h>
#include <emmintrin.h>

#include "CommonEnums.h"

//#include "Platform.h"
//#include "Utility.h"

#include <vulkan/vulkan.h>
#include "../SystemTime.h"


// We have expression parsing and evaluation code in multiple places:
// materials, sound shaders, and guis. We should unify them.
const int MAX_EXPRESSION_OPS = 4096;
const int MAX_EXPRESSION_REGISTERS = 4096;

//#define ID_WIN_X86_ASM
#define ID_WIN_X86_SSE2_INTRIN

#define VULKAN 1;

#define SYSTEM_TIME_T int64



struct FileSystem
{
	bool OpenFile(std::string name, std::vector<unsigned char> & contents)
	{
		contents.clear();
		std::ifstream f;
		f.open(name, std::ios::binary);

		if (f.fail())
		{
			std::cout << "Could not open " << name << std::endl;
			return false;
		}

		std::streampos begin;
		std::streampos end;
		begin = f.tellg();
		f.seekg(0, std::ios::end);
		end = f.tellg();

		if ((end - begin) == 0) {
			std::cout << "The " << name << " file is empty." << std::endl;
			return false;
		}
		contents.resize(static_cast<size_t>(end - begin));
		f.seekg(0, std::ios::beg);
		f.read(reinterpret_cast<char*>(contents.data()), end - begin);

		f.close();

		return true;

	}

	std::vector<std::string> OpenTextFile(std::string name)
	{
		std::ifstream f;
		std::vector<std::string> result;
		std::string line;
		//f.open(".\\" + name, std::ios::in);  //this should be dlike this
		f.open(name, std::ios::in);


		if (f.is_open())
		{
			while (getline(f, line))
			{
				result.push_back(line);
			}
		}

		f.close();

		return result;

	}
};


template< typename _type_ >
inline void SwapValues(_type_ & a, _type_ & b) {
	_type_ c = a;
	a = b;
	b = c;
}


typedef unsigned char		byte;		// 8 bits
typedef unsigned short		word;		// 16 bits
typedef unsigned int		dword;		// 32 bits
typedef unsigned int		uint;
typedef unsigned long		ulong;

typedef signed char			int8;
typedef unsigned char		uint8;
typedef short int			int16;
typedef unsigned short int	uint16;
typedef int					int32;
typedef unsigned int		uint32;
typedef long long			int64;
typedef unsigned long long	uint64;

typedef unsigned short polyIndex;

typedef uint64 vertexCacheHandle;
#if 1

typedef unsigned short polyIndex;
#define GL_INDEX_TYPE		GL_UNSIGNED_SHORT

#else

typedef unsigned int triIndex_t;
#define GL_INDEX_TYPE		GL_UNSIGNED_INT

#endif
typedef uint64 vertCacheHandle;


extern FileSystem* file_system;


/*
================================================================================================

PC Windows

================================================================================================
*/

#ifdef _WIN32

#define	CPUSTRING						"x86"

#define	BUILD_STRING					"win-" CPUSTRING
#define BUILD_OS_ID						0

#define ALIGN16( x )					__declspec(align(16)) x
#define ALIGNTYPE16						__declspec(align(16))
#define ALIGNTYPE128					__declspec(align(128))
#define FORMAT_PRINTF( x )

#define PATHSEPARATOR_STR				"\\"
#define PATHSEPARATOR_CHAR				'\\'
#define NEWLINE							"\r\n"

#define ID_INLINE						inline
#define ID_FORCE_INLINE					__forceinline

#define ID_INLINE_EXTERN				extern inline
#define ID_FORCE_INLINE_EXTERN			extern __forceinline

// we should never rely on this define in our code. this is here so dodgy external libraries don't get confused
#ifndef WIN32
#define WIN32
#endif

#endif

/*
================================================================================================

Defines and macros usable in all code

================================================================================================
*/

#ifndef BIT
#define BIT( num )				( 1ULL << ( num ) )
#endif

#define ALIGN( x, a ) ( ( ( x ) + ((a)-1) ) & ~((a)-1) )

#define _alloca16( x )					((void *)ALIGN( (UINT_PTR)_alloca( ALIGN( x, 16 ) + 16 ), 16 ) )
#define _alloca128( x )					((void *)ALIGN( (UINT_PTR)_alloca( ALIGN( x, 128 ) + 128 ), 128 ) )

#define likely( x )	( x )
#define unlikely( x )	( x )



// A macro to disallow the copy constructor and operator= functions
// NOTE: The macro contains "private:" so all members defined after it will be private until
// public: or protected: is specified.
#define DISALLOW_COPY_AND_ASSIGN(TypeName)	\
private:									\
  TypeName(const TypeName&);				\
  void operator=(const TypeName&);




#if defined( WIN32 )

#pragma warning( disable: 4458 )	// warning C4458: hiding class members
#pragma warning( disable: 4459 )	// warning C4459: hiding global members
#pragma warning( disable: 4244 )	// warning C4244: conversion from 'double' to 'float', possible loss of data

// disable some /analyze warnings here
#pragma warning( disable: 6255 )	// warning C6255: _alloca indicates failure by raising a stack overflow exception. Consider using _malloca instead. (Note: _malloca requires _freea.)
#pragma warning( disable: 6262 )	// warning C6262: Function uses '36924' bytes of stack: exceeds /analyze:stacksize'32768'. Consider moving some data to heap
#pragma warning( disable: 6326 )	// warning C6326: Potential comparison of a constant with another constant

#pragma warning( disable: 6031 )	//  warning C6031: Return value ignored
// this warning fires whenever you have two calls to new in a function, but we assume new never fails, so it is not relevant for us
#pragma warning( disable: 6211 )	// warning C6211: Leaking memory 'staticModel' due to an exception. Consider using a local catch block to clean up memory

// we want to fix all these at some point...
#pragma warning( disable: 6246 )	// warning C6246: Local declaration of 'es' hides declaration of the same name in outer scope. For additional information, see previous declaration at line '969' of 'w:\tech5\rage\game\ai\fsm\fsm_combat.cpp': Lines: 969
#pragma warning( disable: 6244 )	// warning C6244: Local declaration of 'viewList' hides previous declaration at line '67' of 'w:\tech5\engine\renderer\rendertools.cpp'

// win32 needs this, but 360 doesn't
#pragma warning( disable: 6540 )	// warning C6540: The use of attribute annotations on this function will invalidate all of its existing __declspec annotations [D:\tech5\engine\engine-10.vcxproj]


// We need to inform the compiler that Error() and FatalError() will
// never return, so any conditions that leeds to them being called are
// guaranteed to be false in the following code
#define NO_RETURN __declspec(noreturn)

#endif

// I don't want to disable "warning C6031: Return value ignored" from /analyze
// but there are several cases with sprintf where we pre-initialized the variables
// being scanned into, so we truly don't care if they weren't all scanned.
// Rather than littering #pragma statements around these cases, we can assign the
// return value to this, which means we have considered the issue and decided that
// it doesn't require action.
// The volatile qualifier is to prevent:PVS-Studio warnings like:
// False	2	4214	V519	The 'ignoredReturnValue' object is assigned values twice successively. Perhaps this is a mistake. Check lines: 545, 547.	Rage	collisionmodelmanager_debug.cpp	547	False
extern volatile int ignoredReturnValue;

#define MAX_TYPE( x )			( ( ( ( 1 << ( ( sizeof( x ) - 1 ) * 8 - 1 ) ) - 1 ) << 8 ) | 255 )
#define MIN_TYPE( x )			( - MAX_TYPE( x ) - 1 )
#define MAX_UNSIGNED_TYPE( x )	( ( ( ( 1U << ( ( sizeof( x ) - 1 ) * 8 ) ) - 1 ) << 8 ) | 255U )



#define MIN_UNSIGNED_TYPE( x )	0


// one/zero is flipped on src/dest so a gl state of 0 is SRC_ONE,DST_ZERO
static const uint64 GLS_SRCBLEND_ONE = 0 << 0;
static const uint64 GLS_SRCBLEND_ZERO = 1 << 0;
static const uint64 GLS_SRCBLEND_DST_COLOR = 2 << 0;
static const uint64 GLS_SRCBLEND_ONE_MINUS_DST_COLOR = 3 << 0;
static const uint64 GLS_SRCBLEND_SRC_ALPHA = 4 << 0;
static const uint64 GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA = 5 << 0;
static const uint64 GLS_SRCBLEND_DST_ALPHA = 6 << 0;
static const uint64 GLS_SRCBLEND_ONE_MINUS_DST_ALPHA = 7 << 0;
static const uint64 GLS_SRCBLEND_BITS = 7 << 0;

static const uint64 GLS_DSTBLEND_ZERO = 0 << 3;
static const uint64 GLS_DSTBLEND_ONE = 1 << 3;
static const uint64 GLS_DSTBLEND_SRC_COLOR = 2 << 3;
static const uint64 GLS_DSTBLEND_ONE_MINUS_SRC_COLOR = 3 << 3;
static const uint64 GLS_DSTBLEND_SRC_ALPHA = 4 << 3;
static const uint64 GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA = 5 << 3;
static const uint64 GLS_DSTBLEND_DST_ALPHA = 6 << 3;
static const uint64 GLS_DSTBLEND_ONE_MINUS_DST_ALPHA = 7 << 3;
static const uint64 GLS_DSTBLEND_BITS = 7 << 3;

//------------------------
// these masks are the inverse, meaning when set the glColorMask value will be 0,
// preventing that channel from being written
//------------------------
static const uint64 GLS_DEPTHMASK = 1 << 6;
static const uint64 GLS_REDMASK = 1 << 7;
static const uint64 GLS_GREENMASK = 1 << 8;
static const uint64 GLS_BLUEMASK = 1 << 9;
static const uint64 GLS_ALPHAMASK = 1 << 10;
static const uint64 GLS_COLORMASK = (GLS_REDMASK | GLS_GREENMASK | GLS_BLUEMASK);

static const uint64 GLS_POLYMODE_LINE = 1 << 11;
static const uint64 GLS_POLYGON_OFFSET = 1 << 12;

static const uint64 GLS_DEPTHFUNC_LESS = 0 << 13;
static const uint64 GLS_DEPTHFUNC_ALWAYS = 1 << 13;
static const uint64 GLS_DEPTHFUNC_GREATER = 2 << 13;
static const uint64 GLS_DEPTHFUNC_EQUAL = 3 << 13;
static const uint64 GLS_DEPTHFUNC_BITS = 3 << 13;

static const uint64 GLS_CULL_FRONTSIDED = 0 << 15;
static const uint64 GLS_CULL_BACKSIDED = 1 << 15;
static const uint64 GLS_CULL_TWOSIDED = 2 << 15;
static const uint64 GLS_CULL_BITS = 2 << 15;
static const uint64 GLS_CULL_MASK = GLS_CULL_FRONTSIDED | GLS_CULL_BACKSIDED | GLS_CULL_TWOSIDED;

static const uint64 GLS_BLENDOP_ADD = 0 << 18;
static const uint64 GLS_BLENDOP_SUB = 1 << 18;
static const uint64 GLS_BLENDOP_MIN = 2 << 18;
static const uint64 GLS_BLENDOP_MAX = 3 << 18;
static const uint64 GLS_BLENDOP_BITS = 3 << 18;

// stencil bits
static const uint64 GLS_STENCIL_FUNC_REF_SHIFT = 20;
static const uint64 GLS_STENCIL_FUNC_REF_BITS = 0xFFll << GLS_STENCIL_FUNC_REF_SHIFT;

static const uint64 GLS_STENCIL_FUNC_MASK_SHIFT = 28;
static const uint64 GLS_STENCIL_FUNC_MASK_BITS = 0xFFll << GLS_STENCIL_FUNC_MASK_SHIFT;

#define GLS_STENCIL_MAKE_REF( x ) ( ( (uint64)(x) << GLS_STENCIL_FUNC_REF_SHIFT ) & GLS_STENCIL_FUNC_REF_BITS )
#define GLS_STENCIL_MAKE_MASK( x ) ( ( (uint64)(x) << GLS_STENCIL_FUNC_MASK_SHIFT ) & GLS_STENCIL_FUNC_MASK_BITS )

// Next 12 bits act as front+back unless GLS_SEPARATE_STENCIL is set, in which case it acts as front.
static const uint64 GLS_STENCIL_FUNC_ALWAYS = 0ull << 36;
static const uint64 GLS_STENCIL_FUNC_LESS = 1ull << 36;
static const uint64 GLS_STENCIL_FUNC_LEQUAL = 2ull << 36;
static const uint64 GLS_STENCIL_FUNC_GREATER = 3ull << 36;
static const uint64 GLS_STENCIL_FUNC_GEQUAL = 4ull << 36;
static const uint64 GLS_STENCIL_FUNC_EQUAL = 5ull << 36;
static const uint64 GLS_STENCIL_FUNC_NOTEQUAL = 6ull << 36;
static const uint64 GLS_STENCIL_FUNC_NEVER = 7ull << 36;
static const uint64 GLS_STENCIL_FUNC_BITS = 7ull << 36;

static const uint64 GLS_STENCIL_OP_FAIL_KEEP = 0ull << 39;
static const uint64 GLS_STENCIL_OP_FAIL_ZERO = 1ull << 39;
static const uint64 GLS_STENCIL_OP_FAIL_REPLACE = 2ull << 39;
static const uint64 GLS_STENCIL_OP_FAIL_INCR = 3ull << 39;
static const uint64 GLS_STENCIL_OP_FAIL_DECR = 4ull << 39;
static const uint64 GLS_STENCIL_OP_FAIL_INVERT = 5ull << 39;
static const uint64 GLS_STENCIL_OP_FAIL_INCR_WRAP = 6ull << 39;
static const uint64 GLS_STENCIL_OP_FAIL_DECR_WRAP = 7ull << 39;
static const uint64 GLS_STENCIL_OP_FAIL_BITS = 7ull << 39;

static const uint64 GLS_STENCIL_OP_ZFAIL_KEEP = 0ull << 42;
static const uint64 GLS_STENCIL_OP_ZFAIL_ZERO = 1ull << 42;
static const uint64 GLS_STENCIL_OP_ZFAIL_REPLACE = 2ull << 42;
static const uint64 GLS_STENCIL_OP_ZFAIL_INCR = 3ull << 42;
static const uint64 GLS_STENCIL_OP_ZFAIL_DECR = 4ull << 42;
static const uint64 GLS_STENCIL_OP_ZFAIL_INVERT = 5ull << 42;
static const uint64 GLS_STENCIL_OP_ZFAIL_INCR_WRAP = 6ull << 42;
static const uint64 GLS_STENCIL_OP_ZFAIL_DECR_WRAP = 7ull << 42;
static const uint64 GLS_STENCIL_OP_ZFAIL_BITS = 7ull << 42;

static const uint64 GLS_STENCIL_OP_PASS_KEEP = 0ull << 45;
static const uint64 GLS_STENCIL_OP_PASS_ZERO = 1ull << 45;
static const uint64 GLS_STENCIL_OP_PASS_REPLACE = 2ull << 45;
static const uint64 GLS_STENCIL_OP_PASS_INCR = 3ull << 45;
static const uint64 GLS_STENCIL_OP_PASS_DECR = 4ull << 45;
static const uint64 GLS_STENCIL_OP_PASS_INVERT = 5ull << 45;
static const uint64 GLS_STENCIL_OP_PASS_INCR_WRAP = 6ull << 45;
static const uint64 GLS_STENCIL_OP_PASS_DECR_WRAP = 7ull << 45;
static const uint64 GLS_STENCIL_OP_PASS_BITS = 7ull << 45;

// Next 12 bits act as back and are only active when GLS_SEPARATE_STENCIL is set.
static const uint64 GLS_BACK_STENCIL_FUNC_ALWAYS = 0ull << 48;
static const uint64 GLS_BACK_STENCIL_FUNC_LESS = 1ull << 48;
static const uint64 GLS_BACK_STENCIL_FUNC_LEQUAL = 2ull << 48;
static const uint64 GLS_BACK_STENCIL_FUNC_GREATER = 3ull << 48;
static const uint64 GLS_BACK_STENCIL_FUNC_GEQUAL = 4ull << 48;
static const uint64 GLS_BACK_STENCIL_FUNC_EQUAL = 5ull << 48;
static const uint64 GLS_BACK_STENCIL_FUNC_NOTEQUAL = 6ull << 48;
static const uint64 GLS_BACK_STENCIL_FUNC_NEVER = 7ull << 48;
static const uint64 GLS_BACK_STENCIL_FUNC_BITS = 7ull << 48;

static const uint64 GLS_BACK_STENCIL_OP_FAIL_KEEP = 0ull << 51;
static const uint64 GLS_BACK_STENCIL_OP_FAIL_ZERO = 1ull << 51;
static const uint64 GLS_BACK_STENCIL_OP_FAIL_REPLACE = 2ull << 51;
static const uint64 GLS_BACK_STENCIL_OP_FAIL_INCR = 3ull << 51;
static const uint64 GLS_BACK_STENCIL_OP_FAIL_DECR = 4ull << 51;
static const uint64 GLS_BACK_STENCIL_OP_FAIL_INVERT = 5ull << 51;
static const uint64 GLS_BACK_STENCIL_OP_FAIL_INCR_WRAP = 6ull << 51;
static const uint64 GLS_BACK_STENCIL_OP_FAIL_DECR_WRAP = 7ull << 51;
static const uint64 GLS_BACK_STENCIL_OP_FAIL_BITS = 7ull << 51;

static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_KEEP = 0ull << 54;
static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_ZERO = 1ull << 54;
static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_REPLACE = 2ull << 54;
static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_INCR = 3ull << 54;
static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_DECR = 4ull << 54;
static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_INVERT = 5ull << 54;
static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_INCR_WRAP = 6ull << 54;
static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_DECR_WRAP = 7ull << 54;
static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_BITS = 7ull << 54;

static const uint64 GLS_BACK_STENCIL_OP_PASS_KEEP = 0ull << 57;
static const uint64 GLS_BACK_STENCIL_OP_PASS_ZERO = 1ull << 57;
static const uint64 GLS_BACK_STENCIL_OP_PASS_REPLACE = 2ull << 57;
static const uint64 GLS_BACK_STENCIL_OP_PASS_INCR = 3ull << 57;
static const uint64 GLS_BACK_STENCIL_OP_PASS_DECR = 4ull << 57;
static const uint64 GLS_BACK_STENCIL_OP_PASS_INVERT = 5ull << 57;
static const uint64 GLS_BACK_STENCIL_OP_PASS_INCR_WRAP = 6ull << 57;
static const uint64 GLS_BACK_STENCIL_OP_PASS_DECR_WRAP = 7ull << 57;
static const uint64 GLS_BACK_STENCIL_OP_PASS_BITS = 7ull << 57;

static const uint64 GLS_SEPARATE_STENCIL = GLS_BACK_STENCIL_OP_FAIL_BITS | GLS_BACK_STENCIL_OP_ZFAIL_BITS | GLS_BACK_STENCIL_OP_PASS_BITS;
static const uint64 GLS_STENCIL_OP_BITS = GLS_STENCIL_OP_FAIL_BITS | GLS_STENCIL_OP_ZFAIL_BITS | GLS_STENCIL_OP_PASS_BITS | GLS_SEPARATE_STENCIL;
static const uint64 GLS_STENCIL_FRONT_OPS = GLS_STENCIL_OP_FAIL_BITS | GLS_STENCIL_OP_ZFAIL_BITS | GLS_STENCIL_OP_PASS_BITS;
static const uint64 GLS_STENCIL_BACK_OPS = GLS_SEPARATE_STENCIL;

static const uint64 GLS_DEPTH_TEST_MASK = 1ull << 60;
static const uint64 GLS_CLOCKWISE = 1ull << 61;
static const uint64 GLS_MIRROR_VIEW = 1ull << 62;
static const uint64 GLS_OVERRIDE = 1ull << 63;		// override the render prog state

static const uint64 GLS_KEEP = GLS_DEPTH_TEST_MASK;
static const uint64 GLS_DEFAULT = 0;

#define STENCIL_SHADOW_TEST_VALUE		128
#define STENCIL_SHADOW_MASK_VALUE		255
#define	MAX_IMAGE_NAME	256

// These structures are used for memory mapping bimage files, but
	// not for the normal loading, so be careful making changes.
	// Values are big endien to reduce effort on consoles.
#define BIMAGE_VERSION 10
#define BIMAGE_MAGIC (unsigned int)( ('B'<<0)|('I'<<8)|('M'<<16)|(BIMAGE_VERSION<<24) )


const int MAX_SHADER_STAGES = 256;

const int MAX_TEXGEN_REGISTERS = 4;

const int MAX_ENTITY_SHADER_PARMS = 12;
const int MAX_GLOBAL_SHADER_PARMS = 12;	// ? this looks like it should only be 8



struct FrameTimeTick {
	FrameTimeTick() {
		Reset();
	}

	void Reset() {
		
		start_game_time = 0;
		finish_game_time = 0;
		finish_draw_time = 0;
		start_render_time = 0;
		finish_render_time = 0;

		frontend_time = 0;
		backend_time = 0;


		gpu_time = 0;

		samples = 0;
		backend_total_time = 0;
		gpu_total_time = 0;
		backend_time_avg = 0;
		gpu_time_avg = 0;
	}

	void Update(FrameTimeTick & other) {
		
		start_game_time = other.start_game_time;
		finish_game_time = other.finish_game_time;
		finish_draw_time = other.finish_draw_time;
		start_render_time = other.start_render_time;
		finish_render_time = other.finish_render_time;

		frontend_time = other.frontend_time;
		backend_time = other.backend_time;
		gpu_time = other.gpu_time;
		

		samples++;
		backend_total_time += backend_time;
		gpu_total_time += gpu_time;
		backend_time_avg = backend_total_time / samples;
		gpu_time_avg = gpu_total_time / samples;
	}

	
	uint64	start_game_time;
	uint64	finish_game_time;
	uint64	finish_draw_time;
	uint64	start_render_time;
	uint64	finish_render_time;

	uint64	frontend_time;
	uint64	backend_time;
	
	
	uint64	gpu_time;

	uint64	samples;
	uint64	backend_total_time;
	uint64	gpu_total_time;
	double	backend_time_avg;
	double	gpu_time_avg;
};






