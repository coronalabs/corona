//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidVideoObject_H__
#define _Rtt_AndroidVideoObject_H__

#include "Rtt_AndroidDisplayObject.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidVideoObject : public AndroidDisplayObject
{
	public:
		typedef AndroidVideoObject Self;
		typedef AndroidDisplayObject Super;

	public:
		AndroidVideoObject( const Rect& bounds, AndroidDisplayObjectRegistry *displayObjectRegistry, NativeToJavaBridge *ntjb );
		virtual ~AndroidVideoObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();

		// DisplayObject
		virtual const LuaProxyVTable& ProxyVTable() const;

	protected:
		static int Load( lua_State *L );
		static int Play( lua_State *L );
		static int Pause( lua_State *L );
		static int Seek( lua_State *L );

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidVideoObject_H__
