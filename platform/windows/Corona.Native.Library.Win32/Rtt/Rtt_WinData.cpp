//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core/Rtt_Build.h"

#include "Rtt_WinData.h"

#include "Rtt_LuaContext.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

WinData::~WinData()
{
}

const char* 
WinData::Read( size_t numBytes, size_t& numBytesRead ) const
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
	return NULL;
}

void 
WinData::Seek( SeekOrigin origin, S32 offset ) const
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

size_t 
WinData::Length() const
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
	return 0; 
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

