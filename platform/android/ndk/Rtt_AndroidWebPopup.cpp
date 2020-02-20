//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"

#include "Rtt_AndroidWebPopup.h"

#include "Core/Rtt_String.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_AndroidWebViewObject.h"
#include "Rtt_MKeyValueIterable.h"
#include "Rtt_Runtime.h"
#include "AndroidDisplayObjectRegistry.h"
#include "NativeToJavaBridge.h"
#include "JavaToNativeBridge.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidWebPopup::AndroidWebPopup(AndroidDisplayObjectRegistry *displayObjectRegistry, NativeToJavaBridge *ntjb) :
	Super(),
	fDisplayObjectRegistry( displayObjectRegistry ),
	fWebView( NULL ),
	fBaseUrl( NULL ),
	fHasBackground( true ),
	fAutoCancelEnabled( true ),
	fBaseDirectory( MPlatform::kUnknownDir ),
	fNativeToJavaBridge(ntjb)
{
}

AndroidWebPopup::~AndroidWebPopup()
{
	Rtt_DELETE( fWebView );
	Rtt_DELETE( fBaseUrl );
}

void
AndroidWebPopup::Show( const MPlatform& platform, const char *url )
{
	// Create a new web view for this popup, if not done already.
	if (!fWebView)
	{
		Rect bounds;
		GetScreenBounds(fNativeToJavaBridge->GetRuntime()->GetDisplay(), bounds);
		fWebView = Rtt_NEW( &platform.GetAllocator(), AndroidWebViewObject(bounds, fDisplayObjectRegistry, fNativeToJavaBridge) );
		fWebView->InitializeAsPopup(fAutoCancelEnabled);
		fWebView->SetBackgroundVisible(fHasBackground);
	}
	
	// Display the requested web page.
	if (fBaseDirectory != MPlatform::kUnknownDir)
	{
		fWebView->Request(url, fBaseDirectory);
	}
	else if (fBaseUrl)
	{
		fWebView->Request(url, fBaseUrl->GetString());
	}
	else
	{
		fWebView->Request(url, NULL);
	}
}

bool
AndroidWebPopup::Close()
{
	SetCallback(NULL);
	if (fWebView)
	{
		Rtt_DELETE( fWebView );
		fWebView = NULL;
	}
	return true;
}

int
AndroidWebPopup::GetWebViewId() const
{
	return fWebView ? fWebView->GetId() : AndroidDisplayObjectRegistry::INVALID_ID;
}

void
AndroidWebPopup::Reset()
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

void
AndroidWebPopup::SetPostParams( const MKeyValueIterable& params )
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

int
AndroidWebPopup::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	if ( strcmp( "baseUrl", key ) == 0 )
	{
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		lua_pushboolean( L, fHasBackground ? 1 : 0 );
	}
	else if ( strcmp( "autoCancel", key ) == 0 )
	{
		lua_pushboolean( L, fAutoCancelEnabled ? 1 : 0 );
	}
	else
	{
		result = 0;
	}

	return result;
}

bool
AndroidWebPopup::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "baseUrl", key ) == 0 )
	{
		// TODO: Currently assumes this property is only set once

		if ( lua_isstring( L, valueIndex ) )
		{
			Rtt_ASSERT( MPlatform::kUnknownDir == fBaseDirectory );

			Rtt_Allocator * allocator = LuaContext::GetRuntime( L )->GetAllocator();
			fBaseUrl =  Rtt_NEW( allocator, String( allocator ) );
			fBaseUrl->Set( lua_tostring( L, valueIndex ) );
		}
		else if ( lua_islightuserdata( L, valueIndex ) )
		{
			Rtt_ASSERT( ! fBaseUrl );

			fBaseDirectory = (MPlatform::Directory)EnumForUserdata(
				 LuaLibSystem::Directories(),
				 lua_touserdata( L, valueIndex ),
				 MPlatform::kNumDirs,
				 MPlatform::kUnknownDir );
		}
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		fHasBackground = lua_toboolean( L, valueIndex ) ? true : false;
	}
	else if ( strcmp( "autoCancel", key ) == 0 )
	{
		fAutoCancelEnabled = lua_toboolean( L, valueIndex ) ? true : false;
	}
	else
	{
		result = false;
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

