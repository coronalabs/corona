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

#include "Rtt_DependencyLoader.h"
#include "Rtt_CKWorkflow.h"
#include "Rtt_CKWorkFlowConstants.h"
#include "Rtt_DependencyUtils.h"
#include "Core/Rtt_Assert.h"
#include "Core/Rtt_Build.h"
#include "Rtt_Runtime.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaContainer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
	int luaload_loader_ccdata(lua_State* L);
}


namespace Corona
{

	//License file public key (for license.ccdata)
static const char pKey64IOS[] = "MIIDUDCCAjgCCQCSIyWX9o7eWjANBgkqhkiG9w0BAQUFADBqMQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZvcm5pYTESMBAGA1UEBxMJUGFsbyBBbHRvMRkwFwYDVQQKExBDb3JvbmEgTGFicyBJbmMuMRcwFQYDVQQDEw5jb3JvbmFsYWJzLmNvbTAeFw0xNDAzMTMyMTU5MDZaFw0yNDAzMTAyMTU5MDZaMGoxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpDYWxpZm9ybmlhMRIwEAYDVQQHEwlQYWxvIEFsdG8xGTAXBgNVBAoTEENvcm9uYSBMYWJzIEluYy4xFzAVBgNVBAMTDmNvcm9uYWxhYnMuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuH0VFxky2yjY4HHDyQrAYw9KAT92EevkBOVATeaytPBqBWSJIOCdjqxCMhHIOCrEnU4LmOlLbflHWP91zeTYR7t90sTXhAkzOGc0DRt+6wkDl08jk2dObV0cDzOyL10LhFpWcBwwuP2UuT1hSEXWY5w0/hk1i2OlFg7KNtG5u5ZY6xtiLSHp6g3LAY9xUDT5K2JWeQMMuG+HGvky3XJANZwc7gXWskRtwgpwL4uB1crtGdv+x4pCOwHswH6K6mJkZF1GMBKM1z2qYDadO2h4jBKB3VIww0MSReJjEx/6UmKWPXBvnfx8PU8GPgJy9HJHURHLB6Em1SCF7MQHGP0TNwIDAQABMA0GCSqGSIb3DQEBBQUAA4IBAQAos2wNU0che4t0XsnXZk/TPKi6OuQPDsiAmZ3pGxZeA4tRpT86arDipZzr8mqqQYOV4ow4z/JDQNY7+SlGlh58NbsuTBkZeo3u+oxUMTxnm0FJW8lda3L9nkvQNwTrbKiVKiXyXMJSxjB27tYtKpt3QYXbd+g/HhMQVNK7RaqplWGAQP0jdGm+qqWwVpNDfVW7LT8emoSGULIv+3Z/Qxi3zZkEOdmH2nUg4Qg5AOyZGRB5FcE2O253rb0+QMJqdEkZsLlHPb/ZwdAyOatmaKw62LNaOwf9Spi+MT9zHCH1586T4bXvEV5oYjqT4jXEdAtLP/wyw+p8mx3xehKvnuCl";

#ifndef Rtt_IPHONE_ENV
	
static const char pKey64Android[] = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuH0VFxky2yjY4HHDyQrAYw9KAT92EevkBOVATeaytPBqBWSJIOCdjqxCMhHIOCrEnU4LmOlLbflHWP91zeTYR7t90sTXhAkzOGc0DRt+6wkDl08jk2dObV0cDzOyL10LhFpWcBwwuP2UuT1hSEXWY5w0/hk1i2OlFg7KNtG5u5ZY6xtiLSHp6g3LAY9xUDT5K2JWeQMMuG+HGvky3XJANZwc7gXWskRtwgpwL4uB1crtGdv+x4pCOwHswH6K6mJkZF1GMBKM1z2qYDadO2h4jBKB3VIww0MSReJjEx/6UmKWPXBvnfx8PU8GPgJy9HJHURHLB6Em1SCF7MQHGP0TNwIDAQAB";

static const char pKey64WindowsPhone[] = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuH0VFxky2yjY4HHDyQrAYw9KAT92EevkBOVATeaytPBqBWSJIOCdjqxCMhHIOCrEnU4LmOlLbflHWP91zeTYR7t90sTXhAkzOGc0DRt+6wkDl08jk2dObV0cDzOyL10LhFpWcBwwuP2UuT1hSEXWY5w0/hk1i2OlFg7KNtG5u5ZY6xtiLSHp6g3LAY9xUDT5K2JWeQMMuG+HGvky3XJANZwc7gXWskRtwgpwL4uB1crtGdv+x4pCOwHswH6K6mJkZF1GMBKM1z2qYDadO2h4jBKB3VIww0MSReJjEx/6UmKWPXBvnfx8PU8GPgJy9HJHURHLB6Em1SCF7MQHGP0TNwIDAQAB";

#endif
	
static const char pKeyData64[] = "";


const char *DependencyLoader::GetK()
{
	const char* result;
	#if defined( Rtt_IPHONE_ENV )
		result = pKey64IOS;
	#elif defined( Rtt_ANDROID_ENV )
		result = pKey64Android;
	#elif defined( Rtt_WIN_PHONE_ENV )
		result = pKey64WindowsPhone;
	#else
		result = pKeyData64;
	#endif
	return result;
}

// ----------------------------------------------------------------------------

//Corona ccdata file preloader
int
DependencyLoader::CCDataLoader( lua_State *L )
{

	int result = 0;
	int oldTop = lua_gettop( L );

	const char *libName = luaL_checkstring( L, 1 );
	
	// [Lua] return loader_ccdata( libName, & Check );
	
	// [1] Convert json to Lua table
	Rtt::luaload_loader_ccdata( L ); // this pushes the corresponding Lua chunk (function) on the stack
	lua_pushstring( L, libName );
	lua_pushcfunction( L, & DependencyUtils::DecodeBase64 );

	//Push closure to include the proper key data
	lua_pushstring( L, pKeyData64 );
	lua_pushcclosure( L, & DependencyUtils::Check, 1 );
	
	int status = Lua::DoCall( L, 3, 1 );
	
	if ( 0 == status )
	{
		//Top of the stack is now a table with a chunk in the "data" field
		int tableIndex = lua_gettop(L);
		lua_getfield(L,tableIndex, kData);
		
		int dataIndex = lua_gettop( L );
		if ( LUA_TFUNCTION == lua_type( L, dataIndex ) )
		{
			lua_remove(L, tableIndex);
			result = 1;
		}
		else
		{
			lua_settop( L, oldTop );
			lua_pushnil( L );
			result = 1;
		}
		
	}

	return result;
}

//Called by CoronaKit to verify licensing
bool
DependencyLoader::CCDependencyCheck( const Rtt::Runtime& sender )
{
	lua_State *L = sender.VMContext().L();

	bool result = false;

	int top = lua_gettop(L);
	{
		const char *productStringId = "coronacards";
		const char *libName = "license";
		
		// [1] Convert json to Lua table
		Rtt::luaload_loader_ccdata( L ); // this pushes the corresponding Lua chunk (function) on the stack
		lua_pushstring( L, libName );
		lua_pushcfunction( L, & DependencyUtils::DecodeBase64 );
		
		
		lua_pushstring( L, GetK() );
		lua_pushcclosure( L, & DependencyUtils::Check, 1 );
		
		int status = Lua::DoCall( L, 3, 1 );
		
		if (status == 0)
		{
			//Top of the stack is now a table with a chunk in the "data" field
			int tableIndex = lua_gettop(L);
			if ( LUA_TTABLE == lua_type( L, tableIndex ) )
			{
				lua_getfield(L,tableIndex, kData);
				
				status = Lua::DoCall( L, 0, 1 );
				if ( 0 == status )
				{
					int dataIndex = lua_gettop( L );
					if ( LUA_TTABLE == lua_type( L, dataIndex ) )
					{
						Rtt::CKWorkflow workFlow;
						workFlow.Init(L);
				
						lua_getfield(L,tableIndex, kMetaData);
						int metaIndex = lua_gettop( L );

						if ( LUA_TTABLE == lua_type( L, metaIndex ) )
						{
							Rtt::LuaMap metaMap(L, metaIndex);
							
							bool compareResult = workFlow.CompareValues(metaMap);
							if ( compareResult )
							{
								Rtt::CKWorkflow::CheckResult r = workFlow.PassesCheck(productStringId);
								
								//Show native messaging
								workFlow.DidCheck(L, r, productStringId);
								
								//Only valid options to return true
								if ( Rtt::CKWorkflow::kTrialLicense == r )
								{
									sender.SetShowingTrialMessage(true);
									result = true;
								}
								if ( Rtt::CKWorkflow::kPaidLicense == r )
								{
									result = true;
								}
							}
							else
							{
								Rtt::CKWorkflow workFlow;
								workFlow.DidCheck(L, Rtt::CKWorkflow::kInvalid, productStringId);
							}
							
							lua_pop( L, 2 );
						}
					}
				}
			}
			else
			{
				// The Lua script failed to load the license file. Fetch an error message explaining why.
				const char *errorMessage = NULL;
				if ( LUA_TSTRING == lua_type( L, tableIndex ) )
				{
					errorMessage = lua_tostring(L, tableIndex);
				}
				if (Rtt_StringIsEmpty(errorMessage))
				{
					errorMessage = "License file not found.";
				}

				// Log the license error message.
				Rtt_LogException("ERROR: %s", errorMessage);

				// Display an alert dialog for the license error.
				Rtt::CKWorkflow workFlow;
				Rtt::CKWorkflow::AlertSettings alertSettings =
						workFlow.CreateAlertSettingsFor(Rtt::CKWorkflow::kMissingLicense, productStringId);
				alertSettings.Message = errorMessage;
				workFlow.ShowAlertUsing(L, alertSettings);
			}
//			lua_remove(L, tableIndex);
		}
	}
	lua_settop(L,top);

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

