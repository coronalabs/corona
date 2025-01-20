//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidMapViewObject_H__
#define _Rtt_AndroidMapViewObject_H__

#include "Rtt_AndroidDisplayObject.h"
#include "Rtt_MPlatform.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidMapViewObject : public AndroidDisplayObject
{
	public:
		typedef AndroidMapViewObject Self;
		typedef AndroidDisplayObject Super;

	public:
		AndroidMapViewObject( const Rect& bounds, AndroidDisplayObjectRegistry *displayObjectRegistry, NativeToJavaBridge *ntjb );
		virtual ~AndroidMapViewObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();

	public:
		// DisplayObject
		virtual const LuaProxyVTable& ProxyVTable() const;

	protected:
		static int GetUserLocation(lua_State *L);
		static int SetRegion(lua_State *L);
		static int SetCenter(lua_State *L);
		static int RequestLocation( lua_State *L ); // New. Asynchronous.
		static int GetAddressLocation( lua_State *L ); // Old. Synchronous.
		static int AddMarker(lua_State *L);
		static int RemoveMarker(lua_State *L);
		static int RemoveAllMarkers(lua_State *L);
		static int NearestAddress(lua_State *L);

	public:
		// MLuaBindable
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );

	private:
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidMapViewObject_H__
