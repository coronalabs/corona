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
		// STEVE CHANGE
		struct ExtraOptions {
			U8 depthBits;
			U8 stencilBits;
		};
		// /STEVE CHANGE
		FrameBufferObject( Rtt_Allocator* allocator, Texture* texture, ExtraOptions * options = NULL ); // <- STEVE CHANGE
		virtual ResourceType GetType() const;
		Texture* GetTexture() const;
		
		// STEVE CHANGE
		U8 GetDepthBits() const { return fDepthBits; }
		U8 GetStencilBits() const { return fStencilBits; }
		// /STEVE CHANGE 
	private:
		Texture* fTexture;
		// STEVE CHANGE
		U8 fDepthBits;
		U8 fStencilBits;
		// /STEVE CHANGE
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_FrameBufferObject_H__
