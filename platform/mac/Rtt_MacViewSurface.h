//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacViewSurface_H__
#define _Rtt_MacViewSurface_H__

#include "Rtt_PlatformSurface.h"

#include "Rtt_GPU.h"

// ----------------------------------------------------------------------------

@class GLView;

namespace Rtt
{

// ----------------------------------------------------------------------------

class MacViewSurface : public PlatformSurface
{
	Rtt_CLASS_NO_COPIES( MacViewSurface )

	public:
		typedef PlatformSurface Super;
		typedef PlatformSurfaceDelegate Delegate;

	public:
		MacViewSurface( GLView* view );
		MacViewSurface( GLView* view, S32 adaptiveWidth, S32 adaptiveHeight );
		virtual ~MacViewSurface();

	public:
		virtual void SetCurrent() const;
		virtual void Flush() const;

	public:
		virtual S32 Width() const;
		virtual S32 Height() const;

		virtual S32 AdaptiveWidth() const;
		virtual S32 AdaptiveHeight() const;

	public:
		virtual DeviceOrientation::Type GetOrientation() const;

	public:
		virtual S32 DeviceWidth() const;
		virtual S32 DeviceHeight() const;

	public:
		virtual void SetDelegate( Delegate* delegate );

	private:
		GLView* fView;
		Delegate* fDelegate;
		S32 fAdaptiveWidth;
		S32 fAdaptiveHeight;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacViewSurface_H__
