//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Rtt_AndroidTextFieldObject.h"
#include "Rtt_AndroidFont.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"
#include "NativeToJavaBridge.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_LuaLibDisplay.h"


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidTextFieldObject::AndroidTextFieldObject(
		const Rect& bounds, AndroidDisplayObjectRegistry *displayObjectRegistry, NativeToJavaBridge *ntjb, bool isSingleLine )
:	Super( bounds, displayObjectRegistry, ntjb ),
	fIsSingleLine( isSingleLine ),
	fFontSettings( ntjb->GetRuntime()->GetAllocator() ),
	fIsFontSizeScaled( true )
{
	fFontSettings.SetSize(ntjb->GetRuntime()->Platform().GetStandardFontSize());
}

AndroidTextFieldObject::~AndroidTextFieldObject()
{
}

bool
AndroidTextFieldObject::Initialize()
{
	Rtt_ASSERT( ! GetView() );

	Rect screenBounds;
	GetScreenBounds( screenBounds );

	int result = fNativeToJavaBridge->TextFieldCreate(
			GetId(), screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height(), fIsSingleLine );
	if (!fIsSingleLine)
	{
		// Multiline text fields should be disable by default to match Corona's iOS behavior.
		fNativeToJavaBridge->TextFieldSetEditable(GetId(), false);
	}

	Super::InitializeView( this );

	return result != 0;
}

const LuaProxyVTable&
AndroidTextFieldObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetTextFieldObjectProxyVTable();
}

int
AndroidTextFieldObject::setTextColor( lua_State *L )
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	if ( & o->ProxyVTable() == & PlatformDisplayObject::GetTextFieldObjectProxyVTable() )
	{
		int index = 2;
		bool isByteColorRange = o->IsByteColorRange();
		Color c = LuaLibDisplay::toColor( L, index, isByteColorRange );
		RGBA rgba = ( (ColorUnion*)(& c) )->rgba;
		
		int r = (int)rgba.r;
		int g = (int)rgba.g;
		int b = (int)rgba.b;
		int a = (int)rgba.a;

		NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
		bridge->TextFieldSetColor( ((AndroidDisplayObject*)o)->GetId(), r, g, b, a );
	}

	return 0;
}

int
AndroidTextFieldObject::setReturnKey( lua_State *L )
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	if ( & o->ProxyVTable() == & PlatformDisplayObject::GetTextFieldObjectProxyVTable() )
	{
		const char *keyType = lua_tostring(L, 2);

		NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
		bridge->TextFieldSetReturnKey( ((AndroidDisplayObject*)o)->GetId(), keyType);
	}

	return 0;
}

int
AndroidTextFieldObject::setSelection( lua_State *L )
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );
	if ( & o->ProxyVTable() == & PlatformDisplayObject::GetTextFieldObjectProxyVTable() )
	{
		int startPosition = (int)lua_tonumber(L, 2);
		int endPosition = (int)lua_tonumber(L, 3);

		NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
		bridge->TextFieldSetSelection( ((AndroidDisplayObject*)o)->GetId(), startPosition, endPosition);
	}

	return 0;
}

int
AndroidTextFieldObject::getSelection( lua_State *L )
{
    PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject(L, 1);
    if (&o->ProxyVTable() == &PlatformDisplayObject::GetTextFieldObjectProxyVTable())
    {
        NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata(L, lua_upvalueindex(1));

        int startPosition, endPosition;
        if (bridge->TextFieldGetSelection(((AndroidDisplayObject*)o)->GetId(), startPosition, endPosition))
        {
            lua_pushnumber(L, startPosition);
            lua_pushnumber(L, endPosition);
            return 2;
        }
    }

    return 0;
}

// TODO: move these somewhere in librtt, so all platforms use same constants
static const char kDefaultInputType[] = "default";
static const char kUrlInputType[] = "url";
static const char kNumberInputType[] = "number";
static const char kPhoneInputType[] = "phone";
static const char kEmailInputType[] = "email";

