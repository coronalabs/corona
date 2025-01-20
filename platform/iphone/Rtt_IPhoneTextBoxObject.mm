//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneTextBoxObject.h"

#import <UIKit/UIKit.h>

#import "CoronaViewPrivate.h"

#include "Rtt_AppleTextAlignment.h"

#include "Rtt_IPhoneFont.h"
#include "Rtt_IPhonePlatformBase.h"
#include "Rtt_IPhoneText.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_StageObject.h"

#ifdef Rtt_IPHONE_ENV
	#define Rtt_TEXT_EDITABLE 1
#endif

// ----------------------------------------------------------------------------

@implementation Rtt_UITextView

@synthesize owner;

- (id)initWithFrame:(CGRect)rect
{
	self = [super initWithFrame:rect];
	if ( self )
	{
		owner = nil;
	}
	return self;
}

// Remove the text and reset the color when the text field has focus
- (BOOL) becomeFirstResponder
{
	[self resetTextView];
	return [super becomeFirstResponder];
}

- (void) setUserColor:(UIColor *)textColor
{
	[_userColor release];
	_userColor = textColor;
	[_userColor retain];
	[self updatePlaceholder];
}

- (void) setPlaceholder:(NSString *)placeholder
{
	[_placeholder release];
	_placeholder = placeholder;
	[_placeholder retain];
	[self updatePlaceholder];
}

- (void) setText:(NSString *)text
{
	[self resetTextView];
	[super setText:text];
}

- (NSString*) getText
{
	if ( _showingPlaceholder )
	{
		return @"";
	}
	return super.text;
}

- (void) resetTextView
{
	if ( _showingPlaceholder )
	{
		[super setText:@""];
		_showingPlaceholder = false;
	}
	
	UIColor *color = _userColor;
	if ( !color )
	{
		if (@available(iOS 13.0, *)) {
			color = [UIColor labelColor];
		} else {
			color = [UIColor blackColor];
		}
	}
	if( ! [[self textColor] isEqual:color] )
	{
		[self setTextColor:color];
	}
}

- (void) updatePlaceholder
{
	if ( ( [[self text] isEqualToString:@""] || _showingPlaceholder ) && _placeholder )
	{
		[super setText:_placeholder];
		if (@available(iOS 13.0, *)) {
			[self setTextColor:[UIColor placeholderTextColor]];
		} else {
			[self setTextColor:[UIColor lightGrayColor]];
		}
		_showingPlaceholder = true;
	}
	else
	{
		[self resetTextView];
	}
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneTextBoxObject::IPhoneTextBoxObject( const Rect& bounds )
:	Super( bounds ),
	fIsFontSizeScaled( false )
{
}

IPhoneTextBoxObject::~IPhoneTextBoxObject()
{
	Rtt_UITextView *t = (Rtt_UITextView*)GetView();
	t.owner = NULL;
}

bool
IPhoneTextBoxObject::Initialize()
{
	Rtt_ASSERT( ! GetView() );

	CoronaView *coronaView = GetCoronaView();

	Rect screenBounds;
	GetScreenBounds( screenBounds );
	CGRect r = CGRectMake( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );
	Rtt_UITextView *t = [[Rtt_UITextView alloc] initWithFrame:r];
#ifdef Rtt_TEXT_EDITABLE
	t.editable = NO;
#endif
	//t.borderStyle = UITextBorderStyleRoundedRect;
	t.delegate = coronaView;
	t.owner = this;
	
	// This works-around an iOS bug where you can't change a UITextView's font settings until you assign it text first.
	[t setText: @" "];
	[t setText: @""];

	UIView *parent = coronaView;
	[parent addSubview:t];

	Super::InitializeView( t );
	[t release];

	return t;
}

const LuaProxyVTable&
IPhoneTextBoxObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetTextBoxObjectProxyVTable();
}

int
IPhoneTextBoxObject::setTextColor( lua_State *L )
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	if ( & o->ProxyVTable() == & PlatformDisplayObject::GetTextBoxObjectProxyVTable() )
	{
		bool isBytecolorRange = o->IsByteColorRange();
		UIColor *c = IPhoneText::GetTextColor( L, 2, isBytecolorRange );
		UIView *v = ((IPhoneTextBoxObject*)o)->GetView();
		Rtt_UITextView *t = (Rtt_UITextView*)v;
		[t setUserColor:c];
	}

	return 0;
}
    
