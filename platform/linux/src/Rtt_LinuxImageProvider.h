//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core/Rtt_Types.h"
#include "Core/Rtt_String.h"
#include "Rtt_PlatformImageProvider.h"
#include "Rtt_LinuxVideoObject.h"
#include "Rtt_PlatformData.h"
#include "Rtt_Runtime.h"

namespace Rtt
{
	/// Provides an image picker window. Selected image can be sent to a Lua listener function.
	class LinuxImageProvider : public PlatformImageProvider
	{
	public:
		LinuxImageProvider(const ResourceHandle<lua_State> &handle);
		virtual ~LinuxImageProvider();
		virtual bool Supports(int source) const;
		virtual bool Show(int source, const char *filePath, lua_State *L);
	};
}; // namespace Rtt
