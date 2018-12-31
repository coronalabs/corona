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
