//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
