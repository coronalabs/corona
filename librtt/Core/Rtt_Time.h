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

#ifndef __Rtt_Time_H__
#define __Rtt_Time_H__

// ----------------------------------------------------------------------------

#include "Rtt_Macros.h"

#if defined( Rtt_APPLE_ENV ) || defined( Rtt_ANDROID_ENV ) || defined( Rtt_WEBOS_ENV ) || defined( Rtt_EMSCRIPTEN_ENV ) || defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV ) || defined( Rtt_NINTENDO_ENV ) || defined( Rtt_LINUX_ENV )
	#include <stdint.h>
	typedef uint64_t Rtt_AbsoluteTime;
#elif defined( Rtt_SYMBIAN_ENV )
	Rtt_STATIC_ASSERT( false );
#else
	Rtt_STATIC_ASSERT( false );
#endif

Rtt_EXPORT Rtt_AbsoluteTime Rtt_GetAbsoluteTime(void);
Rtt_EXPORT uint64_t Rtt_AbsoluteToMilliseconds( Rtt_AbsoluteTime absoluteTime );
Rtt_EXPORT uint64_t Rtt_AbsoluteToMicroseconds( Rtt_AbsoluteTime absoluteTime );

// ----------------------------------------------------------------------------

#endif // __Rtt_Time_H__
