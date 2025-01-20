//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AppleTextAlignment.h"

#include "Rtt_MacTextBoxObject.h"
#import "Rtt_AppleTextDelegateWrapperObjectHelper.h"

#import "AppDelegate.h"

#import "Rtt_MacFont.h"
#import <AppKit/NSApplication.h>

#include "Rtt_Lua.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_StageObject.h"

// ----------------------------------------------------------------------------

static void
DispatchEvent( Rtt_NSTextView* textbox, Rtt::UserInputEvent::Phase phase )
{
	using namespace Rtt;
	
	MacTextBoxObject *o = textbox.owner;
	if ( o )
	{
		UserInputEvent e( phase );
		o->DispatchEventWithTarget( e );
	}
}

static void
DispatchEvent( Rtt_NSTextView* textbox, int startpos, int numdeleted, const char* replacementstring, const char* oldstring, const char* newstring )
{
	using namespace Rtt;

	MacTextBoxObject *o = textbox.owner;
	if ( o )
	{
		UserInputEvent e( startpos, numdeleted, replacementstring, oldstring, newstring );
		o->DispatchEventWithTarget( e );
	}
}


@implementation Rtt_NSScrollView
@synthesize owner;
@end

@implementation Rtt_NSTextView

@synthesize owner;

- (BOOL) becomeFirstResponder
{
	// Remove the placeholder text and reset the color when the text box gets focus
	[self resetTextView];

	BOOL didBecomeFirstResponder = [super becomeFirstResponder];

	// Send the "began" event
	if (didBecomeFirstResponder && [self delegate] != nil)
	{
		DispatchEvent( self, Rtt::UserInputEvent::kBegan );
	}

	return didBecomeFirstResponder;
}

- (void) textDidEndEditing:(NSNotification*)notification
{
	[self updatePlaceholder];

	if( nil != [self delegate] )
	{
		DispatchEvent( self, Rtt::UserInputEvent::kSubmitted );
	}

	// Don't cache the value of [self delegate] because it might get removed by an event
	if( nil != [self delegate] )
	{
		DispatchEvent( self, Rtt::UserInputEvent::kEnded );
	}
}

- (BOOL)textView:(NSTextView*)textView shouldChangeTextInRange:(NSRange)range replacementString:(NSString*)string
{
	// location (first number) is the start position of the string
	// length (second number) is the length or number of the characters replaced/deleted
	//	NSLog(@"%@, %@, %@", NSStringFromSelector(_cmd), NSStringFromRange(range), string );
	
	Rtt_AppleTextDelegateWrapperObjectHelper* eventData = [[Rtt_AppleTextDelegateWrapperObjectHelper alloc] init];
	eventData.textWidget = textView;
	eventData.theRange = range;
	eventData.replacementString = string;
	
	[self performSelector:@selector(dispatchEditingEventForTextView:) withObject:eventData afterDelay:0.0];
	
	return YES;
}

- (void) dispatchEditingEventForTextView:(Rtt_AppleTextDelegateWrapperObjectHelper*)eventData
{
	Rtt_NSTextView* textView = (Rtt_NSTextView*)eventData.textWidget;
	NSRange range = eventData.theRange;
	NSString* string = eventData.replacementString;
	NSString* oldstring = eventData.originalString;
	
	if ( nil != [self delegate] )
	{
		// Don't forget to add 1 for Lua index conventions
		DispatchEvent( textView, (int) range.location+1, (int) range.length, [string UTF8String], [oldstring UTF8String], [textView.string UTF8String] );
	}
	
	// Data must be released or it will leak because it created so it could be passed through this callback.
	[eventData release];
}

- (void) setUserColor:(NSColor *)textColor
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

- (void) setString:(NSString *)string
{
	[self resetTextView];
	[super setString:string];
}

- (NSString*) getString
{
	if ( _showingPlaceholder )
	{
		return @"";
	}
	
	return super.string;
}

- (void) resetTextView
{
	if ( _showingPlaceholder )
	{
		[super setString:@""];
		_showingPlaceholder = false;
	}
	
	NSColor *color = _userColor;
	if ( !color )
	{
		if(@available(macOS 10.10, *))
		{
			color = [NSColor labelColor];
		}
		else
		{
			color = [NSColor blackColor];
		}
	}
	[self setTextColor:color];
}

