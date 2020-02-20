//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_SimulatorRecents__
#define __Rtt_SimulatorRecents__

// ----------------------------------------------------------------------------

#include <string>

namespace Rtt
{
	//STL strings are pushed into separate file container
	//to avoid compilation issue with including stl directly
	//in the appdelegate header
	struct RecentProjectInfo
	{
		std::string formattedString;
		std::string formattedInfo;
		std::string fullURLString;
	};
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_SimulatorRecents__
