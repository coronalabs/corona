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

#ifndef _Rtt_PlatformVideoProvider_H__
#define _Rtt_PlatformVideoProvider_H__

#include "Core/Rtt_Types.h"

#include "Rtt_PlatformMediaProviderBase.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class PlatformVideoProvider : public PlatformMediaProviderBase
{
	public:
		typedef PlatformMediaProviderBase Super;

		PlatformVideoProvider( const ResourceHandle<lua_State> & handle ) : PlatformMediaProviderBase( handle )
		{
		}

	public:

	public:
		virtual bool Supports( int source ) const = 0;
		
		virtual bool Show( int source, lua_State* L, int maxTime, int quality ) = 0;

	public:

	protected:

		struct VideoProviderResult
		{
			VideoProviderResult() : SelectedVideoFileName(NULL)
			{ }
			const char* SelectedVideoFileName;
			int Duration;
			long Size;
		};

		static void AddProperties( lua_State *L, void* userdata );

// ----------------------------------------------------------------------------

};

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformVideoProvider_H__
