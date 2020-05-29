//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_CPUResourcePool_H__
#define _Rtt_CPUResourcePool_H__

#include "Renderer/Rtt_MCPUResourceObserver.h"
#include <map>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class CPUResource;

class CPUResourcePool : public MCPUResourceObserver
{
	Rtt_CLASS_NO_COPIES( CPUResourcePool )

	public:
		typedef CPUResourcePool Self;

	public:
		CPUResourcePool();
		virtual ~CPUResourcePool();
		
	public:
		virtual void ReleaseGPUResources();

	public:
		//Should only be called by the cpu resources to register/unregister
		virtual void AttachResource(CPUResource *resource);
		virtual void DetachResource(CPUResource *resource);
		
	private:
		//Not a fan of this practice, let's come up with a better implementation (linked list?)
		std::map<const CPUResource*,CPUResource*> fCPUResources;

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_CPUResourcePool_H__
