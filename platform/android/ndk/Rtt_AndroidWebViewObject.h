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


#ifndef _Rtt_AndroidWebViewObject_H__
#define _Rtt_AndroidWebViewObject_H__

#include "Rtt_AndroidDisplayObject.h"
#include "Rtt_MPlatform.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidWebViewObject : public AndroidDisplayObject
{
	public:
		typedef AndroidWebViewObject Self;
		typedef AndroidDisplayObject Super;

	public:
		AndroidWebViewObject( const Rect& bounds, AndroidDisplayObjectRegistry *displayObjectRegistry, NativeToJavaBridge *ntjb );
		virtual ~AndroidWebViewObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();
		bool InitializeAsPopup(bool autoCancelEnabled);

	public:
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

	public:
		void Request(const char *url, const MPlatform::Directory baseDirectory);
		void Request(const char *url, const char *baseUrl);
		bool IsPopup() const { return fIsPopup; }
		bool IsAutoCancelEnabled() const { return fAutoCancelEnabled; }
		bool CanGoBack() const { return fCanGoBack; }
		bool CanGoForward() const { return fCanGoForward; }
		void SetCanGoBack(bool canGoBack) { fCanGoBack = canGoBack; }
		void SetCanGoForward(bool canGoForward) { fCanGoForward = canGoForward; }

	public:
		// MLuaBindable
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );
	
	private:
		bool fIsPopup;
		bool fAutoCancelEnabled;
		bool fCanGoBack;
		bool fCanGoForward;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidWebViewObject_H__
