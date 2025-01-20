//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_Types_H__
#define __Rtt_Types_H__

#include <stddef.h>

// ----------------------------------------------------------------------------

// Word has it that Visual Studio finally gets stdint.h in 2010
#if defined(_MSC_VER) && _MSC_VER < 1600
	typedef signed char S8;
	typedef unsigned char U8;
	typedef signed short S16;
	typedef unsigned short U16;
	typedef signed long S32;
	typedef unsigned long U32;
	typedef signed long long S64;
	typedef unsigned long long U64;
#else
	#include <stdint.h>
	typedef int8_t S8;
	typedef uint8_t U8;
	typedef int16_t S16;
	typedef uint16_t U16;
	typedef int32_t S32;
	typedef uint32_t U32;
	typedef int64_t S64;
	typedef uint64_t U64;
#endif

typedef struct Rtt_Allocator Rtt_Allocator;

// ----------------------------------------------------------------------------

#ifdef __cplusplus

namespace Rtt
{

// ----------------------------------------------------------------------------

typedef S32 RefCount;

struct Range
{
	S32 start;
	S32 count;
};

// ----------------------------------------------------------------------------

} // Rtt

#endif // __cplusplus

// ----------------------------------------------------------------------------

#endif // __Rtt_Types_H__
