//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_MacRemotePlayerHost.h"

#import <Foundation/Foundation.h>

// ----------------------------------------------------------------------------

@interface StreamDelegate : NSObject
{
	Rtt::MacRemotePlayerHost* host;
}
- (id)initWithHost:(Rtt::MacRemotePlayerHost*)aHost;
@end

@implementation StreamDelegate
- (id)initWithHost:(Rtt::MacRemotePlayerHost*)aHost
{
	self = [super init];
	if ( self )
	{
		host = aHost;
	}
	return self;
}

@end


@implementation StreamDelegate(NSStreamDelegate)

- (void) stream:(NSStream*)stream handleEvent:(NSStreamEvent)eventCode
{
/*
	switch ( eventCode )
	{
		case NSStreamEventOpenCompleted:
		{
			[self destroyPicker];
			
			[_server release];
			_server = nil;

			if (stream == _inStream)
				_inReady = YES;
			else
				_outReady = YES;
			
			if (_inReady && _outReady) {
				alertView = [[UIAlertView alloc] initWithTitle:@"Game started!" message:nil delegate:nil cancelButtonTitle:nil otherButtonTitles:@"Continue", nil];
				[alertView show];
				[alertView release];
			}
			break;
		}
		case NSStreamEventHasBytesAvailable:
		{
			if (stream == _inStream) {
				uint8_t b;
				unsigned int len = 0;
				len = [_inStream read:&b maxLength:sizeof(uint8_t)];
				if(!len) {
					if ([stream streamStatus] != NSStreamStatusAtEnd)
						[self _showAlert:@"Failed reading data from peer"];
				} else {
					//We received a remote tap update, forward it to the appropriate view
					if(b & 0x80)
						[(TapView*)[_window viewWithTag:b & 0x7f] touchDown:YES];
					else
						[(TapView*)[_window viewWithTag:b] touchUp:YES];
				}
			}
			break;
		}
		case NSStreamEventEndEncountered:
		{
			NSArray*				array = [_window subviews];
			TapView*				view;
			UIAlertView*			alertView;
			
			NSLog(@"%s", _cmd);
			
			//Notify all tap views
			for(view in array)
				[view touchUp:YES];
			
			alertView = [[UIAlertView alloc] initWithTitle:@"Peer Disconnected!" message:nil delegate:self cancelButtonTitle:nil otherButtonTitles:@"Continue", nil];
			[alertView show];
			[alertView release];

			break;
		}
	}
*/
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacRemotePlayerHost::MacRemotePlayerHost()
:	fServer( NULL ),
	fInStream( nil ),
	fOutStream( nil ),
	fStreamDelegate( [[StreamDelegate alloc] initWithHost:this] )
{
}

MacRemotePlayerHost::~MacRemotePlayerHost()
{
	[fStreamDelegate release];
	delete fServer;
}

bool
MacRemotePlayerHost::Start()
{
	bool result = Rtt_VERIFY( new MacServer( * this ) );

	if ( result )
	{
		NSError* error;
		result = Rtt_VERIFY( fServer->Start( &error ) );

		if ( result )
		{
			// Start advertising to clients, passing nil for the name to tell Bonjour to pick use default name
			NSString* domain = @"local";
			NSString* protocol = MacServer::BonjourType( @"rtt_remote" );
			result = fServer->EnableBonjour( domain, protocol, nil ); Rtt_ASSERT( result );
			Rtt_WARN_SIM( result, ( "Failed to advertise server. Domain(%s) Protocol(%s)\n",
					[domain UTF8String], [protocol UTF8String] ) );
		}
		else
		{
			Rtt_TRACE_SIM( ( "Failed creating server: %s\n", [[error description] UTF8String] ) );
		}
	}

	return result;
}

void
MacRemotePlayerHost::OpenStreams()
{
	fInStream.delegate = fStreamDelegate;
	[fInStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	[fInStream open];
	fOutStream.delegate = fStreamDelegate;
	[fOutStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	[fOutStream open];
}

void
MacRemotePlayerHost::DidEnableBonjour( MacServer* server, NSString* name ) const
{
}

void
MacRemotePlayerHost::DidNotEnableBonjour( MacServer* server, NSDictionary* errorDict ) const
{
}

void
MacRemotePlayerHost::DidAcceptConnection( MacServer* server, NSInputStream* istr, NSOutputStream* ostr )
{
	if ( fInStream || fOutStream || server != fServer )
		return;

	// TODO: Why delete the server?
	delete fServer;
	fServer = NULL;
	
	fInStream = istr;
	[istr retain];
	fOutStream = ostr;
	[ostr retain];

	OpenStreams();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

