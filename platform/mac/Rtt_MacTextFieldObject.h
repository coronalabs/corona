//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacTextFieldObject_H__
#define _Rtt_MacTextFieldObject_H__

#include "Rtt_MacDisplayObject.h"

#import <AppKit/NSTextField.h>
#import <AppKit/NSSecureTextField.h>
#import <AppKit/NSTextView.h>

// ----------------------------------------------------------------------------

@class Rtt_NSTextField;
@class Rtt_NSSecureTextField;

namespace Rtt
{

// ----------------------------------------------------------------------------

class MacTextFieldObject : public MacDisplayObject
{
	public:
		typedef MacTextFieldObject Self;
		typedef MacDisplayObject Super;

	public:
		MacTextFieldObject( const Rect& bounds );
		virtual ~MacTextFieldObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();

	public:
		// DisplayObject
		virtual const LuaProxyVTable& ProxyVTable() const;

	public:
		// MLuaBindable
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );
		bool rejectDisallowedCharacters(const char *str);

	protected:
		virtual void DidRescaleSimulator( float previousScale, float currentScale );
		static int setTextColor( lua_State *L );
		static int setSelection( lua_State *L );
		static int getSelection( lua_State *L );

	
	private:
		bool fIsFontSizeScaled;
		bool fNoEmoji;
		bool fNumbersOnly;
		bool fDecimalNumbersOnly;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

@interface Rtt_NSTextField : NSTextField
{
	Rtt::MacTextFieldObject *owner;
}

@property(nonatomic, assign) Rtt::MacTextFieldObject *owner;

@end


@interface Rtt_NSSecureTextField : NSSecureTextField
{
	Rtt::MacTextFieldObject *owner;
}

@property(nonatomic, assign) Rtt::MacTextFieldObject *owner;

@end
/*
@interface Rtt_NSTextFieldCell : NSTextFieldCell
{
	Rtt::MacTextFieldObject *owner;
	NSTextView* fieldEditor;
}
@property(nonatomic, assign) Rtt::MacTextFieldObject *owner;
@end
*/
/* Cocoa won't let me use this class
@interface Rtt_NSSecureTextFieldCell : NSSecureTextFieldCell
{
	Rtt::MacTextFieldObject *owner;
	NSTextView* fieldEditor;
}
@property(nonatomic, assign) Rtt::MacTextFieldObject *owner;

@end
*/
/*
@interface Rtt_FieldEditorTextView : NSTextView
{
	Rtt::MacTextFieldObject *owner;
}
@property(nonatomic, assign) Rtt::MacTextFieldObject *owner;
@end
*/

// ----------------------------------------------------------------------------

#endif // _Rtt_MacTextFieldObject_H__
