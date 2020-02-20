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
#	include "Rtt_PlatformData.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

/// <summary>Provide a file-like interface to arbitrary platform data/buffer.</summary>
class WinRTData : public PlatformData
{
	public:
		virtual ~WinRTData();

		virtual const char* Read(size_t numBytes, size_t& numBytesRead) const;
		virtual void Seek(SeekOrigin origin, S32 offset) const;
		virtual size_t Length() const;
};

} // namespace Rtt
