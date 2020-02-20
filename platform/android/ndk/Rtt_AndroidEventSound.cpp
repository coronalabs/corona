//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

	fNativeToJavaBridge->LoadSound( (uintptr_t) this, filePath, true );
	return true;
}


void 
AndroidEventSound::Play()
{
	fNativeToJavaBridge->PlaySound( (uintptr_t) this, mySoundName.GetString(), false );
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



