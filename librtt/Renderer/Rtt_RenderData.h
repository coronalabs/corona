//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_RenderData_H__
#define _Rtt_RenderData_H__

#include "Core/Rtt_Types.h"
#include "Core/Rtt_Real.h"
#include "Renderer/Rtt_RenderTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Geometry;
class Program;
class Texture;
class Uniform;

// ----------------------------------------------------------------------------

// RenderData encapsulates all of the information needed to draw a single 
// entity including its geometry, shader programs, and other render state.
// Every renderable entity should have a RenderData instance and pass this
// to the Renderer each frame that it is to be drawn.
struct RenderData
{
	
	RenderData();
	void Log( const char *msg = NULL ) const;

	Geometry* fGeometry;
	Program* fProgram;
	Texture* fFillTexture0;
	Texture* fFillTexture1;
	Texture* fMaskTexture;
	Uniform* fMaskUniform;
	Uniform* fUserUniform0;
	Uniform* fUserUniform1;
	Uniform* fUserUniform2;
	Uniform* fUserUniform3;
//	Real fMinDepthRange;
//	Real fMaxDepthRange;
	BlendMode fBlendMode;
	RenderTypes::BlendEquation fBlendEquation;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_RenderData_H__
