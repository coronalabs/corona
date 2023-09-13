//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneTextFieldObject.h"

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


// ----------------------------------------------------------------------------

@implementation Rtt_UITextField

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

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static bool IsIOS_7_OrHigher()
{
	return (floor(NSFoundationVersionNumber) >= NSFoundationVersionNumber_iOS_7_0);
}

static const char kDefaultInputType[] = "default";
static const char kUrlInputType[] = "url";
static const char kNumberInputType[] = "number";
static const char kPhoneInputType[] = "phone";
static const char kEmailInputType[] = "email";
static const char kDecimalInputType[] = "decimal";
static const char kNoEmojiInputType[] = "no-emoji";

// iOS-specific input types
static const char kUIKeyboardTypeASCIICapableString[] = "UIKeyboardTypeASCIICapable";
static const char kUIKeyboardTypeNumbersAndPunctuationString[] = "UIKeyboardTypeNumbersAndPunctuation";
static const char kUIKeyboardTypeNamePhonePadString[] = "UIKeyboardTypeNamePhonePad";
static const char kUIKeyboardTypeTwitterString[] = "UIKeyboardTypeTwitter";
static const char kUIKeyboardTypeWebSearchString[] = "UIKeyboardTypeWebSearch";

static const char *
StringForUIKeyboardType( UIKeyboardType keyboardType )
{
	const char *result = kDefaultInputType;

	switch ( keyboardType )
	{
		case UIKeyboardTypeURL:
			result = kUrlInputType;
			break;
		case UIKeyboardTypeNumberPad:
			result = kNumberInputType;
			break;
		case UIKeyboardTypeDecimalPad:
			result = kDecimalInputType;
			break;
		case UIKeyboardTypePhonePad:
			result = kPhoneInputType;
			break;
		case UIKeyboardTypeEmailAddress:
			result = kEmailInputType;
			break;

		// IOS-specific types
		case UIKeyboardTypeASCIICapable:
			result = kUIKeyboardTypeASCIICapableString;
			break;
		case UIKeyboardTypeNumbersAndPunctuation:
			result = kUIKeyboardTypeNumbersAndPunctuationString;
			break;
		case UIKeyboardTypeNamePhonePad:
			result = kUIKeyboardTypeNamePhonePadString;
			break;
		case UIKeyboardTypeTwitter:
			result = kUIKeyboardTypeTwitterString;
			break;
		case UIKeyboardTypeWebSearch:
			if ( IsIOS_7_OrHigher() )
			{
				result = kUIKeyboardTypeWebSearchString;
			}
			break;
		default:
			break;
	}

	return result;
}