- (void) updatePlaceholder
{
	if ( ( [[self string] isEqualToString:@""] || _showingPlaceholder ) && _placeholder )
	{
		if(@available(macOS 10.10, *))
		{
			[self setTextColor:[NSColor placeholderTextColor]];
		}
		else
		{
			[self setTextColor:[NSColor grayColor]];
		}
		[super setString:_placeholder];
		_showingPlaceholder = true;
	}
	else
	{
		[self resetTextView];
	}
}

@end


namespace Rtt
{

// ----------------------------------------------------------------------------

MacTextBoxObject::MacTextBoxObject( const Rect& bounds )
:	Super( bounds ),
	fIsFontSizeScaled( true )
{
}

MacTextBoxObject::~MacTextBoxObject()
{
	// Nil out the delegate to prevent any notifications from being triggered on this dead object
	NSScrollView* scrollview = (NSScrollView*)GetView();
	NSTextView* textview = (NSTextView*)[scrollview documentView];
	[textview setDelegate:nil];
}
	
bool
MacTextBoxObject::Initialize()
{
	Rtt_ASSERT( ! GetView() );
	
	Rect screenBounds;
	GetScreenBounds( screenBounds );
	
	//	const Rect& stageBounds = StageBounds();
	//	NSRect r = NSMakeRect( stageBounds.xMin, stageBounds.yMin, stageBounds.Width(), stageBounds.Height() );
	NSRect r = NSMakeRect( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );

	
	// Annoying: To get scrollable TextViews, it must be placed in a NSScrollView.
	Rtt_NSScrollView* scrollview = [[Rtt_NSScrollView alloc] initWithFrame:r];
	[scrollview setOwner:this];

	NSSize contentsize = [scrollview contentSize];
	
	[scrollview setBorderType:NSNoBorder];
	[scrollview setHasVerticalScroller:YES];
	[scrollview setHasHorizontalScroller:NO];
	[scrollview setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	
	// Set up the NSTextView
	Rtt_NSTextView* textview = [[Rtt_NSTextView alloc] initWithFrame:NSMakeRect(0, 0, contentsize.width, contentsize.height)];

	[textview setDelegate:textview];

	textview.owner = this;
	[textview setEditable:NO];
	[textview setVerticallyResizable:NO];
	
	[textview setMinSize:NSMakeSize(0.0, contentsize.height)];
	[textview setMaxSize:NSMakeSize(FLT_MAX, FLT_MAX)];
	[textview setVerticallyResizable:YES];
	[textview setHorizontallyResizable:NO];
	[textview setAutoresizingMask:NSViewWidthSizable];
	
	[[textview textContainer] setContainerSize:NSMakeSize(contentsize.width, FLT_MAX)];
	[[textview textContainer] setWidthTracksTextView:YES];
	
	
	// Put the textview in the scrollview
	[scrollview setDocumentView:textview];
	[scrollview setAutohidesScrollers:YES];
	
	// Set up horizontal scrollbar
	[[textview enclosingScrollView] setHasHorizontalScroller:YES];
	[textview setHorizontallyResizable:YES];
	[textview setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
	[[textview textContainer] setContainerSize:NSMakeSize(FLT_MAX, FLT_MAX)];
	[[textview textContainer] setWidthTracksTextView:NO];
	
	
	// Will retain the view
	// Note: The scrollview is the parent view, not the textview. This means we must fish out the textview when we need to operate on it elsewhere in this code.
	Super::InitializeView( scrollview );

	// Use the simulator's default font size, if provided.
	if ( IsInSimulator() )
	{
		float fontSize = Rtt_RealToFloat( GetStage()->GetDisplay().GetRuntime().Platform().GetStandardFontSize() );
		if ( ( fontSize > 0 ) && ( fontSize != [textview.font pointSize] ) )
		{
			MacFont *font = Rtt_NEW( LuaContext::GetAllocator( L ), MacFont( [textview font] ) );
			font->SetSize( fontSize );
			[textview setFont:(NSFont*)font->NativeObject()];
			Rtt_DELETE(font);
		}
	}
	
	// We have a problem because  Build(matrix) is computed some time after the object was created.
	// If we insert the object when created, when this method is invoked shortly later, you can see
	// a visible jump in the object's position which looks bad.
	// The workaround is to defer adding the object to the view until after this method has been computed at least once.

	[textview release];
	[scrollview release];

	return (scrollview != nil);

}

const LuaProxyVTable&
MacTextBoxObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetTextBoxObjectProxyVTable();
}

int
MacTextBoxObject::setTextColor( lua_State *L )
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	if ( & o->ProxyVTable() == & PlatformDisplayObject::GetTextBoxObjectProxyVTable() )
	{
		bool isBytecolorRange = o->IsByteColorRange();
		NSColor *c = MacTextObject::GetTextColor( L, 2, isBytecolorRange );
		NSView *v = ((MacTextBoxObject*)o)->GetView();
		NSScrollView *scrollview = (NSScrollView*)v;
		Rtt_NSTextView *t = (Rtt_NSTextView*)[scrollview documentView];
		[t setUserColor:c];
	}
	return 0;
}

int
MacTextBoxObject::setSelection( lua_State *L )
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	if ( & o->ProxyVTable() == & PlatformDisplayObject::GetTextBoxObjectProxyVTable() )
	{
		NSView *v = ((MacTextBoxObject*)o)->GetView();
		NSScrollView *scrollview = (NSScrollView*)v;
		Rtt_NSTextView *t = (Rtt_NSTextView*)[scrollview documentView];
		double startPosition = lua_tonumber( L, 2);
		double endPosition = lua_tonumber( L, 3);
		NSRange range;
		
		NSInteger maxLength = [[t string] length];
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
		
		range.location = startPosition;
		range.length = endPosition - startPosition;
		t.selectedRange = range;
	}
	return 0;
}

