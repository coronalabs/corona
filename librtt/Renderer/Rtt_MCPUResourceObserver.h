//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_CPUResourceObserver_H__
#define _Rtt_CPUResourceObserver_H__

// ----------------------------------------------------------------------------

struct Rtt_Allocator;

namespace Rtt
{

class CPUResource;

// ----------------------------------------------------------------------------

class MCPUResourceObserver
{

public:
	virtual ~MCPUResourceObserver(){}; //To avoid warning
	virtual void ReleaseGPUResources() = 0;
	virtual void AttachResource(CPUResource *resource) = 0;
	virtual void DetachResource(CPUResource *resource) = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_CPUResourceObserver_H__