int
AndroidTextFieldObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	if ( strcmp( "text", key ) == 0 )
	{
		Rtt_Allocator * allocator = LuaContext::GetAllocator( L );
		String text( allocator );
		
		fNativeToJavaBridge->TextFieldGetText( GetId(), &text );
		lua_pushstring( L, text.GetString() );
	}
	else if ( strcmp( "size", key ) == 0 )
	{
		float fontSize = fFontSettings.GetSize();
		if ( fIsFontSizeScaled )
		{
			// Convert pixel size to Corona scaled content units.
			fontSize *= fNativeToJavaBridge->GetRuntime()->GetDisplay().GetSxUpright();
		}
		else
		{
			// Convert pixel size to Android DP (Device-independent Pixel) units, which is the legacy behavior.
			fontSize /= (float)fNativeToJavaBridge->GetApproximateScreenDpi() / 160.0f;
		}
		lua_pushnumber( L, fontSize );
	}
	else if ( strcmp( "font", key ) == 0 )
	{
		float fontSize = fFontSettings.GetSize();
		if ( fIsFontSizeScaled )
		{
			// Convert pixel size to Corona scaled content units.
			fontSize *= fNativeToJavaBridge->GetRuntime()->GetDisplay().GetSxUpright();
		}
		else
		{
			// Convert pixel size to Android DP (Device-independent Pixel) units, which is the legacy behavior.
			fontSize /= (float)fNativeToJavaBridge->GetApproximateScreenDpi() / 160.0f;
		}
		Rtt_Allocator *allocatorPointer = LuaContext::GetAllocator( L );
		AndroidFont *fontPointer = Rtt_NEW(allocatorPointer, AndroidFont(
				*allocatorPointer, fFontSettings.GetName(), Rtt_FloatToReal(fontSize), fFontSettings.IsBold()));
		result = LuaLibNative::PushFont( L, fontPointer );
	}
	else if ( strcmp( "isFontSizeScaled", key ) == 0 )
	{
		lua_pushboolean( L, fIsFontSizeScaled ? 1 : 0 );
	}
	else if ( strcmp( "setTextColor", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, setTextColor, 1 );
	}
	else if ( strcmp( "setReturnKey", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, setReturnKey, 1 );
	}
	else if ( strcmp( "setSelection", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, setSelection, 1 );
	}
	else if ( strcmp( "getSelection", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, getSelection, 1 );
	}
	else if ( strcmp( "align", key ) == 0 )
	{
		Rtt_Allocator * allocator = LuaContext::GetAllocator( L );
		String align( allocator );
		
		fNativeToJavaBridge->TextFieldGetAlign( GetId(), &align );
		lua_pushstring( L, align.GetString() );
	}
	else if ( strcmp( "isSecure", key ) == 0 )
	{
		if ( fIsSingleLine )
		{
			bool secure = fNativeToJavaBridge->TextFieldGetSecure( GetId() );
			lua_pushboolean( L, secure );
		}
		else
			result = 0;
	}
	else if ( strcmp( "inputType", key ) == 0 )
	{
		if ( fIsSingleLine )
		{
			Rtt_Allocator * allocator = LuaContext::GetAllocator( L );
			String inputType( allocator );
			
			fNativeToJavaBridge->TextFieldGetInputType( GetId(), &inputType );
		
			lua_pushstring( L, inputType.GetString() );
		}
		else
			result = 0;
	}
	else if ( strcmp( "isEditable", key ) == 0 )
	{
		// This propery is only supported on multiline text boxes to match iOS behavior.
		if ( !fIsSingleLine )
		{
			bool isEditable = fNativeToJavaBridge->TextFieldIsEditable(GetId());
			lua_pushboolean( L, isEditable ? 1 : 0 );
		}
	}
	else if ( strcmp( "placeholder", key ) == 0 )
	{
		Rtt_Allocator * allocator = LuaContext::GetAllocator( L );
		String placeholder( allocator );

		fNativeToJavaBridge->TextFieldGetPlaceholder( GetId(), &placeholder );
		if ( placeholder.GetString() )
		{
			lua_pushstring( L, placeholder.GetString() );
		}
		else
		{
			lua_pushnil( L );
		}
	}
	else if ( strcmp( "margin", key ) == 0 )
	{
		// The margin/padding between the edge of this field and its text in content coordinates.
		if (fIsSingleLine)
		{
			float value = (float)fNativeToJavaBridge->GetDefaultTextFieldPaddingInPixels();
			value *= fNativeToJavaBridge->GetRuntime()->GetDisplay().GetSxUpright();
			lua_pushnumber( L, value );
		}
	}
	else
	{
		result = Super::ValueForKey( L, key );
	}

	return result;
}

