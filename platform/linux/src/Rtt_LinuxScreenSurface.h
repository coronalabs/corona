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
#include "Rtt_LinuxContext.h"

namespace Rtt
{
	class LinuxPlatform;

	class LinuxScreenSurface : public PlatformSurface
	{
		Rtt_CLASS_NO_COPIES(LinuxScreenSurface)

	public:
		LinuxScreenSurface();
		virtual ~LinuxScreenSurface();
		virtual void SetCurrent() const;
		virtual void Flush() const;
		virtual S32 Width() const;
		virtual S32 Height() const;
		virtual DeviceOrientation::Type GetOrientation() const;
		virtual S32 DeviceWidth() const;
		virtual S32 DeviceHeight() const;
		void setWindow(void *ctx) { fContext = (SolarAppContext*) ctx; }
		void getWindowSize(int *w, int *h);

	private:
		SolarAppContext* fContext;
	};

	class LinuxOffscreenSurface : public PlatformSurface
	{
	public:
		LinuxOffscreenSurface(const PlatformSurface &parent);
		virtual ~LinuxOffscreenSurface();
		virtual void SetCurrent() const;
		virtual void Flush() const;
		virtual S32 Width() const;
		virtual S32 Height() const;

	private:
		S32 fWidth;
		S32 fHeight;
	};
}; // namespace Rtt
