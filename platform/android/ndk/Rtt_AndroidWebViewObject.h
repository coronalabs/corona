//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
		static int InjectJS( lua_State *L );
		static int RegisterCallback( lua_State *L );
		static int On( lua_State *L );
		static int Send( lua_State *L );

	public:
		void Request(const char *url, const MPlatform::Directory baseDirectory);
		void Request(const char *url, const char *baseUrl);
		void InjectJSCode( const char *jsCode );
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