bool
AndroidTextFieldObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "text", key ) == 0 )
	{
		const char *s = lua_tostring( L, valueIndex );
		if ( Rtt_VERIFY( s ) )
		{
			fNativeToJavaBridge->TextFieldSetText( GetId(), s );
		}
	}
	else if ( strcmp( "size", key ) == 0 )
	{
		float fontSize = 0;
		if ( lua_isnumber( L, valueIndex ) )
		{
			fontSize = (float)lua_tonumber( L, valueIndex );
			if ( fIsFontSizeScaled )
			{
				// Convert Corona scaled content size to pixels.
				fontSize /= fNativeToJavaBridge->GetRuntime()->GetDisplay().GetSxUpright();
			}
			else
			{
				// Convert Android DP (Device-independent Pixel) units to pixels, which is the legacy behavior.
				fontSize *= (float)fNativeToJavaBridge->GetApproximateScreenDpi() / 160.0f;
			}
		}
		if ( fontSize < 1.0f )
		{
			fontSize = fNativeToJavaBridge->GetRuntime()->Platform().GetStandardFontSize();
		}
		if ( fontSize > 0 )
		{
			fFontSettings.SetSize( fontSize );
			fNativeToJavaBridge->TextFieldSetSize( GetId(), fontSize );
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
					// Convert Corona scaled content size to pixels.
					fontSize /= fNativeToJavaBridge->GetRuntime()->GetDisplay().GetSxUpright();
				}
				else
				{
					// Convert Android DP (Device-independent Pixel) units to pixels, which is the legacy behavior.
					fontSize *= (float)fNativeToJavaBridge->GetApproximateScreenDpi() / 160.0f;
				}
			}
			else
			{
				fontSize = fNativeToJavaBridge->GetRuntime()->Platform().GetStandardFontSize();
			}
			fFontSettings.SetName( font->Name() );
			fFontSettings.SetSize( fontSize );
			fFontSettings.SetIsBold( ((AndroidFont*)font)->IsBold() );
			fNativeToJavaBridge->TextFieldSetFont(
					GetId(), fFontSettings.GetName(), fFontSettings.GetSize(), fFontSettings.IsBold());
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
		if ( fIsSingleLine ) 
		{
			bool secure = lua_toboolean( L, valueIndex );
			fNativeToJavaBridge->TextFieldSetSecure( GetId(), secure );
		}
	}
	else if ( strcmp( "align", key ) == 0 )
	{
		const char * align = lua_tostring( L, valueIndex );
		fNativeToJavaBridge->TextFieldSetAlign( GetId(), align );
	}
	else if ( strcmp( "inputType", key ) == 0 )
	{
		if ( fIsSingleLine ) 
		{
			const char * inputType = lua_tostring( L, valueIndex );
			
			if ( inputType ) 
			{
				fNativeToJavaBridge->TextFieldSetInputType( GetId(), inputType );
			}
		}
	}
	else if ( strcmp( "isEditable", key ) == 0 )
	{
		// This propery is only supported on multiline text boxes to match iOS behavior.
		if ( !fIsSingleLine )
		{
			bool isEditable = lua_toboolean( L, valueIndex );
			fNativeToJavaBridge->TextFieldSetEditable( GetId(), isEditable );
		}
	}
	else if ( strcmp( "placeholder", key ) == 0 )
	{
		const char *placeholder = lua_tostring(L, valueIndex);
		fNativeToJavaBridge->TextFieldSetPlaceholder( GetId(), placeholder);
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

