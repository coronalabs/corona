//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidStoreTransaction_H__
#define _Rtt_AndroidStoreTransaction_H__

#include "Core/Rtt_Types.h"
#include "Core/Rtt_String.h"
#include "Rtt_PlatformInAppStore.h"


namespace Rtt
{

/// Stores information about a single product transaction.
class AndroidStoreTransaction : public PlatformStoreTransaction
{
	public:
		AndroidStoreTransaction(Rtt_Allocator *allocator);
		virtual ~AndroidStoreTransaction();
		
		PlatformStoreTransaction::State GetState() const;
		PlatformStoreTransaction::ErrorType GetErrorType() const;
		const char* GetErrorString() const;
		const char* GetProductIdentifier() const;
		const char* GetReceipt() const;
		const char* GetSignature() const;
		const char* GetIdentifier() const;
		const char* GetDate() const;
		const char* GetOriginalReceipt() const;
		const char* GetOriginalIdentifier() const;
		const char* GetOriginalDate() const;
		
		void SetState(PlatformStoreTransaction::State value);
		void SetErrorType(PlatformStoreTransaction::ErrorType value);
		void SetErrorString(const char *value);
		void SetProductIdentifier(const char *value);
		void SetReceipt(const char *value);
		void SetSignature(const char *value);
		void SetIdentifier(const char *value);
		void SetDate(const char *value);
		void SetOriginalReceipt(const char *value);
		void SetOriginalIdentifier(const char *value);
		void SetOriginalDate(const char *value);
	
	private:
		PlatformStoreTransaction::State fState;
		PlatformStoreTransaction::ErrorType fErrorType;
		String fErrorString;
		String fProductId;
		String fReceipt;
		String fSignature;
		String fTransactionId;
		String fTransactionTime;
		String fOriginalReceipt;
		String fOriginalTransactionId;
		String fOriginalTransactionTime;
};

} // namespace Rtt

#endif // _Rtt_AndroidStoreTransaction_H__
