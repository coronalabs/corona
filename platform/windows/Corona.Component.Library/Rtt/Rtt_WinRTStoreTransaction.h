// ----------------------------------------------------------------------------
// 
// Rtt_WinRTStoreTransaction.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_PlatformInAppStore.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

/// Stores information about a single product transaction.
class WinRTStoreTransaction : public PlatformStoreTransaction
{
	public:
		WinRTStoreTransaction();
		virtual ~WinRTStoreTransaction();

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
};

} // namespace Rtt
