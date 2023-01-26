//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_FrameBufferObject_H__
#define _Rtt_FrameBufferObject_H__

#include "Renderer/Rtt_CPUResource.h"
#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Texture;

// ----------------------------------------------------------------------------

class FrameBufferObject : public CPUResource
{
	public:
		typedef CPUResource Super;
		typedef FrameBufferObject Self;

	public:
		struct ExtraOptions {
			U8 depthBits;
			U8 stencilBits;
			bool mustClear;
		};

		FrameBufferObject( Rtt_Allocator* allocator, Texture* texture, ExtraOptions * options = NULL );
	
		virtual ResourceType GetType() const;
		Texture* GetTexture() const;
		U8 GetDepthBits() const { return fDepthBits; }
		U8 GetStencilBits() const { return fStencilBits; }
		bool GetMustClear() const { return fMustClear; }

	private:
		Texture* fTexture;
		U8 fDepthBits;
		U8 fStencilBits;
		bool fMustClear;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_FrameBufferObject_H__
