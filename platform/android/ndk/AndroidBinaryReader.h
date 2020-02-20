//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _AndroidBinaryReader_H__
#define _AndroidBinaryReader_H__

#include "Core/Rtt_Types.h"
#include "AndroidBinaryReadResult.h"

// Forward declarations.
struct Rtt_Allocator;


class AndroidBinaryReader
{
	public:
		AndroidBinaryReader(Rtt_Allocator *allocatorPointer);
		virtual ~AndroidBinaryReader();

		Rtt_Allocator* GetAllocator() const;
		virtual bool IsOpen() = 0;
		bool IsClosed();
		virtual void Close() = 0;
		AndroidBinaryReadResult StreamTo(U8 *value);
		AndroidBinaryReadResult StreamTo(U8 *bytes, U32 count);

	protected:
		virtual AndroidBinaryReadResult OnStreamTo(U8 *bytes, U32 count) = 0;

	private:
		Rtt_Allocator *fAllocatorPointer;
};

#endif // _AndroidBinaryReader_H__
