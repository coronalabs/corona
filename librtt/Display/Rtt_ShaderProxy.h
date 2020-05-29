//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShaderProxy_H__
#define _Rtt_ShaderProxy_H__

#include "Display/Rtt_Shader.h"
#include "Core/Rtt_SharedPtr.h"
#include "Renderer/Rtt_RenderData.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------
class ShaderProxy : public Shader
{
public:
	ShaderProxy();
	virtual ~ShaderProxy();
	
public:
	void SetShader(SharedPtr<Shader> inputShader);
	void SetInput(int inputChannel, SharedPtr<Shader> shader);

	//Treat as protected
	virtual void Log(std::string preprend, bool last);
	virtual void SetTextureBounds( const TextureInfo& textureInfo );
	virtual bool HasChildren();
	virtual void UpdateCache( const TextureInfo& textureInfo, const RenderData& objectData );
	virtual Texture *GetTexture() const;
	virtual void RenderToTexture( Renderer& renderer, Geometry& cache ) const;
	
private:
	Shader *GetRawPtr();
	Shader *GetRawPtr() const;
	
private:
	SharedPtr<Shader> fInputShader;
	
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderProxy_H__
