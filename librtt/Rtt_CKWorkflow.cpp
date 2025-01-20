//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Core/Rtt_Version.h"
#include "Rtt_CKWorkflow.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContainer.h"
#include <sstream>
#include <ctype.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

	int luaload_loader_callback(lua_State* L);

// ----------------------------------------------------------------------------


CKWorkflow::CKWorkflow(  )
: fEmailAddress()
{

}

CKWorkflow::~CKWorkflow()
{

}

CKWorkflow::LicenseStatus CKWorkflow::StringToStatus( const char *str )
{
	LicenseStatus result = License_Default;

	if ( str )
	{
		if ( 0 == strcmp( str, "0" ) )
		{
			result = License_Default;
		}
		else if ( 0 == strcmp( str, "1" ) )
		{
			result = License_Trial;
		}
		else if ( 0 == strcmp( str, "2" ) )
		{
			result = License_Paid;
		}
	}

	return result;
}

std::string CKWorkflow::GetCurrentPlatform()
{
	std::string result = "unknown";
	#if defined( Rtt_IPHONE_ENV )
		result = "ios";
	#elif defined( Rtt_ANDROID_ENV )
		result = "android";
	#elif defined( Rtt_WIN_PHONE_ENV )
		result = "wp8";
	#endif
	return result;
}

static const char kLicenses[] = "licenses";
static const char kLicenseCode[] = "licenseCode";
static const char kLicenseVersion[] = "licenseVersion";
static const char kLicenseCreationDate[] = "licenseCreationDate";
static const char kLicenseExpirationDate[] = "licenseExpirationDate";
static const char kProductId[] = "productId";
static const char kPlatform[] = "platform";
static const char kBuild[] = "build";

static const char kEmail[] = "email";
static const char kLicenseUrl[] = "licenseUrl";
static const char kLicenseNotice[] = "licenseNotice";


void CKWorkflow::Init(lua_State* L)
{
	LuaMap map(L, -1);
	std::vector<std::string> keys = map.GetKeys();
	
	LuaMap *licenseInfoBlocks = static_cast<LuaMap*>(map.GetData( kLicenses ));
	if ( licenseInfoBlocks )
	{
		std::vector<std::string> licenseInfoKeys = licenseInfoBlocks->GetKeys();
		for (size_t i = 0; i < licenseInfoKeys.size(); i++)
		{
			std::string key = licenseInfoKeys[i];
			LuaMap *licenseInfoBlock = static_cast<LuaMap*>(licenseInfoBlocks->GetData(key));
			
			if ( licenseInfoBlock )
			{
				LuaString *licenseCode = static_cast<LuaString*>(licenseInfoBlock->GetData( kLicenseCode ));
				LuaString *licenseVersion = static_cast<LuaString*>(licenseInfoBlock->GetData( kLicenseVersion ));
				LuaString *licenseCreationDate = static_cast<LuaString*>(licenseInfoBlock->GetData( kLicenseCreationDate ));
				LuaString *licenseExpirationDate = static_cast<LuaString*>(licenseInfoBlock->GetData( kLicenseExpirationDate ));
				//LuaString *appBundleId = static_cast<LuaString*>(licenseInfoBlock->GetData("appBundleId"));
				LuaString *productId = static_cast<LuaString*>(licenseInfoBlock->GetData( kProductId ));
				
				bool fullyFormedCheck = (NULL != licenseCode &&
										NULL != licenseVersion &&
										NULL != licenseCreationDate &&
										NULL != licenseExpirationDate &&
										NULL != productId);
					 
				Rtt_ASSERT( fullyFormedCheck ); //License should be fully formed

				//Required parameters
				if ( fullyFormedCheck )
				{
					//Parameters may not be there depending on product
					LuaString *platform = static_cast<LuaString*>(licenseInfoBlock->GetData( kPlatform ));
					LuaString *build = static_cast<LuaString*>(licenseInfoBlock->GetData( kBuild ));
					
					LicenseInfo info;
					info.fLicenseCode = StringToStatus(licenseCode->GetString().c_str());
					info.fLicenseVersion = licenseVersion->GetString();
					std::stringstream licenseCreationStr(licenseCreationDate->GetString().c_str());
					std::stringstream licenseExpirationStr(licenseExpirationDate->GetString().c_str());
					licenseCreationStr >> info.fLicenseCreationDate;
					licenseExpirationStr >> info.fLicenseExpirationDate;
					//info.fAppBundleId = appBundleId->GetString();
					info.fProductId = productId->GetString();
					
					
					if (platform)
					{
						info.fPlatform = platform->GetString();
					}
					
					if (build)
					{
						info.fBuild = build->GetString();
					}
					
					fLicenseInfo[(int)i] = info;
				}
			}
			
		}
	}
	
	LuaString *emailAddress = static_cast<LuaString*>(map.GetData( kEmail ));
	if ( emailAddress )
	{
		fEmailAddress = emailAddress->GetString();
	}
	
	LuaString *licenseUrl = static_cast<LuaString*>(map.GetData( kLicenseUrl ));
	if ( licenseUrl )
	{
		fLicenseUrl = licenseUrl->GetString();
	}
	
	LuaString *licenseNotice = static_cast<LuaString*>(map.GetData( kLicenseNotice ));
	if ( licenseNotice )
	{
		fLicenseNotice = licenseNotice->GetString();
	}
	
	//For internal use only
	// this->Log();

}

