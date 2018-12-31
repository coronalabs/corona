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

#ifndef _Rtt_PlatformServices_H__
#define _Rtt_PlatformServices_H__

#include "Core/Rtt_Types.h"
#include "Core/Rtt_String.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class MPlatform;
class PlatformConnection;

// ----------------------------------------------------------------------------

class MPlatformServices
{
	public:
		virtual const MPlatform& Platform() const = 0;

	public:
		// Factory methods
		// NOTE: Allocations must occur using Rtt_NEW
		virtual PlatformConnection* CreateConnection( const char* url ) const = 0;

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
