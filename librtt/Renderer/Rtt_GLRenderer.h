 //////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_GLRenderer_H__
#define _Rtt_GLRenderer_H__

#include "Renderer/Rtt_Renderer.h"

// ----------------------------------------------------------------------------

struct Rtt_Allocator;

namespace Rtt
{

class GPUResource;
class CPUResource;

// ----------------------------------------------------------------------------

class GLRenderer : public Renderer
{
	public:
		typedef Renderer Super;
		typedef GLRenderer Self;

	public:
		GLRenderer( Rtt_Allocator* allocator );

	protected:
		// Create an OpenGL resource appropriate for the given CPUResource.
		virtual GPUResource* Create( const CPUResource* resource );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GLRenderer_H__
