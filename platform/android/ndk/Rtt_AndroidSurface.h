//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidSurface_H__
#define _Rtt_AndroidSurface_H__

#include "librtt/Rtt_PlatformSurface.h"

#include "librtt/Rtt_GPU.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class GLView;

// ----------------------------------------------------------------------------

// this may be mac specific
/*
class AndroidSurfaceDelegate : public PlatformSurfaceDelegate
{
	public:
		virtual void WillFlush( const AndroidSurface& surface ) const;
		virtual void DidFlush( const AndroidSurface& surface ) const;
};
*/

class AndroidSurface : public PlatformSurface
{
    Rtt_CLASS_NO_COPIES( AndroidSurface )

	public:
		AndroidSurface( GLView* view );

	public:
		virtual ~AndroidSurface();

	public:
		virtual void SetCurrent() const;
		virtual void Flush() const;

	private:
        GLView* fView;
};

// this is probably unused
/*
class AndroidOffscreenSurface : public PlatformSurface
{
    Rtt_CLASS_NO_COPIES( AndroidOffscreenSurface )

    public:
        AndroidOffscreenSurface( const PlatformSurface& parent );
        virtual ~AndroidOffscreenSurface();

    public:
        virtual void SetCurrent() const;
        virtual void Flush() const;
        virtual const SurfaceProperties& Properties() const;

    private:
};
*/

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidSurface_H__
