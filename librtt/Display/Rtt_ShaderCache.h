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

#ifndef _Rtt_ShaderCache_H__
#define _Rtt_ShaderCache_H__

#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_RenderData.h"
#include "Display/Rtt_Shader.h"
#include "Core/Rtt_SharedPtr.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class ShaderComposite;
	
class ShaderCache
{
public:
	ShaderCache(ShaderComposite& fOwner);
	virtual ~ShaderCache();

public:
	RenderData *GetRenderData();
	Geometry *GetGeometry();
	Texture *GetTexture();
	FrameBufferObject *GetFBO();
	SharedPtr<Shader> GetInput();
	
	void SetInput(const SharedPtr<Shader> &input);
	void SetRenderData(RenderData &data);
	void SetGeometry(Geometry *geometry);
	void SetTexture(Texture *texture);
	void SetFBO(FrameBufferObject *fbo);

private:
	ShaderComposite& fOwner;
	RenderData fData;
	
	//Queued Release
	Geometry *fGeometry;		//Owns
	FrameBufferObject *fFBO;	//Owns
	Texture *fTexture;			//Owns(if we have an FBO)

	
	//Move this to shared PTR
	SharedPtr< Shader> fInput;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderCache_H__
