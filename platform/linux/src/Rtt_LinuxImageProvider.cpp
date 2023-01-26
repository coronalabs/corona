//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_LinuxImageProvider.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Display/Rtt_Display.h"

namespace Rtt
{
	LinuxImageProvider::LinuxImageProvider(const ResourceHandle<lua_State> &handle)
		: PlatformImageProvider(handle)
	{
	}

	LinuxImageProvider::~LinuxImageProvider()
	{
	}

	bool LinuxImageProvider::Supports(int source) const
	{
		switch (source)
		{
			case PlatformImageProvider::kPhotoLibrary:
				Rtt_LogException("Photo library is not supported on this platform.\n");
				break;

			case PlatformImageProvider::kCamera:
				Rtt_LogException("Camera is not supported on this platform.\n");
				break;
		}

		return false;
	}

	bool LinuxImageProvider::Show(int source, const char* filePath, lua_State* L)
	{
		switch (source)
		{
			case PlatformImageProvider::kPhotoLibrary:
				Rtt_LogException("Can't open the photo library as is not supported on this platform.\n");
				break;

			case PlatformImageProvider::kCamera:
				Rtt_LogException("Can't open the Camera is not supported on this platform.\n");
				break;
		}

		return false;
	}
}; // namespace Rtt
