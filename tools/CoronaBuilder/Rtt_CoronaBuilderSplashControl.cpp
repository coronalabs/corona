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
