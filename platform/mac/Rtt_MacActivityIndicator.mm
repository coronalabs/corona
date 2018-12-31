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

