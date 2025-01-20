//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacServer_H__
#define _Rtt_MacServer_H__

#include <CoreFoundation/CFStream.h>

// ----------------------------------------------------------------------------

@class NSData;
@class NSDictionary;
@class NSError;
@class NSInputStream;
@class NSNetService;
@class NSOutputStream;
@class NSString;

namespace Rtt
{

class MacServer;

// ----------------------------------------------------------------------------

class MServerDelegate
{
	public:
		virtual void DidEnableBonjour( MacServer* server, NSString* name ) const = 0;
		virtual void DidNotEnableBonjour( MacServer* server, NSDictionary* errorDict ) const = 0;
		virtual void DidAcceptConnection( MacServer* server, NSInputStream* istr, NSOutputStream* ostr ) = 0;
};

class MacServer
{
	public:
		typedef enum _ErrorCode
		{
			kNoErr = 0,
			kCouldNotBindToIPv4Address,
			kCouldNotBindToIPv6Address,
			kNoSocketsAvailable
		}
		ErrorCode;

	public:
		MacServer( MServerDelegate& delegate );
		~MacServer();

	public:
		MServerDelegate& Delegate() { return fDelegate; }
		const MServerDelegate& Delegate() const { return fDelegate; }

	protected:
		static void AcceptCallBack(
						CFSocketRef socket,
						CFSocketCallBackType type,
						CFDataRef address,
						const void *data,
						void *info );
	public:
		bool Start( NSError** error );
		bool Stop();

		// Pass "nil" for the default local domain
		// Pass only the application protocol for "protocol" e.g. "myApp"
		bool EnableBonjour( NSString* domain, NSString* protocol, NSString* name );
		void DisableBonjour();

	protected:
		void HandleNewConnection( NSData* address, CFReadStreamRef istr, CFWriteStreamRef ostr );

	public:
		static NSString* BonjourType( NSString* identifier );

	private:
		MServerDelegate& fDelegate;
		uint16_t fPort;
		CFSocketRef fIpv4Socket;
		NSNetService* fNetService;
		id fNetServiceDelegate;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#if 0

// TODO: Convert this to a C++ class.  There's no reason it should be Obj-C
#import <Foundation/Foundation.h>

@class MacServer;

typedef enum _MacServerErrorCode
{
	kMacServerNoErr = 0,
    kMacServerCouldNotBindToIPv4Address,
    kMacServerCouldNotBindToIPv6Address,
    kMacServerNoSocketsAvailable
}
MacServerErrorCode;

@protocol MacServerDelegate< NSObject >
@optional
- (void) serverDidEnableBonjour:(MacServer*)server withName:(NSString*)name;
- (void) server:(MacServer*)server didNotEnableBonjour:(NSDictionary *)errorDict;
- (void) didAcceptConnectionForServer:(MacServer*)server inputStream:(NSInputStream *)istr outputStream:(NSOutputStream *)ostr;
@end


@interface MacServer : NSObject
{
	@private
		id _delegate;
		uint16_t _port;
		CFSocketRef _ipv4socket;
		NSNetService* _netService;
}
	
- (BOOL)start:(NSError **)error;
- (BOOL)stop;
- (BOOL)enableBonjourWithDomain:(NSString*)domain applicationProtocol:(NSString*)protocol name:(NSString*)name; //Pass "nil" for the default local domain - Pass only the application protocol for "protocol" e.g. "myApp"
- (void)disableBonjour;

@property(assign) id<MacServerDelegate> delegate;

+ (NSString*) bonjourTypeFromIdentifier:(NSString*)identifier;

@end
#endif


#endif // _Rtt_MacServer_H__
