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

#ifndef __Rtt_MessageDigest_H__
#define __Rtt_MessageDigest_H__

// ----------------------------------------------------------------------------

#include <stdio.h>

namespace Rtt
{

// ----------------------------------------------------------------------------

class MD4
{
	public:
		enum
		{
			kNumBytesDigest = 16
		};

	public:
		static void Hash( FILE* f, U8* bytes );
		static void Hash( const void * data, U32 length, U8* bytes );
		static void Hash( const char* str, U8* dstBytes );
};

// ----------------------------------------------------------------------------

class MessageDigest
{
	public:
		typedef enum _HashFunction
		{
			kMD4 = 0,

			kNumFunctions
		}
		HashFunction;

		static void Test();

	public:
		MessageDigest();
		~MessageDigest();

	public:
		void Initialize( Rtt_Allocator* allocator, const char* filePath, HashFunction h );

	protected:
		void Alloc( Rtt_Allocator* allocator, size_t numBytes );

	public:
		const U8* operator*() const { return fBytes; }
		size_t NumBytes() const { return fNumBytes; }

	public:
		void Print( const char* src );

	private:
		U8* fBytes;
		size_t fNumBytes;
};

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_MessageDigest_H__
