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
