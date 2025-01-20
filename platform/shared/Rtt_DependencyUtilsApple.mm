//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <CoreFoundation/CFData.h>
#include <Security/Secbase.h>
#include <dlfcn.h>
#include <CommonCrypto/CommonCrypto.h>
#import "NSData+Base64.h"
#include "Rtt_DependencyUtils.h"

// ----------------------------------------------------------------------------

namespace Corona
{

// ----------------------------------------------------------------------------

static SecKeyRef LoadKey( const char *pubKey )
{
	SecKeyRef result = NULL;
 
	//NSData *certData = [NSData dataWithContentsOfFile:(NSString *)path];
	
	NSString *pKey = [NSString stringWithUTF8String:pubKey];
	
	NSData *certData = [NSData Rtt_dataFromBase64String:pKey];

	if ( certData )
	{
		SecCertificateRef cert = SecCertificateCreateWithData(NULL, (CFDataRef)certData);
		SecTrustRef trust = NULL;
		SecPolicyRef policy = NULL;

		if (cert != NULL)
		{
			policy = SecPolicyCreateBasicX509();

			if (policy)
			{
				if (SecTrustCreateWithCertificates((CFTypeRef)cert, policy, &trust) == noErr)
				{
					SecTrustResultType resultType;
					if (SecTrustEvaluate(trust, &resultType) == noErr)
					{
						result = SecTrustCopyPublicKey(trust);
					}
				}
			}
		}

		if (policy) CFRelease(policy);
		if (trust) CFRelease(trust);
		if (cert) CFRelease(cert);
	}

	return result;
}

static bool Verify(const char *data, int dataLen, CFDataRef dataSig, const char *pKey)
{

	unsigned char hash[CC_SHA1_DIGEST_LENGTH + 1];
	
	CC_SHA1(data, dataLen, hash);
	
	//CFMutableStringRef key = CFStringCreateMutable(NULL, 0);
	//NSBundle *bundle = [NSBundle mainBundle];
	//key = (CFMutableStringRef)[bundle pathForResource:[NSString stringWithUTF8String:keyPath] ofType:@"der"];
	
	SecKeyRef publicKeyRef = LoadKey( pKey );

	bool result  = false;

	if ( publicKeyRef )
	{
		OSStatus status = SecKeyRawVerify(
									  publicKeyRef,
									  kSecPaddingPKCS1SHA1,
									  hash,
									  CC_SHA1_DIGEST_LENGTH,
									  CFDataGetBytePtr( dataSig ),
									 (size_t)CFDataGetLength( dataSig ));
				
		//0 means no errors, 1 means errors
		result = (0 == status);
	}

	return result;
}
				
int
DependencyUtils::DecodeBase64( lua_State *L )
{
	const char *payloadData = lua_tostring( L, 1 );

	NSString *payloadBase64 = [NSString stringWithUTF8String:payloadData];
	NSData *data = [NSData Rtt_dataFromBase64String:(NSString *)payloadBase64];
	
	lua_pushlstring(L, (const char *)[data bytes], [data length]);
	
	return 1;
}

int
DependencyUtils::Check( lua_State *L )
{
	const char *publicKey = lua_tostring( L, lua_upvalueindex( 1 ) );

	//int sigLength = lua_objlen(L,1);
	const char *signature = lua_tostring( L, 1 );
	
	int dataLength = (int) lua_objlen(L,2);
	const char *payloadData = lua_tostring( L, 2 );
		
	NSString *signatureBase64 = [NSString stringWithUTF8String:signature];
	CFDataRef dataSig = (CFDataRef)[NSData Rtt_dataFromBase64String:(NSString *)signatureBase64];
	
	bool result =  Verify(payloadData, dataLength, dataSig, publicKey);
	
	lua_pushboolean(L, result);
	
	return 1;
}

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

