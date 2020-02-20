//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MKeyValueIterable_H__
#define _Rtt_MKeyValueIterable_H__

#include "Rtt_MIterable.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class MKeyValueIterable : public MIterable
{
	public:
		virtual const char* Key() const = 0;
		virtual const char* Value() const = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MKeyValueIterable_H__
