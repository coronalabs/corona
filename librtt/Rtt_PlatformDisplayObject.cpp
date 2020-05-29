//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_PlatformDisplayObject.h"

#include "Corona/CoronaLog.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_StageObject.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_LuaResource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const char PlatformDisplayObject::kUserInputEvent[] = "userInput";

const LuaProxyVTable&
PlatformDisplayObject::GetTextFieldObjectProxyVTable()
{
	return LuaPlatformTextFieldObjectProxyVTable::Constant();
}

const LuaProxyVTable&
PlatformDisplayObject::GetTextBoxObjectProxyVTable()
{
	return LuaPlatformTextBoxObjectProxyVTable::Constant();
}

const LuaProxyVTable&
PlatformDisplayObject::GetMapViewObjectProxyVTable()
{
	return LuaPlatformMapViewObjectProxyVTable::Constant();
}

const LuaProxyVTable&
PlatformDisplayObject::GetWebViewObjectProxyVTable()
{
	return LuaPlatformWebViewObjectProxyVTable::Constant();
}

const LuaProxyVTable&
PlatformDisplayObject::GetVideoObjectProxyVTable()
{
	return LuaPlatformVideoObjectProxyVTable::Constant();
}

// ----------------------------------------------------------------------------

PlatformDisplayObject::PlatformDisplayObject()
:	fHandle( NULL ),
	fContentToScreenSx( Rtt_REAL_1 ),
	fContentToScreenSy( Rtt_REAL_1 )
{
}

PlatformDisplayObject::~PlatformDisplayObject()
{
	Rtt_DELETE( fHandle );
}

void
PlatformDisplayObject::Preinitialize( const Display& display )
{
	Real sx, sy;
	display.CalculateContentToScreenScale( sx, sy );

	SetContentToScreenSx( Rtt_RealDiv( Rtt_REAL_1, sx ) );
	SetContentToScreenSy( Rtt_RealDiv( Rtt_REAL_1, sy ) );
}

int
PlatformDisplayObject::DispatchEventWithTarget( const MEvent& e )
{
	int status = 0;

	lua_State *L = GetL();
	if ( L )
	{
		status = Super::DispatchEventWithTarget( L, e, 0 );
	}

	return status;
}

void
PlatformDisplayObject::SetHandle( Rtt_Allocator *allocator, const ResourceHandle< lua_State >& handle )
{
	Rtt_DELETE( fHandle );
	fHandle = Rtt_NEW( allocator, ResourceHandle< lua_State >( handle ) );
}

lua_State*
PlatformDisplayObject::GetL() const
{
	lua_State *L = NULL;

	if ( fHandle )
	{
		L = fHandle->Dereference();
	}

	return L;
}

// TODO: Is this duplication of code in RenderingStream/GPUStream which was used by screen capture
void
PlatformDisplayObject::CalculateScreenBounds(
	const Display& display,
	Real contentToScreenSx,
	Real contentToScreenSy,
	Rect& inOutBounds )
{
	Real offsetX = display.GetXOriginOffset();
	Real offsetY = display.GetYOriginOffset();

	inOutBounds.Translate( offsetX, offsetY );
	inOutBounds.Scale( contentToScreenSx, contentToScreenSy );
}

void
PlatformDisplayObject::GetScreenBounds( Rect& outBounds ) const
{
	outBounds = StageBounds();

	Real offsetX, offsetY;
	GetContentOffsets( offsetX, offsetY );

	outBounds.Translate( offsetX, offsetY );
	outBounds.Scale( GetContentToScreenSx(), GetContentToScreenSy() );
}

void
PlatformDisplayObject::GetContentOffsets( Real& outX, Real& outY ) const
{
	const StageObject *stage = GetStage();
	if ( Rtt_VERIFY( stage ) )
	{
		// Lazily grab the offsets so they match the orientation of the content
		const Display& display = stage->GetDisplay();

		outX = display.GetXOriginOffset();
		outY = display.GetYOriginOffset();
	}
	else
	{
		outX = Rtt_REAL_0;
		outY = Rtt_REAL_0;
	}
}
bool
PlatformDisplayObject::HitTest( Real contentX, Real contentY )
{

	return false;
}

/*
void
PlatformDisplayObject::Build( const Matrix& parentToDstSpace )
{
	Rtt_ASSERT( ! IsOrphan() );

	fWidget->Build( parentToDstSpace, * this );
}

void
PlatformDisplayObject::Translate( Real dx, Real dy )
{
	Super::Translate( dx, dy );

	fWidget->Translate( dx, dy, * this );
}

void
PlatformDisplayObject::Draw( Renderer& renderer ) const
{
	Rtt_ASSERT( ! IsOrphan() );

	if ( ShouldDraw() )
	{
		fWidget->Invalidate();

		// Queue for drawing *after* all Corona (non-native) display objects are drawn
		// PlatformDisplayObjects should be drawn in Rtt::Scene order

		// TODO: RenderingStream should have a way to queue PlatformDisplayObjects
	}
}
*/

int
PlatformDisplayObject::GetNativeProperty( lua_State *L, const char key[] ) const
{
	CORONA_LOG_WARNING( "[object.getNativeProperty()] The native object does not support the key (%s) on this platform.", key );
	return 0;
}

bool
PlatformDisplayObject::SetNativeProperty( lua_State *L, const char key[], int valueIndex )
{
	CORONA_LOG_WARNING( "[object.setNativeProperty()] The native object does not support the key (%s) on this platform.", key );
	return false;
}

int
PlatformDisplayObject::getNativeProperty( lua_State *L )
{
	int result = 0;

	Self *receiver = NULL;
	const char *key = NULL;
	
	if ( lua_type(L, 1) == LUA_TSTRING )
	{ // for compatibility: originally this was implemented as object.getNativeProperty
		receiver = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
		key = lua_tostring( L, 1 );
	}
	else if( lua_type(L, 2) == LUA_TSTRING )
	{
		receiver = (Self*)LuaProxy::GetProxyableObject( L, 1 );
		Rtt_WARN_SIM_PROXY_TYPE( L, 1, PlatformDisplayObject );
		key = lua_tostring( L, 2 );
	}
	
	if ( key && receiver )
	{
		result = receiver->GetNativeProperty( L, key );
	}
	else
	{
		CORONA_LOG_WARNING( "[object.getNativeProperty()] Key parameter is mandatory" );
	}

	
	return result;
}

int
PlatformDisplayObject::setNativeProperty( lua_State *L )
{
	int result = 0;
	
	Self *receiver = NULL;
	const char *key = NULL;
	int valueIndex = 2;

	if ( lua_type(L, 1) == LUA_TSTRING )
	{ // for compatibility: originally this was implemented as object.setNativeProperty
		receiver = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
		key = lua_tostring( L, 1 );
		valueIndex = 2;
	}
	else if( lua_type(L, 2) == LUA_TSTRING )
	{
		receiver = (Self*)LuaProxy::GetProxyableObject( L, 1 );
		Rtt_WARN_SIM_PROXY_TYPE( L, 1, PlatformDisplayObject );
		key = lua_tostring( L, 2 );
		valueIndex = 3;
	}
	
	if ( key && receiver )
	{
		bool success = receiver->SetNativeProperty( L, key, valueIndex );
		lua_pushboolean( L, success );
		result = 1;
	}
	else
	{
		CORONA_LOG_WARNING( "[object.setNativeProperty()] Key parameter is mandatory" );
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

