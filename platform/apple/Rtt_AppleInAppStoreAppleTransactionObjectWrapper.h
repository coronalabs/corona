//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleInAppStoreAppleTransactionObjectWrapper_H__
#define _Rtt_AppleInAppStoreAppleTransactionObjectWrapper_H__

#include "Rtt_PlatformInAppStore.h"

@class SKPaymentTransaction;
@class SKProduct;
@class SKPayment;
@class NSError;
// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AppleStoreTransaction : public PlatformStoreTransaction
{
	public:
		typedef PlatformStoreTransaction Super;

	public:
		AppleStoreTransaction( SKPaymentTransaction *transaction );
		~AppleStoreTransaction();

	public:		
		virtual PlatformStoreTransaction::State GetState() const;
		virtual PlatformStoreTransaction::ErrorType GetErrorType() const;
		virtual const char* GetErrorString() const;
	
		virtual const char* GetProductIdentifier() const;
		
		virtual const char* GetReceipt() const;
		virtual const char* GetSignature() const;
		virtual const char* GetIdentifier() const;
		virtual const char* GetDate() const;

		// For restored items
		virtual const char* GetOriginalReceipt() const;
		virtual const char* GetOriginalIdentifier() const;
		virtual const char* GetOriginalDate() const;		
		
	public:	
		virtual SKPaymentTransaction* GetObject();

	protected:
		SKPaymentTransaction *fTransaction;
};

// ----------------------------------------------------------------------------

// Currently used for handling Apple restore transaction errors because there no SKPaymentTransaction in that case.
class AppleStoreRestoreTransactionError : public AppleStoreTransaction
{
	public:
		typedef AppleStoreTransaction Super;

	public:
		AppleStoreRestoreTransactionError( NSError* error );
		~AppleStoreRestoreTransactionError();

	public:		
		virtual PlatformStoreTransaction::State GetState() const;
		virtual PlatformStoreTransaction::ErrorType GetErrorType() const;
		virtual const char* GetErrorString() const;
	
		virtual const char* GetProductIdentifier() const;
		
		virtual const char* GetReceipt() const;
		virtual const char* GetSignature() const;
		virtual const char* GetIdentifier() const;
		virtual const char* GetDate() const;

		// For restored items
		virtual const char* GetOriginalReceipt() const;
		virtual const char* GetOriginalIdentifier() const;
		virtual const char* GetOriginalDate() const;		
		
	public:	

	protected:
		NSError* fError;
};

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AppleInAppStoreAppleTransactionObjectWrapper_H__
