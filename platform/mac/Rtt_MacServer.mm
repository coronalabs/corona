//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "Rtt_MacServer.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
//#include <CFNetwork/CFSocketStream.h>
#import <Foundation/Foundation.h>



@interface NetServiceDelegate : NSObject
{
	Rtt::MacServer* server;
}

- (id)initWithServer: (Rtt::MacServer*)server;

@end

@implementation NetServiceDelegate

- (id)initWithServer:(Rtt::MacServer*)aServer
{
	self = [super init];
	if ( self )
	{
		server = aServer;
	}
	return self;
}

// Bonjour will not allow conflicting service instance names (in the same domain),
// and may have automatically renamed the service if there was a conflict.  We pass
// the name back to the delegate so that the name can be displayed to the user.
// See http://developer.apple.com/networking/bonjour/faq.html for more information.

- (void)netServiceDidPublish:(NSNetService *)sender
{
	server->Delegate().DidEnableBonjour( server, sender.name );
}

- (void)netService:(NSNetService *)sender didNotPublish:(NSDictionary *)errorDict
{
	[self netServiceDidPublish:sender];
	server->Delegate().DidNotEnableBonjour( server, errorDict );
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacServer::MacServer( MServerDelegate& delegate )
:	fDelegate( delegate ),
	fNetServiceDelegate( [[NetServiceDelegate alloc] initWithServer:this] )
{
}

MacServer::~MacServer()
{
	Stop();
	[fNetServiceDelegate release];
}

// This function is called by CFSocket when a new connection comes in.
// We gather some data here, and convert the function call to a method
// invocation on MacServer.
void
MacServer::AcceptCallBack(
	CFSocketRef socket,
	CFSocketCallBackType type,
	CFDataRef address,
	const void *data,
	void *info )
{
	MacServer *server = (MacServer *)info;
	if ( kCFSocketAcceptCallBack == type )
	{
		// for an AcceptCallBack, the data parameter is a pointer to a CFSocketNativeHandle
		CFSocketNativeHandle nativeSocketHandle = *(CFSocketNativeHandle *)data;
		uint8_t name[SOCK_MAXADDRLEN];
		socklen_t namelen = sizeof(name);
		NSData *peer = nil;
		if ( 0 == getpeername(nativeSocketHandle, (struct sockaddr *)name, &namelen) )
		{
			peer = [NSData dataWithBytes:name length:namelen];
		}
		CFReadStreamRef readStream = NULL;
		CFWriteStreamRef writeStream = NULL;
		CFStreamCreatePairWithSocket( kCFAllocatorDefault, nativeSocketHandle, &readStream, &writeStream );
		if ( readStream && writeStream )
		{
			CFReadStreamSetProperty(readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
			CFWriteStreamSetProperty(writeStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
			server->HandleNewConnection( peer, readStream, writeStream );
		}
		else
		{
			// on any failure, need to destroy the CFSocketNativeHandle 
			// since we are not going to use it any more
			close(nativeSocketHandle);
		}
		if (readStream) { CFRelease(readStream); }
		if (writeStream) { CFRelease(writeStream); }
	}
}

bool
MacServer::Start( NSError** error )
{
	bool result = true;
	ErrorCode errorCode = kNoErr;

	CFSocketContext socketCtxt = {0, this, NULL, NULL, NULL};
	fIpv4Socket = CFSocketCreate(
		kCFAllocatorDefault,
		PF_INET, SOCK_STREAM, IPPROTO_TCP, kCFSocketAcceptCallBack,
		(CFSocketCallBack) & AcceptCallBack,
		& socketCtxt );

	if ( NULL == fIpv4Socket )
	{
		errorCode = kNoSocketsAvailable;
		result = NO;
	}
	else
	{
		// int yes = 1;
		// setsockopt( CFSocketGetNative(_ipv4socket), SOL_SOCKET, SO_REUSEADDR, (void *)&yes, sizeof(yes) );
		setsockopt( CFSocketGetNative(fIpv4Socket), SOL_SOCKET, SO_REUSEADDR, NULL, 0 );

		// set up the IPv4 endpoint; use port 0, so the kernel will choose 
		// an arbitrary port for us, which will be advertised using Bonjour
		struct sockaddr_in addr4;
		memset( &addr4, 0, sizeof(addr4) );
		addr4.sin_len = sizeof(addr4);
		addr4.sin_family = AF_INET;
		addr4.sin_port = 0;
		addr4.sin_addr.s_addr = htonl(INADDR_ANY);
		NSData *address4 = [NSData dataWithBytes:&addr4 length:sizeof(addr4)];

		if ( kCFSocketSuccess != CFSocketSetAddress(fIpv4Socket, (CFDataRef)address4) )
		{
			errorCode = kCouldNotBindToIPv4Address;
			result = NO;
		}
		else
		{
			// now that the binding was successful, we get the port number 
			// -- we will need it for the NSNetService
			NSData *addr = [(NSData *)CFSocketCopyAddress(fIpv4Socket) autorelease];
			memcpy(&addr4, [addr bytes], [addr length]);
			fPort = ntohs(addr4.sin_port);

			// set up the run loop sources for the sockets
			CFRunLoopRef cfrl = CFRunLoopGetCurrent();
			CFRunLoopSourceRef source4 = CFSocketCreateRunLoopSource(kCFAllocatorDefault, fIpv4Socket, 0);
			CFRunLoopAddSource(cfrl, source4, kCFRunLoopCommonModes);
			CFRelease(source4);
		}
	}

	if ( ! result )
	{
		if (error)
		{
			*error = [[NSError alloc] initWithDomain:@"MacServerErrorDomain" code:errorCode userInfo:nil];
		}
		if (fIpv4Socket) { CFRelease(fIpv4Socket); }
		fIpv4Socket = NULL;
	}
	return result;
}

bool
MacServer::Stop()
{
	DisableBonjour();

	if ( fIpv4Socket )
	{
		CFSocketInvalidate( fIpv4Socket );
		CFRelease( fIpv4Socket );
		fIpv4Socket = NULL;
	}

	return true;
}

bool
MacServer::EnableBonjour( NSString* domain, NSString* protocol, NSString* name )
{
	bool result = false;

	if (![domain length])
	{
		domain = @""; // use default Bonjour registration domains, typically just ".local"
	}

	if (![name length])
	{
		name = @""; // use default Bonjour name, e.g. the name assigned to the device in iTunes
	}

	if (!protocol || ![protocol length] || fIpv4Socket == NULL)
	{
	}
	else
	{
		NSNetService* netService =
			[[NSNetService alloc] initWithDomain:domain type:protocol name:name port:fPort];

		if ( netService )
		{
			fNetService = netService;
			[netService scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
			[netService publish];
			[netService setDelegate:fNetServiceDelegate];
			result = true;
		}
	}

	return result;
}

void
MacServer::DisableBonjour()
{
	NSNetService* netService = fNetService;
	if ( netService )
	{
		[netService stop];
		[netService removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
		fNetService = nil;
	}
}

void
MacServer::HandleNewConnection( NSData* address, CFReadStreamRef istr, CFWriteStreamRef ostr )
{
	fDelegate.DidAcceptConnection( this, (NSInputStream*)istr, (NSOutputStream*)ostr );
}

NSString*
MacServer::BonjourType( NSString* identifier )
{
	return ![identifier length] ? nil : [NSString stringWithFormat:@"_%@._tcp.", identifier];
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------



#if 0

static NSString * const MacServerErrorDomain = @"MacServerErrorDomain";

@interface MacServer ()
@property(nonatomic,retain) NSNetService* netService;
@property(assign) uint16_t port;
@end

@implementation MacServer

@synthesize delegate=_delegate, netService=_netService, port=_port;

/*
- (id)init
{
    return self;
}
*/

- (void)dealloc
{
	[self stop];
	[super dealloc];
}

- (void)handleNewConnectionFromAddress:(NSData *)addr
			inputStream:(NSInputStream *)istr
			outputStream:(NSOutputStream *)ostr
{
	// if the delegate implements the delegate method, call it  
	id delegate = self.delegate;

	if ( delegate
		 && [delegate respondsToSelector:@selector(didAcceptConnectionForServer:inputStream:outputStream:)] )
	{ 
		[delegate didAcceptConnectionForServer:self inputStream:istr outputStream:ostr];
	}
}

// This function is called by CFSocket when a new connection comes in.
// We gather some data here, and convert the function call to a method
// invocation on MacServer.
static void
MacServerAcceptCallBack(
	CFSocketRef socket,
	CFSocketCallBackType type,
	CFDataRef address,
	const void *data,
	void *info )
{
	MacServer *server = (MacServer *)info;
	if ( kCFSocketAcceptCallBack == type )
	{
		// for an AcceptCallBack, the data parameter is a pointer to a CFSocketNativeHandle
		CFSocketNativeHandle nativeSocketHandle = *(CFSocketNativeHandle *)data;
		uint8_t name[SOCK_MAXADDRLEN];
		socklen_t namelen = sizeof(name);
		NSData *peer = nil;
		if ( 0 == getpeername(nativeSocketHandle, (struct sockaddr *)name, &namelen) )
		{
			peer = [NSData dataWithBytes:name length:namelen];
		}
		CFReadStreamRef readStream = NULL;
		CFWriteStreamRef writeStream = NULL;
		CFStreamCreatePairWithSocket( kCFAllocatorDefault, nativeSocketHandle, &readStream, &writeStream );
		if ( readStream && writeStream )
		{
			CFReadStreamSetProperty(readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
			CFWriteStreamSetProperty(writeStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
			[server handleNewConnectionFromAddress:peer
					inputStream:(NSInputStream *)readStream
					outputStream:(NSOutputStream *)writeStream];
		}
		else
		{
			// on any failure, need to destroy the CFSocketNativeHandle 
			// since we are not going to use it any more
			close(nativeSocketHandle);
		}
		if (readStream) { CFRelease(readStream); }
		if (writeStream) { CFRelease(writeStream); }
	}
}

- (BOOL)start:(NSError **)error
{
	BOOL result = YES;
	MacServerErrorCode errorCode = kMacServerNoErr;

	CFSocketContext socketCtxt = {0, self, NULL, NULL, NULL};
	_ipv4socket = CFSocketCreate(
		kCFAllocatorDefault,
		PF_INET, SOCK_STREAM, IPPROTO_TCP, kCFSocketAcceptCallBack,
		(CFSocketCallBack) & MacServerAcceptCallBack,
		& socketCtxt );

	if ( NULL == _ipv4socket )
	{
		errorCode = kMacServerNoSocketsAvailable;
		result = NO;
		goto exit_gracefully;
	}

	// int yes = 1;
	// setsockopt( CFSocketGetNative(_ipv4socket), SOL_SOCKET, SO_REUSEADDR, (void *)&yes, sizeof(yes) );
	setsockopt( CFSocketGetNative(_ipv4socket), SOL_SOCKET, SO_REUSEADDR, NULL, 0 );

	// set up the IPv4 endpoint; use port 0, so the kernel will choose 
	// an arbitrary port for us, which will be advertised using Bonjour
	struct sockaddr_in addr4;
	memset( &addr4, 0, sizeof(addr4) );
	addr4.sin_len = sizeof(addr4);
	addr4.sin_family = AF_INET;
	addr4.sin_port = 0;
	addr4.sin_addr.s_addr = htonl(INADDR_ANY);
	NSData *address4 = [NSData dataWithBytes:&addr4 length:sizeof(addr4)];

	if ( kCFSocketSuccess != CFSocketSetAddress(_ipv4socket, (CFDataRef)address4) )
	{
		errorCode = kMacServerCouldNotBindToIPv4Address;
		result = NO;
		goto exit_gracefully;
	}

	// now that the binding was successful, we get the port number 
	// -- we will need it for the NSNetService
	NSData *addr = [(NSData *)CFSocketCopyAddress(_ipv4socket) autorelease];
	memcpy(&addr4, [addr bytes], [addr length]);
	self.port = ntohs(addr4.sin_port);

	// set up the run loop sources for the sockets
	CFRunLoopRef cfrl = CFRunLoopGetCurrent();
	CFRunLoopSourceRef source4 = CFSocketCreateRunLoopSource(kCFAllocatorDefault, _ipv4socket, 0);
	CFRunLoopAddSource(cfrl, source4, kCFRunLoopCommonModes);
	CFRelease(source4);

exit_gracefully:
	if ( ! result )
	{
		if (error)
		{
			*error = [[NSError alloc] initWithDomain:MacServerErrorDomain code:errorCode userInfo:nil];
		}
		if (_ipv4socket) { CFRelease(_ipv4socket); }
		_ipv4socket = NULL;
	}
	return result;
}

- (BOOL)stop
{
	[self disableBonjour];

	if (_ipv4socket) {
		CFSocketInvalidate(_ipv4socket);
		CFRelease(_ipv4socket);
		_ipv4socket = NULL;
	}

	return YES;
}

- (BOOL) enableBonjourWithDomain:(NSString*)domain applicationProtocol:(NSString*)protocol name:(NSString*)name
{
	BOOL result = NO;

	if (![domain length])
	{
		domain = @""; // use default Bonjour registration domains, typically just ".local"
	}

	if (![name length])
	{
		name = @""; // use default Bonjour name, e.g. the name assigned to the device in iTunes
	}

	if (!protocol || ![protocol length] || _ipv4socket == NULL)
	{
	}
	else
	{
		NSNetService* netService =
			[[NSNetService alloc] initWithDomain:domain type:protocol name:name port:self.port];

		if ( netService )
		{
			self.netService = netService;
			[netService scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
			[netService publish];
			[netService setDelegate:self];
			result = YES;
		}
	}

	return result;
}

// Bonjour will not allow conflicting service instance names (in the same domain),
// and may have automatically renamed the service if there was a conflict.  We pass
// the name back to the delegate so that the name can be displayed to the user.
// See http://developer.apple.com/networking/bonjour/faq.html for more information.

- (void)netServiceDidPublish:(NSNetService *)sender
{
	id delegate = self.delegate;
	if ( delegate && [delegate respondsToSelector:@selector(serverDidEnableBonjour:withName:)] )
	{
		[delegate serverDidEnableBonjour:self withName:sender.name];
	}
}

- (void)netService:(NSNetService *)sender didNotPublish:(NSDictionary *)errorDict
{
	[super netServiceDidPublish:sender];
	id delegate = self.delegate;
	if ( delegate && [self.delegate respondsToSelector:@selector(server:didNotEnableBonjour:)] )
	{
		[delegate server:self didNotEnableBonjour:errorDict];
	}
}

- (void) disableBonjour
{
	NSNetService* netService = self.netService;
	if ( netService )
	{
		[netService stop];
		[netService removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
		self.netService = nil;
	}
}

- (NSString*) description
{
	return [NSString stringWithFormat:@"<%@ = 0x%08X | port %d | netService = %@>",
		[self class], (long)self, self.port, self.netService];
}

+ (NSString*) bonjourTypeFromIdentifier:(NSString*)identifier
{
	return ![identifier length] ? nil : [NSString stringWithFormat:@"_%@._tcp.", identifier];
}

@end

#endif // 0

