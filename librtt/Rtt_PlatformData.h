//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformData_H__
#define _Rtt_PlatformData_H__

#include "Rtt_Resource.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

// Provide a file-like interface to arbitrary platform data/buffer
class PlatformData : public Resource
{
	public:
		typedef enum _SeekOrigin
		{
			kSet = 0,
			kCurrent,
			kEnd
		}
		SeekOrigin;

	public:
		static void Initialize( lua_State* L );
		static const char* Name();

	public:
		virtual ~PlatformData();

	protected:
//		bool Push( lua_State* L );

	public:
		// TODO: Once called, Lua assumes ownership. So it can only be called
		// *once*. We've encountered this b4. Might be worth creating some
		// generic ref counting mechanism for objects owned by Lua or investigate
		// userdata environments...
		bool PushIterator( lua_State* L );

	public:
		// virtual const char* Type() const = 0;

		// WARNING: Read() returns a read-only buffer that is *not* NULL-terminated
		virtual const char* Read( size_t numBytes, size_t& numBytesRead ) const = 0;
		virtual void Seek( SeekOrigin origin, S32 offset ) const = 0;
		virtual size_t Length() const = 0;
};

/*
class PlatformImageData : public PlatformData
{
	public:
		PlatformImageData( PlatformBitmap* bitmap, PlatformData* source );
		virtual ~PlatformImageData();

	private:
		PlatformBitmap* fBitmap;
		PlatformData* fSource;
};
*/
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformData_H__
