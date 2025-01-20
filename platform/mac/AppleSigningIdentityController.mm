//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "AppleSigningIdentityController.h"

#include "Rtt_TargetDevice.h"

#include <CommonCrypto/CommonDigest.h>
#import <CoreFoundation/CFData.h>

// Should be "const" but one of the Cocoa APIs expects non-const
NSString *kExpiredProfile = @"Expired";

#define LOG_CSSM_DATA 1

// -----------------------------------------------------------------------------

@implementation IdentityMenuItem

@synthesize identity;
@synthesize provisionPath;
@synthesize fingerprint;

-(id)initWithIdentity:(NSString*)inIdentity provisionPath:(NSString*)path provisionName:(NSString *)provisionName expirationDate:(NSString *)expirationDate fingerprint:(NSString *)certFingerprint
{
    // If no provisionName could be extracted, then append filename
	NSString *provisionLabel = provisionName ? provisionName : [[path lastPathComponent] stringByDeletingPathExtension];
	/*
	 NSString* title = [NSString stringWithFormat:@"%@ %@[%@]",
					   inIdentity,
					   (expirationDate != nil ? [kExpiredProfile stringByAppendingString:@" "] : @""),
					   provisionLabel];
	 */
	NSString* title = [NSString stringWithFormat:@"%@ %@[%@]",
					   provisionLabel,
					   (expirationDate != nil ? [kExpiredProfile stringByAppendingString:@" "] : @""),
					   inIdentity];

	// Rtt_TRACE(( "initWithIdentity: %s / %s / %s", [inIdentity UTF8String], [provisionName UTF8String], [certFingerprint UTF8String] ));

	self = [super initWithTitle:title action:NULL keyEquivalent:@""];

	if ( self )
	{
		identity = [inIdentity retain];
		provisionPath = [path retain];
        fingerprint = [certFingerprint retain];
	}

	return self;
}

-(void)dealloc
{
	[provisionPath release];
	[identity release];
	[super dealloc];
}

@end

// Some of the Security framework stuff is deprecated in OSX 10.7 and above
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static NSString *kNoCertificateFound = @"No certificate found";

// ----------------------------------------------------------------------------

#ifdef EXTRA_CERTIFICATE_DETAIL
// Experimental code to get more info from a certificate
static NSString * stringFromDNwithSubjectName(CFArrayRef array)
{
	NSMutableString * out = [[NSMutableString alloc] init];
	const void *keys[] = { kSecOIDCommonName, kSecOIDEmailAddress, kSecOIDOrganizationalUnitName, kSecOIDOrganizationName, kSecOIDLocalityName, kSecOIDStateProvinceName, kSecOIDCountryName };
	const void *labels[] = { "CN", "E", "OU", "O", "L", "S", "C", "E" };

	for(unsigned int i = 0; i < sizeof(keys)/sizeof(keys[0]);  i++)
	{
		for (CFIndex n = 0 ; n < CFArrayGetCount(array); n++)
		{
			CFDictionaryRef dict = (CFDictionaryRef) CFArrayGetValueAtIndex(array, n);
			if (CFGetTypeID(dict) != CFDictionaryGetTypeID())
				continue;
			CFTypeRef dictkey = CFDictionaryGetValue(dict, kSecPropertyKeyLabel);
			if (!CFEqual(dictkey, keys[i]))
				continue;
			CFStringRef str = (CFStringRef) CFDictionaryGetValue(dict, kSecPropertyKeyValue);
			[out appendFormat:@"%s=%@ ", labels[i], (__bridge NSString*)str];
		}
	}
	return [NSString stringWithString:out];
}

