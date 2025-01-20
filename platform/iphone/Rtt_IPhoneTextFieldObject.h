//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneTextFieldObject_H__
#define _Rtt_IPhoneTextFieldObject_H__

#include "Rtt_IPhoneDisplayObject.h"

#import <UIKit/UITextField.h>

// ----------------------------------------------------------------------------

@class Rtt_UITextField;

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneTextFieldObject : public IPhoneDisplayObject
{
	public:
		typedef IPhoneTextFieldObject Self;
		typedef IPhoneDisplayObject Super;

	public:
		IPhoneTextFieldObject( const Rect& bounds );
		virtual ~IPhoneTextFieldObject();

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
		bool rejectEmoji(const char *str);

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );
	
	private:
		bool fIsFontSizeScaled;
		bool fNoEmoji;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

@interface Rtt_UITextField : UITextField
{
	Rtt::IPhoneTextFieldObject *owner;
}

@property(nonatomic, assign) Rtt::IPhoneTextFieldObject *owner;

@end

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneTextFieldObject_H__
