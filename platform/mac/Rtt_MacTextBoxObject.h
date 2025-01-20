//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacTextBoxObject_H__
#define _Rtt_MacTextBoxObject_H__

#include "Rtt_MacDisplayObject.h"

#import <AppKit/NSTextView.h>
#import <AppKit/NSScrollView.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class MacTextBoxObject : public MacDisplayObject
{
	public:
		typedef MacTextBoxObject Self;
		typedef MacDisplayObject Super;

	public:
		MacTextBoxObject( const Rect& bounds );
		virtual ~MacTextBoxObject();

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
	
	protected:
		virtual void DidRescaleSimulator( float previousScale, float currentScale );
		static int setTextColor( lua_State *L );
		static int setSelection( lua_State *L );
		static int getSelection( lua_State *L );

	private:
//		Rect fSelfBounds;
//		NSTextView *fTextBox;
		bool fIsFontSizeScaled;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------


@interface Rtt_NSTextView : NSTextView <NSTextViewDelegate>
{
	Rtt::MacTextBoxObject *owner;
}

@property(nonatomic, assign) Rtt::MacTextBoxObject *owner;
@property(nonatomic, retain) NSString *placeholder;
@property(nonatomic, assign) bool showingPlaceholder;
@property(nonatomic, retain) NSColor *userColor;

@end


@interface Rtt_NSScrollView : NSScrollView
{
	Rtt::MacTextBoxObject *owner;
}

@property(nonatomic, assign) Rtt::MacTextBoxObject *owner;

@end



#endif // _Rtt_MacTextBoxObject_H__
