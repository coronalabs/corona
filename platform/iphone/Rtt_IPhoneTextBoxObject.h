//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneTextBoxObject_H__
#define _Rtt_IPhoneTextBoxObject_H__

#include "Rtt_IPhoneDisplayObject.h"

#import <UIKit/UITextView.h>

// ----------------------------------------------------------------------------

@class Rtt_UITextView;

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneTextBoxObject : public IPhoneDisplayObject
{
	public:
		typedef IPhoneTextBoxObject Self;
		typedef IPhoneDisplayObject Super;

	public:
		IPhoneTextBoxObject( const Rect& bounds );
		virtual ~IPhoneTextBoxObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();

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
		bool fIsFontSizeScaled;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

@interface Rtt_UITextView : UITextView
{
	Rtt::IPhoneTextBoxObject *owner;
}

- (void) updatePlaceholder;

@property(nonatomic, assign) Rtt::IPhoneTextBoxObject *owner;
@property(nonatomic, retain) NSString *placeholder;
@property(nonatomic, assign) bool showingPlaceholder;
@property(nonatomic, retain) UIColor *userColor;

@end

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneTextBoxObject_H__
