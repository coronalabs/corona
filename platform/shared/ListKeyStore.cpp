//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Rtt_JavaHost.h"
#include "Rtt_FileSystem.h"
#include "ListKeyStore.h"

#include <string>
#include <vector>

#if Rtt_WIN_ENV
#include "Interop/Ipc/CommandLine.h"
#endif

#if USE_JNI
#include "jniUtils.h"
#endif

#ifndef Rtt_LINUX_ENV
const char *ReplaceString(const char *c_subject, const char * c_search, const char * c_replace)
{
	std::string subject = c_subject;
	std::string search = c_search;
	std::string replace = c_replace;

	ReplaceString(subject, search, replace);

	return strdup(subject.c_str());
}

void ReplaceString(std::string& subject, const std::string& search, const std::string& replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}
#endif

ListKeyStore::ListKeyStore() : myAliases( NULL ), myCount( 0 )
{
#if USE_JNI
#if !defined(Rtt_MAC_ENV) && !defined(Rtt_LINUX_ENV)
	Rtt::JavaHost::Initialize();
#endif
#endif
}

ListKeyStore::~ListKeyStore()
{
	if ( myAliases != NULL )
	{
		for ( int i = 0; i < myCount; i++ )
		{
			delete myAliases[i];
		}
		delete myAliases;
	}
}

std::string
ListKeyStore::EscapeArgument(std::string arg)
{
	std::string result = arg;

#if defined(Rtt_MAC_ENV ) || defined(Rtt_LINUX_ENV)

	// On macOS escape shell special characters in the strings by replacing single quotes with "'\''" and
	// then enclosing in single quotes
	ReplaceString(result, "'", "'\\''");	// escape single quotes
	result = "'" + result + "'";

#else

	// On Windows escape shell special characters in the strings by replacing double quotes with "\"" and
	// then enclosing in double quotes
	ReplaceString(result, "\"", "\\\"");	// escape double quotes
	result = "\"" + result + "\"";

#endif

	return result;
}

//
// ListKeyStore::IsValidKeyStore
//
// Attempting to list the keystore using the given password will tell us if the password is good
//
bool
ListKeyStore::IsValidKeyStore( const char * keyStore, const char * password )
{
	char cmdBuf[20480];
	std::string keystoreStr = EscapeArgument(keyStore);
	std::string passwordStr = EscapeArgument(password);

#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)

	const char kCmdFormat[] = "JAVA_TOOL_OPTIONS='-Duser.language=en' /usr/bin/keytool -list -keystore %s -storepass %s -rfc";

	snprintf(cmdBuf, sizeof(cmdBuf), kCmdFormat, keystoreStr.c_str(), passwordStr.c_str() );

	return system( cmdBuf );

#else // Windows

	std::string keytoolPath;
	std::string jdkPath = Rtt::JavaHost::GetJdkPath();

	keytoolPath = jdkPath + "\\bin\\keytool.exe";

	const char kCmdFormat[] = "\"%s\" -list -keystore %s -storepass %s -rfc";

	snprintf(cmdBuf, sizeof(cmdBuf), kCmdFormat, keytoolPath.c_str(), keystoreStr.c_str(), passwordStr.c_str() );

	Interop::Ipc::CommandLine::SetOutputCaptureEnabled(false);
	Interop::Ipc::CommandLineRunResult result = Interop::Ipc::CommandLine::RunUntilExit(cmdBuf);

	return result.GetExitCode();

#endif

#if USE_JNI
	Rtt_ASSERT( Rtt::JavaHost::GetEnv() );

	if ( !Rtt::JavaHost::GetEnv() )
		return KeyStoreStatusBad5;

	if ( Rtt::JavaHost::GetEnv() != NULL )
	{
		static const char *		kListKeyStore = "com/ansca/util/ListKeyStore";

		jclass clazz = Rtt::JavaHost::GetEnv()->FindClass( kListKeyStore );

		if ( clazz != NULL )
		{
			jmethodID mid = Rtt::JavaHost::GetEnv()->GetStaticMethodID( clazz,
			                "isKeyStoreValid",
			                "(Ljava/lang/String;Ljava/lang/String;)I" );

			if ( mid != NULL )
			{
				jstringParam keyStoreJ( Rtt::JavaHost::GetEnv(), keyStore );
				jstringParam passwordJ( Rtt::JavaHost::GetEnv(), password );

				if ( keyStoreJ.isValid() && passwordJ.isValid() )
				{
					jint jresult;

					jresult = Rtt::JavaHost::GetEnv()->CallStaticIntMethod( clazz, mid, keyStoreJ.getValue(), passwordJ.getValue() );

					return jresult;
				}
			}
		}
	}
	return KeyStoreStatusBad5;
#endif // USE_JNI
}

