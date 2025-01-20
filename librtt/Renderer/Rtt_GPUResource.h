//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_GPUResource_H__
#define _Rtt_GPUResource_H__

#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class CPUResource;

// ----------------------------------------------------------------------------

// GPUResources encapsulate the creation, update, and destruction of resources
// specific to a given API (e.g. OpenGL, DirectX, etc). There is a one-to-many
// relationship between the "frontend" ResourceData classes and the "backend"
// GPUResource classes. It is important that derived classes create and destroy
// resources through the provided functions and not through the constructor or
// destructor. This gives the application the flexibility to handle events such
// as context loss in the way most appropriate for each unique platform.
class GPUResource
{
    public:
        typedef GPUResource Self;
        typedef void* Handle;

    public:
        GPUResource();
        virtual ~GPUResource();

        // Allocate GPU resources appropriate for the given data. It is the
        // responsibility of the caller to ensure that Create() is not called
        // multiple times unless it is safe to do so (e.g. Destroy() had been 
        // called or the context was lost and all GPU resources destroyed).
        virtual void Create( CPUResource* resource ) = 0;
        
        // Called when the source data for this GPUResource is manipulated. 
        // Derived classes should update their internal resources to match.
        virtual void Update( CPUResource* resource ) = 0;
        
        // Destroy any allocated internal resources. It is the responsibility
        // of the caller to ensure that this function is not invoked on a 
        // GPUResource that has previously been destroyed or whose internal 
        // resources were destroyed due to some event such as a context loss.
        virtual void Destroy() = 0;

        // Return the opaque handle representing an API-specific GPU resource.
        inline Handle GetHandle() const { return fHandle; }

    protected:
        Handle fHandle;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GPUResource_H__
