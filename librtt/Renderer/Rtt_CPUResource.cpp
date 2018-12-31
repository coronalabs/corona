//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
