//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacMapViewObject_H__
#define _Rtt_MacMapViewObject_H__

#include "Rtt_MacDisplayObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class MacMapViewObject : public MacDisplayObject
{
	public:
		typedef MacMapViewObject Self;
		typedef MacDisplayObject Super;

	public:
		MacMapViewObject( const Rect& bounds );
		virtual ~MacMapViewObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();

		// DisplayObject
		virtual const LuaProxyVTable& ProxyVTable() const;

	public:
		static int getUserLocation( lua_State *L );
		static int setRegion( lua_State *L );
		static int setCenter( lua_State *L );
		static int requestLocation( lua_State *L ); // New. Asynchronous.
		static int getAddressLocation( lua_State *L ); // Old. Synchronous.
		static int addMarker( lua_State *L );
		static int removeMarker( lua_State *L );
		static int removeAllMarkers( lua_State *L );
		static int nearestAddress( lua_State *L );

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacMapViewObject_H__
