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

