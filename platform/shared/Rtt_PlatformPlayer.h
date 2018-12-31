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

#ifndef _Rtt_PlatformPlayer_H__
#define _Rtt_PlatformPlayer_H__

#include "Rtt_DeviceOrientation.h"
#include "Rtt_Runtime.h"
#include "Rtt_RuntimeDelegatePlayer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class MCallback;
class MPlatform;

// ----------------------------------------------------------------------------

// Base class
// 
// A PlatformPlayer instance is essentially a ( Runtime, MPlatform ) pair.
// 
// The MPlatform instance must ***outlive*** the Runtime instance.
class PlatformPlayer
{
	Rtt_CLASS_NO_COPIES( PlatformPlayer )

		typedef PlatformPlayer Self;

	public:
		static const char* DefaultAppFilePath();
		static const char* InteractiveFilePath();

	public:
		// For platforms using OpenGL, the context must be available/init'd
		// before calling this c-tor. 
		PlatformPlayer( const MPlatform& platform, MCallback *viewCallback );
		virtual ~PlatformPlayer();

	public:
		void Start(
				const char* appFilePath,
				bool connectToDebugger,
				DeviceOrientation::Type launchOrientation );

	public:
		const MPlatform& GetPlatform() const { return fPlatform; }
		const Runtime& GetRuntime() const { return fRuntime; }

	public:
		MPlatform& GetPlatform()
		{
			return const_cast< MPlatform& >(
				static_cast< const Self* >( this )->GetPlatform() );
		}

		Runtime& GetRuntime()
		{
			return const_cast< Runtime& >(
				static_cast< const Self* >( this )->GetRuntime() );
		}

	private:
		const MPlatform& fPlatform;
		Runtime fRuntime;
		RuntimeDelegatePlayer fRuntimeDelegate;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformPlayer_H__
