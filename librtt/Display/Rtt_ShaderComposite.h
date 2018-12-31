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
		virtual void Draw( Renderer& renderer, const RenderData& objectData ) const;
		
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
