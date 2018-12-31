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

#ifndef _Rtt_IPhoneWebViewObject_H__
#define _Rtt_IPhoneWebViewObject_H__

#include "Rtt_IPhoneDisplayObject.h"

#include "Rtt_MPlatform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneWebViewObject : public IPhoneDisplayObject
{
	public:
		typedef IPhoneWebViewObject Self;
		typedef IPhoneDisplayObject Super;

	public:
		IPhoneWebViewObject( const Rect& bounds );
		virtual ~IPhoneWebViewObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();

		// DisplayObject
		virtual const LuaProxyVTable& ProxyVTable() const;

	protected:
		static int Load( lua_State *L );
		static int Request( lua_State *L );
		static int Stop( lua_State *L );
		static int Back( lua_State *L );
		static int Forward( lua_State *L );
		static int Reload( lua_State *L );
		static int Resize( lua_State *L );
		static int DeleteCookies( lua_State *L );
		// static int SetBackgroundColor( lua_State *L );

	public:
		void Load( NSString *htmlBody, NSURL *baseURL );
		void Request( NSString *urlString, NSURL *baseUrl );
		void Stop();
		void Resize( const Rect& bounds );

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );

	protected:
		virtual id GetNativeTarget() const;

	private:
		static NSURL* GetBaseURLFromLuaState(lua_State *L, int index);

	private:
		NSString *fMethod;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneWebViewObject_H__
