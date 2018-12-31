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

#ifndef _Rtt_IPhoneDisplayObject_H__
#define _Rtt_IPhoneDisplayObject_H__

#include "Rtt_PlatformDisplayObject.h"

#import <UIKit/UIStringDrawing.h>
#import <UIKit/UITextInputTraits.h>

// ----------------------------------------------------------------------------

@class CoronaView;
@class NSString;
@class UIColor;
@class UIView;

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneDisplayObject : public PlatformDisplayObject
{
	public:
		typedef IPhoneDisplayObject Self;
		typedef PlatformDisplayObject Super;

	public:
		static IPhoneDisplayObject* ToObject( lua_State *L, int index, const LuaProxyVTable& table );

	public:
		IPhoneDisplayObject( const Rect& bounds );
		virtual ~IPhoneDisplayObject();

		virtual void Preinitialize( const Display& display );
		void InitializeView( UIView *view );

		void SetFocus();

	public:
		// DisplayObject
		virtual void DidMoveOffscreen();
		virtual void WillMoveOnscreen();

	public:
		// MDrawable
		virtual void Prepare( const Display& display );
		virtual void Translate( Real dx, Real dy );
		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBounds( Rect& rect ) const;
		virtual void SetSelfBounds( Real width, Real height );

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );

	protected:
		virtual id GetNativeTarget() const;
	
		virtual int GetNativeProperty( lua_State *L, const char key[] ) const;
		virtual bool SetNativeProperty( lua_State *L, const char key[], int valueIndex );

	protected:
		UIView* GetView() const { return fView; }
		CoronaView* GetCoronaView() const;

	private:
		Rect fSelfBounds;
		Vertex2 fViewCenter;
		UIView *fView;
		CoronaView *fCoronaView;
		BOOL fHidden; // used by DidMoveOffscreen/WillMoveOnscreen
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneDisplayObject_H__
