//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
