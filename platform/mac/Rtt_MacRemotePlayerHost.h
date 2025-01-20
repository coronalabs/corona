//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacRemotePlayerHost_H__
#define _Rtt_MacRemotePlayerHost_H__

#include "Rtt_MacServer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class MacRemotePlayerHost : public MServerDelegate
{
	public:
		MacRemotePlayerHost();
		~MacRemotePlayerHost();

	public:
		bool Start();

	protected:
		void OpenStreams();

	public:
		virtual void DidEnableBonjour( MacServer* server, NSString* name ) const;
		virtual void DidNotEnableBonjour( MacServer* server, NSDictionary* errorDict ) const;
		virtual void DidAcceptConnection( MacServer* server, NSInputStream* istr, NSOutputStream* ostr );

	private:
		mutable MacServer* fServer;
		NSInputStream* fInStream;
		NSOutputStream* fOutStream;
		id fStreamDelegate;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacRemotePlayerHost_H__
