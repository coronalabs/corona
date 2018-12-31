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
