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
