//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShaderInput_H__
#define _Rtt_ShaderInput_H__

#include "Renderer/Rtt_FrameBufferObject.h"
#include "Display/Rtt_Shader.h"
#include "Core/Rtt_SharedPtr.h"
#include "Renderer/Rtt_RenderData.h"
#include "Rtt_LuaUserdataProxy.h"
#include <map>
#include <string>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class ShaderInput : public Shader
{
	public:
		typedef Shader Super;
		typedef ShaderInput Self;

	public:
		ShaderInput( int inputChannel, std::string paintInput);
		virtual ~ShaderInput();
		
		virtual Shader *Clone( Rtt_Allocator *allocator ) const;
		
		virtual void Log(std::string preprend, bool last);
				
	protected:
		virtual void SetTextureBounds( const TextureInfo& textureInfo);
		
		virtual void RenderToTexture( Renderer& renderer, Geometry& cache ) const;

		virtual void UpdateCache( const TextureInfo& textureInfo, const RenderData& objectData );

		virtual void Prepare( RenderData& objectData, int w, int h, ShaderResource::ProgramMod mod );
		
		virtual Texture *GetTexture() const;

	private:
		int fInputChannel;
		std::string fPaintInput;
		Texture *fPaint; // Does NOT own.


};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderInput_H__
