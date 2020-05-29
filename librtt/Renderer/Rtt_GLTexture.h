//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_GLTexture_H__
#define _Rtt_GLTexture_H__

#include "Renderer/Rtt_GL.h"
#include "Renderer/Rtt_GPUResource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class GLTexture : public GPUResource
{
	public:
		typedef GPUResource Super;
		typedef GLTexture Self;

	public:
		virtual void Create( CPUResource* resource );
		virtual void Update( CPUResource* resource );
		virtual void Destroy();
		virtual void Bind( U32 unit );

		virtual GLuint GetName();
private:
	GLint fCachedFormat;
	unsigned long fCachedWidth, fCachedHeight;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GLTexture_H__
