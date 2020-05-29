//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_WinData_H__
#define _Rtt_WinData_H__

#include "Rtt_PlatformData.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// Provide a file-like interface to arbitrary platform data/buffer
class WinData : public PlatformData
{

	public:
		virtual ~WinData();

	protected:

	public:
		virtual const char* Read( size_t numBytes, size_t& numBytesRead ) const;
		virtual void Seek( SeekOrigin origin, S32 offset ) const;
		virtual size_t Length() const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_WinData_H__
