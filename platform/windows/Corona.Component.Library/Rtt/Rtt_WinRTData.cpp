//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Rtt_WinRTData.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_LuaContext.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTData::~WinRTData()
{
}

#pragma endregion


#pragma region Public Member Functions
const char* WinRTData::Read(size_t numBytes, size_t& numBytesRead) const
{
	return NULL;
}

void WinRTData::Seek(SeekOrigin origin, S32 offset) const
{
}

size_t WinRTData::Length() const
{
	return 0;
}

#pragma endregion

} // namespace Rtt
