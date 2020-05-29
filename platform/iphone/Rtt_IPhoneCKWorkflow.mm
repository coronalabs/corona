//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

