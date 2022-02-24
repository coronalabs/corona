//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core/Rtt_Build.h"
#include "Rtt_PlatformData.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/// <summary>Provide a file-like interface to arbitrary platform data/buffer.</summary>
class EmscriptenData : public PlatformData
{
	public:
		virtual ~EmscriptenData();

		virtual const char* Read(size_t numBytes, size_t& numBytesRead) const;
		virtual void Seek(SeekOrigin origin, S32 offset) const;
		virtual size_t Length() const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
