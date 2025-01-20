//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_VideoTexture_H__
#define _Rtt_VideoTexture_H__

#include "Renderer/Rtt_Texture.h"
#include "Renderer/Rtt_VideoSource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class VideoTexture : public Texture
{
	public:
		typedef Texture Super;
		typedef VideoTexture Self;
	public:
		VideoTexture( Rtt_Allocator* allocator );

	public:
		virtual ResourceType GetType() const;

		virtual U32 GetWidth() const;
		virtual U32 GetHeight() const;
		virtual Format GetFormat() const;
		virtual Filter GetFilter() const;

		virtual VideoSource GetSource() const;
		virtual void SetSource( VideoSource source );

	private:
		VideoSource fSource;
		
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_VideoTexture_H__
