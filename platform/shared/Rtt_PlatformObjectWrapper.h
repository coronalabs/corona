//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformObjectWrapper_H__
#define _Rtt_PlatformObjectWrapper_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// Base class for creating classes that wrap platform-specific ObjectWrappers,
// e.g. strings, dictionaries, etc.
class PlatformObjectWrapper
{
	public:
		virtual ~PlatformObjectWrapper();

	public:
		virtual void* PlatformObject() const = 0;

	private:
		
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformObjectWrapper_H__
