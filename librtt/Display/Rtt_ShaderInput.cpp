//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShaderInput.h"
#include "Display/Rtt_ShaderData.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

ShaderInput::ShaderInput( int inputChannel, std::string paintInput)
:Super(),
fPaint(NULL)
{
	fInputChannel = inputChannel;
	fPaintInput = paintInput;
}

ShaderInput::~ShaderInput()
{
	fPaint = NULL;
}

void
ShaderInput::Log(std::string preprend, bool last)
{
	printf( "%sShaderInput::Log(%p)::(InputSource:%s)\n", preprend.c_str(), this, fPaintInput.c_str() );
}

Shader*
ShaderInput::Clone( Rtt_Allocator *allocator ) const
{
	ShaderData *data = NULL;
	if ( fData )
	{
		data = fData->Clone( allocator );
	}
	return Rtt_NEW( allocator, ShaderInput(fInputChannel,fPaintInput ) );
}

void
ShaderInput::UpdateCache( const TextureInfo& textureInfo, const RenderData& objectData )
{
	if (fInputChannel == 0)
	{
		if (fPaintInput == "paint1")
		{
			fPaint = objectData.fFillTexture0;
		}
		else
		{
			fPaint = objectData.fFillTexture1;
		}
	}

	if (fInputChannel == 1)
	{
		if (fPaintInput == "paint1")
		{
			fPaint = objectData.fFillTexture0;
		}
		else
		{
			fPaint = objectData.fFillTexture1;
		}
	}
}

void
ShaderInput::RenderToTexture( Renderer& renderer, Geometry& cache ) const
{

}

void
ShaderInput::SetTextureBounds( const TextureInfo& textureInfo )
{

}

void
ShaderInput::Prepare( RenderData& objectData, int w, int h, ShaderResource::ProgramMod mod )
{

}

Texture*
ShaderInput::GetTexture() const
{
	return fPaint;
}
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

