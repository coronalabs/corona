//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenWebPopup.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_MKeyValueIterable.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#pragma region Constructors/Destructors
EmscriptenWebPopup::EmscriptenWebPopup()
:	Super()
{
}

EmscriptenWebPopup::~EmscriptenWebPopup()
{
}

#pragma endregion


#pragma region Public Member Functions
void EmscriptenWebPopup::Show(const MPlatform& platform, const char *url)
{
}

bool EmscriptenWebPopup::Close()
{
	return false;
}

void EmscriptenWebPopup::Reset()
{
}

void EmscriptenWebPopup::SetPostParams(const MKeyValueIterable& params)
{
}

int EmscriptenWebPopup::ValueForKey(lua_State *L, const char key[]) const
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

bool EmscriptenWebPopup::SetValueForKey(lua_State *L, const char key[], int valueIndex)
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

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
