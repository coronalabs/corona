// ----------------------------------------------------------------------------
// 
// HttpProgressEventArgs.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "HttpProgressEventArgs.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Networking {

#pragma region Consructors/Destructors
HttpProgressEventArgs::HttpProgressEventArgs(int64 bytesTransferred)
{
	// Make sure the given argument is valid.
	if (bytesTransferred < 0)
	{
		bytesTransferred = 0;
	}

	// Initialize member variables.
	fBytesTransferred = bytesTransferred;
	fTotalBytesToTransfer = -1;
}

HttpProgressEventArgs::HttpProgressEventArgs(int64 bytesTransferred, int64 totalBytesToTransfer)
{
	// Make sure the given arguments are valid.
	if (bytesTransferred < 0)
	{
		bytesTransferred = 0;
	}
	if (totalBytesToTransfer < -1)
	{
		totalBytesToTransfer = -1;
	}

	// Initialize member variables.
	fBytesTransferred = bytesTransferred;
	fTotalBytesToTransfer = totalBytesToTransfer;
}

#pragma endregion


#pragma region Public Methods/Properties
bool HttpProgressEventArgs::IsTotalBytesToTransferKnown::get()
{
	return (fTotalBytesToTransfer >= 0);
}

int64 HttpProgressEventArgs::TotalBytesToTransfer::get()
{
	return fTotalBytesToTransfer;
}

int64 HttpProgressEventArgs::BytesTransferred::get()
{
	return fBytesTransferred;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
