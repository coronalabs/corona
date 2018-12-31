// ----------------------------------------------------------------------------
// 
// Rtt_WinRTWebPopup.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Rtt_WinRTWebPopup.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_Lua.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_LuaLibSystem.h"
#	include "Rtt_MKeyValueIterable.h"
#	include "Rtt_Runtime.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTWebPopup::WinRTWebPopup()
:	Super()
{
}

WinRTWebPopup::~WinRTWebPopup()
{
}

#pragma endregion


#pragma region Public Member Functions
void WinRTWebPopup::Show(const MPlatform& platform, const char *url)
{
}

bool WinRTWebPopup::Close()
{
	return false;
}

void WinRTWebPopup::Reset()
{
}

void WinRTWebPopup::SetPostParams(const MKeyValueIterable& params)
{
}

int WinRTWebPopup::ValueForKey(lua_State *L, const char key[]) const
{
	Rtt_ASSERT(key);

	int result = 1;

	if (strcmp("baseUrl", key) == 0)
	{
	}
	else if (strcmp("hasBackground", key) == 0)
	{
	}
	else if (strcmp("autoCancel", key) == 0)
	{
	}
	else
	{
		result = 0;
	}

	return result;
}

bool WinRTWebPopup::SetValueForKey(lua_State *L, const char key[], int valueIndex)
{
	Rtt_ASSERT(key);

	bool result = true;

	if (strcmp("baseUrl", key) == 0)
	{
#if 0
		if ( lua_isstring( L, valueIndex ) )
		{
			Rtt_ASSERT( MPlatform::kUnknownDir == fBaseDirectory );

			Rtt_Allocator * allocator = LuaContext::GetRuntime( L )->GetAllocator();
			fBaseUrl =  Rtt_NEW( allocator, String( allocator ) );
			fBaseUrl->Set( lua_tostring( L, valueIndex ) );
		}
		else if ( lua_islightuserdata( L, valueIndex ) )
		{
			Rtt_ASSERT( ! fBaseUrl );

			fBaseDirectory = (MPlatform::Directory)EnumForUserdata(
				LuaLibSystem::Directories(),
				lua_touserdata( L, valueIndex ),
				MPlatform::kNumDirs,
				MPlatform::kUnknownDir );
		}
#endif
	}
	else if (strcmp("hasBackground", key) == 0)
	{
	}
	else if (strcmp("autoCancel", key) == 0)
	{
	}
	else
	{
		result = false;
	}

	return result;
}

#pragma endregion

} // namespace Rtt
