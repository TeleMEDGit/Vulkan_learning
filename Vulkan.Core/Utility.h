//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once


#include "Common/Common.h"
#include "System/Heap.h"
#include "idLib/Thread.h"
//#include "Model.h"


#include <cassert>

//============================================  this typedefs should go to common system file with other related items
//
//namespace Math
//{
//	class DrawVertex;
//}
namespace Utility
{
	enum
	{
		IMAGE_DEFAULT = 0, // only used for desired_channels

		IMAGE_GREY = 1,
		IMAGE_GREY_ALPHA = 2,
		IMAGE_RGB = 3,
		IMAGE_RGB_ALPHA = 4
	};
	bool AssertFailed(const char *file, int line, const char *expression);
	inline void Print(const char* msg) { printf(msg); }
	inline void Print(const wchar_t* msg) { wprintf(msg); }

	inline void Printf(const char* format, ...)
	{
		char buffer[256];
		va_list ap;
		va_start(ap, format);
		
		vsprintf_s(buffer, 256, format, ap);
		Print(buffer);
	}

	inline void Printf(const wchar_t* format, ...)
	{
		wchar_t buffer[256];
		va_list ap;
		va_start(ap, format);
		vswprintf(buffer, 256, format, ap);
		Print(buffer);
	}

#ifndef RELEASE
	inline void PrintSubMessage(const char* format, ...)
	{
		Print("--> ");
		char buffer[256];
		va_list ap;
		va_start(ap, format);
		vsprintf_s(buffer, 256, format, ap);
		Print(buffer);
		Print("\n");
	}
	inline void PrintSubMessage(const wchar_t* format, ...)
	{
		Print("--> ");
		wchar_t buffer[256];
		va_list ap;
		va_start(ap, format);
		vswprintf(buffer, 256, format, ap);
		Print(buffer);
		Print("\n");
	}
	inline void PrintSubMessage(void)
	{
	}
#endif

	void SIMDMemCopy(void* __restrict Dest, const void* __restrict Source, size_t NumQuadwords);
	void SIMDMemFill(void* __restrict Dest, __m128 FillVector, size_t NumQuadwords);


	std::wstring MakeWStr(const std::string& str);

	bool LoadTextureDataFromFile(char const                 * filename,
		int                          num_requested_components,
		std::vector<unsigned char> & image_data,
		int                        * image_width = nullptr,
		int                        * image_height = nullptr,
		int                        * image_num_components = nullptr,
		int                        * image_data_size = nullptr);


	//bool GetBinaryFileContents(std::string const          & filename,
	//	std::vector<unsigned char> & contents);





	//struct Mesh {
	//	std::vector<Math::DrawVertex>  Data; //TDO change this float to Math::DrawVertex in order to map Vertex Description

	//	struct Part {
	//		uint32_t  VertexOffset;
	//		uint32_t  VertexCount;
	//	};

	//	std::vector<Part>   Parts;
	//};

	







} // namespace Utility




#define ID_WIN_X86_SSE2_INTRIN
  //=====================================================================

#define _mm_madd_ps( a, b, c )				_mm_add_ps( _mm_mul_ps( (a), (b) ), (c) )

#define idreleaseassert( x )	(void)( ( !!( x ) ) || ( Utility::AssertFailed( __FILE__, __LINE__, #x ) ) );

#define release_assert( x )	idreleaseassert( x )

#define assert_2_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) &  1 ) == 0 )
#define assert_4_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) &  3 ) == 0 )
#define assert_8_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) &  7 ) == 0 )
#define assert_16_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) & 15 ) == 0 )
#define assert_32_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) & 31 ) == 0 )
#define assert_64_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) & 63 ) == 0 )
#define assert_128_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) & 127 ) == 0 )
#define assert_aligned_to_type_size( ptr )	assert( ( ((UINT_PTR)(ptr)) & ( sizeof( (ptr)[0] ) - 1 ) ) == 0 )

#if !defined( __TYPEINFOGEN__ )

template<bool> struct compile_time_assert_failed;
template<> struct compile_time_assert_failed<true> {};
template<int x> struct compile_time_assert_test {};
#define compile_time_assert_join2( a, b )	a##b
#define compile_time_assert_join( a, b )	compile_time_assert_join2(a,b)
#define compile_time_assert( x )			typedef compile_time_assert_test<sizeof(compile_time_assert_failed<(bool)(x)>)> compile_time_assert_join(compile_time_assert_typedef_, __LINE__)

#define assert_sizeof( type, size )						compile_time_assert( sizeof( type ) == size )
#define assert_sizeof_8_byte_multiple( type )			compile_time_assert( ( sizeof( type ) &  7 ) == 0 )
#define assert_sizeof_16_byte_multiple( type )			compile_time_assert( ( sizeof( type ) & 15 ) == 0 )
#define assert_offsetof( type, field, offset )			compile_time_assert( offsetof( type, field ) == offset )
#define assert_offsetof_8_byte_multiple( type, field )	compile_time_assert( ( offsetof( type, field ) & 7 ) == 0 )
#define assert_offsetof_16_byte_multiple( type, field )	compile_time_assert( ( offsetof( type, field ) & 15 ) == 0 )

