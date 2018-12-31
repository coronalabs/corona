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
