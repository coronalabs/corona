//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
