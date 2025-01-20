//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_PlatformAudioRecorder.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTAudioRecorder : public PlatformAudioRecorder
{
	public:
		typedef WinRTAudioRecorder Self;

		WinRTAudioRecorder(const ResourceHandle<lua_State> &handle, Rtt_Allocator &allocator, const char *file);
		virtual ~WinRTAudioRecorder();

		virtual void Start();
		virtual void Stop();
		virtual void NotificationCallback(int status);
};

} // namespace Rtt
