//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidTextFieldObject_H__
#define _Rtt_AndroidTextFieldObject_H__

#include "AndroidFontSettings.h"
#include "Rtt_AndroidDisplayObject.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidTextFieldObject : public AndroidDisplayObject
{
	public:
		typedef AndroidTextFieldObject Self;
		typedef AndroidDisplayObject Super;

	public:
		AndroidTextFieldObject( const Rect& bounds, AndroidDisplayObjectRegistry *displayObjectRegistry, NativeToJavaBridge *ntjb, bool isSingleLine );
		virtual ~AndroidTextFieldObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();

	public:
		// DisplayObject
		virtual const LuaProxyVTable& ProxyVTable() const;

	protected:
		static int setTextColor( lua_State *L );
		static int setReturnKey( lua_State *L );
		static int setSelection( lua_State *L );
		static int getSelection( lua_State *L );

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );

	private:
		/// Set TRUE if this is a single line text field. Set FALSE for a multiline text box.
		/// This value is not expected to change after initialization.
		bool fIsSingleLine;

		/// Keeps a copy of the current font settings. Needed to make this object's Lua "font" property work.
		AndroidFontSettings fFontSettings;

		/// Set TRUE if the font size applied to this object is relative to Corona's content scaling.
		/// Set FALSE if the font size is using Android's DP units, which is the legacy behavior.
		bool fIsFontSizeScaled;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidTextFieldObject_H__
