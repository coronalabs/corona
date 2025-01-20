//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_CKWorkflow_H__
#define _Rtt_CKWorkflow_H__


#include <map>
#include <string>
#include <ctime>

// ----------------------------------------------------------------------------
struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

class LuaMap;

class CKWorkflow
{
	public:
		typedef enum
		{
			kMissingLicense = 0,
			kMissingProduct,
			kTrialLicense,
			kTrialExpiration,
			kPaidLicense,
			kPaidExpiration,
			kInvalid,
		}
		CheckResult;

		struct AlertSettings
		{
			std::string Title;
			std::string Message;
			std::string OkayButtonText;
			std::string ActionButtonText;
			std::string ActionButtonUrl;
			bool ShouldQuit;

			AlertSettings()
			:	ShouldQuit(false)
			{
			}
		};

	private:
		//Make sure to map any required enums to the existing
		typedef enum
		{
			License_Default = 0,
			License_Trial = 1,
			License_Paid = 2,
		}
		LicenseStatus;

		typedef struct
		{
			CKWorkflow::LicenseStatus fLicenseCode;
			std::string fLicenseVersion;
			std::time_t fLicenseCreationDate;
			std::time_t fLicenseExpirationDate;
			std::string fAppBundleId;
			std::string fProductId;
			std::string fPlatform;
			std::string fBuild;
	
		} LicenseInfo;

	public:
		CKWorkflow( );
		virtual ~CKWorkflow();

	public:
		void Init( lua_State* L);
		int GetNumProducts();
	
	public:
		LicenseStatus GetLicenseStatus( int productIndex );
		std::string GetLicenseVersion( int productIndex );
		std::time_t GetLicenseCreationDate( int productIndex );
		std::time_t GetLicenseExpirationDate( int productIndex );
		std::string GetAppBundleId( int productIndex );
		std::string GetProductId( int productIndex );
		std::string GetPlatform( int productIndex );
		std::string GetBuild( int productIndex );
		CKWorkflow::CheckResult PassesCheck( std::string productId );
		bool CompareValues(LuaMap &metaMap);
		
		//TODO virtualize and override for specific implementations
		void DidCheck(lua_State *L, CKWorkflow::CheckResult r, std::string productId);

		/// Creates the default alert dialog configuration to be used with the given license validation result and product.
		/// <p>
		/// This method should be used when the caller wants to fetch the default alert messaging for the validation result
		/// and wants to customize the messaging to provide more details for the platform or product. The caller would then
		/// pass the returned settings object to the ShowAlertUsing() method to display an alert dialog with those settings.
		/// @param r The license validation result such as kMissingLicense, kTrial, kPaidExpired, etc.
		///          This defines what message is to be used in the alert and what URL the action button should invoke.
		/// @param productId The unique string name of the product that is being authorized.
		/// @return Returns an alert dialog configuration object which assigned the title, message, and buttons to
		///         their defaults for the given parameters. This configuration object can then be passed to the
		///         ShowAlertUsing() method to display an alert dialog with those settings.
		AlertSettings CreateAlertSettingsFor(CKWorkflow::CheckResult r, const std::string &productId);

		/// Displays an alert dialog using the given settings.
		/// @param L Lua state pointer needed to display the alert dialog and handle the button presses.
		/// @param settings Provides the alert dialog's title, message, and button configurations.
		void ShowAlertUsing(lua_State *L, const AlertSettings &settings);

	private:
		int GetProductIndexById( std::string productId );
		void LogTime(char info[], time_t timeInput);
		void Log();


		//Private helper
		LicenseStatus StringToStatus( const char *str );
		time_t GetProductCreationTime();
		std::string GetCurrentPlatform();
		std::string GetProductBuild();
		
	private:
		std::string fEmailAddress;
		std::string fLicenseUrl;
		std::string fLicenseNotice;
		std::map<int,LicenseInfo> fLicenseInfo;

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneCKWorkflow_H__
