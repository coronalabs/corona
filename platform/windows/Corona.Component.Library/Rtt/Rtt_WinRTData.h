// ----------------------------------------------------------------------------
// 
// Rtt_WinRTData.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

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
