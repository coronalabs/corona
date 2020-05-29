//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_CPUResourcePool.h"
#include "Renderer/Rtt_CPUResource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

CPUResourcePool::CPUResourcePool()
{
	
}

CPUResourcePool::~CPUResourcePool()
{
	
}
void CPUResourcePool::ReleaseGPUResources()
{
	for(std::map<const CPUResource*,CPUResource*>::iterator iter = fCPUResources.begin(); iter != fCPUResources.end(); ++iter)
	{
		iter->second->ReleaseGPUResource();
	}
}
void CPUResourcePool::AttachResource(CPUResource *resource)
{
	const CPUResource *ref = static_cast<const CPUResource*>(resource);
	fCPUResources[ref] = resource;

}
void CPUResourcePool::DetachResource(CPUResource *resource)
{
	fCPUResources.erase(resource);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

