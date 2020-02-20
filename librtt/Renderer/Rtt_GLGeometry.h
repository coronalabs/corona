//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_GLGeometry_H__
#define _Rtt_GLGeometry_H__

#include "Renderer/Rtt_GL.h"
#include "Renderer/Rtt_GPUResource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class GLGeometry : public GPUResource
{
	public:
		typedef GPUResource Super;
		typedef GLGeometry Self;

	public:
		GLGeometry();

		virtual void Create( CPUResource* resource );
		virtual void Update( CPUResource* resource );
		virtual void Destroy();
		virtual void Bind();

	private:
		GLvoid* fPositionStart;
		GLvoid* fTexCoordStart;
		GLvoid* fColorScaleStart;
		GLvoid* fUserDataStart;
		GLuint fVAO;
		GLuint fVBO;
		GLuint fIBO;
		U32 fVertexCount;
		U32 fIndexCount;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GLGeometry_H__
