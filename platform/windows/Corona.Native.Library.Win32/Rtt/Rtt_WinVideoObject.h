//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_WinDisplayObject.h"


#pragma region Forward Declarations
namespace Interop
{
	class RuntimeEnvironment;
}
extern "C"
{
	struct lua_State;
}

#pragma endregion


namespace Rtt
{

class WinVideoObject : public WinDisplayObject
{
	public:
		typedef WinVideoObject Self;
		typedef WinDisplayObject Super;

	public:
		WinVideoObject(Interop::RuntimeEnvironment& environment, const Rect& bounds);
		virtual ~WinVideoObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();
		virtual Interop::UI::Control* GetControl() const;

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

}	// namespace Rtt
