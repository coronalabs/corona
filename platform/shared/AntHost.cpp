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

#if USE_JNI

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "AntHost.h"
#include "jniUtils.h"
#include "Rtt_JavaHost.h"

// Static member
jHashMapParam *	AntHost::myHashMapParam = NULL;

AntHost::~AntHost()
{
	// delete myHashMapParam;
}

void 
AntHost::SetProperty( const char * key, const char * value )
{
	if ( !Rtt::JavaHost::GetEnv() )
		return;

	if ( myHashMapParam == NULL )
	{
		myHashMapParam = new jHashMapParam( Rtt::JavaHost::GetEnv() );
	}

	myHashMapParam->put( key, value );
}

// http://onjava.com/pub/a/onjava/2002/07/24/antauto.html?page=1

/**
 * Call an ant task by invoking the wrapper AntInvoke.
 *
 * @return		-1 on failure, -2 on massive failure, 0 on ant failure, and 1 on success.
 */
int 
AntHost::AntCall( const char * buildFilePath, const char * target, int logLevel, Rtt::String * result )
{
	JNIEnv * env = Rtt::JavaHost::GetEnv();
	Rtt_ASSERT( env );

	if ( !env || myHashMapParam == NULL )
		return -2;

	static const char *		kAntInvoke = "com/ansca/util/AntInvoke";
	int resultCode = -1;

	jclass clazz = env->FindClass( kAntInvoke );

	if (logLevel > 0)
	{
		Rtt_TRACE_SIM(("AntCall: buildFilePath: %s; target: %s; logLevel %d\n", buildFilePath, target, logLevel ));
	}

	//	public class com.ansca.util.AntInvoke {
	//  public com.ansca.util.AntInvoke();
	//  descriptor: ()V
	//
	//  protected static java.lang.String runAntTask(java.lang.String, java.lang.String, int, java.util.Map<java.lang.String, java.lang.String>);
	//  descriptor: (Ljava/lang/String;Ljava/lang/String;ILjava/util/Map;)Ljava/lang/String;
	//  }
	if ( clazz != NULL ) {
		jmethodID mid = env->GetStaticMethodID( clazz, 
			"runAntTask", 
			"(Ljava/lang/String;Ljava/lang/String;ILjava/util/Map;)Ljava/lang/String;" );
		
		if ( mid != NULL ) {
			jstringParam pathJ( env, buildFilePath );
			jstringParam targetJ( env, target );
			
			if ( pathJ.isValid() && targetJ.isValid() && myHashMapParam->isValid() )
			{
				// Execute the ant task.
				// Will return an error message if the ant task failed.
				// Will return an empty string if the ant task succeeded.
				jobject objectResult = env->CallStaticObjectMethod(
											clazz, mid, pathJ.getValue(), targetJ.getValue(), logLevel,
											myHashMapParam->getHashMapObject());
				if (env->ExceptionCheck())
				{
					// Java exception occurred. Fetch its error message.
					jthrowable exception = env->ExceptionOccurred();
					env->ExceptionClear();
					result->Set("Java exception occurred.");
					jclass exceptionClass = env->GetObjectClass(exception);
					mid = env->GetMethodID(exceptionClass, "toString", "()Ljava/lang/String;");
					objectResult = env->CallObjectMethod(exception, mid);
					if (objectResult)
					{
						jstringResult stringResult(env, (jstring)objectResult);
						if (stringResult.isValidString())
						{
							result->Set(stringResult.getUTF8());
						}
					}
					resultCode = 0;
				}
				else if (objectResult != NULL)
				{
					// Copy the returned Java string message.
					// If the message is empty, then the ant task was executed successfully.
					resultCode = 1;
					jstringResult stringResult(env, (jstring)objectResult);
					if (stringResult.isValidString())
					{
						result->Set(stringResult.getUTF8());
						if (result->IsEmpty() == false)
						{
							resultCode = 0;
						}
					}
				}
			}
		}
	}

	return resultCode;
}

#endif // USE_JNI
