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
#include "Rtt_IPhoneTimer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------


IPhoneTimer::IPhoneTimer( MCallback& callback, GLKViewController *viewController )
:	Super( callback ),
	fViewController(viewController),
	fInterval( 0x8000000 )
{

}

IPhoneTimer::~IPhoneTimer()
{
	Stop();
	fViewController = nil;
}
	
void
IPhoneTimer::Start()
{
	if ( IsRunning() )
	{
		return;
	}
	
	NSInteger fps = 1000.0 / fInterval;
	
	fViewController.preferredFramesPerSecond = fps;

}

void
IPhoneTimer::Stop()
{

}

void
IPhoneTimer::SetInterval( U32 milliseconds )
{
	if ( fInterval != milliseconds )
	{
		bool wasRunning = IsRunning();

		if ( wasRunning ) { Stop(); }

		fInterval = milliseconds;

		if ( wasRunning ) { Start(); }
	}
}

bool
IPhoneTimer::IsRunning() const
{
	return (fViewController.paused == YES);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

