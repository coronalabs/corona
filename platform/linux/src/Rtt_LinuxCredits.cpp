//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_LinuxCredits.h"
#include "Rtt_LuaResource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#pragma region Public Member Functions
void EmscriptenCredits::Init(const char *appId, const char *uid, LuaResource *listener)
{
	Super::Init(appId, uid, listener);
}

void EmscriptenCredits::ShowOffers()
{
}

void EmscriptenCredits::RequestUpdate()
{
}

#pragma endregion

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
