//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_PlatformInAppStore.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/// Stores information about a single product transaction.
class EmscriptenStoreTransaction : public PlatformStoreTransaction
{
	public:
		EmscriptenStoreTransaction();
		virtual ~EmscriptenStoreTransaction();

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

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
