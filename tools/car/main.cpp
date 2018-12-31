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

#include "Rtt_Car.cpp"


#if defined(Rtt_WIN_ENV) && defined(_UNICODE)
#include "WinString.h"

int
wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	WinString stringConverter;
	const char **utf8ArgumentArray = NULL;
	int result;

	// Convert the UTF-16 argument strings into UTF-8 strings and copy them into an array.
	if (argc > 0)
	{
		utf8ArgumentArray = (const char**)malloc(argc * sizeof(char*));
		for (int index = 0; index < argc; index++)
		{
			char *utf8String = NULL;
			stringConverter.SetTCHAR(argv[index]);
			if (stringConverter.GetLength() > 0)
			{
				size_t byteLength = strlen(stringConverter.GetUTF8()) + 1;
				utf8String = (char*)malloc(sizeof(char)* byteLength);
				memcpy(utf8String, stringConverter.GetUTF8(), byteLength);
			}
			utf8ArgumentArray[index] = utf8String;
		}
		stringConverter.Clear();
	}

	// Pass the arguments to the archiver.
	result = Rtt_CarMain(argc, utf8ArgumentArray);

	// Delete the UTF-8 array created above.
	if (utf8ArgumentArray)
	{
		for (int index = 0; index < argc; index++)
		{
			const char *utf8String = utf8ArgumentArray[index];
			if (utf8String)
			{
				free((void*)utf8String);
			}
		}
		free(utf8ArgumentArray);
		utf8ArgumentArray = NULL;
	}

	// Return the archiver's result.
	return result;
}

#else

int
main(int argc, const char *argv[])
{
	return Rtt_CarMain(argc, argv);
}

#endif