bool CKWorkflow::CompareValues(LuaMap &metaMap)
{
	bool result = false;
	
	LuaString *emailAddress = static_cast<LuaString*>(metaMap.GetData( kEmail ));
	LuaString *licenseUrl = static_cast<LuaString*>(metaMap.GetData( kLicenseUrl ));
	LuaString *licenseNotice = static_cast<LuaString*>(metaMap.GetData( kLicenseNotice ));
	if ( emailAddress && licenseUrl && licenseNotice)
	{
		std::string email = emailAddress->GetString();
		std::string url = licenseUrl->GetString();
		std::string notice = licenseNotice->GetString();
		
		if ( fEmailAddress.compare(email) == 0 &&
			fLicenseUrl.compare(url) == 0 &&
			fLicenseNotice.compare(notice) == 0 )
		{
			result = true;
		}
	}
	return result;
}

int CKWorkflow::GetNumProducts()
{
	return (int)fLicenseInfo.size();
}

CKWorkflow::LicenseStatus CKWorkflow::GetLicenseStatus(int productIndex)
{
	return fLicenseInfo[productIndex].fLicenseCode;
}

std::string CKWorkflow::GetLicenseVersion(int productIndex)
{
	return fLicenseInfo[productIndex].fLicenseVersion;
}

std::time_t CKWorkflow::GetLicenseCreationDate(int productIndex)
{
	return fLicenseInfo[productIndex].fLicenseCreationDate;
}

std::time_t CKWorkflow::GetLicenseExpirationDate(int productIndex)
{
	return fLicenseInfo[productIndex].fLicenseExpirationDate;
}

std::string CKWorkflow::GetAppBundleId(int productIndex)
{
	return fLicenseInfo[productIndex].fAppBundleId;
}

std::string CKWorkflow::GetProductId(int productIndex)
{
	return fLicenseInfo[productIndex].fProductId;
}

std::string CKWorkflow::GetPlatform( int productIndex )
{
	return fLicenseInfo[productIndex].fPlatform;
}

std::string CKWorkflow::GetBuild( int productIndex )
{
	return fLicenseInfo[productIndex].fBuild;
}

int CKWorkflow::GetProductIndexById(std::string productId)
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

time_t CKWorkflow::GetProductCreationTime()
{
	time_t currentTime;
	time ( &currentTime );
	struct tm *timeinfo;
    timeinfo = gmtime ( &currentTime );
#ifdef Rtt_IS_LOCAL_BUILD
	timeinfo->tm_year = 2000 - 1900;
#else
    timeinfo->tm_year = Rtt_BUILD_YEAR - 1900;
#endif
	timeinfo->tm_mon = Rtt_BUILD_MONTH - 1;
    timeinfo->tm_mday = Rtt_BUILD_DAY;
	timeinfo->tm_hour = 0;
    timeinfo->tm_min = 0;
    timeinfo->tm_sec = 0;
    timeinfo->tm_isdst = 0;
    time_t productCreationTime = mktime ( timeinfo );
	return productCreationTime;
}

std::string CKWorkflow::GetProductBuild()
{
	return std::string(Rtt_STRING_BUILD);
}

static const char kDefaultUrl[] = "http://coronacards.com/?utm_source=coronacards";
static const char kTrialUrl[] = "http://coronacards.com/trial?utm_source=coronacards";
static const char kRenewUrl[] = "http://coronacards.com/renew?utm_source=coronacards";
static const char kErrorUrl[] = "http://coronacards.com/error?utm_source=coronacards";
static const char kLearnMore[] = "Learn more";

static const char kOK[] = "OK";
static const char kQuit[] = "Quit";

