// ----------------------------------------------------------------------------
// 
// NetworkConnectionStatus.h
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

/// <summary>
///  <para>Provides information about the device's current network connection state.</para>
///  <para>Instances of this class are immutable.</para>
/// </summary>
public ref class NetworkConnectionStatus sealed
{
	public:
		/// <summary>Creates a new network connection status object with the given information.</summary>
		/// <param name="isConnected">
		///  <para>Set true if connected to the network. Note that this doesn't mean the device has Internet access.</para>
		///  <para>Set false if the device does not have network access.</para>
		/// </param>
		/// <param name="isUsingCellularService">
		///  <para>Set true if connected to the network/Internet via a cellular service.</para>
		///  <para>Set false if connected to the network via WiFi or Ethernet.</para>
		///  <para>Ignored if the "isConnected" parameter is set to false.</para>
		/// </param>
		NetworkConnectionStatus(bool isConnected, bool isUsingCellularService);
		
		/// <summary>Determines if the device is connected to the network.</summary>
		/// <remarks>
		///  Note that just because the device has network access does not necessarily mean that it has Internet access.
		///  The device could be connected to a private LAN or a pay-to-use WiFi network.
		/// </remarks>
		/// <value>Set true if connected to the network. Set false if the network is not available.</value>
		property bool IsConnected { bool get(); }

		/// <summary>
		///  <para>Determines if the device is connected to the network/Internet via a cellular service.</para>
		///  <para>This is useful in protecting end-users who care about data usage.</para>
		///  <para>This property is not applicable is the <see cref="IsConnected"/> property is set false.</para>
		/// </summary>
		/// <value>
		///  <para>Set true if the device is connected to the network via a cellular service.</para>
		///  <para>Set false if the device is connected to the network via WiFi or Ethernet.</para>
		/// </value>
		property bool IsUsingCelluarService { bool get(); }

	private:
		/// <summary>Set true if connected to the network.</summary>
		bool fIsConnected;

		/// <summary>
		///  <para>Set true if connected to the network/Internet via the celluar service.</para>
		///  <para>Set false if connected via WiFI or Ethernet.</para>
		///  <para>Not applicable if "fIsConnected" is set to false.</para>
		/// </summary>
		bool fIsUsingCelluarService;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
