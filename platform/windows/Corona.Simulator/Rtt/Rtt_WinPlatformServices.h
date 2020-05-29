//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Build.h"
#include "Core\Rtt_String.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformServices.h"


namespace Rtt
{

class WinPlatformServices : public MPlatformServices
{
	Rtt_CLASS_NO_COPIES( WinPlatformServices )

	public:
		typedef WinPlatformServices Self;

	public:
		WinPlatformServices( const MPlatform& platform );
//		~WinPlatformServices();

	protected:
		bool RequestAdminAuthorization(const char *name) const;

	public:
		// MPlatformServices
		virtual const MPlatform& Platform() const;

		virtual void GetPreference( const char *key, String * value ) const;

		// Pass NULL for value to remove pref
		virtual void SetPreference( const char *key, const char *value ) const;

		virtual void GetSecurePreference(const char *key, String * value) const;
		virtual bool SetSecurePreference( const char *key, const char *value ) const;

		virtual void GetLibraryPreference( const char *key, String * value ) const;
		virtual void SetLibraryPreference( const char *key, const char *value ) const;

		virtual bool IsInternetAvailable() const;
		virtual bool IsLocalWifiAvailable() const;
		virtual void Terminate() const;

		virtual void Sleep( int milliseconds ) const;

	private:
		const MPlatform& fPlatform;
};

} // namespace Rtt
