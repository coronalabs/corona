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
		virtual PlatformConnection* CreateConnection( const char* url ) const;

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
