//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_MacActivityIndicator.h"
#import "AppDelegate.h"
#import "SPILDTopLayerView.h"
#import "YRKSpinningProgressIndicatorLayer.h"

namespace Rtt
{
	
	// ----------------------------------------------------------------------------
	
MacActivityIndicator::MacActivityIndicator()
	:	fIndicatorView( nil )
{
}
	
MacActivityIndicator::~MacActivityIndicator()
{
	[fIndicatorView removeFromSuperview];
	[fIndicatorView release];
}

	
void
MacActivityIndicator::ShowActivityIndicator()
{
	if( nil != fIndicatorView )
	{
		return;
	}
	
	AppDelegate* delegate = (AppDelegate*)[NSApp delegate];	
	GLView* hostview = [delegate layerHostView];
//	[fIndicatorView setFrameSize:[hostview frame].size];
	NSSize size = [hostview frame].size;
	fIndicatorView = [[SPILDTopLayerView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height)];

	[hostview addSubview:fIndicatorView];
	[[fIndicatorView progressIndicatorLayer] startProgressAnimation];
}
void
MacActivityIndicator::HideActivityIndicator()
{
	[[fIndicatorView progressIndicatorLayer] stopProgressAnimation];
	[fIndicatorView removeFromSuperview];
	[fIndicatorView release];
	fIndicatorView = nil;
}

// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

