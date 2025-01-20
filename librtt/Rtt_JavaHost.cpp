//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


//
// TODO: Move this into platform/shared (this is not used by librtt)
//

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Rtt_JavaHost.h"

#if USE_JNI
#include "jniUtils.h"
#endif

#ifdef Rtt_WIN_ENV
#include "windows.h"
#include "shellapi.h"  // ShellExecute()
#include "WinString.h"
#include "Resource.h"  // error string ids
#ifdef Rtt_NO_GUI
#include "Rtt_WinConsolePlatform.h"
#else
#include "Interop/ApplicationServices.h"
#endif

#elif Rtt_MAC_ENV
#include <dlfcn.h>
#endif // Rtt_WIN_ENV


namespace Rtt {

#ifdef Rtt_WIN_ENV
#define JAVA_PATH_SEPARATOR ";"
#else
#define JAVA_PATH_SEPARATOR ":"
#endif // Rtt_WIN_ENV

#if USE_JNI

typedef jint (JNICALL *pCREATEJAVAVM)(JavaVM **p_vm, JNIEnv **p_env, void *vm_args);
// static global variables initialized by LoadJvm(), uninitialized by Terminate()
// global to this file, but not class variables so classes including JavaHost.h don't need windows.h
static pCREATEJAVAVM g_pJNI_CreateJavaVM = NULL;

#ifdef Rtt_WIN_ENV
static HMODULE g_hJvmDll = NULL;
#elif Rtt_MAC_ENV
static void *g_pJavaVMLibrary = NULL;
#endif // Rtt_WIN_ENV

JavaVM * JavaHost::sJavaVM = NULL;
JNIEnv * JavaHost::sJNIEnv = NULL;
String JavaHost::sJarPath;
int JavaHostTerminator::sVMCount = 0;

void
JavaHost::AddJar( const char * const jarPath )
{
	Rtt_ASSERT( sJNIEnv == NULL );

	if ( sJarPath.GetString() != NULL )
		sJarPath.Append( JAVA_PATH_SEPARATOR );
	sJarPath.Append( jarPath );
}

void
JavaHost::Initialize()
{
	jint jresult = 0;

	if ( sJNIEnv == NULL ) {
		JavaVMInitArgs vm_args;
		JavaVMOption options[1];
		char jarPathArg[1024];

		sprintf( jarPathArg, "-Djava.class.path=%s", sJarPath.GetString() );
		
		options[0].optionString = jarPathArg;
		
        vm_args.version = JNI_VERSION_1_6;
		
		vm_args.options = options;
		vm_args.nOptions = 1;
		vm_args.ignoreUnrecognized = JNI_FALSE;

        if (LoadJvm())
        {
			jresult = (*g_pJNI_CreateJavaVM)( &sJavaVM, &sJNIEnv, &vm_args );
        }
	}
	
	Rtt_ASSERT( sJNIEnv != NULL );
}


void JavaHost::Terminate()
{
	if ( sJavaVM != NULL )
    {
		jint jresult;
		
		sJavaVM->DetachCurrentThread();
		jresult = sJavaVM->DestroyJavaVM();
		sJavaVM = NULL;
		sJNIEnv = NULL;
	}

#ifdef Rtt_WIN_ENV
	if( NULL != g_hJvmDll )
    {
        FreeLibrary( g_hJvmDll );
        g_hJvmDll = NULL;
    }
#elif Rtt_MAC_ENV
    if( NULL != g_pJavaVMLibrary )
    {
        dlclose( g_pJavaVMLibrary );
        g_pJavaVMLibrary = NULL;
    }
#endif
}
#endif // USE_JNI

bool JavaHost::IsJavaInstalled()
{
#if Rtt_MAC_ENV

    const char *failureText = "No Java runtime present";
    Rtt::String cmdResult;
    FILE *pipeFP = popen("2>&1 /usr/libexec/java_home --task JNI", "r");

    if (pipeFP != NULL)
    {
        char buf[BUFSIZ];

        while (fgets(buf, BUFSIZ, pipeFP) != NULL)
        {
            cmdResult.Append(buf);
        }

        pclose(pipeFP);
    }

    return (strstr(cmdResult.GetString(), failureText) == NULL);

#elif Rtt_WIN_ENV

    return (JavaHost::GetJrePath() != NULL);

#endif
}

#ifdef Rtt_MAC_ENV

// Dynamically load JavaVM.framework
bool JavaHost::LoadJvm()
{
    if (g_pJNI_CreateJavaVM != NULL)
    {
        return true;
    }

    g_pJavaVMLibrary = dlopen("/System/Library/Frameworks/JavaVM.framework/JavaVM", RTLD_LAZY);

    if (g_pJavaVMLibrary != NULL)
    {
        g_pJNI_CreateJavaVM = (pCREATEJAVAVM) dlsym(g_pJavaVMLibrary, "JNI_CreateJavaVM");
    }

    return (g_pJNI_CreateJavaVM != NULL);
}

#elif Rtt_WIN_ENV

#define JAVA_REGISTRY_VERSION     _T("CurrentVersion")

#define JDK_REGISTRY_LOCATION    _T("Software\\JavaSoft\\Java Development Kit")
#define JRE_REGISTRY_LOCATION    _T("Software\\JavaSoft\\Java Runtime Environment")
#define JDK_REGISTRY_LOCATION2    _T("Software\\JavaSoft\\JDK")
#define JRE_REGISTRY_LOCATION2    _T("Software\\JavaSoft\\JRE")
#define JDK_REGISTRY_JAVAHOME    _T("JavaHome")

#if USE_JNI

// Windows code to get JRE settings from registry and dynamically load jvm.dll

#define JRE_REGISTRY_RUNTIMELIB  _T("RuntimeLib")

static HKEY GetVersionKey( LPCTSTR sKey )
{
	HKEY hJreKey = NULL;
    WinString strVersion;
	HKEY hJreVersionKey = NULL;

    // Look for JRE/JDK registry entry
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, sKey, 0, KEY_READ, &hJreKey) == ERROR_SUCCESS)
	{
		DWORD dwType=REG_NONE;
		DWORD dwCount=0;
		// Get Version string size
		LONG lResult = RegQueryValueEx(hJreKey, JAVA_REGISTRY_VERSION, NULL, &dwType,
			NULL, &dwCount);
		if (lResult == ERROR_SUCCESS && dwType == REG_SZ)
		{
            // Get Version string value
			strVersion.Expand( dwCount/sizeof(TCHAR) );
			lResult = RegQueryValueEx(hJreKey, JAVA_REGISTRY_VERSION, NULL, &dwType,
				(LPBYTE)strVersion.GetBuffer(), &dwCount);

			if (lResult == ERROR_SUCCESS && dwType == REG_SZ)
			{
				// Open version key
				if( RegOpenKeyEx(hJreKey, strVersion.GetTCHAR(), 0, KEY_READ,
					&hJreVersionKey) != ERROR_SUCCESS )
				{
                    hJreVersionKey = NULL;  // just in case we get a bad value
				}
			}
		}
		RegCloseKey(hJreKey);
	}

    return hJreVersionKey;
}
// Dynamically load jvm.dll
// function is static member of JavaHost
bool JavaHost::LoadJvm()
{
	WinString registryKeyPath;
	WinString runtimeBinPath;
	WinString runtimeLibraryPath;

	TCHAR buf[MAX_PATH + 100];
	TCHAR sMsg[MAX_PATH];

	// Fetch the path to the Java virtual machine DLL via the registry.
	if (!CopyJavaRuntimeRegistryKeyPathTo(&registryKeyPath))
	{
		return false;
	}
	if (!JavaRegistryLookup(registryKeyPath.GetTCHAR(), JRE_REGISTRY_RUNTIMELIB, &runtimeLibraryPath))
	{
		return false;
	}

	// Load the "jvm.dll" library.
	g_hJvmDll = LoadLibrary(runtimeLibraryPath.GetTCHAR());
	if (NULL == g_hJvmDll)
	{
		DWORD err = GetLastError();

		LoadString(GetModuleHandle(NULL), IDS_COULDNOTLOAD_s_ERR_d, sMsg, MAX_PATH);
		_stprintf_s(buf, MAX_PATH + 100, sMsg, runtimeLibraryPath.GetTCHAR(), err);
		MessageBox(NULL, buf, NULL, MB_OK | MB_ICONEXCLAMATION);

		return false;
	}

	g_pJNI_CreateJavaVM = (pCREATEJAVAVM)GetProcAddress(g_hJvmDll, "JNI_CreateJavaVM");
	if (NULL == g_pJNI_CreateJavaVM)
	{
		LoadString(GetModuleHandle(NULL), IDS_FUNCTION_s_IN_s, sMsg, MAX_PATH);
		_stprintf_s(buf, MAX_PATH + 100, sMsg, _T("JNI_CreateJavaVM"), runtimeLibraryPath.GetTCHAR());
		MessageBox(NULL, buf, NULL, MB_OK | MB_ICONEXCLAMATION);

		return false;
	}

	return true;
}
#endif // USE_JNI

