// ----------------------------------------------------------------------------
// 
// HttpProgressEventArgs.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Networking {

/// <summary>Provides the number of bytes transferred when uploading an HTTP request or downloading an HTTP response.</summary>
public ref class HttpProgressEventArgs sealed
{
	public:
		/// <summary>
		///  Creates event arguments storing the number of bytes transferred for HTTP responses where the
		///  total number of bytes to be received is unknown.
		/// </summary>
		/// <param name="bytesTransferred">
		///  <para>Number of bytes transmitted so far.</para>
		///  <para>A negative value will be changed to zero.</para>
		/// </param>
		HttpProgressEventArgs(int64 bytesTransferred);
		
		/// <summary>
		///  Creates event arguments storing the number of bytes transferred and the total bytes expected
		///  when the operation finishes.
		/// </summary>
		/// <param name="bytesTransferred">
		///  <para>Number of bytes transmitted so far.</para>
		///  <para>A negative value will be changed to zero.</para>
		/// </param>
		/// <param name="totalBytesToTransfer">
		///  <para>Total number of bytes expected when the HTTP request upload or response download finishes.</para>
		///  <para>Set to zero if the HTTP response does not have a body.</para>
		///  <para>Set to -1 if this total is unknown.</para>
		/// </param>
		HttpProgressEventArgs(int64 bytesTransferred, int64 totalBytesToTransfer);

		/// <summary>
		///  <para>Determines if the total number of bytes to be received from the server is known.</para>
		///  <para>This determines if the <see cref="TotalBytesToTransfer"/> property will return a valid value.</para>
		/// </summary>
		/// <value>
		///  <para>
		///   Returns true if property <see cref="TotalBytesToTransfer"/> will provide the total number of bytes
		///   expected to be received from the server.
		///  </para>
		///  <para>Returns false if the server did not provide the number of bytes to be received.</para>
		/// </value>
		property bool IsTotalBytesToTransferKnown { bool get(); }

		/// <summary>Gets the total number of bytes expected to be received from the server.</summary>
		/// <value>
		///  <para>Number of bytes expected to be received from the server.</para>
		///  <para>Zero if the HTTP response does not have a body.</para>
		///  <para>-1 if the server did not provide the number of bytes to be sent in the response header.</para>
		/// </value>
		property int64 TotalBytesToTransfer { int64 get(); }

		/// <summary>Gets the total number of bytes transferred so far.</summary>
		/// <value>Number of bytes transferred so far.</value>
		property int64 BytesTransferred { int64 get(); }

	private:
		/// <summary>The total number of bytes making up the HTTP request or response's body. -1 if unknown.</summary>
		int64 fTotalBytesToTransfer;

		/// <summary>Total number of bytes transferred so far.</summary>
		int64 fBytesTransferred;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
