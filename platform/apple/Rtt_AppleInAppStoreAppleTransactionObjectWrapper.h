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
