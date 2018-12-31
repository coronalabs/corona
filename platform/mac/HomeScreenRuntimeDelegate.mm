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

#import "HomeScreenRuntimeDelegate.h"
#import "CoronaWindowController.h"
#import <AppKit/AppKit.h>

#include "Rtt_MacSimulatorServices.h"

// Modules
#include "Rtt_LuaLibSimulator.h"

// Librtt
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

@class AppDelegate;

namespace Rtt
{

// ----------------------------------------------------------------------------

class ProjectLoadedEvent : public VirtualEvent
{
public:
	ProjectLoadedEvent(){};
	
	virtual const char* Name() const
	{
		static const char kName[] = "_projectLoaded";
		return kName;
	}
};

// ----------------------------------------------------------------------------

HomeScreenRuntimeDelegate::HomeScreenRuntimeDelegate( CoronaWindowController *owner, NSString *resourcePath )
:	fOwner( owner ),
	fSimulatorServices( new MacSimulatorServices( (AppDelegate*)[NSApp delegate], owner, resourcePath ) )
{
}

HomeScreenRuntimeDelegate::~HomeScreenRuntimeDelegate()
{
	delete fSimulatorServices;
}

void
HomeScreenRuntimeDelegate::WillLoadMain( const Runtime& sender ) const
{
	lua_State *L = sender.VMContext().L();
	lua_pushlightuserdata( L, fSimulatorServices );
	LuaContext::RegisterModuleLoader( L, LuaLibSimulator::kName, LuaLibSimulator::Open, 1 );

	fOwner.view.glView.sendAllMouseEvents = YES;
}

void
HomeScreenRuntimeDelegate::DidLoadMain( const Runtime& sender ) const
{
}

void
HomeScreenRuntimeDelegate::ProjectLoaded(Runtime& sender)
{

	ProjectLoadedEvent event;
	sender.DispatchEvent( event );
}
void
HomeScreenRuntimeDelegate::NewProject() const
{
	fSimulatorServices->NewProject();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
