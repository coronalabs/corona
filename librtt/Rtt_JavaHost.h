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
