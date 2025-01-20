//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_PlatformSurface.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>


#pragma region Forward Declarations
namespace Interop
{
	class RuntimeEnvironment;
}

#pragma endregion


namespace Rtt
{

class WinScreenSurface : public PlatformSurface
{
	Rtt_CLASS_NO_COPIES(WinScreenSurface)

	public:
		typedef PlatformSurface Super;

		WinScreenSurface(Interop::RuntimeEnvironment& environment);
		virtual ~WinScreenSurface();

		DeviceOrientation::Type GetOrientation() const;
		virtual void SetCurrent() const;
		virtual void Flush() const;
		virtual S32 Width() const;
		virtual S32 Height() const;
		virtual S32 DeviceWidth() const;
		virtual S32 DeviceHeight() const;
		virtual S32 AdaptiveWidth() const;
		virtual S32 AdaptiveHeight() const;

	private:
		Interop::RuntimeEnvironment& fEnvironment;
		mutable int fPreviousClientWidth;
		mutable int fPreviousClientHeight;
};

}	// namespace Rtt