bool
ListKeyStore::GetAliasList( const char * keyStore, const char * password )
{
	char cmdBuf[20480];
	std::string keystoreStr = EscapeArgument(keyStore);
	std::string passwordStr = EscapeArgument(password);

#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)

	const char kCmdFormat[] = "JAVA_TOOL_OPTIONS='-Duser.language=en' /usr/bin/keytool -list -keystore %s -storepass %s -rfc | sed -n 's/^Alias name: //p'";

	snprintf(cmdBuf, sizeof(cmdBuf), kCmdFormat, keystoreStr.c_str(), passwordStr.c_str() );

	FILE *keytoolResult = popen( cmdBuf, "r" );

	if (keytoolResult == NULL)
	{
		Rtt_LogException("ListKeyStore::GetAliasList: /usr/bin/keytool failed");
		return false;
	}

	std::vector<std::string> results;
	char buf[BUFSIZ];

	while (fgets(buf, BUFSIZ, keytoolResult) != NULL)
	{
		buf[strlen(buf) - 1] = 0; // zap trailing newline
		std::string bufStr(buf);
		results.push_back(bufStr);
	}

	pclose(keytoolResult);

	InitAliasList( (int)results.size() );

	for ( size_t i = 0; i < results.size(); i++ )
	{
		SetAlias( (int)i, results[i].c_str() );
	}

#else // Windows

	std::string keytoolPath;
	std::string jdkPath = Rtt::JavaHost::GetJdkPath();

	keytoolPath = jdkPath + "\\bin\\keytool.exe";
	_putenv_s("JAVA_TOOL_OPTIONS", "-Duser.language=en");

	const char kCmdFormat[] = "\"%s\" -list -keystore %s -storepass %s -rfc";

	_snprintf(cmdBuf, sizeof(cmdBuf), kCmdFormat, keytoolPath.c_str(), keystoreStr.c_str(), passwordStr.c_str());

	Interop::Ipc::CommandLine::SetOutputCaptureEnabled(true);
	Interop::Ipc::CommandLineRunResult result = Interop::Ipc::CommandLine::RunUntilExit(cmdBuf);

	std::vector<std::string> results;
	std::string output = result.GetOutput();
	std::string aliasName = "Alias name: ";
	size_t pos = 0;
	size_t lastPos = 0;
	std::string token;
	std::string delimiter = "\r\n";
	while ((pos = output.find(delimiter, pos)) != std::string::npos)
	{
		token = output.substr(lastPos, pos - lastPos);
		pos += delimiter.length();
		lastPos = pos;

		// Only save lines that start with "Alias name: "
		if (token.compare(0, aliasName.length(), aliasName) == 0)
		{
			results.push_back(token.substr(aliasName.length()));
		}
	}

	InitAliasList( (int)results.size() );

	for ( size_t i = 0; i < results.size(); i++ )
	{
		SetAlias( (int)i, results[i].c_str() );
	}

#endif // Rtt_MAC_ENV

	return (results.size() > 0);

#if USE_JNI
	Rtt_ASSERT( Rtt::JavaHost::GetEnv() );

	bool result = false;
	const char * msg;

	if ( !Rtt::JavaHost::GetEnv() )
		return result;

	if ( Rtt::JavaHost::GetEnv() != NULL )
	{

		static const char *		kListKeyStore = "com/ansca/util/ListKeyStore";

		jclass clazz = Rtt::JavaHost::GetEnv()->FindClass( kListKeyStore );

		if ( clazz != NULL )
		{
			jmethodID mid = Rtt::JavaHost::GetEnv()->GetStaticMethodID( clazz,
			                "listAliases",
			                "(Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String;" );

			if ( mid != NULL )
			{
				jstringParam keyStoreJ( Rtt::JavaHost::GetEnv(), keyStore );
				jstringParam passwordJ( Rtt::JavaHost::GetEnv(), password );

				if ( keyStoreJ.isValid() && passwordJ.isValid() )
				{
					jobject resultArray;

					resultArray = Rtt::JavaHost::GetEnv()->CallStaticObjectMethod( clazz, mid, keyStoreJ.getValue(), passwordJ.getValue() );

					if ( resultArray != NULL )
					{
						jsize arraySize = Rtt::JavaHost::GetEnv()->GetArrayLength((jobjectArray) resultArray);

						InitAliasList( arraySize );

						for ( int i = 0; i < arraySize; i++ )
						{
							jstring stringElement = (jstring) Rtt::JavaHost::GetEnv()->GetObjectArrayElement( (jobjectArray) resultArray, i );

							const char * utf = Rtt::JavaHost::GetEnv()->GetStringUTFChars( stringElement, NULL );
							SetAlias( i, utf );
							Rtt::JavaHost::GetEnv()->ReleaseStringUTFChars( stringElement, utf );
						}

						Rtt::JavaHost::GetEnv()->DeleteLocalRef(resultArray);

						result = true;
					}
				}
			}
		}
		else
		{
			JniException e( Rtt::JavaHost::GetEnv() );

			if ( e.CheckException() )
			{
				msg = e.getUTF8();
			}
		}

	}

	return result;
#endif // USE_JNI
}

