//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
