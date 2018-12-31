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

#ifndef _Rtt_Shader_H__
#define _Rtt_Shader_H__

#include "Core/Rtt_SharedPtr.h"
#include "Display/Rtt_ShaderTypes.h"
#include "Display/Rtt_ShaderResource.h"
#include "Renderer/Rtt_Texture.h"

#include <string>

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

struct TextureInfo
{
	int fWidth;
	int fHeight;
	Texture::Format fFormat;
	Texture::Filter fFilter;
	Texture::Wrap fWrap;
};

class FrameBufferObject;
class Display;
class Paint;
struct RenderData;
class Renderer;
class ShaderData;
class ShaderResource;
class Texture;
class Geometry;

// ----------------------------------------------------------------------------

// Shader instances are per-paint
// Each shader:
// * has a weak reference to a shared ShaderResource
// * owns a ShaderData instance which stores the params for the ShaderResource's program
class Shader
{
	public:
		typedef Shader Self;

	public:
		Shader( Rtt_Allocator *allocator, const SharedPtr< ShaderResource >& resource, ShaderData *data );
		virtual ~Shader();
		
	protected:
		Shader();
		
		virtual void Initialize(){}
		
	public:
		virtual Shader *Clone( Rtt_Allocator *allocator ) const;
				
		virtual void Prepare( RenderData& objectData, int w, int h, ShaderResource::ProgramMod mod );

		virtual void Draw( Renderer& renderer, const RenderData& objectData ) const;
		virtual void Log(std::string preprend, bool last);
		virtual void Log();

	public:	//Proxy uses these, they should be treated as protected
		virtual void UpdatePaint( RenderData& data ) const;
		virtual void UpdateCache( const TextureInfo& textureInfo, const RenderData& objectData );

		virtual Texture *GetTexture() const;
		virtual void RenderToTexture( Renderer& renderer, Geometry& cache ) const;
		virtual void SetTextureBounds( const TextureInfo& textureInfo);
		
	public:
		virtual void PushProxy( lua_State *L ) const;
		virtual void DetachProxy(); // Called by Adapter's WillFinalize()???

	public:
		const ShaderData *GetData() const { return fData; }
		ShaderData *GetData() { return fData; }
		ShaderTypes::Category GetCategory() const { return fCategory; }

		// TODO: Rename to observer???
		Paint *GetPaint() const; //{ return fOwner; }
		void SetPaint( Paint *newValue ) { fOwner = newValue; }

		virtual bool UsesUniforms() const;
		virtual bool HasChildren(){return false;}
		virtual bool IsTerminal(Shader *shader) const;
	
		//Shaders need to know about the root node so that
		//changes to their shader data can invalidate the corresponding
		//paint object
		void SetRoot( const Shader *root ) { fRoot = root; }
		bool IsOutermostTerminal() const { return NULL != fOwner; }

	protected:
		SharedPtr< ShaderResource > fResource;
		Rtt_Allocator *fAllocator;
		mutable ShaderData *fData;
		ShaderTypes::Category fCategory;
		Paint *fOwner; // weak ptr
		FrameBufferObject *fFBO;
		Texture *fTexture;
		const Shader *fRoot; // Weak reference
		
		
		// Cache for a shader's output
		mutable RenderData *fRenderData;
		mutable bool fOutputReady;
		mutable bool fDirty;

	// TODO: Figure out better alternative
	friend class ShaderComposite;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Shader_H__