// Look for JRE registry entry, return RuntimeLib
static bool JavaRegistryLookup( LPCTSTR sLocation, LPCTSTR sKey, WinString* pstrValue )
{
	LONG lResult;
    bool bResult = false;

    if( NULL == pstrValue )
        return false;

	HKEY hJreVersionKey = NULL;

	// search for 32 bit registry
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sLocation, 0, KEY_READ, &hJreVersionKey);
    if ((ERROR_SUCCESS == lResult) && (NULL != hJreVersionKey))
	{
		// Get value size
		DWORD dwType=REG_NONE;
		DWORD dwCount=0;
		LONG lResult = RegQueryValueEx(hJreVersionKey, sKey, NULL, &dwType,
			NULL, &dwCount);
		if (lResult == ERROR_SUCCESS && dwType == REG_SZ)
		{
			// Get value
			(*pstrValue).Expand( dwCount/sizeof(TCHAR) );
			lResult = RegQueryValueEx(hJreVersionKey, sKey, NULL, &dwType,
				(LPBYTE)(*pstrValue).GetBuffer(), &dwCount);
			if (lResult == ERROR_SUCCESS && dwType == REG_SZ)
               bResult = true;
		}
		RegCloseKey(hJreVersionKey);
		return bResult;
	}

	// search for 64 bit registry
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sLocation, 0, KEY_READ | KEY_WOW64_64KEY, &hJreVersionKey);
    if ((ERROR_SUCCESS == lResult) && (NULL != hJreVersionKey))
	{
		// Get value size
		DWORD dwType=REG_NONE;
		DWORD dwCount=0;
		LONG lResult = RegQueryValueEx(hJreVersionKey, sKey, NULL, &dwType,
			NULL, &dwCount);
		if (lResult == ERROR_SUCCESS && dwType == REG_SZ)
		{
			// Get value
			(*pstrValue).Expand( dwCount/sizeof(TCHAR) );
			lResult = RegQueryValueEx(hJreVersionKey, sKey, NULL, &dwType,
				(LPBYTE)(*pstrValue).GetBuffer(), &dwCount);
			if (lResult == ERROR_SUCCESS && dwType == REG_SZ)
               bResult = true;
		}
		RegCloseKey(hJreVersionKey);
		return bResult;
	}

	return bResult;
}

