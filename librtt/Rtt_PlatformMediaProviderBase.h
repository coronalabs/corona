//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformMediaProviderBase_H__
#define _Rtt_PlatformMediaProviderBase_H__

#include "Core/Rtt_Types.h"

#include "Rtt_PlatformModalInteraction.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class PlatformMediaProviderBase : public PlatformModalInteraction
{
	public:
		typedef PlatformModalInteraction Super;

		PlatformMediaProviderBase( const ResourceHandle<lua_State> & handle ) : PlatformModalInteraction( handle )
		{
		}

	public:
		// Use methods: IsProperty() and SetProperty()
		enum PropertyMask
		{
			kAllowEditing = 0x1
		};

	public:
		enum Source
		{
			kPhotoLibrary = 0,
			kCamera,
			kSavedPhotosAlbum,

			kNumSources
		};
		enum Quality
		{
			kLow = 0,
			kMedium,
			kHigh,

			kNumQualitySources
		};
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformMediaProviderBase_H__