int
MacTextBoxObject::getSelection(lua_State* L)
{
    PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject(L, 1);
    if (&o->ProxyVTable() == &PlatformDisplayObject::GetTextBoxObjectProxyVTable())
    {
        NSView *v = ((MacTextBoxObject*)o)->GetView();
        NSScrollView *scrollview = (NSScrollView*)v;
        Rtt_NSTextView *t = (Rtt_NSTextView*)[scrollview documentView];
        NSRange range = t.selectedRange;
        
        lua_pushnumber(L, range.location);
        lua_pushnumber(L, range.location + range.length);

        return 2;
    }
    
    return 0;
}


int
MacTextBoxObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	NSScrollView* scrollview = (NSScrollView*)GetView();
	Rtt_NSTextView* textbox = (Rtt_NSTextView*)[scrollview documentView];

	if ( strcmp( "text", key ) == 0 )
	{
		if ( nil == [textbox getString] )
		{
			lua_pushstring( L, "" ); //[fTextField.text UTF8String] );			
		}
		else
		{
			lua_pushstring( L, [[textbox getString] UTF8String] ); //[fTextField.text UTF8String] );
		}
	}
	else if ( strcmp( "size", key ) == 0 )
	{
		float fontSize = [[textbox font] pointSize];
		if ( fIsFontSizeScaled )
		{
			fontSize *= GetStage()->GetDisplay().GetSxUpright();
		}
		fontSize /= GetSimulatorScale();
		lua_pushnumber( L, fontSize );
	}
	else if ( strcmp( "font", key ) == 0 )
	{
		Display &display = GetStage()->GetDisplay();
		float fontSize = [textbox.font pointSize];
		if ( fIsFontSizeScaled )
		{
			fontSize *= display.GetSxUpright();
		}
		fontSize /= GetSimulatorScale();
		MacFont *font = Rtt_NEW( LuaContext::GetAllocator( L ), MacFont( [textbox font] ) );
		font->SetSize( fontSize );
		result = LuaLibNative::PushFont( L, font );
	}
	else if ( strcmp( "isFontSizeScaled", key ) == 0 )
	{
		lua_pushboolean( L, fIsFontSizeScaled ? 1 : 0 );
	}
	else if ( strcmp( "align", key ) == 0 )
	{
		lua_pushstring( L, AppleAlignment::StringForAlignment( [textbox alignment] ) );
	}
	else if ( strcmp( "setTextColor", key ) == 0 )
	{
		lua_pushcfunction( L, setTextColor );
	}
	else if ( strcmp( "setReturnKey", key ) == 0 )
	{
		lua_pushcfunction( L, setReturnKey);
	}
	else if ( strcmp( "setSelection", key ) == 0 )
	{
		lua_pushcfunction( L, setSelection);
	}
	else if ( strcmp( "getSelection", key ) == 0 )
	{
		lua_pushcfunction( L, getSelection);
	}
	else if ( strcmp( "placeholder", key ) == 0 )
	{
		NSString *text = textbox.placeholder;
		if ( text )
		{
			lua_pushstring( L, [text UTF8String] );
		}
		else
		{
			lua_pushnil( L );
		}
	}
	else if ( strcmp( "hasBackground", key) == 0 )
	{
		lua_pushboolean( L, [textbox drawsBackground] ? 1 : 0 );
	}
	else
	{
		result = Super::ValueForKey( L, key );
	}

	return result;
}

