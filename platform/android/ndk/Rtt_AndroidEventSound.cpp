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


#include "Core/Rtt_Build.h"

#include "Rtt_AndroidEventSound.h"
#include "Rtt_LuaContext.h"
#include "NativeToJavaBridge.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidEventSound::AndroidEventSound( const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, NativeToJavaBridge *ntjb ) : 
	PlatformEventSound( handle ), mySoundName( & allocator ), fNativeToJavaBridge(ntjb)
{
}
		
AndroidEventSound::~AndroidEventSound()
{
}

bool
AndroidEventSound::Load( const char * filePath )
{
	mySoundName.Set( filePath );

	fNativeToJavaBridge->LoadSound( (int) this, filePath, true );
}


void 
AndroidEventSound::Play()
{
	fNativeToJavaBridge->PlaySound( (int) this, mySoundName.GetString(), false );
}

void
AndroidEventSound::ReleaseOnComplete()
{
	Rtt_DELETE( this );
}
	
void 
AndroidEventSound::Stop()
{
	Rtt_ASSERT_NOT_REACHED();
}

void 
AndroidEventSound::Pause()
{
	Rtt_ASSERT_NOT_REACHED();
}

void 
AndroidEventSound::Resume()
{
	Rtt_ASSERT_NOT_REACHED();
}

void 
AndroidEventSound::SetVolume( Rtt_Real volume )
{
	Rtt_ASSERT_NOT_REACHED();
}

Rtt_Real 
AndroidEventSound::GetVolume() const
{
	Rtt_ASSERT_NOT_REACHED();
	
	return 1.0f;
}

// ----------------------------------------------------------------------------

} // namespace Rtt



