//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Rtt_CoronaBuilderSplashControl.h"
#include "Rtt_Data.h"
#include "Rtt_FileSystem.h"
#include "Rtt_Archive.h"

namespace Rtt
{

const unsigned char *Rtt_GetLuaByteCode__CoronaSetup_android(size_t *len);
const unsigned char *Rtt_GetLuaByteCode__CoronaSetup_ios(size_t *len);

#ifdef WIN32
const unsigned char *Rtt_GetLuaByteCode__CoronaSetup_ios(size_t *len) 
{
	printf("ERROR: Cannot build iOS app on Windows\n");
	return NULL; 
}
#endif

bool
Rtt_CoronaBuilderAddSplashControl(Rtt::String &carFilename)
{
	size_t bytecodeLen = 0;
	const unsigned char *bytecode = NULL;

#ifdef WIN32
	char tmpDirTemplate[_MAX_PATH];
	const char* tmp = getenv("TMP");
	if (tmp == NULL)
	{
		tmp = getenv("TEMP");
	}

	if (tmp)
	{
		_snprintf(tmpDirTemplate, sizeof(tmpDirTemplate), "%s\\CBASCXXXXXX", tmp);
	}
	else
	{
		strcpy(tmpDirTemplate, "\\tmp\\CBASCXXXXXX");
	}
#else
	char tmpDirTemplate[] = "/tmp/CBASCXXXXXX";
#endif

	const char *tmpDirName = Rtt_MakeTempDirectory(tmpDirTemplate);
	Rtt::String tmpFilename;

	if (Rtt_StringEndsWith(carFilename, ".corona-archive"))
	{
		bytecode = Rtt_GetLuaByteCode__CoronaSetup_ios(&bytecodeLen);
	}
	else
	{
		bytecode = Rtt_GetLuaByteCode__CoronaSetup_android(&bytecodeLen);
	}

	Rtt::Data<const unsigned char> bytecodeData(bytecode, (int)bytecodeLen);

	tmpFilename.Set(tmpDirName);
	tmpFilename.AppendPathComponent("_CoronaSetup.lu");

	if (Rtt_WriteDataToFile(tmpFilename, bytecodeData) != bytecodeLen)
	{
		// print an error?
		return false;
	}

	const char *srcPaths[1] = { tmpFilename };
	Archive::Serialize( carFilename, 1, srcPaths );

	return true;
}

} // namespace Rtt
