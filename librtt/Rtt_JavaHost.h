//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

// JavaHost.h : Java VM
//
#ifndef _H_JavaHost_
#define _H_JavaHost_

#include <string.h>

#if USE_JNI
struct JavaVM_;
typedef JavaVM_ JavaVM;
struct JNIEnv_;
typedef JNIEnv_ JNIEnv;
#endif

namespace Rtt {

class String;

class JavaHost
{
#if USE_JNI
private:
	static JavaVM * sJavaVM;
	static JNIEnv * sJNIEnv;
	static String sJarPath;
#endif

	JavaHost() { }

public:
	
#if USE_JNI
	static void AddJar(const char * const jarPath);
	static void Initialize();
	static void Terminate();
	
	static JNIEnv * GetEnv()
	{
		return sJNIEnv;
	}
	static bool LoadJvm();
#endif

    static bool IsJavaInstalled();

#ifdef Rtt_WIN_ENV
	static const char *GetJdkPath();
	static const char *GetJrePath();
#endif // Rtt_WIN_ENV
};

#if USE_JNI
class JavaHostTerminator
{
	static int sVMCount;
	
public:
	JavaHostTerminator()
	{
		sVMCount++;
	}

	~JavaHostTerminator()
	{
		if ( --sVMCount == 0 )
		{
			JavaHost::Terminate();
		}
	}
};
#endif // USE_JNI

} // namespace Rtt

#endif