int
IPhoneTextBoxObject::setReturnKey( lua_State *L )
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	if ( & o->ProxyVTable() == & PlatformDisplayObject::GetTextBoxObjectProxyVTable() )
	{
        const char* imeType = lua_tostring(L, -1);
        
		UIView *v = ((IPhoneTextBoxObject*)o)->GetView();
		Rtt_UITextView *t = (Rtt_UITextView*)v;
        t.returnKeyType = IPhoneText::GetUIReturnKeyTypeFromIMEType(imeType);
	}
    
	return 0;
}

int
IPhoneTextBoxObject::setSelection( lua_State *L )
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	if ( & o->ProxyVTable() == & PlatformDisplayObject::GetTextBoxObjectProxyVTable() )
	{
		double startPosition = lua_tonumber(L, 2);
		double endPosition = lua_tonumber(L, 3);
		
		UIView *v = ((IPhoneTextBoxObject*)o)->GetView();
		Rtt_UITextView *t = (Rtt_UITextView*)v;
		int maxLength = (int)[t.text length];
		if (startPosition > maxLength)
		{
			startPosition = maxLength;
			endPosition = maxLength;
		}
		
		if (endPosition > maxLength)
		{
			endPosition = maxLength;
		}
		
		if (startPosition < 0)
		{
			startPosition = 0;
		}

		if (endPosition < 0)
		{
			endPosition = 0;
		}

		if (startPosition > endPosition)
		{
			startPosition = endPosition;
		}
		
		NSRange range;
		range.location = startPosition;
		range.length = endPosition - startPosition;
		
		t.selectedRange = range;
	}
	
	return 0;
}

int
IPhoneTextBoxObject::getSelection(lua_State *L)
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&o->ProxyVTable() == &PlatformDisplayObject::GetTextBoxObjectProxyVTable())
	{
		UIView *v = ((IPhoneTextBoxObject*)o)->GetView();
		Rtt_UITextView *t = (Rtt_UITextView*)v;

		NSRange range = t.selectedRange;

		lua_pushnumber(L, range.location);
		lua_pushnumber(L, range.location + range.length);
		
		return 2;
	}

	return 0;
}
	
int
IPhoneTextBoxObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	Rtt_UITextView *t = (Rtt_UITextView*)GetView();

	if ( strcmp( "text", key ) == 0 )
	{
		lua_pushstring( L, [[t getText] UTF8String] );
	}
	else if ( strcmp( "size", key ) == 0 )
	{
		float fontSize = [t.font pointSize];
		if ( fIsFontSizeScaled )
		{
			Display &display = GetStage()->GetDisplay();
			fontSize *= display.GetSxUpright();
			fontSize *= ((IPhonePlatformBase&)display.GetRuntime().Platform()).GetView().contentScaleFactor;
		}
		lua_pushnumber( L, fontSize );
	}
	else if ( strcmp( "font", key ) == 0 )
	{
		float fontSize = [t.font pointSize];
		if ( fIsFontSizeScaled )
		{
			Display &display = GetStage()->GetDisplay();
			fontSize *= display.GetSxUpright();
			fontSize *= ((IPhonePlatformBase&)display.GetRuntime().Platform()).GetView().contentScaleFactor;
		}
		IPhoneFont *font = Rtt_NEW( LuaContext::GetAllocator( L ), IPhoneFont( t.font ) );
		font->SetSize( fontSize );
		result = LuaLibNative::PushFont( L, font );
	}
	else if ( strcmp( "isFontSizeScaled", key ) == 0 )
	{
		lua_pushboolean( L, fIsFontSizeScaled ? 1 : 0 );
	}
	else if ( strcmp( "isEditable", key ) == 0 )
	{
#ifdef Rtt_TEXT_EDITABLE
		lua_pushboolean( L, t.editable );
#else
		lua_pushboolean( L, false );
#endif
	}
	else if ( strcmp( "setTextColor", key ) == 0 )
	{
		lua_pushcfunction( L, setTextColor );
	}
    else if ( strcmp( "setReturnKey", key ) == 0 )
	{
		lua_pushcfunction( L, setReturnKey );
	}
	else if ( strcmp( "setSelection", key) == 0 )
	{
		lua_pushcfunction( L, setSelection );
	}
	else if ( strcmp( "getSelection", key) == 0 )
	{
		lua_pushcfunction( L, getSelection );
	}
	else if ( strcmp( "align", key ) == 0 )
	{
		lua_pushstring( L, AppleAlignment::StringForAlignment( t.textAlignment ) );
	}
	else if ( strcmp( "placeholder", key ) == 0 )
	{
		lua_pushstring( L, [t.placeholder UTF8String] );
	}
	else
	{
		result = Super::ValueForKey( L, key );
	}

	return result;
}

