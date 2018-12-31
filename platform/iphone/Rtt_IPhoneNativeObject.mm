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
