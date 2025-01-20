//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AppleInAppStoreAppleTransactionObjectWrapper.h"

#import <CoreFoundation/CoreFoundation.h>
#import <StoreKit/StoreKit.h>

// ----------------------------------------------------------------------------

namespace Rtt
{
	
// ----------------------------------------------------------------------------
#pragma mark Apple Transaction

AppleStoreTransaction::AppleStoreTransaction( SKPaymentTransaction *transaction ) 
:	Super(),
	fTransaction( [transaction retain] )
{
}

AppleStoreTransaction::~AppleStoreTransaction() 
{
	[fTransaction release];
}

PlatformStoreTransaction::State
AppleStoreTransaction::GetState() const
{
	PlatformStoreTransaction::State result = kTransactionStateUndefined;

	if ( fTransaction )
	{
		switch( fTransaction.transactionState )
		{
			case SKPaymentTransactionStatePurchased:
			{
				result = kTransactionStatePurchased;
				break;
			}
			case SKPaymentTransactionStateFailed:
			{
				result = ( fTransaction.error && ( SKErrorPaymentCancelled == fTransaction.error.code ) )
					? kTransactionStateCancelled
					: kTransactionStateFailed;
				break;
			}
			case SKPaymentTransactionStateRestored:
			{
				result = kTransactionStateRestored;
				break;
			}
			case SKPaymentTransactionStatePurchasing:
			{
				result = kTransactionStatePurchasing;
				break;
			}
			default:
			{
				break;			
			}
		}
	}

	return result;
}

PlatformStoreTransaction::ErrorType
AppleStoreTransaction::GetErrorType() const
{
	PlatformStoreTransaction::ErrorType result = kTransactionErrorNone;

	if ( fTransaction && fTransaction.error )
	{
		switch( fTransaction.error.code )
		{
			case SKErrorPaymentCancelled:
			{
				result = kTransactionErrorPaymentCancelled;
				break;
			}
			case SKErrorClientInvalid:
			{
				result = kTransactionErrorClientInvalid;
				break;
			}
			case SKErrorPaymentInvalid:
			{
				result = kTransactionErrorPaymentInvalid;
				break;
			}
			case SKErrorPaymentNotAllowed:
			{
				result = kTransactionErrorPaymentNotAllowed;
				break;
			}
			default:
			{
				result = kTransactionErrorUnknown;
				break;			
			}
		}
	}

	return result;
}

const char*
AppleStoreTransaction::GetErrorString() const
{
	return [[[fTransaction error] localizedDescription] UTF8String];
}

const char*
AppleStoreTransaction::GetProductIdentifier() const
{
	return [[[fTransaction payment] productIdentifier] UTF8String];
}

const char*
AppleStoreTransaction::GetReceipt() const
{
#if defined( Rtt_IPHONE_ENV )
	return [[[fTransaction transactionReceipt] base64EncodedStringWithOptions:0] UTF8String];
#else
	return NULL;
#endif
}

const char*
AppleStoreTransaction::GetSignature() const
{
	return NULL;
}

const char*
AppleStoreTransaction::GetIdentifier() const
{
	return [[fTransaction transactionIdentifier] UTF8String];
}

const char*
AppleStoreTransaction::GetDate() const
{
	return [[[fTransaction transactionDate] description] UTF8String];		
}

const char*
AppleStoreTransaction::GetOriginalReceipt() const
{
#if defined( Rtt_IPHONE_ENV )
	return [[[[fTransaction originalTransaction] transactionReceipt] base64EncodedStringWithOptions:0] UTF8String];
#else
	return NULL;
#endif
}


const char*
AppleStoreTransaction::GetOriginalIdentifier() const
{
	return [[[fTransaction originalTransaction] transactionIdentifier] UTF8String];
}

const char*
AppleStoreTransaction::GetOriginalDate() const
{
	return [[[[fTransaction originalTransaction] transactionDate] description] UTF8String];		
}

SKPaymentTransaction*
AppleStoreTransaction::GetObject()
{
	return fTransaction;
}

// ----------------------------------------------------------------------------
#pragma mark Restore Transaction Error

AppleStoreRestoreTransactionError::AppleStoreRestoreTransactionError( NSError* error ) 
:	Super( nil ),
	fError( [error retain] )
{
}

AppleStoreRestoreTransactionError::~AppleStoreRestoreTransactionError() 
{
	[fError release];
}

PlatformStoreTransaction::State
AppleStoreRestoreTransactionError::GetState() const
{
	return kTransactionStateFailed;
}

PlatformStoreTransaction::ErrorType
AppleStoreRestoreTransactionError::GetErrorType() const
{
	return kTransactionErrorRestoreFailed;
}

const char*
AppleStoreRestoreTransactionError::GetErrorString() const
{
	return [[fError localizedDescription] UTF8String];
}

const char*
AppleStoreRestoreTransactionError::GetProductIdentifier() const
{
	return NULL;
}

const char*
AppleStoreRestoreTransactionError::GetReceipt() const
{
	return NULL;
}

const char*
AppleStoreRestoreTransactionError::GetSignature() const
{
	return NULL;
}

const char*
AppleStoreRestoreTransactionError::GetIdentifier() const
{
	return NULL;
}

const char*
AppleStoreRestoreTransactionError::GetDate() const
{
	return NULL;
}

const char*
AppleStoreRestoreTransactionError::GetOriginalReceipt() const
{
	return NULL;
}


const char*
AppleStoreRestoreTransactionError::GetOriginalIdentifier() const
{
	return NULL;
}

const char*
AppleStoreRestoreTransactionError::GetOriginalDate() const
{
	return NULL;
}


} // namespace Rtt

// ----------------------------------------------------------------------------

