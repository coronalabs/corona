//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "NetworkConnectionStatus.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Networking {

#pragma region Consructors/Destructors
NetworkConnectionStatus::NetworkConnectionStatus(bool isConnected, bool isUsingCellularService)
{
	fIsConnected = isConnected;
	fIsUsingCelluarService = isConnected ? isUsingCellularService : false;
}

#pragma endregion


#pragma region Public Methods/Properties
bool NetworkConnectionStatus::IsConnected::get()
{
	return fIsConnected;
}

bool NetworkConnectionStatus::IsUsingCelluarService::get()
{
	return fIsUsingCelluarService;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
