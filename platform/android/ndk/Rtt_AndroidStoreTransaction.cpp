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


#include "Core/Rtt_Build.h"
#include "Rtt_AndroidStoreTransaction.h"


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/// Creates a new transaction object initialized to its defaults.
/// @param allocator Pointer to an allocator used to create objects. Cannot be NULL.
AndroidStoreTransaction::AndroidStoreTransaction(Rtt_Allocator *allocator)
:	PlatformStoreTransaction(),
	fState(kTransactionStateUndefined),
	fErrorType(kTransactionErrorNone),
	fErrorString(allocator),
	fProductId(allocator),
	fReceipt(allocator),
	fSignature(allocator),
	fTransactionId(allocator),
	fTransactionTime(allocator),
	fOriginalReceipt(allocator),
	fOriginalTransactionId(allocator),
	fOriginalTransactionTime(allocator)
{
}

/// Destructor. Cleans up resources.
AndroidStoreTransaction::~AndroidStoreTransaction()
{
}

/// Gets the state of the transaction such as purchased, canceled, failed, etc.
/// @return Returns the state of the transaction.
PlatformStoreTransaction::State AndroidStoreTransaction::GetState() const
{
	return fState;
}

/// Gets the error that might have occurred during the transaction.
/// @return Returns kTransactionErrorNone if no error occurred.
///         Otherwise returns a specific error code such as canceled, not allowed, invalid, etc.
PlatformStoreTransaction::ErrorType AndroidStoreTransaction::GetErrorType() const
{
	return fErrorType;
}

/// Gets an error message if an error occurred. Should be called if GetErrorType() does not return None.
/// @return Returns a string detailing why an error occurred.
///         Returns NULL or empty string if no error occurred or if details were not provided.
const char* AndroidStoreTransaction::GetErrorString() const
{
	return fErrorString.GetString();
}

/// Gets the unique string ID of the product involved in the transaction.
/// @return Returns the unique string ID of the product. Returns NULL or empty string upon error.
const char* AndroidStoreTransaction::GetProductIdentifier() const
{
	return fProductId.GetString();
}

/// Gets the receipt of the transaction in string form.
/// @return Returns the receipt as a string. Returns NULL or empty string upon error.
const char* AndroidStoreTransaction::GetReceipt() const
{
	return fReceipt.GetString();
}

/// Gets the digital signature string that this transaction was signed with.
/// @return Returns the signature as a string. Returns NULL or empty string if a signatur was not assigned.
const char* AndroidStoreTransaction::GetSignature() const
{
	return fSignature.GetString();
}

/// Gets the unique string ID of the transaction. Needed to confirm purchases via the Lua store.finishTransaction() function.
/// @return Returns the unique string ID of the transaction. Returns NULL or empty string upon error.
const char* AndroidStoreTransaction::GetIdentifier() const
{
	return fTransactionId.GetString();
}

/// Gets the transaction date and time in string form.
/// @return Returns a date and time as a localized string. Returns NULL or empty string if there was an error.
const char* AndroidStoreTransaction::GetDate() const
{
	return fTransactionTime.GetString();
}

/// Gets the "restored" receipt of the transaction in string form.
/// Only applicable if this is a "restore" transaction.
/// @return Returns the receipt as a string.
///         Returns NULL or empty string if this is not a "restore" transaction.
const char* AndroidStoreTransaction::GetOriginalReceipt() const
{
	return fOriginalReceipt.GetString();
}

/// Gets the unique string ID of the "restored" purchase transaction.
/// Only applicable if this is a "restore" transaction.
/// @return Returns the unique string ID of the transaction.
///         Returns NULL or empty string if this is not a "restore" transaction.
const char* AndroidStoreTransaction::GetOriginalIdentifier() const
{
	return fOriginalTransactionId.GetString();
}

/// Gets the "restored" purchase transaction date and time in string form.
/// Only applicable if this is a "restore" transaction.
/// @return Returns a date and time as a localized string.
///         Returns NULL or empty string if this is not a "restore" transaction.
const char* AndroidStoreTransaction::GetOriginalDate() const
{
	return fOriginalTransactionTime.GetString();
}

/// Sets the state of the transaction.
/// @param value The state of the transaction such as purchased, canceled, or failed.
void AndroidStoreTransaction::SetState(PlatformStoreTransaction::State value)
{
	fState = value;
}

/// Sets the type of error that occurred during the transaction, if any.
/// @param value The error that occurred during the transaction such as canceled, not allowed, etc.
///              Set to kTransactionErrorNone if no error occurred.
void AndroidStoreTransaction::SetErrorType(PlatformStoreTransaction::ErrorType value)
{
	fErrorType = value;
}

/// Sets a detailed message for when an error occurs.
/// @param value The string to be copied to this object. Set to NULL to clear this object's error message.
void AndroidStoreTransaction::SetErrorString(const char *value)
{
	fErrorString.Set(value);
}

/// Sets the unique string ID of the product involved in the transaction.
/// @param value The string ID to be copied to this object. Set to NULL to clear this object's product ID.
void AndroidStoreTransaction::SetProductIdentifier(const char *value)
{
	fProductId.Set(value);
}

/// Sets the receipt string for this transaction.
/// @param value The string to be copied to this object. Set to NULL to clear this object's receipt string.
void AndroidStoreTransaction::SetReceipt(const char *value)
{
	fReceipt.Set(value);
}

/// Sets the signature string that the transaction was digitally signed with.
/// @param value The string to be copied to this object. Set to NULL to clear this object's signature string.
void AndroidStoreTransaction::SetSignature(const char *value)
{
	fSignature.Set(value);
}

/// Sets the unique string ID of the transaction.
/// @param value The string ID to be copied to the object. Set to NULL to clear this object's string ID.
void AndroidStoreTransaction::SetIdentifier(const char *value)
{
	fTransactionId.Set(value);
}

/// Sets the date and time the transaction occurred as a localized string.
/// @param value The date and time string to be copied to this object. Set to NULL to clear this object's string.
void AndroidStoreTransaction::SetDate(const char *value)
{
	fTransactionTime.Set(value);
}

/// Sets the receipt string of the restored transaction.
/// This value is only to be assigned if this is a "restore" transaction.
/// @param value The receipt string to be copied to this object. Set to NULL to clear this object's string.
void AndroidStoreTransaction::SetOriginalReceipt(const char *value)
{
	fOriginalReceipt.Set(value);
}

/// Sets the unique string ID of the restored transaction.
/// This value is only to be assigned if this is a "restore" transaction.
/// @param value The unique string ID to be copied to this object. Set to NULL to clear this object's string.
void AndroidStoreTransaction::SetOriginalIdentifier(const char *value)
{
	fOriginalTransactionId.Set(value);
}

/// Sets the date and time the "restored" transaction occurred as a localized string.
/// This value is only to be assigned if this is a "restore" transaction.
/// @param value The date and time string to be copied to this object. Set to NULL to clear this object's string.
void AndroidStoreTransaction::SetOriginalDate(const char *value)
{
	fOriginalTransactionTime.Set(value);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
