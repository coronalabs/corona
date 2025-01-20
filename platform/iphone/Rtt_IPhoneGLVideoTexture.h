//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneGLVideoTexture_H__
#define _Rtt_IPhoneGLVideoTexture_H__

#include "Renderer/Rtt_GLTexture.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class AVCapture;

// ----------------------------------------------------------------------------

class IPhoneGLVideoTexture : public GLTexture
{
	public:
		typedef GLTexture Super;
		typedef IPhoneGLVideoTexture Self;

	public:
		IPhoneGLVideoTexture();
		virtual ~IPhoneGLVideoTexture();

	public:
		virtual void Create( CPUResource* resource );
		virtual void Update( CPUResource* resource );
		virtual void Destroy();
		virtual void Bind( U32 unit );

	private:
		AVCapture *fAVCapture;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneGLVideoTexture_H__
