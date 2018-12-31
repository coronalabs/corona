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
#include "Rtt_IPhoneCKWorkflow.h"


#include "Rtt_Lua.h"
//#include "lua.h"
//#include "lauxlib.h"
//#include "lualib.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------


IPhoneCKWorkflow::IPhoneCKWorkflow( )
{

}

IPhoneCKWorkflow::~IPhoneCKWorkflow()
{

}

void IPhoneCKWorkflow::Decrypt(std::string luaByteCode)
{
	lua_State *L = luaL_newstate();
	
		
	lua_close( L );
}

int IPhoneCKWorkflow::GetNumProducts()
{
	return fLicenseInfo.size();
}

IPhoneCKWorkflow::LicenseCode IPhoneCKWorkflow::GetLicenseType(int productIndex)
{
	return fLicenseInfo[productIndex].fLicenseCode;
}

std::string IPhoneCKWorkflow::GetLicenseVersion(int productIndex)
{
	return fLicenseInfo[productIndex].fLicenseVersion;
}

std::time_t IPhoneCKWorkflow::GetLicenseCreationDate(int productIndex)
{
	return fLicenseInfo[productIndex].fLicenseCreationDate;
}

std::time_t IPhoneCKWorkflow::GetLicenseExpirationDate(int productIndex)
{
	return fLicenseInfo[productIndex].fLicenseExpirationDate;
}

std::string IPhoneCKWorkflow::GetAppBundleId(int productIndex)
{
	return fLicenseInfo[productIndex].fAppBundleId;
}

std::string IPhoneCKWorkflow::GetEmailAddress(int productIndex)
{
	return fLicenseInfo[productIndex].fEmailAddress;
}

std::string IPhoneCKWorkflow::GetProductId(int productIndex)
{
	return fLicenseInfo[productIndex].fProductId;
}

int IPhoneCKWorkflow::GetProductIndexById(std::string productId)
{
	for (int i = 0; i < (int)fLicenseInfo.size(); i++)
	{
		if (productId == fLicenseInfo[i].fProductId)
		{
			return i;
		}
	}
	return -1;
}

bool IPhoneCKWorkflow::PassesTrialModeCheck(std::string productId, std::time_t productCreationTime)
{
	bool result = false;
	int productIndex = GetProductIndexById(productId);
	if (productIndex == -1)
	{
		result = false;
	}
	
	LicenseCode licenseType = GetLicenseType(productIndex);
	if ( licenseType != IPhoneCKWorkflow::ID_TRIAL )
	{
		result = false;
	}
	
	std::time_t licenseCreation = GetLicenseCreationDate(productIndex);
	
	//TODO: IMPORTANT
	//TimeStamp check with server ?  (If current date is > creation time + 30 days)
	//return false
	
	std::time_t licenseExpiration = GetLicenseExpirationDate(productIndex);
	
	if ( productCreationTime <= licenseExpiration )
	{
		return true;
	}
	
	return result;
	
}
bool IPhoneCKWorkflow::PassesPaidModeCheck(std::string productId, std::time_t productCreationTime)
{
	bool result = false;
	int productIndex = GetProductIndexById(productId);
	if (productIndex == -1)
	{
		result = false;
	}
	
	LicenseCode licenseType = GetLicenseType(productIndex);
	if ( licenseType != IPhoneCKWorkflow::IO_PAID )
	{
		result = false;
	}
	
	std::time_t licenseCreation = GetLicenseCreationDate(productIndex);
	
	//TODO
	//TimeStamp check with server ?  (If current date is > creation time + 30 days) return false
	
	std::time_t licenseExpiration = GetLicenseExpirationDate(productIndex);
	
	if ( productCreationTime <= licenseExpiration )
	{
		return true;
	}
	
	return result;
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

