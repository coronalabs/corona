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