//
// ListKeyStore::AreKeyStoreAndAliasPasswordsValid
//
// Attempting to jarsign a fake jar file using the given keystore, alias and password will tell us if the password is good
//
bool
ListKeyStore::AreKeyStoreAndAliasPasswordsValid( const char *keyStore, const char *keyPW, const char *alias, const char *aliasPW, const char *resourcesDir)
{
	char cmdBuf[20480];
	std::string keyStoreStr = EscapeArgument(keyStore);
	std::string keyPWStr = EscapeArgument(keyPW);
	std::string aliasStr = EscapeArgument(alias);
	std::string aliasPWStr = EscapeArgument(aliasPW);
	std::string resourcesDirStr = resourcesDir;

	std::string tmpDirStr = Rtt_GetSystemTempDirectory();
	std::string srcTestJarStr = resourcesDirStr + "/_coronatest.jar";
	std::string dstTestJarStr = tmpDirStr + "/_coronatest.jar";

	if (! Rtt_CopyFile(srcTestJarStr.c_str(), dstTestJarStr.c_str()))
	{
		return -1;
	}

	Rtt_Log("Testing credentials for '%s': ", keyStore);

#if defined(Rtt_MAC_ENV) || defined(Rtt_LINUX_ENV)

	const char kCmdFormat[] = "JAVA_TOOL_OPTIONS='-Duser.language=en' /usr/bin/jarsigner -tsa http://timestamp.digicert.com -keystore %s -storepass %s -keypass %s %s %s; exit $?";

	snprintf(cmdBuf, sizeof(cmdBuf), kCmdFormat, keyStoreStr.c_str(), keyPWStr.c_str(), aliasPWStr.c_str(), dstTestJarStr.c_str(), aliasStr.c_str() );

	return (system( cmdBuf ) == 0);

#else // Windows

	std::string keytoolPath;
	std::string jdkPath = Rtt::JavaHost::GetJdkPath();

	keytoolPath = jdkPath + "\\bin\\jarsigner.exe";
	_putenv_s("JAVA_TOOL_OPTIONS", "-Duser.language=en");

	const char kCmdFormat[] = "\"%s\" -tsa http://timestamp.digicert.com -keystore %s -storepass %s -keypass %s %s %s";

	snprintf(cmdBuf, sizeof(cmdBuf), kCmdFormat, keytoolPath.c_str(), keyStoreStr.c_str(), keyPWStr.c_str(), aliasPWStr.c_str(), dstTestJarStr.c_str(), aliasStr.c_str() );

	Interop::Ipc::CommandLine::SetOutputCaptureEnabled(true);
	Interop::Ipc::CommandLineRunResult result = Interop::Ipc::CommandLine::RunUntilExit(cmdBuf);

	std::string output = result.GetOutput();
	Rtt_Log(output.c_str());

	return (result.GetExitCode() == 0);

#endif

}
