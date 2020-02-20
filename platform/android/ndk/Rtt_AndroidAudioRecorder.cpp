//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
		
	fIsRunning = fNativeToJavaBridge->RecordStart( (uintptr_t) this, fFile.GetString() );
}

void
AndroidAudioRecorder::Stop()
{
	if ( ! fIsRunning )
		return;

	fIsRunning = false;

	fNativeToJavaBridge->RecordStop( (uintptr_t) this );
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
		result = fNativeToJavaBridge->RecordGetBytes( (uintptr_t) this, data );
	} while ( result );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

