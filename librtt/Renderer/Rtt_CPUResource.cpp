//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_CPUResource.h"

#include "Renderer/Rtt_Renderer.h"
#include "Core/Rtt_Allocator.h"
#include "Renderer/Rtt_GPUResource.h"
#include "Renderer/Rtt_MCPUResourceObserver.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kFrameBufferObjectString[] = "fbo";
static const char kGeometryString[] = "geometry";
static const char kProgramString[] = "program";
static const char kTextureString[] = "texture";
static const char kUniformString[] = "uniform";

const char *
CPUResource::StringForType( ResourceType t )
{
	const char *result = NULL;

	switch ( t )
	{
		case kFrameBufferObject:
			result = kFrameBufferObjectString;
			break;
		case kGeometry:
			result = kGeometryString;
			break;
		case kProgram:
			result = kProgramString;
			break;
		case kTexture:
			result = kTextureString;
			break;
		case kUniform:
			result = kUniformString;
			break;
		default:
			break;
	}

	Rtt_ASSERT( result );

	return result;
}

// ----------------------------------------------------------------------------

CPUResource::CPUResource(Rtt_Allocator* allocator )
:	fAllocator( allocator ),
	fGPUResource( NULL ),
	fCpuObserver( NULL ),
	fRenderer( NULL )
{
	
}

CPUResource::~CPUResource()
{
	DetachObserver();
	if( fGPUResource && fRenderer )
	{
		fRenderer->QueueDestroy( fGPUResource );
	}
	
}
void
CPUResource::ReleaseGPUResource()
{
	if (fGPUResource != NULL)
	{
		fGPUResource->Destroy();
		delete fGPUResource;
		fGPUResource = NULL;
	}
}
void 
CPUResource::Invalidate()
{
	if( fGPUResource && fRenderer )
	{
		fRenderer->QueueUpdate( this );
	}
}

void 
CPUResource::Allocate()
{
	// Intentionally empty. Derived classes allocate any memory they need.
}

void 
CPUResource::Deallocate()
{
	// Intentionally empty. Derived classes deallocate memory they allocated.
}

GPUResource* 
CPUResource::GetGPUResource() const
{
	return fGPUResource;
}

void 
CPUResource::SetGPUResource( GPUResource* resource )
{
	fGPUResource = resource;
}

void
CPUResource::AttachObserver(MCPUResourceObserver *cpuObserver)
{
	if( ( ! fCpuObserver ) && cpuObserver )
	{
		fCpuObserver = cpuObserver;
		fCpuObserver->AttachResource(this);
	}
}

void
CPUResource::DetachObserver()
{
	if( fCpuObserver )
	{
		fCpuObserver->DetachResource(this);
		fCpuObserver = NULL;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
