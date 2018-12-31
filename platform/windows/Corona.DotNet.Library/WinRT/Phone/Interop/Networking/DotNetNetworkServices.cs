using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.Networking
{
	/// <summary>
	///  Provides networking features such as the ability to transmit/receive HTTP messages,
	///  fetch the device's network status, etc.
	/// </summary>
	public class DotNetNetworkServices : Corona.WinRT.Interop.Networking.INetworkServices
	{
		#region Constructors
		/// <summary>Creates a new object providing accessing to networking features implemented in .NET.</summary>
		public DotNetNetworkServices()
		{
		}

		#endregion


		#region Public Methods
		/// <summary>Fetches the device's current network connection status.</summary>
		/// <returns>Returns the device's current network connection status.</returns>
		public Corona.WinRT.Interop.Networking.NetworkConnectionStatus FetchConnectionStatus()
		{
			// Determine if we have network access.
			bool isConnected = Microsoft.Phone.Net.NetworkInformation.DeviceNetworkInformation.IsNetworkAvailable;

			// If we have network access, then determine if its via cellular service.
			// Note: The below assumes if you have cell service and no wifi access, then we're connected via cellular.
			bool isUsingCellularService = false;
			if (isConnected)
			{
				if (Microsoft.Phone.Net.NetworkInformation.DeviceNetworkInformation.IsWiFiEnabled == false)
				{
					if (Microsoft.Phone.Net.NetworkInformation.DeviceNetworkInformation.IsCellularDataEnabled ||
					    Microsoft.Phone.Net.NetworkInformation.DeviceNetworkInformation.IsCellularDataRoamingEnabled)
					{
						isUsingCellularService = true;
					}
				}
			}

			// Return the network status.
			return new Corona.WinRT.Interop.Networking.NetworkConnectionStatus(isConnected, isUsingCellularService);
		}

		/// <summary>Creates an object used to send an HTTP request and receive a response.</summary>
		/// <returns>Returns an object used to communicate with an HTTP server.</returns>
		public Corona.WinRT.Interop.Networking.IHttpRequestOperation CreateHttpRequest()
		{
			return new Corona.WinRT.Interop.Networking.DotNetHttpRequestOperation();
		}

		#endregion
	}
}
