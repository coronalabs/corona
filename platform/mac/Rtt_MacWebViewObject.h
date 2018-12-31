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

#ifndef _Rtt_MacWebViewObject_H__
#define _Rtt_MacWebViewObject_H__

#include "Rtt_MacDisplayObject.h"

#import <WebKit/WebKit.h>

// ----------------------------------------------------------------------------

@class Rtt_NSWebView;

namespace Rtt
{

// ----------------------------------------------------------------------------

class MacWebViewObject : public MacDisplayObject
{
	public:
		typedef MacWebViewObject Self;
		typedef MacDisplayObject Super;

	public:
		MacWebViewObject( const Rect& bounds );
		virtual ~MacWebViewObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();

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
		// static int SetBackgroundColor( lua_State *L );

	public:
		void Load( NSString *htmlBody, NSURL *baseURL );
		void Request( NSString *urlString, NSURL *baseUrl );
		void Stop();
		void Resize( const Rect& bounds );

	public:
		// MLuaBindable
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );
		
	private:
		static NSURL* GetBaseURLFromLuaState(lua_State *L, int index);
		

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

// These WebKit protocols are not explicitly declared until 10.11 SDK, so
// declare dummy protocols to keep the build working on earlier SDKs.

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 101100
@protocol WebUIDelegate
@end
@protocol WebFrameLoadDelegate
@end
@protocol WebPolicyDelegate
@end
#endif

@interface Rtt_WebView : WebView <WebUIDelegate, WebPolicyDelegate, WebFrameLoadDelegate>
{
	Rtt::MacWebViewObject *owner;
}

@property(nonatomic, assign) Rtt::MacWebViewObject *owner;

@end

// ----------------------------------------------------------------------------

#endif // _Rtt_MacWebViewObject_H__