static NSString* stringFromCertificateWithLongDescription(SecCertificateRef certificateRef)
{
	if (certificateRef == NULL)
		return @"";

	CFStringRef commonNameRef;
	OSStatus status;
	if ((status=SecCertificateCopyCommonName(certificateRef, &commonNameRef)) != errSecSuccess)
	{
		NSLog(@"Could not extract name from cert: %@",
			  SecCopyErrorMessageString(status, NULL));
		return @"Unreadable cert";
	};

	CFStringRef summaryRef = SecCertificateCopySubjectSummary(certificateRef);
	if (summaryRef == NULL)
		summaryRef = (CFStringRef) CFRetain(commonNameRef);

	CFErrorRef error;

	const void *keys[] = { kSecOIDX509V1SubjectName, kSecOIDX509V1IssuerName };
	const void *labels[] = { "Subject", "Issuer" };
	CFArrayRef keySelection = CFArrayCreate(NULL, keys , sizeof(keys)/sizeof(keys[0]), &kCFTypeArrayCallBacks);

	CFDictionaryRef vals = SecCertificateCopyValues(certificateRef, keySelection, &error);
	NSMutableString *longDesc = [[NSMutableString alloc] init];

	for(unsigned int i = 0; i < sizeof(keys)/sizeof(keys[0]); i++)
	{
		CFDictionaryRef dict = (CFDictionaryRef) CFDictionaryGetValue(vals, keys[i]);
		CFArrayRef values = (CFArrayRef) CFDictionaryGetValue(dict, kSecPropertyKeyValue);
		if (values == NULL)
			continue;
		[longDesc appendFormat:@"%s:%@\n\n", labels[i], stringFromDNwithSubjectName(values)];
	}

	CFRelease(vals);
	CFRelease(summaryRef);
	CFRelease(commonNameRef);

	return longDesc;
}
#endif // EXTRA_CERTIFICATE_DETAIL

