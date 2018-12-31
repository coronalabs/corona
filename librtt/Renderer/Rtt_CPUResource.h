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

#ifndef _Rtt_CPUResource_H__
#define _Rtt_CPUResource_H__

//#include "Core/Rtt_Array.h"


// ----------------------------------------------------------------------------

struct Rtt_Allocator;

namespace Rtt
{

class GPUResource;
class Renderer;
class MCPUResourceObserver;

// ----------------------------------------------------------------------------

class CPUResource
{
	public:
		typedef CPUResource Self;

		// To avoid RTTI and any accidentally overlapping type declarations,
		// the allowable resource types are explicitly enumerated here.
		typedef enum _ResourceType
		{
			kFrameBufferObject,
			kGeometry,
			kProgram,
			kTexture,
			kUniform,
			kVideoTexture,
			kNumResourceTypes,
		}
		ResourceType;

		static const char *StringForType( ResourceType t );

	public:
		CPUResource( Rtt_Allocator* allocator );
		virtual ~CPUResource();

		// Each derived CPUResource is required to return a unique type to
		// facilitate fast type determination without RTTI.
		virtual ResourceType GetType() const = 0;

		// Flush any changes made to this CPUResource since the last call to
		// Invalidate(). Typically, this involves subloading to the GPU.
		virtual void Invalidate();

		// Called implicitly upon construction of most CPUResources, this 
		// function is only ever expected to be called when an application has
		// previously called Deallocate() but now needs to update the contents
		// of the associated GPUResource.
		virtual void Allocate();

		// To save memory, applications may expressly deallocate the storage
		// used by a CPUResource. However, after deallocation, it is invalid
		// to manipulate the CPUResource or call Invalidate() on it. To do so,
		// applications must call Allocate() and reinitialize the data. Note,
		// Deallocate() should not be called until after the GPUResource has
		// been created and has had a chance to copy the data to the GPU.
		virtual void Deallocate();

		// Get the GPUResource associated with this CPUResource. It uses the
		// data in this class to create, update, and destroy any GPU-specific
		// resources needed for rendering.
		GPUResource* GetGPUResource() const;

		// Set the GPUResource associated with this CPUResource. Note, the 
		// given resource must be of a type appropriate for this CPUResource.
		void SetGPUResource( GPUResource* resource );
		
		void ReleaseGPUResource();
		
		void AttachObserver(MCPUResourceObserver *cpuObserver);
		void DetachObserver();

	protected:
		Rtt_Allocator* GetAllocator() const { return fAllocator; }

	private:
		Rtt_Allocator* fAllocator;

		friend class Renderer;
		Renderer* fRenderer;

		// This is stored here for efficiency so that during rendering
		// the GPUResource does not need to be looked up in a container.
		GPUResource* fGPUResource;
		
		bool fAddedToCleanupPool;
		
		MCPUResourceObserver *fCpuObserver;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_CPUResource_H__
