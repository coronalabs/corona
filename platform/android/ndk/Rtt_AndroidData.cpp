//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"

#include "Rtt_AndroidData.h"

#include "Rtt_LuaContext.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

AndroidData::~AndroidData()
{
}

const char* 
AndroidData::Read( size_t numBytes, size_t& numBytesRead ) const
{
	return NULL; // ??? unimplemented
}

void 
AndroidData::Seek( SeekOrigin origin, S32 offset ) const
{
	// ??? unimplemented
}

size_t 
AndroidData::Length() const
{
	return 0; // ??? unimplemented
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

