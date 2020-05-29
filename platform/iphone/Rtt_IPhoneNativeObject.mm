//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneNativeObject.h"

#import <UIKit/UIView.h>
#import "CoronaNativeObjectAdapter.h"

#include "Rtt_Lua.h"

#ifdef Rtt_DEBUG
	// Used in asserts in Initialize()
	#include "Display/Rtt_Display.h"
	#include "Rtt_Runtime.h"
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneNativeObject::IPhoneNativeObject( const Rect& bounds, id<CoronaNativeObjectAdapter> adapter )
:	Super( bounds ),
	fAdapter( [adapter retain] )
{
}

IPhoneNativeObject::~IPhoneNativeObject()
{
	[fAdapter release];
}

bool
IPhoneNativeObject::Initialize()
{
	Rtt_ASSERT( ! GetView() );

	Rect screenBounds;
	GetScreenBounds( screenBounds );
	CGRect r = CGRectMake( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );
	UIView *v = fAdapter.view;
	v.frame = r;

	CoronaView *parent = GetCoronaView();
	[parent addSubview:v];

	Super::InitializeView( v );

	return ( nil != v );
}

int
IPhoneNativeObject::ValueForKey( lua_State *L, const char key[] ) const
{
	int result = [fAdapter indexForState:L key:key];
	if ( 0 == result )
	{
		result = Super::ValueForKey( L, key );
	}
	return result;
}

bool
IPhoneNativeObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = [fAdapter newIndexForState:L key:key index:valueIndex];

	if ( ! result )
	{
		result = Super::SetValueForKey( L, key, valueIndex );
	}

	return result;
}

id
IPhoneNativeObject::GetNativeTarget() const
{
	return fAdapter.view;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
