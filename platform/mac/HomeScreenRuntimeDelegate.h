//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_RuntimeDelegatePlayer.h"

// ----------------------------------------------------------------------------

@class CoronaWindowController;
@class NSString;

namespace Rtt
{

class MacSimulatorServices;

// ----------------------------------------------------------------------------

// TODO: Rename this as all extension windows (including the home screen) use this.
class HomeScreenRuntimeDelegate : public RuntimeDelegatePlayer
{
	public:
		HomeScreenRuntimeDelegate( CoronaWindowController *owner, NSString *resourcePath );
		~HomeScreenRuntimeDelegate();

	// MRuntimeDelegate
	public:
	
		virtual void WillLoadMain( const Runtime& sender ) const;
		virtual void DidLoadMain( const Runtime& sender ) const;

	public:
		virtual void NewProject() const;
		virtual void ProjectLoaded(Runtime& sender);

	private:
		CoronaWindowController *fOwner;
		Rtt::MacSimulatorServices *fSimulatorServices;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