static const char kMissingLicenseMessage[] = "License file not found. This is needed to use this software product. Please contact Corona Labs to acquire a valid license.";
static const char kMissingProductMessage[] = "The provided license file does not authorize this software to run on this platform. Please contact Corona Labs to acquire a valid license.";
static const char kInvalidLicenseMessage[] = "Invalid License";
static const char kTrialLicenseMessage[] = "Trial License Active";
static const char kTrialLicenseExpiredMessage[] = "Trial License Expired";
static const char kLicenseExpiredMessage[] = "Your license is not authorized to use the current version of Corona that you are running. Your license is only authorized to run older builds. Please contact Corona Labs to acquire an updated license.";

void CKWorkflow::DidCheck(lua_State *L, CKWorkflow::CheckResult r, std::string productId)
{
	if (r != kPaidLicense)
	{
		AlertSettings alertSettings = CreateAlertSettingsFor(r, productId);
		ShowAlertUsing(L, alertSettings);
	}
}

CKWorkflow::AlertSettings CKWorkflow::CreateAlertSettingsFor(CKWorkflow::CheckResult r, const std::string &productId)
{
	// Initialize alert dialog settings.
	AlertSettings alertSettings;
	alertSettings.ActionButtonText = kLearnMore;
	alertSettings.ActionButtonUrl = kDefaultUrl;
	alertSettings.ShouldQuit = false;
	
	// Set up the alert title.
	alertSettings.Title = productId;
	if (Rtt_StringCompareNoCase(productId.c_str(), "coronacards") == 0)
	{
		// Make sure to use the correct casing of the "CoronaCards" prodcut.
		alertSettings.Title = "CoronaCards";
	}
	else if (productId.length() > 0)
	{
		// Use the product string ID as the title. Capitalize the first letter.
		alertSettings.Title = productId;
		alertSettings.Title[0] = toupper(alertSettings.Title[0]);
	}
	else
	{
		// Product ID not provided. Default the alert title to this.
		alertSettings.Title = "Info";
	}
	
	// Set up the rest of the alert dialog settings based on the license verification result.
	switch ( r )
	{
		case kMissingLicense: {
			alertSettings.Message = kMissingLicenseMessage;
			alertSettings.OkayButtonText = kQuit;
			alertSettings.ShouldQuit = true;
		} break;

		case kMissingProduct: {
			alertSettings.Message = kMissingProductMessage;
			alertSettings.OkayButtonText = kQuit;
			alertSettings.ActionButtonUrl = kErrorUrl;
			alertSettings.ShouldQuit = true;
		} break;

		case kTrialLicense: {
			alertSettings.Message = kTrialLicenseMessage;
			alertSettings.OkayButtonText = kOK;
			alertSettings.ActionButtonUrl = kTrialUrl;
			alertSettings.ShouldQuit = false;
		} break;

		case kPaidExpiration: {
			alertSettings.Message = kLicenseExpiredMessage;
			alertSettings.OkayButtonText = kQuit;
			alertSettings.ActionButtonUrl = kRenewUrl;
			alertSettings.ShouldQuit = true;
		} break;

		case kPaidLicense: {
			alertSettings.ShouldQuit = false;
		} break;

		case kTrialExpiration: {
			alertSettings.Message = kTrialLicenseExpiredMessage;
			alertSettings.OkayButtonText = kQuit;
			alertSettings.ActionButtonUrl = kTrialUrl;
			alertSettings.ShouldQuit = true;
		} break;
		
		case kInvalid: {
			alertSettings.Message = kInvalidLicenseMessage;
			alertSettings.OkayButtonText = kQuit;
			alertSettings.ActionButtonUrl = kErrorUrl;
			alertSettings.ShouldQuit = true;
		} break;
		
		default: {
			alertSettings.Message = kMissingLicenseMessage;
			alertSettings.OkayButtonText = kQuit;
			alertSettings.ActionButtonUrl = kErrorUrl;
			alertSettings.ShouldQuit = true;
		} break;
	}

	// Return the alert dialog settings by value.
	return alertSettings;
}

void CKWorkflow::ShowAlertUsing(lua_State *L, const CKWorkflow::AlertSettings &settings)
{
	// Validate.
	if (!L)
	{
		return;
	}

	// Push the corresponding Lua chunk function on the stack.
	luaload_loader_callback(L);

	// Push the given alert dialog settings as a Lua table.
	lua_newtable(L);
	{
		lua_pushstring(L, settings.Title.c_str());
		lua_setfield(L, -2, "title");

		lua_pushstring(L, settings.Message.c_str());
		lua_setfield(L, -2, "message");

		lua_pushstring(L, settings.OkayButtonText.c_str());
		lua_setfield(L, -2, "buttonOK");

		lua_pushstring(L, settings.ActionButtonText.c_str());
		lua_setfield(L, -2, "buttonAction");

		lua_pushstring(L, settings.ActionButtonUrl.c_str());
		lua_setfield(L, -2, "url");

		lua_pushboolean(L, settings.ShouldQuit ? 1 : 0);
		lua_setfield(L, -2, "shouldQuit");
	}

	// Invoke the Lua callback function pushed above.
	Lua::DoCall(L, 1, 0);
}

