//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MDisplayDelegate_H__
#define _Rtt_MDisplayDelegate_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

class Display;

// ----------------------------------------------------------------------------

class MDisplayDelegate
{
	public:
		virtual void WillDispatchFrameEvent( const Display& sender ) = 0;
//		virtual void DidDispatchFrameEvent( const Display& sender );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MDisplayDelegate_H__