//
// IsCertCurrent
//
// Determine if the given certificate is usable based on its validity dates
//
static BOOL
IsCertCurrent(SecCertificateRef certificateRef, NSString *certCommonName)
{
	CFErrorRef error;

	const void *dateKeys[] = { kSecOIDX509V1ValidityNotBefore, kSecOIDX509V1ValidityNotAfter };
	CFAbsoluteTime validityDates[sizeof(dateKeys)/sizeof(dateKeys[0])];
	CFArrayRef dateKeySelection = CFArrayCreate(NULL, dateKeys, sizeof(dateKeys)/sizeof(dateKeys[0]), &kCFTypeArrayCallBacks);

	CFDictionaryRef dateVals = SecCertificateCopyValues(certificateRef, dateKeySelection, &error);

	for (unsigned int i = 0; i < sizeof(dateKeys)/sizeof(dateKeys[0]); i++)
	{
		CFNumberRef validityRef = (CFNumberRef)CFDictionaryGetValue((CFDictionaryRef)CFDictionaryGetValue(dateVals, dateKeys[i]), kSecPropertyKeyValue);
		CFAbsoluteTime validity;
		CFNumberGetValue(validityRef, kCFNumberDoubleType, &validity);

		validityDates[i] = validity;
	}

	CFRelease(dateVals);

	CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
	BOOL certValid = YES;
	NSDateFormatter *dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
	NSLocale *enUSPOSIXLocale = [NSLocale localeWithLocaleIdentifier:@"en_US_POSIX"];

	[dateFormatter setLocale:enUSPOSIXLocale];
	[dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ssZZZZZ"]; // ISO 8601

	if (now < validityDates[0])
	{
		NSDate *certDate = [NSDate dateWithTimeIntervalSinceReferenceDate:validityDates[1]];
		Rtt_Log("ERROR: certificate: '%s' not valid until %s",
				[certCommonName UTF8String], [[dateFormatter stringFromDate:certDate] UTF8String]);

		certValid = NO;
	}
	else if (now >= validityDates[1])
	{
		NSDate *certDate = [NSDate dateWithTimeIntervalSinceReferenceDate:validityDates[1]];
		Rtt_Log("ERROR: certificate: '%s' expired on %s",
				[certCommonName UTF8String], [[dateFormatter stringFromDate:certDate] UTF8String]);

		certValid = NO;
	}
	else if ((now + (7 * 24 * 60 * 60)) > validityDates[1])
	{
		NSDate *certDate = [NSDate dateWithTimeIntervalSinceReferenceDate:validityDates[1]];
		Rtt_Log("WARNING: certificate: '%s' will expire in less than a week on %s",
				[certCommonName UTF8String], [[dateFormatter stringFromDate:certDate] UTF8String]);
	}

	return certValid;
}

static NSString *
GetCertificateFingerprint( SecCertificateRef cert )
{
    static unsigned char sha1[CC_SHA1_DIGEST_LENGTH];
    memset(sha1, 0, sizeof(sha1));

    CSSM_DATA cert_data = { 0, NULL };
    OSStatus status = SecCertificateGetData(cert, &cert_data);
    if (status)
    {
        return @"";
    }

    Rtt_ASSERT(NULL != cert_data.Data);
    Rtt_ASSERT(0 != cert_data.Length);

    CC_SHA1(cert_data.Data, (CC_LONG) cert_data.Length, sha1);

    NSString *hexSha1 = @"";
    for (size_t i = 0; i < CC_SHA1_DIGEST_LENGTH; i++)
    {
        hexSha1 = [hexSha1 stringByAppendingFormat:@"%02X", sha1[i]];
    }

#ifdef xDEBUG
    CFStringRef name = NULL;
    NSString* certIdentity = nil;

    if ( ( noErr == SecCertificateCopyCommonName( cert, & name ) ) && name )
    {
        certIdentity = (NSString*)name;
    }
    Rtt_TRACE(("GetCertificateFingerprint: %s \"%s\"\n", [hexSha1 UTF8String], [certIdentity UTF8String]));
    CFRelease( name );

#ifdef EXTRA_CERTIFICATE_DETAIL
	Rtt_TRACE(("GetCertificateFingerprint: %s", [stringFromCertificateWithLongDescription(cert) UTF8String] ));
#endif // EXTRA_CERTIFICATE_DETAIL
#endif // DEBUG
    
    return hexSha1;
}

static bool
DoesCertificateHavePrivateKey( SecCertificateRef certificate )
{
	SecIdentityRef identity = NULL;

	OSStatus err = SecIdentityCreateWithCertificate( NULL, certificate, & identity );
	bool result = ( noErr == err && identity );

	if ( result )
    {
        // Rtt_TRACE(("DoesCertificateHavePrivateKey: %s (%s)\n", (result ? "YES" : "NO"), [GetCertificateFingerprint( certificate ) UTF8String]));

        CFRelease( identity );
    }

	return result;
}

static bool
DoesCertificateMatchProvisionId( SecCertificateRef certificate, NSString* fingerprint )
{
	bool result = false;

    NSString *certFingerprint = GetCertificateFingerprint( certificate );

    result = [certFingerprint isEqualToString:fingerprint];

    // Rtt_TRACE(("DoesCertificateMatchProvisionId: %s (%s == %s)\n", (result ? "YES" : "NO"), [fingerprint UTF8String], [certFingerprint UTF8String]));

	return result;
}

typedef bool (*CertificateMatchesCallback)( SecCertificateRef certificate, NSString *fingerprint );

// Caller must release
static SecCertificateRef
FindCertificate( NSString *identity, CertificateMatchesCallback callback, NSString *fingerprint )
{
	SecCertificateRef result = NULL;

	SecKeychainAttributeList list = { 0, NULL };
	SecKeychainSearchRef search;
	if ( Rtt_VERIFY( noErr == SecKeychainSearchCreateFromAttributes( NULL, kSecCertificateItemClass, & list, & search ) ) )
	{
		SecKeychainItemRef found = NULL;
		for ( OSStatus err = SecKeychainSearchCopyNext( search, & found );
			  ! result && errSecItemNotFound != err && Rtt_VERIFY( noErr == err ) && found;
			  err = SecKeychainSearchCopyNext( search, & found ) )
		{
			SecCertificateRef certificate = (SecCertificateRef)found;
			CFStringRef name = NULL;

            if ( ( noErr == SecCertificateCopyCommonName( certificate, & name ) ) && name )
			{
				NSString* certIdentity = (NSString*)name;
				if ( [identity isEqualToString:certIdentity]
					 && ( ! callback || (*callback)( certificate, fingerprint ) ) )
				{
					result = certificate;
					CFRetain( certificate );
				}
				CFRelease( name );
			}
		}

		CFRelease( search );
	}

	return result;
}

static bool
CertificateMatches( SecCertificateRef certificate, NSString *fingerprint )
{
	return DoesCertificateMatchProvisionId( certificate, fingerprint )
			&& DoesCertificateHavePrivateKey( certificate );
}

static bool
CertificateExists( NSString *commonName, NSString *fingerprint )
{
	bool result = false;

	// Search keychain for a certificate with a matching fingerprint
	SecCertificateRef certificate = FindCertificate( commonName, &CertificateMatches, fingerprint );

	if ( NULL != certificate )
	{
		// Found a cert, check its validity dates
        result = IsCertCurrent(certificate, commonName);
		CFRelease( certificate );
	}

	return result;
}

// End of the deprecated Security framework stuff
#pragma GCC diagnostic pop

static NSString*
FindSigningIdentity( NSDictionary *provisionDict, NSString **certFingerprint, NSMutableArray *certIdentities )
{
    NSString *result = nil;

    [certIdentities removeAllObjects];

    for ( NSData *developerCertData in [provisionDict valueForKey:@"DeveloperCertificates"] )
    {
        // The Base64 decoding has already been done for us
        CFDataRef certData = CFDataCreate( NULL, (const UInt8 *) [developerCertData bytes], [developerCertData length] );

        SecCertificateRef certRef = SecCertificateCreateWithData(NULL, certData );

        *certFingerprint = GetCertificateFingerprint( certRef );

        if (certRef != NULL )
        {
            CFStringRef commonName = NULL;

            if ( ( noErr == SecCertificateCopyCommonName( certRef, &commonName ) ) && commonName != NULL )
            {
                NSString *commonNameStr = (NSString *) commonName;

                // Rtt_TRACE(( "[commonName] = %s - %s\n", [commonNameStr UTF8String], [*certFingerprint UTF8String]));

                if (CertificateExists( commonNameStr, *certFingerprint ))
                {
                    result = commonNameStr;

                    // One valid cert is enough
                    break;
                }
                else if (! [certIdentities containsObject:commonNameStr])
                {
                    // Record the public certs for which we don't have a private cert
                    if (commonNameStr != nil)
                    {
                        [certIdentities addObject:[NSString stringWithFormat:@"%@ [%@]", commonNameStr, *certFingerprint]];
                    }
                }
            }
            
            if (certRef != NULL)
            {
                CFRelease( certRef );
            }
        }
    }

	return result;
}

// ----------------------------------------------------------------------------

@implementation AppleSigningIdentityController

+ (NSString*)defaultProvisionPath
{
	NSArray* paths = NSSearchPathForDirectoriesInDomains( NSLibraryDirectory, NSUserDomainMask, true );
	NSString *provisionDir = [paths objectAtIndex:0];
	provisionDir = [provisionDir stringByAppendingPathComponent:@"MobileDevice/Provisioning Profiles/"];
	return provisionDir;
}

+ (NSString*)newProvisionPath
{
	NSArray* paths = NSSearchPathForDirectoriesInDomains( NSLibraryDirectory, NSUserDomainMask, true );
	NSString *provisionDir = [paths objectAtIndex:0];
    provisionDir = [provisionDir stringByAppendingPathComponent:@"Developer/Xcode/UserData/Provisioning Profiles/"];
	return provisionDir;
}




+ (BOOL)hasProvisionedDevices:(NSString*)provisionFile
{
    NSDictionary *provisionProfile = [AppleSigningIdentityController loadProvisioningProfile:provisionFile];

    return ([provisionProfile valueForKey:@"ProvisionedDevices"] != nil);
}

// Return a dictionary which maps signing identities to certificate fingerprints for the current user
+ (NSDictionary *) getSigningIdentities
{
	NSMutableDictionary *resultDict = [[NSMutableDictionary alloc] initWithCapacity:10];

	NSMutableDictionary *query = [NSMutableDictionary dictionaryWithObjectsAndKeys:
								  (__bridge id)kCFBooleanTrue, (__bridge id)kSecReturnRef,
								  (__bridge id)kSecMatchLimitAll, (__bridge id)kSecMatchLimit,
								  nil];

	NSArray *secItemClasses = [NSArray arrayWithObjects:
							   (__bridge id)kSecClassIdentity,
							   nil];

	for (id secItemClass in secItemClasses)
	{
		[query setObject:secItemClass forKey:(__bridge id)kSecClass];

		CFTypeRef queryResult = NULL;

		SecItemCopyMatching((__bridge CFDictionaryRef)query, &queryResult);
		//NSLog(@"%@", (__bridge id)result);

		NSDictionary *queryResultDict = (__bridge id)queryResult;

		for (id dict in queryResultDict)
		{
			SecCertificateRef certificateRef;

			OSStatus osStatus = SecIdentityCopyCertificate(
				(SecIdentityRef) dict,
				&certificateRef);

			if (osStatus == errSecSuccess)
			{
				CFStringRef name = NULL;
				NSString* certName = nil;

				if ( ( noErr == SecCertificateCopyCommonName( certificateRef, &name ) ) && name )
				{
					certName = (NSString*)name;
				}

				NSString *certFingerprint = GetCertificateFingerprint(certificateRef);
				// NSLog(@"dict: (%@) %@ %@", certificateRef, certName, certFingerprint);

				[resultDict setObject:certFingerprint forKey:certName];

				CFRelease( name );
			}
		}

		if (queryResult != NULL)
		{
			CFRelease(queryResult);
		}
	}

	return resultDict;
}

// Return the fingerprint of a valid cert for this provisioning profile and put the common name in the supplied place
+ (NSString*)signingIdentity:(NSString*)provisionFile commonName:(NSString **)commonName
{
	NSString *result = nil;

	if ( [[NSFileManager defaultManager] fileExistsAtPath:provisionFile] )
	{
		NSDictionary *provisionProfile = [AppleSigningIdentityController loadProvisioningProfile:provisionFile];
		NSMutableArray *certIdentities = [NSMutableArray arrayWithCapacity:20];
		NSString *certFingerprint = nil;
		*commonName = FindSigningIdentity( provisionProfile, &certFingerprint, certIdentities );
		result = certFingerprint;
	}

	return result;
}

- (id)init
{
	return [self initWithProvisionPath:[[self class] defaultProvisionPath] newPath:[[self class] newProvisionPath]];
}

- (id)initWithProvisionPath:(NSString*)path newPath:(NSString *)newPath
{
	self = [super init];
	if ( self )
	{
		fPath = [path copy];
		fNewPath = [newPath copy]; // This new path is for Xcode 16 and later
	}
	
    
	return self;
}

- (void)dealloc
{
	[fPath release];
	[fNewPath release];
	[super dealloc];
}

static NSString *
ExtensionForPlatform( Rtt::TargetDevice::Platform platform )
{
	using namespace Rtt;
	switch ( platform )
	{
		case TargetDevice::kIPhonePlatform:
		case TargetDevice::kTVOSPlatform:
			return @"mobileprovision";
			break;
		default:
			return @"provisionprofile";
			break;
	}
}

+ (NSDictionary *)loadProvisioningProfile:(NSString *)path
{
    CMSDecoderRef decoder = NULL;
    CFDataRef dataRef = NULL;
    NSString *plistString = nil;
    NSDictionary *plist = nil;

    @try
    {
        CMSDecoderCreate(&decoder);
        NSData *fileData = [NSData dataWithContentsOfFile:path];
        CMSDecoderUpdateMessage(decoder, fileData.bytes, fileData.length);
        CMSDecoderFinalizeMessage(decoder);
        CMSDecoderCopyContent(decoder, &dataRef);
        plistString = [[NSString alloc] initWithData:(__bridge NSData *)dataRef encoding:NSUTF8StringEncoding];
        NSData *plistData = [plistString dataUsingEncoding:NSUTF8StringEncoding];
        plist = [NSPropertyListSerialization propertyListFromData:plistData mutabilityOption:NSPropertyListImmutable format:nil errorDescription:nil];
    }
    @catch (NSException *exception)
    {
        Rtt_TRACE(( "loadProvisioningProfile: Could not load provisioning profile: %s", [path UTF8String]));
    }
    @finally
    {
        if (decoder != NULL)
        {
            CFRelease(decoder);
        }
        if (dataRef != NULL)
        {
            CFRelease(dataRef);
        }
    }

	// Rtt_TRACE(( "loadProvisioningProfile: %s\n-----\n%s\n-----", [path UTF8String], [[plist description] UTF8String]));
    return plist;
}

- (NSData*) dataFrom64String : (NSString*) stringEncodedWithBase64
{
    NSData *dataFromBase64 = [[NSData alloc] initWithBase64EncodedString:stringEncodedWithBase64 options:NSDataBase64DecodingIgnoreUnknownCharacters];
    return dataFromBase64;
}

static bool ShouldUseProvisionProfileForPlatform(
	NSDictionary *provisionProfile, Rtt::TargetDevice::Platform platform )
{
	bool result = false;

	NSArray *provisionPlatform = [provisionProfile valueForKey:@"Platform"];
	
	switch ( platform )
	{
		case Rtt::TargetDevice::kTVOSPlatform:
			result = [provisionPlatform containsObject:@"tvOS"];
			break;
		case Rtt::TargetDevice::kIPhonePlatform:
			if ( nil == provisionPlatform )
			{
				NSString *provisionUUID = [provisionProfile valueForKey:@"UUID"];
				const char *name = [provisionUUID UTF8String];

				Rtt_Log( "Warning: Provisioning profile (%s) does not specify 'Platform'. Assuming this is an older profile, so defaulting to 'iOS'.\n", (name ? name : "null") );
				result = true;
			}
			else
			{
				result = [provisionPlatform containsObject:@"iOS"];
			}
			break;
		case Rtt::TargetDevice::kOSXPlatform:
			// this is determined by the filename extension used (see ExtensionForPlatform())
			result = true;
			break;
		default:
			// If you hit this case, you need to add a new case
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;
	}
	
	return result;
}

//
// Initialize signing identities menu used for builds
//
- (void)populateMenu:(NSMenu*)menu platform:(Rtt::TargetDevice::Platform)platform delegate:(id)delegate
{
	using namespace Rtt;

	NSString *provisionDir; // Temporary variable to hold the current directory path
    NSMutableArray *allDirContents = [NSMutableArray array]; // Array to hold contents from both paths
	NSMenu *identitiesMenu = menu;
	Rtt_ASSERT(menu);
	NSMenu *iOSTeamProvisioningProfilesSubMenu = [[NSMenu alloc] init];
	NSMenu *disabledSubMenu = [[NSMenu alloc] init];
	NSString *extension = ExtensionForPlatform(platform);
	NSFont *smallFont = [NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:NSMiniControlSize]];
	NSFont *boldFont = [NSFont boldSystemFontOfSize:[NSFont labelFontSize]];
	NSMutableArray *certIdentities = [NSMutableArray arrayWithCapacity:20];

    NSFileManager *fileMgr = [NSFileManager defaultManager];
	// Get contents from both paths
    for (NSString *path in @[fPath, fNewPath]) {
        if ([fileMgr fileExistsAtPath:path]) {
            NSArray *contents = [fileMgr contentsOfDirectoryAtPath:path error:NULL];
            NSString *source = (path == fPath) ? @"old" : @"new";
            for (NSString *item in contents) {
                [allDirContents addObject:@{@"name": item, @"source": source}];
            }
        } else {
            // Do nothing for right now
        }
    }
    
	// Process each file in the combined contents
    for (NSDictionary *fileInfo in allDirContents)
    {
        NSString *filename = fileInfo[@"name"];
        
        if ([[filename pathExtension] isEqualToString:extension])
        {
            // Determine the source of the file and assign the correct directory
            NSString *provisionDir = [fileInfo[@"source"] isEqualToString:@"old"] ? fPath : fNewPath;

            // You can now use provisionDir and filename to create the full path if needed
            NSString *fullPath = [provisionDir stringByAppendingPathComponent:filename];
			NSDictionary *provisionProfile = [AppleSigningIdentityController loadProvisioningProfile:fullPath];
        

			if ( provisionProfile != nil )
			{
				// This chooses between iOS and tvOS style profiles
				if ( ShouldUseProvisionProfileForPlatform( provisionProfile, platform ) )
				{
					NSString *certFingerprint = nil;
					NSString* identity = FindSigningIdentity( provisionProfile, &certFingerprint, certIdentities );
					bool isValidIdentity = ( nil != identity );
					NSString *provisionName = [provisionProfile valueForKey:@"Name"];
					NSString *appIDName = [provisionProfile valueForKey:@"AppIDName"];
					NSString *expirationDate = nil;
					NSDate *provisionExpiration = nil;

					// AppIDNames supplied by Xcode provide no additional info beyond the provisionName so omit them
					if (appIDName != nil && ! [appIDName hasPrefix:@"Xcode "])
					{
						provisionName = [provisionName stringByAppendingString:@" ("];
						provisionName = [provisionName stringByAppendingString:appIDName];
						provisionName = [provisionName stringByAppendingString:@")"];
					}

					provisionExpiration = [provisionProfile valueForKey:@"ExpirationDate"];

					if ([provisionExpiration compare:[NSDate date]] == NSOrderedAscending)
					{
						expirationDate = [provisionExpiration descriptionWithCalendarFormat:@"%Y-%m-%d" timeZone:nil locale:[[NSUserDefaults standardUserDefaults] dictionaryRepresentation]];
					}

					if ( ! isValidIdentity )
					{
						NSString *message = nil;

						if (provisionName == nil)
						{
							// If we have no identity and no provision name then the file is bogus
							message = [NSString stringWithFormat:@"Error: cannot parse provisioning file %@\n\n", [fullPath stringByAbbreviatingWithTildeInPath]];
						}
						else
						{
							// We didn't find any valid cert for this provisioning profile.  Report the identities we did find.
							message = [NSString stringWithFormat:@"Warning: no valid private certificates found for profile \"%@\"\n\tPublic certificate%s: %@",
									   provisionName, ([certIdentities count] == 1 ? "" : "s"), [certIdentities componentsJoinedByString:@", "]];

							if (expirationDate != nil)
							{
								message = [message  stringByAppendingFormat:@"\n\tProfile expired: %@", expirationDate];
								expirationDate = nil;
							}

							message = [message  stringByAppendingFormat:@"\n\tProvisioning file: %@\n\n", [fullPath stringByAbbreviatingWithTildeInPath]];
						}

						Rtt_Log("%s", [message UTF8String]);

						// This goes in the menu in place of the missing identity
						identity = kNoCertificateFound;
					}

					if (expirationDate != nil)
					{
						Rtt_Log("Warning: provisioning profile \"%s\" expired: %s\n\tProvisioning file: %s\n\n",
								[provisionName UTF8String], [expirationDate UTF8String], [[fullPath stringByAbbreviatingWithTildeInPath] UTF8String]);
					}

					IdentityMenuItem* menuItem = [[IdentityMenuItem alloc] initWithIdentity:identity provisionPath:fullPath provisionName:provisionName expirationDate:expirationDate fingerprint:certFingerprint];
					[menuItem autorelease];

#if 0
					// "duplicates" are not really an issue (maybe we'll decide they are later)
					if ([identitiesMenu indexOfItemWithTitle:[NSString stringWithFormat:@"%@ [%@]", identity, provisionName]] != -1)
					{
						Rtt_Log("Warning: duplicate identity: %s [%s]\n\tProvisioning file: %s",
								[identity UTF8String], [provisionName UTF8String], [[fullPath stringByAbbreviatingWithTildeInPath] UTF8String]);
					}
#endif

					if (isValidIdentity)
					{
						SEL menuAction = NSSelectorFromString(@"didSelectIdentity:");
						[menuItem setAction:menuAction];
						[menuItem setTarget:delegate];
					}

					if ([identity isEqualToString:kNoCertificateFound])
					{
						[disabledSubMenu addItem:menuItem];
					}
					else if ([[menuItem title] hasPrefix:@"iOS Team Provisioning Profile"] ||
							 [[menuItem title] hasPrefix:@"iOSTeam Provisioning Profile"])
					{
						[iOSTeamProvisioningProfilesSubMenu addItem:menuItem];
					}
					else
					{
						[identitiesMenu addItem:menuItem];
					}

					[menuItem setEnabled:isValidIdentity];

					// Prevent long cert names from wrapping
					NSMutableParagraphStyle* paragraphStyle = [[NSMutableParagraphStyle alloc] init];
					[paragraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
					NSMutableDictionary* attr = [[NSMutableDictionary alloc] initWithObjectsAndKeys:smallFont, NSFontAttributeName, paragraphStyle, NSParagraphStyleAttributeName, nil ];
					NSMutableAttributedString* formattedTitle = [[NSMutableAttributedString alloc] initWithString:[menuItem title] attributes:attr];

					/*
					 // Make the identity portion bold
					 NSRange range = { 0, [identity length] };
					 [attr setObject:boldFont forKey:NSFontAttributeName];
					 [formattedTitle setAttributes:attr range:range];
					 */
					// Make the identity portion bold
					NSRange range = { 0, [provisionName length] };
					[attr setObject:boldFont forKey:NSFontAttributeName];
					[formattedTitle setAttributes:attr range:range];

					// Now make "Expired" red
					range = [[menuItem title] rangeOfString:kExpiredProfile];
					[attr setObject:[NSColor redColor] forKey:NSForegroundColorAttributeName];
					[formattedTitle setAttributes:attr range:range];

					[menuItem setAttributedTitle:formattedTitle];

					[attr release];
					[paragraphStyle release];
					[formattedTitle release];
				}
			}
			else
			{
				Rtt_Log("Warning: failed to parse provisioning profile: %s", [[fullPath stringByAbbreviatingWithTildeInPath] UTF8String]);
			}
		}
	}

    NSComparisonResult (^menuSorter)(IdentityMenuItem *item1, IdentityMenuItem *item2) = ^(IdentityMenuItem *item1, IdentityMenuItem *item2) {

        // Sort the placeholders at the top
        if ([[item1 title] isEqualToString:NSLocalizedString(@"Choose from the following", nil)])
            return (NSComparisonResult)NSOrderedAscending;
        else if ([[item1 title] isEqualToString:NSLocalizedString(@"None", nil)])
            return (NSComparisonResult)NSOrderedAscending;
        else if (! [item1 isEnabled] && [item2 isEnabled])
            return (NSComparisonResult)NSOrderedDescending;
        else if ([item1 isEnabled] && ! [item2 isEnabled])
            return (NSComparisonResult)NSOrderedAscending;
        else
            return [[item1 title] caseInsensitiveCompare:[item2 title]];
    };

    // Here we try to make the provisioning profile menu as compact as possible.
    // If we have personal profiles we put them on the top level of the menu with "iOS Team Provisioning Profiles" and
    // "Unusable profiles (no private certificate found)"
    // submenus for the rest.  However, if there are no personal profiles we promote the "iOS Team" items to the top level
    // to make the menu easier to use.

    NSMenu *parentMenu = identitiesMenu;

    // The first item is a placeholder
    if ([identitiesMenu numberOfItems] > 1)
    {
        // Sort the "Provisioning Profile" menu into ascending alphabetical order
        // with the disabled certs at the end
        NSArray *sortedValues = [[identitiesMenu itemArray] sortedArrayUsingComparator:menuSorter];

        // Now repopulate the menu with the sorted items
        [identitiesMenu removeAllItems];

        for (NSMenuItem *item : sortedValues)
        {
            [item setEnabled:YES];
            [identitiesMenu addItem:item];
        }
    }
    else
    {
        parentMenu = iOSTeamProvisioningProfilesSubMenu;
    }
    
	// Add the submenus (if they have any items)

    if ([iOSTeamProvisioningProfilesSubMenu numberOfItems] > 0)
    {
        // Sort the "Provisioning Profile" menu into ascending alphabetical order
        // with the disabled certs at the end
        NSArray *sortedValues = [[iOSTeamProvisioningProfilesSubMenu itemArray] sortedArrayUsingComparator:menuSorter];

        // Now repopulate the menu with the sorted items
        [iOSTeamProvisioningProfilesSubMenu removeAllItems];

        for (NSMenuItem *item : sortedValues)
        {
            [item setEnabled:YES];
            [iOSTeamProvisioningProfilesSubMenu addItem:item];
        }

        if (parentMenu == identitiesMenu)
        {
            NSMenuItem *iOSTeamProvisioningProfilesItem = [[NSMenuItem alloc] initWithTitle:@"iOS Team Provisioning Profiles" action:nil keyEquivalent:@""];
            [identitiesMenu addItem:iOSTeamProvisioningProfilesItem];
            [identitiesMenu setSubmenu:iOSTeamProvisioningProfilesSubMenu forItem:iOSTeamProvisioningProfilesItem];
        }
    }

    if ([disabledSubMenu numberOfItems] > 0)
    {
        // Sort this submenu into ascending alphabetical order
        NSArray *sortedValues = [[disabledSubMenu itemArray] sortedArrayUsingComparator:menuSorter];

        // Now repopulate the menu with the sorted items
        [disabledSubMenu removeAllItems];

        for (NSMenuItem *item : sortedValues)
        {
            [item setEnabled:NO];
            [disabledSubMenu addItem:item];
        }
        
        NSMenuItem *disabledItem = [[NSMenuItem alloc] initWithTitle:@"Unusable profiles (no private certificate found)" action:nil keyEquivalent:@""];
        [parentMenu addItem:disabledItem];
        [parentMenu setSubmenu:disabledSubMenu forItem:disabledItem];
    }

    if (parentMenu != identitiesMenu)
    {
        // Copy everything to the menu that came in from our caller
        for (NSMenuItem *item : [parentMenu itemArray])
        {
            [parentMenu removeItem:item];
            [identitiesMenu addItem:item];
        }
    }

    if ([identitiesMenu numberOfItems] == 1)
    {
        // There's nothing in the menu except the "Choose one of the following" placeholder
        SEL menuAction = NSSelectorFromString(@"visitProvisioningProfileURL:");
        NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"Visit https://developer.apple.com to create provisioning profiles" action:menuAction keyEquivalent:@""];

        [identitiesMenu addItem:item];
    }
}

@end

// ----------------------------------------------------------------------------
