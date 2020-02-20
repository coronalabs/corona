//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneCKWorkflow_H__
#define _Rtt_IPhoneCKWorkflow_H__


#include <map>
#include <string>
#include <ctime>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------



class IPhoneCKWorkflow
{
	private:
	//Make sure to map any required enums to the existing
	typedef enum
	{
		ID_TRIAL = 0,
		IO_PAID = -1,
	} LicenseCode;

	typedef struct
	{
		IPhoneCKWorkflow::LicenseCode fLicenseCode;
		std::string fLicenseVersion;
		std::time_t fLicenseCreationDate;
		std::time_t fLicenseExpirationDate;
		std::string fAppBundleId;
		std::string fEmailAddress;
		std::string fProductId;
	
	} LicenseInfo;


	public:
		IPhoneCKWorkflow( );
		virtual ~IPhoneCKWorkflow();

	public:
		void Decrypt( std::string encryptedData );
		int GetNumProducts();
	
	public:
		LicenseCode GetLicenseType( int productIndex );
		std::string GetLicenseVersion( int productIndex );
		std::time_t GetLicenseCreationDate( int productIndex );
		std::time_t GetLicenseExpirationDate( int productIndex );
		std::string GetAppBundleId( int productIndex );
		std::string GetEmailAddress( int productIndex );
		std::string GetProductId( int productIndex );
		
	private:
		int GetProductIndexById( std::string productId );
		bool PassesTrialModeCheck( std::string productId, std::time_t productCreationTime );
		bool PassesPaidModeCheck(std::string productId, std::time_t productCreationTime);
		
	private:
		std::map<int,LicenseInfo> fLicenseInfo;

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneCKWorkflow_H__