#else

#define compile_time_assert( x )
#define assert_sizeof( type, size )
#define assert_sizeof_8_byte_multiple( type )
#define assert_sizeof_16_byte_multiple( type )
#define assert_offsetof( type, field, offset )
#define assert_offsetof_8_byte_multiple( type, field )
#define assert_offsetof_16_byte_multiple( type, field )

#endif

#ifdef ERROR
#undef ERROR
#endif
#ifdef ASSERT
#undef ASSERT
#endif
#ifdef HALT
#undef HALT
#endif

#define HALT( ... ) ERROR( __VA_ARGS__ ) __debugbreak();

#define MAX_TYPE( x )			( ( ( ( 1 << ( ( sizeof( x ) - 1 ) * 8 - 1 ) ) - 1 ) << 8 ) | 255 )
#define MIN_TYPE( x )			( - MAX_TYPE( x ) - 1 )
#define MAX_UNSIGNED_TYPE( x )	( ( ( ( 1U << ( ( sizeof( x ) - 1 ) * 8 ) ) - 1 ) << 8 ) | 255U )
#define MIN_UNSIGNED_TYPE( x )	0

template<class T> T	Max(T x, T y) { return (x > y) ? x : y; }
template<class T> T	Min(T x, T y) { return (x < y) ? x : y; }

#define ALIGN( x, a ) ( ( ( x ) + ((a)-1) ) & ~((a)-1) )

static const SYSTEM_TIME_T	FILE_NOT_FOUND_TIMESTAMP = (SYSTEM_TIME_T)-1;

#ifndef BIT
#define BIT( num )				( 1ULL << ( num ) )
#endif

#define _alloca16( x )					((void *)ALIGN( (UINT_PTR)_alloca( ALIGN( x, 16 ) + 16 ), 16 ) )
#define _alloca128( x )					((void *)ALIGN( (UINT_PTR)_alloca( ALIGN( x, 128 ) + 128 ), 128 ) )

#ifdef RELEASE

#define ASSERT( isTrue, ... ) (void)(isTrue)
#define WARN_ONCE_IF( isTrue, ... ) (void)(isTrue)
#define WARN_ONCE_IF_NOT( isTrue, ... ) (void)(isTrue)
#define ERROR( msg, ... )
#define DEBUGPRINT( msg, ... ) do {} while(0)
#define ASSERT_SUCCEEDED( hr, ... ) (void)(hr)

#else	// !RELEASE

#define STRINGIFY(x) #x
#define STRINGIFY_BUILTIN(x) STRINGIFY(x)
#define ASSERT( isFalse, ... ) \
		if (!(bool)(isFalse)) { \
			Utility::Print("\nAssertion failed in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
			Utility::PrintSubMessage("\'" #isFalse "\' is false"); \
			Utility::PrintSubMessage(__VA_ARGS__); \
			Utility::Print("\n"); \
			__debugbreak(); \
		}

#define ASSERT_SUCCEEDED( hr, ... ) \
		if (FAILED(hr)) { \
			Utility::Print("\nHRESULT failed in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
			Utility::PrintSubMessage("hr = 0x%08X", hr); \
			Utility::PrintSubMessage(__VA_ARGS__); \
			Utility::Print("\n"); \
			__debugbreak(); \
		}


#define WARN_ONCE_IF( isTrue, ... ) \
	{ \
		static bool s_TriggeredWarning = false; \
		if ((bool)(isTrue) && !s_TriggeredWarning) { \
			s_TriggeredWarning = true; \
			Utility::Print("\nWarning issued in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
			Utility::PrintSubMessage("\'" #isTrue "\' is true"); \
			Utility::PrintSubMessage(__VA_ARGS__); \
			Utility::Print("\n"); \
		} \
	}

#define WARN_ONCE_IF_NOT( isTrue, ... ) WARN_ONCE_IF(!(isTrue), __VA_ARGS__)

#define ERROR( ... ) \
		Utility::Print("\nError reported in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
		Utility::PrintSubMessage(__VA_ARGS__); \
		Utility::Print("\n");

#define DEBUGPRINT( msg, ... ) \
	Utility::Printf( msg "\n", ##__VA_ARGS__ );
// Helper define for tracking error locations in code
#define VK_CHECK(X) if ((X)) { Utility::Print("VK_CHECK Failure"); assert((X));}

#define VK_VALIDATE( x, msg ) { \
	if ( !( x ) ) Utility::PrintSubMessage( "VK: %s - %s", msg, x ); }\

#endif

#define BreakIfFailed( hr ) if (FAILED(hr)) __debugbreak()

  // if writing to write-combined memroy, always write indexes as pairs for 32 bit writes
ID_INLINE void WriteIndexPair(polyIndex * dest, const polyIndex a, const polyIndex b) {
	*(unsigned *)dest = (unsigned)a | ((unsigned)b << 16);
}







