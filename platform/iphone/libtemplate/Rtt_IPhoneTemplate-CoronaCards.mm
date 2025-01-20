//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneTemplate.h"

#include "IPhoneExports.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

void
IPhoneTemplate::WillLoadMain()
{

}

bool
IPhoneTemplate::IsProperty( Property key )
{
	//This forces certain exports with CK
	//It's added here as convenience, one change could be to call
	//WillLoadMain and move this there

	bool exportsVerified = IPhoneVerifyExports();

	bool kProperties = kIsCoronaKit;

	return exportsVerified && ( kProperties & key );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

