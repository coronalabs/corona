//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShaderComposite_H__
#define _Rtt_ShaderComposite_H__

#include "Renderer/Rtt_RenderData.h"
#include "Display/Rtt_Shader.h"
#include "Core/Rtt_SharedPtr.h"
#include "Rtt_LuaUserdataProxy.h"
#include "Renderer/Rtt_Texture.h"
#include <map>

// ----------------------------------------------------------------------------

namespace Rtt
{

class Geometry;
struct RenderData;

// ----------------------------------------------------------------------------

class ShaderComposite : public Shader
{
	public:
		typedef Shader Super;
		typedef ShaderComposite Self;
		
	public:
		ShaderComposite( Rtt_Allocator *allocator, const SharedPtr< ShaderResource >& resource, ShaderData *data );
		virtual ~ShaderComposite();
		
		virtual void Initialize(){fInitializedAsComposite = true;}
		
	public:
		virtual Shader *Clone( Rtt_Allocator *allocator ) const;
		void SetInput(int inputChannel, SharedPtr<Shader> shader);
		virtual void Log(std::string preprend, bool last);
			
	protected:
		TextureInfo GetTextureInfo();		
		virtual void UpdateCache( const TextureInfo& textureInfo, const RenderData& objectData );
		virtual Texture *GetTexture() const;
		virtual void RenderToTexture( Renderer& renderer, Geometry& cache ) const;
		Geometry& GetGeometry() const { return * fGeometry; }
		void UpdateGeometry( const TextureInfo& textureInfo);
		
	public:
		virtual void Prepare( RenderData& objectData, int w, int h, ShaderResource::ProgramMod mod );
		virtual void Draw( Renderer& renderer, const RenderData& objectData, const GeometryWriter* writers = NULL, U32 n = 1 ) const;
		
	public:
		virtual void PushProxy( lua_State *L ) const;
		virtual void DetachProxy(); // Called by Adapter's WillFinalize()???
		
		void SetNamedShader(std::string key, Shader* shader);
		Shader *GetNamedShader(std::string key) const;
		std::map< std::string, Shader* > *GetNamedMap(){return &fMap;}
		virtual bool HasChildren(){return (fInput0.NotNull() || fInput1.NotNull());}
		virtual bool IsTerminal(Shader *shader) const;
		
	private:
		mutable LuaUserdataProxy *fProxy;
		
		//Map does not own nodes, each node owns the input nodes
		std::map< std::string, Shader* > fMap;
		SharedPtr<Shader> fInput0;
		SharedPtr<Shader> fInput1;
		Geometry *fGeometry; // for intermediate stages ONLY
		bool fInitializedAsComposite;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderComposite_H__