static bool CopyJavaRuntimeRegistryKeyPathTo(WinString* pPath)
{
    if (pPath)
	{
        WinString versionName;
		if (JavaRegistryLookup(JRE_REGISTRY_LOCATION, JAVA_REGISTRY_VERSION, &versionName))
		{
			pPath->SetTCHAR(JRE_REGISTRY_LOCATION);
			pPath->Append("\\");
			pPath->Append(versionName.GetUTF8());
			return true;
		}

		if (JavaRegistryLookup(JRE_REGISTRY_LOCATION2, JAVA_REGISTRY_VERSION, &versionName))
		{
			pPath->SetTCHAR(JRE_REGISTRY_LOCATION);
			pPath->Append("\\");
			pPath->Append(versionName.GetUTF8());
			return true;
		}
	}
	return false;
}

static bool CopyJavaDevelopmentKitRegistryKeyPathTo(WinString* pPath)
{
    if (pPath)
	{
        WinString versionName;
		if (JavaRegistryLookup(JDK_REGISTRY_LOCATION, JAVA_REGISTRY_VERSION, &versionName))
		{
			pPath->SetTCHAR(JDK_REGISTRY_LOCATION);
			pPath->Append("\\");
			pPath->Append(versionName.GetUTF8());
			return true;
		}

		if (JavaRegistryLookup(JDK_REGISTRY_LOCATION2, JAVA_REGISTRY_VERSION, &versionName))
		{
			pPath->SetTCHAR(JDK_REGISTRY_LOCATION2);
			pPath->Append("\\");
			pPath->Append(versionName.GetUTF8());
			return true;
		}
	}
	return false;
}

LPCTSTR GetApplicationPath()
{
#ifdef Rtt_NO_GUI
	static std::wstring ret = WinConsolePlatform::GetDirectoryPath();
	return ret.c_str();
#else
	return Interop::ApplicationServices::GetDirectoryPath();
#endif
}

