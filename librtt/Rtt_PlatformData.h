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
