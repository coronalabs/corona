//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_LinuxData.h"
#include "Rtt_LuaContext.h"

#pragma once

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#pragma region Constructors/Destructors
EmscriptenData::~EmscriptenData()
{
}

#pragma endregion


#pragma region Public Member Functions
const char* EmscriptenData::Read(size_t numBytes, size_t& numBytesRead) const
{
	return NULL;
}

void EmscriptenData::Seek(SeekOrigin origin, S32 offset) const
{
}

size_t EmscriptenData::Length() const
{
	return 0;
}

#pragma endregion

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
