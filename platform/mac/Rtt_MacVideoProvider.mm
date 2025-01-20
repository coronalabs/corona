//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

