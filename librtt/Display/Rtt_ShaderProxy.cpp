//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Display/Rtt_ShaderProxy.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

ShaderProxy::ShaderProxy()
:Shader()
{
}
ShaderProxy::~ShaderProxy()
{
	Rtt_ASSERT(fInputShader.NotNull());
}
void
ShaderProxy::SetShader(SharedPtr<Shader> inputShader)
{
	fInputShader = inputShader;
}
void
ShaderProxy::Log(std::string preprend, bool last)
{
	printf( "%sShaderProxy::Log(%p)\n", preprend.c_str(), this );
	if (fInputShader.NotNull())
	{
		fInputShader->Log(preprend, last);
	}
}
//	bool IsPointingToPaint(std::string paintId)
//	{
//		bool result = false;
//		if (fInputShader.NotNull() && fInputShader->IsInputNode())
//		{
//			ShaderInput *inp = (ShaderInput*)(&(*fInputShader));
//			
//			result = inp->GetPaint() == paintId;
//		}
//		return result;
//	}
//	void SetInput(int inputChannel, SharedPtr<Shader> shader)
//	{
//		if ( (inputChannel == 0  && IsPointingToPaint("paint1") ) || (inputChannel == 1 && IsPointingToPaint("paint2") ) )
//		{
//			fInputShader = shader;
//		}
//	}
Shader*
ShaderProxy::GetRawPtr()
{
	Shader *result = (Shader*)(&(*fInputShader));
	return result;
}
Shader*
ShaderProxy::GetRawPtr() const
{
	Shader *result = (Shader*)(&(*fInputShader));
	return result;
}
void
ShaderProxy::SetInput(int inputChannel, SharedPtr<Shader> shader)
{
	fInputShader = shader;
}
void
ShaderProxy::SetTextureBounds( const TextureInfo& textureInfo )
{
	fInputShader->SetTextureBounds(textureInfo);
}
bool
ShaderProxy::HasChildren()
{
	return fInputShader->HasChildren();
}
void
ShaderProxy::UpdateCache( const TextureInfo& textureInfo, const RenderData& objectData )
{
	fInputShader->UpdateCache(textureInfo,objectData);
}
Texture*
ShaderProxy::GetTexture() const
{
	return fInputShader->GetTexture();
}
void
ShaderProxy::RenderToTexture( Renderer& renderer, Geometry& cache ) const
{
	fInputShader->RenderToTexture(renderer, cache);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

