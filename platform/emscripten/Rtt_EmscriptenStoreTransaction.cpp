//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenStoreTransaction.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#pragma region Constructors/Destructors
/// Creates a new transaction object initialized to its defaults.
EmscriptenStoreTransaction::EmscriptenStoreTransaction()
:	PlatformStoreTransaction()
{
}

/// Destructor. Cleans up resources.
EmscriptenStoreTransaction::~EmscriptenStoreTransaction()
{
}

#pragma endregion


#pragma region Public Member Functions
/// Gets the state of the transaction such as purchased, canceled, failed, etc.
/// @return Returns the state of the transaction.
PlatformStoreTransaction::State EmscriptenStoreTransaction::GetState() const
{
	return kTransactionStateUndefined;
}

/// Gets the error that might have occurred during the transaction.
/// @return Returns kTransactionErrorNone if no error occurred.
///         Otherwise returns a specific error code such as canceled, not allowed, invalid, etc.
PlatformStoreTransaction::ErrorType EmscriptenStoreTransaction::GetErrorType() const
{
	return kTransactionErrorNone;
}

/// Gets an error message if an error occurred. Should be called if GetErrorType() does not return None.
/// @return Returns a string detailing why an error occurred.
///         Returns NULL or empty string if no error occurred or if details were not provided.
const char* EmscriptenStoreTransaction::GetErrorString() const
{
	return NULL;
}

/// Gets the unique string ID of the product involved in the transaction.
/// @return Returns the unique string ID of the product. Returns NULL or empty string upon error.
const char* EmscriptenStoreTransaction::GetProductIdentifier() const
{
	return NULL;
}

/// Gets the receipt of the transaction in string form.
/// @return Returns the receipt as a string. Returns NULL or empty string upon error.
const char* EmscriptenStoreTransaction::GetReceipt() const
{
	return NULL;
}

/// Gets the digital signature string that this transaction was signed with.
/// @return Returns the signature as a string. Returns NULL or empty string if a signatur was not assigned.
const char* EmscriptenStoreTransaction::GetSignature() const
{
	return NULL;
}

/// Gets the unique string ID of the transaction. Needed to confirm purchases via the Lua store.finishTransaction() function.
/// @return Returns the unique string ID of the transaction. Returns NULL or empty string upon error.
const char* EmscriptenStoreTransaction::GetIdentifier() const
{
	return NULL;
}

/// Gets the transaction date and time in string form.
/// @return Returns a date and time as a localized string. Returns NULL or empty string if there was an error.
const char* EmscriptenStoreTransaction::GetDate() const
{
	return NULL;
}

/// Gets the "restored" receipt of the transaction in string form.
/// Only applicable if this is a "restore" transaction.
/// @return Returns the receipt as a string.
///         Returns NULL or empty string if this is not a "restore" transaction.
const char* EmscriptenStoreTransaction::GetOriginalReceipt() const
{
	return NULL;
}

/// Gets the unique string ID of the "restored" purchase transaction.
/// Only applicable if this is a "restore" transaction.
/// @return Returns the unique string ID of the transaction.
///         Returns NULL or empty string if this is not a "restore" transaction.
const char* EmscriptenStoreTransaction::GetOriginalIdentifier() const
{
	return NULL;
}

/// Gets the "restored" purchase transaction date and time in string form.
/// Only applicable if this is a "restore" transaction.
/// @return Returns a date and time as a localized string.
///         Returns NULL or empty string if this is not a "restore" transaction.
const char* EmscriptenStoreTransaction::GetOriginalDate() const
{
	return NULL;
}

#pragma endregion

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
