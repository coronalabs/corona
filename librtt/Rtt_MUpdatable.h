//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Updatable_H__
#define _Rtt_Updatable_H__

namespace Rtt
{

// ----------------------------------------------------------------------------

class MUpdatable
{
	public:
		virtual void QueueUpdate() = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_Updatable_H__
