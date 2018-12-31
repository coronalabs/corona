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

#include "Renderer/Rtt_GLRenderer.h"

#include "Renderer/Rtt_GLCommandBuffer.h"
#include "Renderer/Rtt_GLFrameBufferObject.h"
#include "Renderer/Rtt_GLGeometry.h"
#include "Renderer/Rtt_GLProgram.h"
#include "Renderer/Rtt_GLTexture.h"
#include "Renderer/Rtt_CPUResource.h"
#include "Core/Rtt_Assert.h"

// TODO: Temporary hack
#ifdef Rtt_IPHONE_ENV
#include "../platform/iphone/Rtt_IPhoneGLVideoTexture.h"
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

GLRenderer::GLRenderer( Rtt_Allocator* allocator )
:   Super( allocator )
{
	fFrontCommandBuffer = Rtt_NEW( allocator, GLCommandBuffer( allocator ) );
	fBackCommandBuffer = Rtt_NEW( allocator, GLCommandBuffer( allocator ) );
}

GPUResource* 
GLRenderer::Create( const CPUResource* resource )
{
	switch( resource->GetType() )
	{
		case CPUResource::kFrameBufferObject: return new GLFrameBufferObject;
		case CPUResource::kGeometry: return new GLGeometry;
		case CPUResource::kProgram: return new GLProgram;
		case CPUResource::kTexture: return new GLTexture;
		case CPUResource::kUniform: return NULL;
#ifdef Rtt_IPHONE_ENV
		case CPUResource::kVideoTexture: return new IPhoneGLVideoTexture;
#endif
		default: Rtt_ASSERT_NOT_REACHED(); return NULL;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
