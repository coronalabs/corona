//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
