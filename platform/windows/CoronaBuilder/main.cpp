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

// #pragma comment(lib, "Lua.Library.Win32\\lua.lib")

#include "stdafx.h"

#include "Core/Rtt_Build.h"

#include "Rtt_CoronaBuilder.h"
#include "Rtt_JavaHost.h"
#include "Rtt_WinConsolePlatform.h"
#include "Rtt_WinPlatformServices.h"

#include "Rtt_String.h"

static bool InitJavaPaths(CString resourceDir);


int main( int argc, const char *argv[] )
{
	using namespace Rtt;

	int result = 0;                                                                                                                                                                                                                                                                                      
	CString resourceDir = _wgetenv(_T("CORONA_PATH"));

	if (resourceDir.GetLength() == 0)
	{
		fprintf(stderr, "CoronaBuilder: cannot find a setting for CORONA_PATH.  Install Corona first\n");

		exit(1);
	}

	resourceDir.TrimRight(_T("\\"));
	resourceDir.Append(_T("\\Resources"));

	// NEEDSWORK - technically this only needs to be done for an actual build
	if (!InitJavaPaths(resourceDir))
	{
		return 1;
	}

	//Interop::RuntimeEnvironment::CreationSettings settings;
	//settings.IsRuntimeCreationEnabled = false;
	//Interop::RuntimeEnvironment *platformPointer = Interop::RuntimeEnvironment::CreateUsing(settings).GetPointer();
	//Interop::SimulatorRuntimeEnvironment *platformPointer = result->GetPlatform();
	WinConsolePlatform *platform = new WinConsolePlatform;
	Rtt::WinPlatformServices *services = new Rtt::WinPlatformServices(*platform);
	CoronaBuilder builder( *platform, *services );
	result = builder.Main( argc, argv );

	delete services;
	delete platform;

	return result;
}

// CheckPathExists - return true if the file/directory exists.
// Make sure paths don't have trailing backslashes
static bool
CheckPathExists(LPCTSTR path)
{
	WIN32_FIND_DATA  data;
	HANDLE handle = FindFirstFile(path, &data);
	if (handle != INVALID_HANDLE_VALUE)
	{
		FindClose(handle);
		return true;
	}
	return false;
}


// CheckJarPath - see if the given path exists and show user a message if it doesn't
static bool
CheckJarPath(CString path)
{
	if (!CheckPathExists(path))
	{
		fprintf(stderr, "CoronaBuilder: cannot find JDK component '%s'.  You may need to re-install Java\n", path);

		return false;
	}

	return true;
}


// InitJavaPaths - check if jarsigner.exe can be found, and warn the user if not.
// If JAVA_HOME is set to the JDK and %JAVA_HOME%\bin is in the path, load the
// required jars (ListKeyStore and ant).
static bool
InitJavaPaths(CString resourceDir)
{
	static bool bInitialized = false;

	// Only do this successfully once (set to true when jarsigner.exe found)
	if (bInitialized)
	{
		return true;
	}

	// Fetch the path to the JDK.
	auto utf8JdkPath = Rtt::JavaHost::GetJdkPath();
	if (nullptr == utf8JdkPath)
	{
		return false;
	}

	// Look for jarsigner.exe in PATH
	WinString jarSignerFilePath;
	jarSignerFilePath.SetUTF8(utf8JdkPath);
	if ((jarSignerFilePath.GetLength() > 0) && !jarSignerFilePath.EndsWith("\\"))
	{
		jarSignerFilePath.Append("\\");
	}
	jarSignerFilePath.Append("bin\\jarsigner.exe");
	int result = SearchPath(NULL, jarSignerFilePath.GetTCHAR(), NULL, 0, NULL, NULL);
	if (!result)
	{
		fprintf(stderr, "CoronaBuilder: could not find the Java Development Kit (JDK).  This is required in order to build for Android.  \n\nIt can be found at http://www.oracle.com/technetwork/java/javase/downloads/index.html\n");
	}
	else
	{
		bInitialized = true;
	}

	// Set JAVA_PATH and PATH environment variables for this process to reference JDK and JRE directories.
	// These are needed by ANT to build apps for Android and to run JAR files used by this app.
	if (bInitialized)
	{
		// Set the JAVA_HOME environment variable.
		WinString stringConverter;
		stringConverter.SetUTF8(utf8JdkPath);
		::SetEnvironmentVariableW(L"JAVA_HOME", stringConverter.GetUTF16());

		// Add to the PATH environment variable.
		stringConverter.SetUTF8(utf8JdkPath);
		if (stringConverter.GetLength() > 0)
		{
			std::wstring newUtf16PathString(L"");

			// Add the JDK's bin directory to the PATH.
			// This ensures that the expected JDK tools are used by ANT.
			if (!stringConverter.EndsWith("\\"))
			{
				stringConverter.Append("\\");
			}
			stringConverter.Append("bin;");
			newUtf16PathString += stringConverter.GetUTF16();

			// Add the JRE's bin directory to the PATH.
			// This is needed by the "jvm.dll" in case its dependencies could not be found
			// in the Windows system directory, which sometimes happens after a Java update.
			auto utf8JrePath = Rtt::JavaHost::GetJrePath();
			if (utf8JrePath)
			{
				stringConverter.SetUTF8(utf8JrePath);
				if (!stringConverter.EndsWith("\\"))
				{
					stringConverter.Append("\\");
				}
				stringConverter.Append("bin;");
				newUtf16PathString += stringConverter.GetUTF16();
			}

			// Fetch the system's current PATH string and append it to the end of the new PATH string.
			// Note: It's very important the above paths are at the beginning of the PATH in case the current system
			//       PATH contains references to incompatible JDK/JRE directories, such as 64-bit versions.
			auto pathCharacterCount = ::GetEnvironmentVariableW(L"PATH", nullptr, 0);
			if (pathCharacterCount > 0)
			{
				pathCharacterCount++;
				auto utf16Buffer = new wchar_t[pathCharacterCount];
				utf16Buffer[0] = L'\0';
				::GetEnvironmentVariableW(L"PATH", utf16Buffer, pathCharacterCount);
				newUtf16PathString += utf16Buffer;
				delete[] utf16Buffer;
			}

			// Apply the new PATH settings to this application process.
			::SetEnvironmentVariableW(L"PATH", newUtf16PathString.c_str());
		}
	}

#if USE_JNI
	// Load JAR files needed to run ANT and read key stores.
	if (bInitialized)
	{
		CString jarPath;
		WinString strJarPath;

		jarPath = resourceDir + _T("\\ListKeyStore.jar");
		if (!CheckJarPath(jarPath))
			return false;
		strJarPath.SetTCHAR(jarPath);
		Rtt::JavaHost::AddJar(strJarPath.GetUTF8());

		jarPath = resourceDir + _T("\\ant.jar");
		if (!CheckJarPath(jarPath))
			return false;
		strJarPath.SetTCHAR(jarPath);
		Rtt::JavaHost::AddJar(strJarPath.GetUTF8());

		jarPath = resourceDir + _T("\\ant-launcher.jar");
		if (!CheckJarPath(jarPath))
			return false;
		strJarPath.SetTCHAR(jarPath);
		Rtt::JavaHost::AddJar(strJarPath.GetUTF8());

		jarPath = resourceDir + _T("\\AntInvoke.jar");
		if (!CheckJarPath(jarPath))
			return false;
		strJarPath.SetTCHAR(jarPath);
		Rtt::JavaHost::AddJar(strJarPath.GetUTF8());
	}
#endif

	return bInitialized;
}


// ----------------------------------------------------------------------------
