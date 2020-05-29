//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MReloadableResource_H__
#define _Rtt_MReloadableResource_H__

#include "Core/Rtt_Real.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class MReloadableResource
{
	public:
		virtual void ReloadResource() = 0;
		virtual void ReleaseResource() = 0;
		virtual void ResetResource() = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MReloadableResource_H__
