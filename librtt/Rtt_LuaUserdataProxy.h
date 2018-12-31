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

#ifndef __Rtt_LuaUserdataProxy__
#define __Rtt_LuaUserdataProxy__

#include "Core/Rtt_Finalizer.h"
#include "Core/Rtt_String.h"
#include "Core/Rtt_StringHash.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class LuaUserdataProxy;

// ----------------------------------------------------------------------------

class MLuaUserdataAdapter
{
	public:
		virtual int ValueForKey(
			const LuaUserdataProxy& sender,
			lua_State *L,
			const char *key ) const = 0;

		virtual bool SetValueForKey(
			LuaUserdataProxy& sender,
			lua_State *L,
			const char *key,
			int valueIndex ) const = 0;

		// This is invoked when sender's object is about to be deleted
		virtual void WillFinalize( LuaUserdataProxy& sender ) const = 0;

        // Return the hash table containing the adapter's property keys
        virtual StringHash *GetHash( lua_State *L ) const = 0;
};

// LuaUserdataProxy
// * Goal:
//     + Enable a native C object to be accessed via property semantics
//     + Decouple ownership, so Lua does not own the native C object
//     + Instead, Lua sees a proxy object
// * Implementation:
//     + Lua's userdata wraps a LuaUserdataProxy instance.
//     + The proxy contains an adapter to bridge the Lua property access and the C object.
//     + The proxy holds onto a _weak_ pointer to the native C object.
//     + The object holds onto a strong reference to the LuaUserdataProxy instance
// * Owernship rules:
//     + Lua owns a userdata object as well as the LuaUserdataProxy instance
//     + LuaUserdataProxy stores a Lua ref so that the userdata is not GC'd prematurely,
//       and so that the userdata can be pushed onto the Lua stack from the C side.
//     + An C-side "observer" object (e.g.ShapeObject) effectively owns this Lua ref.
//       The observer controls the calls to LuaUserdataProxy::AllocRef() and ReleaseRef().
class LuaUserdataProxy
{
	public:
		typedef LuaUserdataProxy Self;

	public:
		static LuaUserdataProxy *ToProxy( lua_State* L, int index );
        static int PropertiesToJSON( lua_State *L, const LuaUserdataProxy *proxy, const char **keys, const int numKeys, String& result );


	public:
		static int Index( lua_State *L );
        static int Index( lua_State *L, int base );
		static int NewIndex( lua_State *L );
		static int GC( lua_State *L );

	public:
		// Initialize the Lua metatable used by the proxy
		static void Initialize( lua_State *L );

	public:
		// Factory method
		static LuaUserdataProxy *New( lua_State *L, void *object );

	protected:
		LuaUserdataProxy( void *object );
		virtual ~LuaUserdataProxy();

	public:
		// The object should call this method to notify the receiver
		// (LuaUserdataProxy) that the object is no longer valid.
		// Typically, this is called from the object's destructor.
		void DetachUserdata();

		// These enable the C/C++ object to reference the Lua userdata.
		// Normally, the object owns the LuaUserdataProxy instance and
		// calls AllocRef/ReleaseRef to retain/release the instance.
		void AllocRef( lua_State *L );
		void ReleaseRef( lua_State *L );

	public:
		// Pushes the Lua userdata on the Lua stack.
		void Push( lua_State *L ) const;

	public:
		void *GetUserdata() { return fObject; }
		const void *GetUserdata() const { return fObject; }

		void SetAdapter( const MLuaUserdataAdapter *newValue ) { fAdapter = newValue; }
		const MLuaUserdataAdapter *GetAdapter() const { return fAdapter; }

	private:
		// LuaUserdataProxy does not own fObject.
		void *fObject;
		const MLuaUserdataAdapter *fAdapter;
		int fRef;
};

// ----------------------------------------------------------------------------

template <>
class ObjectFinalizer< LuaUserdataProxy * >
{
	public:
		static void Collect( LuaUserdataProxy *object )
		{
		}
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_LuaUserdataProxy__