bool
IPhoneTextBoxObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	Rtt_UITextView *t = (Rtt_UITextView*)GetView();

	if ( strcmp( "text", key ) == 0 )
	{
		const char *s = lua_tostring( L, valueIndex );
		if ( Rtt_VERIFY( s ) )
		{
			NSString *newValue = [[NSString alloc] initWithUTF8String:s];
			[t setText:newValue];
			// Pressing undo will try to operate on the old text if this isn't called which can cause a crash
			[t.undoManager removeAllActions];
			[newValue release];
		}
	}
	else if ( strcmp( "size", key ) == 0 )
	{
		float fontSize = 0;
		Display &display = GetStage()->GetDisplay();
		if ( lua_isnumber( L, valueIndex ) )
		{
			fontSize = (float)lua_tonumber( L, valueIndex );
			if ( fIsFontSizeScaled )
			{
				fontSize /= display.GetSxUpright();
				fontSize /= ((IPhonePlatformBase&)display.GetRuntime().Platform()).GetView().contentScaleFactor;
			}
		}
		if ( fontSize < 1.0f )
		{
			fontSize = display.GetRuntime().Platform().GetStandardFontSize();
			fontSize /= ((IPhonePlatformBase&)display.GetRuntime().Platform()).GetView().contentScaleFactor;
		}
		if ( ( fontSize > 0 ) && ( fontSize != [t.font pointSize] ) )
		{
			t.font = [t.font fontWithSize:fontSize];
		}
	}
	else if ( strcmp( "font", key ) == 0 )
	{
		PlatformFont *font = LuaLibNative::ToFont( L, valueIndex );
		if ( font )
		{
			Display &display = GetStage()->GetDisplay();
			float fontSize = font->Size();
			if ( fontSize >= Rtt_REAL_1 )
			{
				if ( fIsFontSizeScaled )
				{
					fontSize /= display.GetSxUpright();
					fontSize /= ((IPhonePlatformBase&)display.GetRuntime().Platform()).GetView().contentScaleFactor;
				}
			}
			else
			{
				fontSize = display.GetRuntime().Platform().GetStandardFontSize();
				fontSize /= ((IPhonePlatformBase&)display.GetRuntime().Platform()).GetView().contentScaleFactor;
			}
			t.font = [(UIFont*)(font->NativeObject()) fontWithSize:fontSize];
		}
	}
	else if ( strcmp( "isFontSizeScaled", key ) == 0 )
	{
		if ( lua_isboolean( L, valueIndex ) )
		{
			fIsFontSizeScaled = lua_toboolean( L, valueIndex ) ? true : false;
		}
	}
	else if ( strcmp( "isEditable", key ) == 0 )
	{
#ifdef Rtt_TEXT_EDITABLE
		bool oldValue = t.editable;
		bool newValue = lua_toboolean( L, valueIndex );
		if ( oldValue != newValue )
		{
			t.editable = newValue;
		}
#endif
	}
	else if ( strcmp( "align", key ) == 0 )
	{
		t.textAlignment = AppleAlignment::AlignmentForString( lua_tostring( L, valueIndex ) );
	}
	else if ( strcmp( "placeholder", key ) == 0 )
	{
		const char *text = lua_tostring(L, valueIndex);
		if (text)
		{
			[t setPlaceholder:[NSString stringWithUTF8String:text]];
		}
		else
		{
			[t setPlaceholder:nil];
		}
	}
	else
	{
		result = Super::SetValueForKey( L, key, valueIndex );
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