bool CheckDirExists(LPCTSTR dirName)
{
	WIN32_FIND_DATA  data;
	HANDLE handle = FindFirstFile(dirName, &data);
	if (handle != INVALID_HANDLE_VALUE)
	{
		FindClose(handle);
		return (0 != (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	}
	return false;
}

// Fetches the Java Development Kit's JavaHome in registry.
const char *JavaHost::GetJdkPath()
{
	static char s_sJdkJavaHome[MAX_PATH] = "";

    if( s_sJdkJavaHome[0] == '\0' )
	{
		// try to find bunled JRE
		TCHAR buffer[MAX_PATH] = _T("");
		WinString rootPath;
		GetFullPathName(_T("jre"), MAX_PATH, buffer, NULL);
		if (buffer[0] && CheckDirExists(buffer)) {
			rootPath.SetTCHAR(buffer);
			strcpy_s(s_sJdkJavaHome, MAX_PATH, rootPath.GetUTF8());
			return s_sJdkJavaHome;
		}

		rootPath.SetUTF16(GetApplicationPath());
		rootPath.Append("\\jre");
		if (CheckDirExists(rootPath.GetUTF16())) {
			strcpy_s(s_sJdkJavaHome, MAX_PATH, rootPath.GetUTF8());
			return s_sJdkJavaHome;
		}

		rootPath.SetUTF16(GetApplicationPath());
		rootPath.Append("\\..\\..\\..\\..\\jre");
		GetFullPathName(rootPath.GetTCHAR(), MAX_PATH, buffer, NULL);
		if (CheckDirExists(rootPath.GetUTF16())) {
			rootPath.SetTCHAR(buffer);
			strcpy_s(s_sJdkJavaHome, MAX_PATH, rootPath.GetUTF8());
			return s_sJdkJavaHome;
		}

        WinString sRegistryKeyPath;
		WinString sValue;
		if (CopyJavaDevelopmentKitRegistryKeyPathTo(&sRegistryKeyPath))
		{
			if (JavaRegistryLookup(sRegistryKeyPath.GetTCHAR(), JDK_REGISTRY_JAVAHOME, &sValue))
			{
				strcpy_s(s_sJdkJavaHome, MAX_PATH, sValue.GetUTF8());
			}
		}
	}
    return s_sJdkJavaHome;
}

// Fetches the Java Runtime Environment's JavaHome in registry.
const char *JavaHost::GetJrePath()
{
	static char s_sJreJavaHome[MAX_PATH] = "";

    if( s_sJreJavaHome[0] == '\0' )
	{
		// try to find bunled JRE
		TCHAR buffer[MAX_PATH] = _T("");
		WinString rootPath;
		GetFullPathName(_T("jre\\jre"), MAX_PATH, buffer, NULL);
		if (buffer[0] && CheckDirExists(buffer)) {
			rootPath.SetTCHAR(buffer);
			strcpy_s(s_sJreJavaHome, MAX_PATH, rootPath.GetUTF8());
			return s_sJreJavaHome;
		}

		rootPath.SetUTF16(GetApplicationPath());
		rootPath.Append("\\jre\\jre");
		if (CheckDirExists(rootPath.GetUTF16())) {
			strcpy_s(s_sJreJavaHome, MAX_PATH, rootPath.GetUTF8());
			return s_sJreJavaHome;
		}

		rootPath.SetUTF16(GetApplicationPath());
		rootPath.Append("\\..\\..\\..\\..\\jre\\jre");
		GetFullPathName(rootPath.GetTCHAR(), MAX_PATH, buffer, NULL);
		if (CheckDirExists(rootPath.GetUTF16())) {
			rootPath.SetTCHAR(buffer);
			strcpy_s(s_sJreJavaHome, MAX_PATH, rootPath.GetUTF8());
			return s_sJreJavaHome;
		}

        WinString sRegistryKeyPath;
		WinString sValue;
		if (CopyJavaRuntimeRegistryKeyPathTo(&sRegistryKeyPath))
		{
			if (JavaRegistryLookup(sRegistryKeyPath.GetTCHAR(), JDK_REGISTRY_JAVAHOME, &sValue))
			{
				strcpy_s(s_sJreJavaHome, MAX_PATH, sValue.GetUTF8());
			}
		}
	}
    return s_sJreJavaHome;
}

#endif // Rtt_WIN_ENV


}  // namespace Rtt
