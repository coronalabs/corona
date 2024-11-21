//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
		static int InjectJS( lua_State *L );
		static int RegisterCallback( lua_State *L );
		static int On( lua_State *L );
		static int Send( lua_State *L );
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

		virtual int GetNativeProperty( lua_State *L, const char key[] ) const;
		virtual bool SetNativeProperty( lua_State *L, const char key[], int valueIndex );
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
