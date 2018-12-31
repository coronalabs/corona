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
