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


#ifndef _Rtt_AndroidVideoProvider_H__
#define _Rtt_AndroidVideoProvider_H__

#include "librtt/Rtt_PlatformVideoProvider.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/// Provides an video picker window. Selected video can be sent to a Lua listener function.
class AndroidVideoProvider : public PlatformVideoProvider
{
	public:
		AndroidVideoProvider( const ResourceHandle<lua_State> & handle, NativeToJavaBridge *ntjb );
		virtual bool Supports( int source ) const;
		virtual bool Show( int source, lua_State* L, int maxTime, int quality );
		virtual bool IsShown() { return fIsVideoProviderShown; }
		virtual void CloseWithResult( const char *selectedVideoFileName, const int duration, const long size );
		void Abort() { fIsVideoProviderShown = false; }
	
	private:
		
		/// Set TRUE if video picker window is currently shown. Set FALSE if not.
		bool fIsVideoProviderShown;
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidVideoProvider_H__
