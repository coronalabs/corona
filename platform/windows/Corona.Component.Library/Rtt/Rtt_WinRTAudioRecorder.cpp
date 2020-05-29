//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Rtt_WinRTAudioRecorder.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Core/Rtt_Data.h"
#	include "Rtt_LuaContext.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTAudioRecorder::WinRTAudioRecorder(
	const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, const char * file)
:	PlatformAudioRecorder(handle, allocator, file)
{
}

WinRTAudioRecorder::~WinRTAudioRecorder()
{
}

#pragma endregion


#pragma region Public Member Functions
void WinRTAudioRecorder::Start()
{
}

void WinRTAudioRecorder::Stop()
{
}

void WinRTAudioRecorder::NotificationCallback(int bytesRead)
{
}

#pragma endregion

} // namespace Rtt