CKWorkflow::CheckResult CKWorkflow::PassesCheck(std::string productId)
{
	CheckResult retCode;
	if (fLicenseInfo.size() > 0)
	{
		// License information was loaded. Default to "missing product" in case we can't find the requested product key.
		retCode = CKWorkflow::kMissingProduct;
	}
	else
	{
		// No license information was loaded.
		retCode = CKWorkflow::kMissingLicense;
	}

	bool finishedChecking = false;
	for (int productIndex = 0; productIndex < (int)fLicenseInfo.size() && finishedChecking == false; productIndex++)
	{
		if (productId == fLicenseInfo[productIndex].fProductId)
		{
			//Only a local check - which means date could be set back to circumvent this "trial" period
			//For trial we should require "tethering" and get the timestamp from the server
			std::time_t licenseExpiration = GetLicenseExpirationDate(productIndex);
			
			std::time_t licenseCreation = GetLicenseCreationDate(productIndex);
			
			LicenseStatus licenseStatus = GetLicenseStatus(productIndex);
			
			
			if ( -1 == licenseExpiration ||
				 -1 == licenseCreation )
			{
					finishedChecking = false;
					retCode = CKWorkflow::kInvalid;
			}
			else
			{
				std::string productPlatform = GetPlatform(productIndex);
				if (GetCurrentPlatform() == productPlatform )
				{
					if (licenseStatus == CKWorkflow::License_Default)
					{
						finishedChecking = false;
						retCode = CKWorkflow::kMissingLicense;
					}
					else if ( licenseStatus == CKWorkflow::License_Trial )
					{
						
						//If the current time > than the expiration then return false (vulnerable to clock changes)
						time_t currentTime;
						time ( &currentTime );
						if ( currentTime < licenseCreation || currentTime > licenseExpiration )
						{
							finishedChecking = false;
							retCode = CKWorkflow::kTrialExpiration;
						}
						else
						{
							finishedChecking = true;
							retCode = CKWorkflow::kTrialLicense;
						}
					}
					else if ( licenseStatus == CKWorkflow::License_Paid )
					{
						//If the product was built before the license expiration then allow it to proceed
						time_t productCreationTime = GetProductCreationTime();
						
						if ( -1 == productCreationTime )
						{
							finishedChecking = false;
							retCode = CKWorkflow::kInvalid;
						}
						else if ( productCreationTime > licenseExpiration )
						{
							finishedChecking = false;
							retCode = CKWorkflow::kPaidExpiration;
						}
						else
						{
							finishedChecking = true;
							retCode = CKWorkflow::kPaidLicense;
						}
					}
				}
			}
		}
	}
	return retCode;
}

void
CKWorkflow::LogTime(char info[], time_t timeInput)
{
	char buff[20];
	struct tm *timeinfo;
	timeinfo = localtime (&timeInput);
	strftime(buff, sizeof(buff), "%b %d %Y", timeinfo);
	Rtt_TRACE(("%s: %s", info, buff));
}

void
CKWorkflow::Log()
{
	for (int productIndex = 0; productIndex < (int)fLicenseInfo.size(); productIndex++)
	{
		Rtt_TRACE(("LicenseInfo: [%d]", (int)productIndex));
		Rtt_TRACE(("LicenseCode: %d", (int)fLicenseInfo[productIndex].fLicenseCode));
		Rtt_TRACE(("LicenseVersion: %s", fLicenseInfo[productIndex].fLicenseVersion.c_str()));
		Rtt_TRACE(("ProductId: %s", fLicenseInfo[productIndex].fProductId.c_str()));
		Rtt_TRACE(("Platform: %s", fLicenseInfo[productIndex].fPlatform.c_str()));
		Rtt_TRACE(("Build: %s", fLicenseInfo[productIndex].fBuild.c_str()));
		char creation[] = "Creation";
		char expiration[] = "Expiration";
		LogTime(creation, fLicenseInfo[productIndex].fLicenseCreationDate);
		LogTime(expiration, fLicenseInfo[productIndex].fLicenseExpirationDate);
		Rtt_TRACE(("--------------------"));
		
	}
	Rtt_TRACE(("EmailAddress: %s", fEmailAddress.c_str()));
	Rtt_TRACE(("EmailAddress: %s", fLicenseUrl.c_str()));
	Rtt_TRACE(("EmailAddress: %s", fLicenseNotice.c_str()));
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

