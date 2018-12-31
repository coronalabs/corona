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

#include "Rtt_MacVideoProvider.h"

#include "Rtt_AppleBitmap.h"
#include "Rtt_AppleData.h"
#include "Rtt_MacSimulator.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_Runtime.h"
#import "Rtt_MacPlatform.h"

#import <AppKit/NSApplication.h>
#import <AppKit/NSOpenPanel.h>
#import <Quartz/Quartz.h> // For ImageKit

#import "AppDelegate.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------
	
MacVideoProvider::MacVideoProvider( const ResourceHandle<lua_State> & handle ) 
	: PlatformVideoProvider( handle ),
	fPickerControllerDelegate( nil ),
	fInterfaceIsUp(false),
	fDstPath( NULL )
{
#if 0 // VideoProvider not currently supported on Mac
	fPickerControllerDelegate = [[MacPickerControllerDelegate alloc] init];
	[fPickerControllerDelegate setImageProvider:this];
#endif
}

MacVideoProvider::~MacVideoProvider()
{
#if 0 // VideoProvider not currently supported on Mac
	[fPickerControllerDelegate release];
#endif
    
    if (fDstPath != NULL )
    {
        free( fDstPath );
    }
}

	
bool
MacVideoProvider::Supports( int source ) const
{
	return true;
}

bool
MacVideoProvider::Show( int source, lua_State* L, int maxTime, int quality )
{
	Rtt_TRACE_SIM( ( "WARNING: Simulator does not support videos.\n" ) );
	return false;
}
	
void
MacVideoProvider::DidDismiss( NSImage* image, bool completed )
{
	AppleFileBitmap* bitmap = NULL;
	if ( image )
	{	
		Rtt_Allocator* allocator __attribute__((unused)) = NULL;
		bitmap = Rtt_NEW( allocator, MacFileBitmap( image ) );
	}

	fInterfaceIsUp = false;

}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

