//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformServices_H__
#define _Rtt_PlatformServices_H__

#include "Core/Rtt_Types.h"
#include "Core/Rtt_String.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class MPlatform;

// ----------------------------------------------------------------------------

class MPlatformServices
{
	public:
		virtual const MPlatform& Platform() const = 0;

	public:
		// Preferences methods
		virtual void GetPreference( const char *key, String * value ) const = 0;

		// Pass NULL for value to remove pref
		virtual void SetPreference( const char *key, const char *value ) const = 0;
//		virtual bool SetGlobalPreference( const char *key, const char *value ) const = 0;

		virtual void GetSecurePreference( const char *key, String * value ) const = 0;
		virtual bool SetSecurePreference( const char *key, const char *value ) const = 0;

		virtual void GetLibraryPreference( const char *key, String * value ) const = 0;
		virtual void SetLibraryPreference( const char *key, const char *value ) const = 0;

	public:
		virtual bool IsInternetAvailable() const = 0;
		virtual bool IsLocalWifiAvailable() const = 0;

	public:
		virtual void Terminate() const = 0;
		virtual void Sleep( int milliseconds ) const = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformServices_H__