static UIKeyboardType
UIKeyboardTypeForString( const char *keyboardType )
{
	UIKeyboardType result = UIKeyboardTypeDefault;

	if ( keyboardType )
	{
		if ( 0 == strcmp( keyboardType, kUrlInputType ) )
		{
			result = UIKeyboardTypeURL;
		}
		else if ( 0 == strcmp( keyboardType, kNumberInputType ) )
		{
			result = UIKeyboardTypeNumberPad;
		}
		else if ( 0 == strcmp( keyboardType, kDecimalInputType ) )
		{
			result = UIKeyboardTypeDecimalPad;
		}
		else if ( 0 == strcmp( keyboardType, kPhoneInputType ) )
		{
			result = UIKeyboardTypePhonePad;
		}
		else if ( 0 == strcmp( keyboardType, kEmailInputType ) )
		{
			result = UIKeyboardTypeEmailAddress;
		}
		else
		{
			// IOS-specific types
			if ( 0 == strcmp( keyboardType, kUIKeyboardTypeASCIICapableString ) )
			{
				result = UIKeyboardTypeASCIICapable;
			}
			else if ( 0 == strcmp( keyboardType, kUIKeyboardTypeNumbersAndPunctuationString ) )
			{
				result = UIKeyboardTypeNumbersAndPunctuation;
			}
			else if ( 0 == strcmp( keyboardType, kUIKeyboardTypeNamePhonePadString ) )
			{
				result = UIKeyboardTypeNamePhonePad;
			}
			else if ( 0 == strcmp( keyboardType, kUIKeyboardTypeTwitterString ) )
			{
				result = UIKeyboardTypeTwitter;
			}
			else if ( 0 == strcmp( keyboardType, kUIKeyboardTypeWebSearchString ) )
			{
				if ( IsIOS_7_OrHigher() )
				{
					result = UIKeyboardTypeWebSearch;
				}
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

static const char kUITextAutocorrectionTypeDefaultString[] = "UITextAutocorrectionTypeDefault";
static const char kUITextAutocorrectionTypeNoString[] = "UITextAutocorrectionTypeNo";
static const char kUITextAutocorrectionTypeYesString[] = "UITextAutocorrectionTypeYes";

static UITextAutocorrectionType
UITextAutocorrectionTypeForString( const char *value )
{
	UITextAutocorrectionType result = UITextAutocorrectionTypeDefault;

	if ( 0 == strcmp( value, kUITextAutocorrectionTypeNoString ) )
	{
		result = UITextAutocorrectionTypeNo;
	}
	else if ( 0 == strcmp( value, kUITextAutocorrectionTypeYesString ) )
	{
		result = UITextAutocorrectionTypeYes;
	}

	return result;
}

static const char *
StringForUITextAutocorrectionType( UITextAutocorrectionType value )
{
	const char *result = kUITextAutocorrectionTypeDefaultString;

	switch( value )
	{
		case UITextAutocorrectionTypeNo:
			result = kUITextAutocorrectionTypeNoString;
			break;
		case UITextAutocorrectionTypeYes:
			result = kUITextAutocorrectionTypeYesString;
			break;
		default:
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------

static const char kUITextSpellCheckingTypeDefaultString[] = "UITextSpellCheckingTypeDefault";
static const char kUITextSpellCheckingTypeNoString[] = "UITextSpellCheckingTypeNo";
static const char kUITextSpellCheckingTypeYesString[] = "UITextSpellCheckingTypeYes";

static UITextSpellCheckingType
UITextSpellCheckingTypeForString( const char *value )
{
	UITextSpellCheckingType result = UITextSpellCheckingTypeDefault;

	if ( 0 == strcmp( value, kUITextSpellCheckingTypeNoString ) )
	{
		result = UITextSpellCheckingTypeNo;
	}
	else if ( 0 == strcmp( value, kUITextSpellCheckingTypeYesString ) )
	{
		result = UITextSpellCheckingTypeYes;
	}

	return result;
}

static const char *
StringForUITextSpellCheckingType( UITextSpellCheckingType value )
{
	const char *result = kUITextSpellCheckingTypeDefaultString;

	switch( value )
	{
		case UITextSpellCheckingTypeNo:
			result = kUITextSpellCheckingTypeNoString;
			break;
		case UITextSpellCheckingTypeYes:
			result = kUITextSpellCheckingTypeYesString;
			break;
		default:
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------

static const char kUITextClearButtonModeDefaultString[] = "UITextClearButtonModeNever";
static const char kUITextClearButtonModeWhileEditingString[] = "UITextClearButtonModeWhileEditing";
static const char kUITextClearButtonModeUnlessEditingString[] = "UITextClearButtonModeUnlessEditing";
static const char kUITextClearButtonModeModeAlwaystring[] = "UITextClearButtonModeAlways";

static UITextFieldViewMode
UITextClearButtonModeForString( const char *value )
{
	UITextFieldViewMode result = UITextFieldViewModeNever;

	if ( 0 == strcmp( value, kUITextClearButtonModeWhileEditingString ) )
	{
		result = UITextFieldViewModeWhileEditing;
	}
	else if ( 0 == strcmp( value, kUITextClearButtonModeUnlessEditingString ) )
	{
		result = UITextFieldViewModeUnlessEditing;
	}
	else if ( 0 == strcmp( value, kUITextClearButtonModeModeAlwaystring ) )
	{
		result = UITextFieldViewModeAlways;
	}

	return result;
}

static const char *
StringForUITextClearButtonModeType( UITextFieldViewMode value )
{
	const char *result = kUITextClearButtonModeDefaultString;

	switch( value )
	{
		case UITextFieldViewModeWhileEditing:
			result = kUITextClearButtonModeWhileEditingString;
			break;
		case UITextFieldViewModeUnlessEditing:
			result = kUITextClearButtonModeUnlessEditingString;
			break;
		case UITextFieldViewModeAlways:
			result = kUITextClearButtonModeModeAlwaystring;
			break;
		default:
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------

IPhoneTextFieldObject::IPhoneTextFieldObject( const Rect& bounds )
:	Super( bounds ),
	fIsFontSizeScaled( true )
{
}

IPhoneTextFieldObject::~IPhoneTextFieldObject()
{
	Rtt_UITextField *t = (Rtt_UITextField*)GetView();
	t.owner = NULL;
}

bool
IPhoneTextFieldObject::Initialize()
{
	Rtt_ASSERT( ! GetView() );

	CoronaView *coronaView = GetCoronaView();

	Rect screenBounds;
	GetScreenBounds( screenBounds );
	CGRect r = CGRectMake( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );
	Rtt_UITextField *t = [[Rtt_UITextField alloc] initWithFrame:r];
	t.borderStyle = UITextBorderStyleRoundedRect;
    t.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
	t.delegate = coronaView;
	t.owner = this;

	UIView *parent = coronaView;
	[parent addSubview:t];

	Super::InitializeView( t );
	[t release];

	return t;
}

// Reject emojis (or more precisely Unicode characters with the "Symbol Other" property; this matches the behavior on Android)
bool IPhoneTextFieldObject::rejectEmoji(const char *str)
{
	if (fNoEmoji)
	{
		NSError *error = NULL;
		static NSRegularExpression *emojiRegex = [[NSRegularExpression regularExpressionWithPattern:@"\\p{So}"
																						   options:0
																							 error:&error] retain];

		NSString *string = [NSString stringWithUTF8String:str];
		return ([emojiRegex numberOfMatchesInString:string options:0 range:NSMakeRange(0, [string length])] > 0);
	}
	else
	{
		return false;
	}
}

const LuaProxyVTable&
IPhoneTextFieldObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetTextFieldObjectProxyVTable();
}

int
IPhoneTextFieldObject::setTextColor( lua_State *L )
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	if ( & o->ProxyVTable() == & PlatformDisplayObject::GetTextFieldObjectProxyVTable() )
	{
		bool isBytecolorRange = o->IsByteColorRange();
		UIColor *c = IPhoneText::GetTextColor( L, 2, isBytecolorRange );
		UIView *v = ((IPhoneTextFieldObject*)o)->GetView();
		Rtt_UITextField *t = (Rtt_UITextField*)v;
		t.textColor = c;
	}

	return 0;
}

int
IPhoneTextFieldObject::setReturnKey( lua_State *L )
{
    PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
    if ( & o->ProxyVTable() == & PlatformDisplayObject::GetTextFieldObjectProxyVTable() )
    {
        const char* imeType = lua_tostring(L, -1);

        UIView *v = ((IPhoneTextFieldObject*)o)->GetView();
        Rtt_UITextField *t = (Rtt_UITextField*)v;
        t.returnKeyType = IPhoneText::GetUIReturnKeyTypeFromIMEType(imeType);
    }

    return 0;
}

int
IPhoneTextFieldObject::setSelection( lua_State *L )
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	if ( & o->ProxyVTable() == & PlatformDisplayObject::GetTextFieldObjectProxyVTable() )
	{
		double startPosition = lua_tonumber(L, 2);
		double endPosition = lua_tonumber(L, 3);

		UIView *v = ((IPhoneTextFieldObject*)o)->GetView();
		Rtt_UITextField *t = (Rtt_UITextField*)v;

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

		UITextPosition *beginning = t.beginningOfDocument;
		UITextPosition *start = [t positionFromPosition:beginning offset:startPosition];
		UITextPosition *end = [t positionFromPosition:beginning offset:endPosition];
		UITextRange *textRange = [t textRangeFromPosition:start toPosition:end];

		t.selectedTextRange = textRange;
	}

	return 0;
}

int
IPhoneTextFieldObject::getSelection(lua_State *L)
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&o->ProxyVTable() == &PlatformDisplayObject::GetTextFieldObjectProxyVTable())
	{
		UIView *v = ((IPhoneTextFieldObject*)o)->GetView();
		Rtt_UITextField *t = (Rtt_UITextField*)v;

		UITextRange *selectedRange = t.selectedTextRange;
	
		NSInteger startPosition = [t offsetFromPosition:t.beginningOfDocument toPosition:selectedRange.start];
		NSInteger endPosition = [t offsetFromPosition:t.beginningOfDocument toPosition:selectedRange.end];

		lua_pushnumber(L, startPosition);
		lua_pushnumber(L, endPosition);
		
		return 2;
	}

	return 0;
}

int
IPhoneTextFieldObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	Rtt_UITextField *t = (Rtt_UITextField*)GetView();

	if ( strcmp( "text", key ) == 0 )
	{
		lua_pushstring( L, [t.text UTF8String] );
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
	else if ( strcmp( "setTextColor", key ) == 0 )
	{
		lua_pushcfunction( L, setTextColor );
	}
    else if ( strcmp( "setReturnKey", key ) == 0 )
	{
		lua_pushcfunction( L, setReturnKey );
	}
	else if ( strcmp( "setSelection", key ) == 0 )
	{
		lua_pushcfunction( L, setSelection );
	}
	else if ( strcmp( "getSelection", key ) == 0 )
	{
		lua_pushcfunction( L, getSelection );
	}
	else if ( strcmp( "align", key ) == 0 )
	{
		lua_pushstring( L, AppleAlignment::StringForAlignment( t.textAlignment ) );
	}
	else if ( strcmp( "isSecure", key ) == 0 )
	{
		lua_pushboolean( L, t.secureTextEntry );
	}
	else if ( strcmp( "placeholder", key ) == 0 )
	{
		NSString *str = t.placeholder;
		if ( str )
		{
			lua_pushstring( L, [str UTF8String] );
		}
		else
		{
			lua_pushnil( L );
		}
	}
	else if ( strcmp( "inputType", key ) == 0 )
	{
		const char *value = NULL;

		if (fNoEmoji)
		{
			value = kNoEmojiInputType;
		}
		else
		{
			value = StringForUIKeyboardType( t.keyboardType );
		}

		lua_pushstring( L, value );
	}
	else if ( strcmp( "autocorrectionType", key ) == 0 )
	{
		const char *value = StringForUITextAutocorrectionType( t.autocorrectionType );
		lua_pushstring( L, value );
	}
	else if ( strcmp( "spellCheckingType", key ) == 0 )
	{
		const char *value = StringForUITextSpellCheckingType( t.spellCheckingType );
		lua_pushstring( L, value );
	}
	else if ( strcmp( "clearButtonMode", key ) == 0 )
	{
		const char *value = StringForUITextClearButtonModeType( t.clearButtonMode );
		lua_pushstring( L, value );
	}
	else if ( strcmp( "margin", key ) == 0 )
	{
		// The margin/padding between the edge of this field and its text in content coordinates.
		float value = 4.0f * GetStage()->GetDisplay().GetSxUpright();
		value *= ((IPhonePlatformBase&)GetStage()->GetDisplay().GetRuntime().Platform()).GetView().contentScaleFactor;
		lua_pushnumber( L, value );
	}
	else
	{
		result = Super::ValueForKey( L, key );
	}

	return result;
}

bool
IPhoneTextFieldObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	Rtt_UITextField *t = (Rtt_UITextField*)GetView();

	if ( strcmp( "text", key ) == 0 )
	{
		const char *s = lua_tostring( L, valueIndex );
		if ( Rtt_VERIFY( s ) )
		{
			NSString *newValue = [[NSString alloc] initWithUTF8String:s];
			t.text = newValue;
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
	else if ( strcmp( "isSecure", key ) == 0 )
	{
		t.secureTextEntry = lua_toboolean( L, valueIndex );
	}
	else if ( strcmp( "placeholder", key ) == 0 )
	{
		const char *str = lua_tostring( L, valueIndex );
		if ( str )
		{
			t.placeholder = [NSString stringWithUTF8String:str];
		}
		else
		{
			t.placeholder = nil;
		}
	}
	else if ( strcmp( "align", key ) == 0 )
	{
		t.textAlignment = AppleAlignment::AlignmentForString( lua_tostring( L, valueIndex ) );
	}
	else if ( strcmp( "inputType", key ) == 0 )
	{
		const char *inputType = lua_tostring( L, valueIndex );

		UIKeyboardType keyboardType = UIKeyboardTypeDefault;

		if (strcmp( inputType, kNoEmojiInputType ) == 0)
		{
			fNoEmoji = true;
		}
		else
		{
			keyboardType = UIKeyboardTypeForString( inputType );
		}

		t.keyboardType = keyboardType;

		switch( keyboardType )
		{
			case UIKeyboardTypeURL:
			case UIKeyboardTypeEmailAddress:
			case UIKeyboardTypeTwitter:
			case UIKeyboardTypeWebSearch:
                t.autocapitalizationType = UITextAutocapitalizationTypeNone;
				break;
			default:
                t.autocapitalizationType = UITextAutocapitalizationTypeSentences;
				break;
		}
	}
	else if ( strcmp( "autocorrectionType", key ) == 0 )
	{
		const char *value = lua_tostring( L, valueIndex );

		UITextAutocorrectionType newValue = UITextAutocorrectionTypeForString( value );
		t.autocorrectionType = newValue;
	}
	else if ( strcmp( "spellCheckingType", key ) == 0 )
	{
		const char *value = lua_tostring( L, valueIndex );

		UITextSpellCheckingType newValue = UITextSpellCheckingTypeForString( value );
		t.spellCheckingType = newValue;
	}
	else if ( strcmp( "clearButtonMode", key ) == 0 )
	{
		const char *value = lua_tostring( L, valueIndex );

        UITextFieldViewMode newValue = UITextClearButtonModeForString( value );
		t.clearButtonMode = newValue;
	}
	else
	{
		// Custom step for UITextField
		if ( strcmp( "hasBackground", key ) == 0 )
		{
			BOOL hasBackground = (BOOL)lua_toboolean( L, valueIndex );
			t.borderStyle = ( hasBackground ? UITextBorderStyleRoundedRect : UITextBorderStyleNone );
		}

		result = Super::SetValueForKey( L, key, valueIndex );
	}

	return result;

/*
	if ( key )
	{
		if ( 0 == strcmp( key, "placeholder" ) )
		{
			const char *s = lua_tostring( L, valueIndex );
			if ( s )
			{
				fTextField.placeholder = [NSString stringWithUTF8String:s];
			}
		}
	}

	return false;
*/
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
