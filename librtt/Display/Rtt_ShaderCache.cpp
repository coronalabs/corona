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

#include "Display/Rtt_ShaderCache.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_Texture.h"
#include "Display/Rtt_ShaderComposite.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_DisplayObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

	ShaderCache::ShaderCache(ShaderComposite& owner)
	:fOwner(owner)
	{
		fGeometry = NULL;
		fTexture = NULL;
		fFBO = NULL;
		//fInput
	}
	ShaderCache::~ShaderCache()
	{
		
		if (fOwner.GetPaint())
		{
			DisplayObject *observer = fOwner.GetPaint()->GetObserver();
			
			observer->QueueRelease( fGeometry );
			//We only own this if we have an FBO associated with this texture
			if (fFBO != NULL)
			{
				observer->QueueRelease( fTexture );
			}
			
			observer->QueueRelease( fFBO );
			
			fGeometry = NULL;
			fTexture = NULL;
			fFBO = NULL;
		}
	
	}
	
	RenderData *ShaderCache::GetRenderData()
	{
		return &fData;
	}
	Geometry *ShaderCache::GetGeometry()
	{
		return fGeometry;
	}
	Texture *ShaderCache::GetTexture()
	{
		return fTexture;
	}
	FrameBufferObject *ShaderCache::GetFBO()
	{
		return fFBO;
	}
	SharedPtr<Shader> ShaderCache::GetInput()
	{
		return fInput;
	}
	void ShaderCache::SetInput(const SharedPtr<Shader> &input)
	{
		fInput = input;
	}
	void ShaderCache::SetRenderData(RenderData &data)
	{
		fData = data;
	}
	void ShaderCache::SetGeometry(Geometry *geometry)
	{
		fGeometry = geometry;
	}
	void ShaderCache::SetTexture(Texture *texture)
	{
		fTexture = texture;
	}
	void ShaderCache::SetFBO(FrameBufferObject *fbo)
	{
		fFBO = fbo;
	}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

