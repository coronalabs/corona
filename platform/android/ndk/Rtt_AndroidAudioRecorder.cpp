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
#include "Core/Rtt_Data.h"

#include "Rtt_AndroidAudioRecorder.h"
#include "Rtt_LuaContext.h"
#include "NativeToJavaBridge.h"

// Swap 16 bits
static short byteSwap16( short value )
{
    // Byte swap
    int b1 = value & 0xff;
    int b2 = (value >> 8) & 0xff;
    return (short) (b1 << 8 | b2);
    
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidAudioRecorder::AndroidAudioRecorder( 
	const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, const char * file, NativeToJavaBridge *ntjb )
	: PlatformAudioRecorder( handle, allocator, file ), fNativeToJavaBridge(ntjb)
{
}

AndroidAudioRecorder::~AndroidAudioRecorder()
{
	Cleanup();
}

void
AndroidAudioRecorder::Cleanup()
{
}

void
AndroidAudioRecorder::Start()
{
	if ( fIsRunning )
		return;
		
	fIsRunning = fNativeToJavaBridge->RecordStart( (int) this, fFile.GetString() );
}

void
AndroidAudioRecorder::Stop()
{
	if ( ! fIsRunning )
		return;

	fIsRunning = false;

	fNativeToJavaBridge->RecordStop( (int) this );
}

void
AndroidAudioRecorder::SetRunningState( bool isRecording )
{
	fIsRunning = isRecording;
}

void
AndroidAudioRecorder::NotificationCallback( int bytesRead )
{
	if ( bytesRead < 0 )
		return;
		
	Rtt::Data<char>	data( & fAllocator );
	bool result;
	
	do {
		result = fNativeToJavaBridge->RecordGetBytes( (int) this, data );
	} while ( result );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