bool
MacTextBoxObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	NSScrollView* scrollview = (NSScrollView*)GetView();
	Rtt_NSTextView* textbox = (Rtt_NSTextView*)[scrollview documentView];

	if ( strcmp( "text", key ) == 0 )
	{
		const char *s = lua_tostring( L, valueIndex );
		if ( Rtt_VERIFY( s ) )
		{
			NSString *newValue = [[NSString alloc] initWithUTF8String:s];
			[textbox setString:newValue];
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
			}
		}
		if ( fontSize < 1.0f )
		{
			fontSize = display.GetRuntime().Platform().GetStandardFontSize();
		}
		fontSize *= GetSimulatorScale();
		if ( ( fontSize > 0 ) && ( fontSize != [textbox.font pointSize] ) )
		{
			MacFont *font = Rtt_NEW( LuaContext::GetAllocator( L ), MacFont( [textbox font] ) );
			font->SetSize( fontSize );
			[textbox setFont:(NSFont*)font->NativeObject()];
			Rtt_DELETE(font);
		}
	}
	else if ( strcmp( "font", key ) == 0 )
	{
		PlatformFont *font = LuaLibNative::ToFont( L, valueIndex );
		if ( font )
		{
			float fontSize = font->Size();
			if ( fontSize >= Rtt_REAL_1 )
			{
				if ( fIsFontSizeScaled )
				{
					fontSize /= GetStage()->GetDisplay().GetSxUpright();
				}
			}
			else
			{
				fontSize = GetStage()->GetDisplay().GetRuntime().Platform().GetStandardFontSize();
			}
			fontSize *= GetSimulatorScale();
			MacFont *scaledFont = Rtt_NEW( LuaContext::GetAllocator( L ), MacFont( (NSFont*)font->NativeObject() ) );
			scaledFont->SetSize( fontSize );
			[textbox setFont:(NSFont*)scaledFont->NativeObject()];
			Rtt_DELETE(scaledFont);
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
		[textbox setEditable:lua_toboolean( L, valueIndex )];
	}
	else if ( strcmp( "align", key ) == 0 )
	{
		[textbox setAlignment:AppleAlignment::AlignmentForString( lua_tostring( L, valueIndex ) )];
	}
	else if ( strcmp( "placeholder", key) == 0 )
	{
		const char *text = lua_tostring( L, valueIndex );
		if ( text )
		{
			[textbox setPlaceholder:[NSString stringWithExternalString:text]];
		}
		else
		{
			[textbox setPlaceholder:nil];
		}
	}
	else if ( strcmp( "hasBackground", key) == 0 )
	{
		int drawsBackground = lua_toboolean( L, valueIndex );
		[scrollview setDrawsBackground:drawsBackground];
		[textbox setDrawsBackground:drawsBackground];
	}
	else
	{
		result = Super::SetValueForKey( L, key, valueIndex );
	}

	return result;
}

void
MacTextBoxObject::DidRescaleSimulator( float previousScale, float currentScale )
{
	// Fetch the native UI's font.
	NSScrollView* scrollview = (NSScrollView*)GetView();
	Rtt_NSTextView* textbox = (Rtt_NSTextView*)[scrollview documentView];
	NSFont* currentFont = [textbox font];
	
	// Scale the current font to the simulator window's new zoom level.
	float fontSize = [currentFont pointSize];
	fontSize /= previousScale;
	fontSize *= currentScale;
	
	// Apply the scaled font to the native UI.
	MacFont *newFont = Rtt_NEW( LuaContext::GetAllocator( L ), MacFont( (NSFont*)currentFont ) );
	newFont->SetSize( fontSize );
	[textbox setFont:(NSFont*)newFont->NativeObject()];
	Rtt_DELETE(newFont);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

