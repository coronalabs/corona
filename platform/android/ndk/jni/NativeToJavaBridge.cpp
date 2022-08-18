//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include <android/log.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>

#include "Rtt_AndroidPlatform.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaLibMedia.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaResource.h"
#include "Rtt_MPlatform.h"
#include "Display/Rtt_PlatformBitmap.h"
#include "Rtt_PlatformWebPopup.h"
#include "Rtt_PreferenceCollection.h"
#include "Rtt_Runtime.h"
#include "NativeToJavaBridge.h"
#include "AndroidImageData.h"
#include "AndroidZipFileEntry.h"
#include "jniUtils.h"


// ----------------------------------------------------------------------------

static const char kNativeToJavaBridge[] = "com/ansca/corona/NativeToJavaBridge";
JavaVM *NativeToJavaBridge::fVM;
// Signatures available through:
//		 javap -classpath bin -protected com.ansca.corona.CoronaBridge -s

// TODO: check for exceptions

// ----------------------------------------------------------------------------

extern "C" void debugPrint(const char *msg)
{
#ifdef Rtt_DEBUG
	__android_log_print(ANDROID_LOG_INFO, "Corona", "%s", msg);
#endif
}

void*
NativeToJavaBridge::JavaToNative( jpointer p )
{
	Rtt_STATIC_ASSERT( sizeof( p ) == sizeof( jpointer ) );
	return (void*)p;
}

NativeToJavaBridge*
NativeToJavaBridge::InitInstance( JNIEnv *env, Rtt::Runtime *runtime, jobject coronaRuntime )
{
	JavaVM * vm;
	jint result = env->GetJavaVM(&vm);
	if ( 0 == result )
	{
		return new NativeToJavaBridge( vm, runtime, coronaRuntime );
	}
	return NULL;
}

// ----------------------------------------------------------------------------

NativeToJavaBridge::NativeToJavaBridge(JavaVM *vm, Rtt::Runtime *runtime, jobject coronaRuntime)
{
	fVM = vm;
	fRuntime = runtime;
	fCoronaRuntime = coronaRuntime;
	// fHasLuaErrorOccurred = false;
	fAlertCallbackResource = NULL;
	fPopupClosedEventResource = NULL;
}

JNIEnv *
NativeToJavaBridge::GetJNIEnv()
{
	JNIEnv * env = NULL;

	if (fVM->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
	{
		debugPrint( "> NativeToJavaBridge::GetJNIEnv null");
	}
	return env;
}

Rtt::Runtime *
NativeToJavaBridge::GetRuntime() const
{
	return fRuntime;
}

Rtt::AndroidPlatform *
NativeToJavaBridge::GetPlatform() const
{
	if (!fRuntime)
	{
		return NULL;
	}
	return (Rtt::AndroidPlatform*)&(fRuntime->Platform());
}

/// Checks if a Java exception was thrown.
/// If so, then the Java exception is caught and a Lua error gets raised so that the error can be handled
/// gracefully by a Lua pcall() or a Java handler given to the CoronaEnvironment.setLuaErrorHandler() method.
/// Note: This function will do a longjmp() if an exception was found to unwind the stack back to the caller
///       of the Lua function. This means that C++ objects on the stack will not have their destructors called.
void
NativeToJavaBridge::HandleJavaException() const
{
	if (fRuntime)
	{
		HandleJavaExceptionUsing(fRuntime->VMContext().L());
	}
}

/// Checks if a Java exception was thrown.
/// If so, then the Java exception is caught and a Lua error gets raised so that the error can be handled
/// gracefully by a Lua pcall() or a Java handler given to the CoronaEnvironment.setLuaErrorHandler() method.
/// Note: This function will do a longjmp() if an exception was found to unwind the stack back to the caller
///       of the Lua function. This means that C++ objects on the stack will not have their destructors called.
/// @param L Pointer to a lua_State to raise a Lua error on if an exception was found. Cannot be NULL.
void
NativeToJavaBridge::HandleJavaExceptionUsing( lua_State *L ) const
{
	// Fetch the JNI environment.
	JNIEnv *jniEnvironmentPointer = GetJNIEnv();
	if (!jniEnvironmentPointer)
	{
		return;
	}

	// Check if a Java exception was thrown.
	// If not, then there is no error to handle.
	if (jniEnvironmentPointer->ExceptionCheck() == JNI_FALSE)
	{
		return;
	}

	// A Java exception was thrown.
	// Catch it here so that the Java side of this application won't terminate.
	jthrowable exception = jniEnvironmentPointer->ExceptionOccurred();
	jniEnvironmentPointer->ExceptionClear();

	// Fetch the exception's error message and stack trace.
	const char *errorMessage = "Java exception occurred.";
	jstringResult stringResult(jniEnvironmentPointer);
	jclassInstance bridge(jniEnvironmentPointer, kNativeToJavaBridge);
	if (bridge.isValid())
	{
		jmethodID getExceptionStackTraceMethodId = bridge.getEnv()->GetStaticMethodID(
					bridge.getClass(),
					"callGetExceptionStackTraceFrom",
					"(Ljava/lang/Throwable;)Ljava/lang/String;");
		jobject objectResult = bridge.getEnv()->CallStaticObjectMethod(
					bridge.getClass(), getExceptionStackTraceMethodId, exception);
		if (objectResult)
		{
			stringResult.setString((jstring)objectResult);
		}
	}
	if (stringResult.isValidString())
	{
		errorMessage = stringResult.getUTF8();
	}

	// Raise a Lua error using the Java exception's error message.
	// This error can then be handled by the following:
	// - A Lua pcall() function.
	// - A Java handler passed to the CoronaEnvironment.setLuaErrorHandler() method.
	// Note: This will do a longjmp() to unwind the stack back to where Lua called our native API.
	if (L)
	{
		luaL_error(L, errorMessage);
	}
}

/// Creates a new Java dictionary object populated with the given Lua state's table entries.
/// @param L The Lua state to read the table entries from.
/// @param t Index to the Lua table to read. It must be greater than zero. ie: It cannot be a relative index.
/// @param bridge The JNI bridge interfae to create the dictionary object in.
/// @return Returns the Java dictionary object populated with the Lua table's entries.
///         Returns NULL if failed to create the dictionary or if given invalid parameters.
NativeToJavaBridge::DictionaryRef
NativeToJavaBridge::DictionaryCreate( lua_State *L, int t, NativeToJavaBridge *bridge )
{
	// Validate arguments.
	if (!L || !bridge || (t <= 0))
	{
		return NULL;
	}
	
	// Do not continue if the referenced Lua object on the stack is not a table/array.
	if (!lua_istable( L, t ))
	{
		return NULL;
	}
	
	// Create the Java dictionary object.
	NativeToJavaBridge::DictionaryRef dict = bridge->DictionaryCreate();
	if (!dict)
	{
		return NULL;
	}
	
	// Add all Lua table entries to the Java dictionary.
	char stringBuffer[32];
	for (lua_pushnil( L ); lua_next( L, t ) != 0; lua_pop( L, 1 ))
	{
		// Fetch the table entry's key.
		const char *keyName = NULL;
		int valueType = lua_type( L, -2 );
		if (LUA_TSTRING == valueType)
		{
			keyName = lua_tostring( L, -2 );
		}
		else if (LUA_TNUMBER == valueType)
		{
			// The key will be a number if we're traversing a Lua array.
			// Convert the numeric index to a string and use that as the key name in the hash table.
			int value = (int)(lua_tonumber( L, -2 ) + 0.5);
			if (snprintf(stringBuffer, sizeof(stringBuffer), "%d", value) > 0)
			{
				keyName = stringBuffer;
			}
		}
		if (!keyName)
		{
			continue;
		}
		
		// Add the key/value pair to the Java dictionary.
		valueType = lua_type( L, -1 );
		switch (valueType)
		{
			case LUA_TSTRING:
				((jHashMapParam*)dict)->put( keyName, lua_tostring( L, -1 ) );
				break;
				
			case LUA_TBOOLEAN:
				((jHashMapParam*)dict)->put( keyName, lua_toboolean( L, -1 ) ? true : false );
				break;
				
			case LUA_TNUMBER:
				((jHashMapParam*)dict)->put( keyName, (double)lua_tonumber( L, -1 ) );
				break;
			
			case LUA_TFUNCTION: // TODO: Exercise this!
				((jHashMapParam*)dict)->put( keyName, lua_tocfunction( L, -1 ) );
				break;

			case LUA_TTABLE:
			{
				Rtt::LuaLibSystem::FileType fileType;
				int addedLuaEntriesCount = Rtt::LuaLibSystem::PathForTable( L, -1, fileType );
				if (addedLuaEntriesCount > 0)
				{
					// The value is a Lua table referencing a file path.
					// Fetch the file path and store it into a Java File object.
					jobject javaByteArray = NULL;
					const char *fileName = lua_tostring( L, -1 );
					if (fileName)
					{
						jFile fileJavaObject( bridge->GetJNIEnv(), fileName );
						if (fileJavaObject.isValid())
						{
							((jHashMapParam*)dict)->put( keyName, fileJavaObject.getValue() );
						}
					}
					lua_pop( L, addedLuaEntriesCount );
				}
				else
				{
					// The value is a Lua table or array. Add it is as a new dictionary object to this dictionary entry.
					jHashMapParam *hashMapPointer = (jHashMapParam*)DictionaryCreate( L, lua_gettop( L ), bridge );
					((jHashMapParam*)dict)->put( keyName, hashMapPointer->getHashMapObject() );
				}
				break;
			}

			default:
				break;
		}
	}

	// Return the populated Java dictionary object.
	return dict;
}

NativeToJavaBridge::DictionaryRef
NativeToJavaBridge::DictionaryCreate()
{
	jHashMapParam *p = new jHashMapParam( GetJNIEnv() );
	return p;
}

void
NativeToJavaBridge::DictionaryDestroy( DictionaryRef dict )
{
	delete (jHashMapParam*)dict;
}

// TODO
/*
void
NativeToJavaBridge::DictionaryGet( DictionaryRef dict, const char *key, Rtt::String& result );
{
	const char *result = NULL;
	if ( dict && key && value )
	{
		jHashMapParam *p = (jHashMapParam*)dict;
		jstring value = p->get( key );
		
		if ( value )
		{
			jstringResult jstr( bridge.getEnv() );
			jstr.setString( (jstring)value );
		
			if ( jstr.isValidString() )
			{
				result.Set( jstr.getUTF8() );
			}
		}
	}
	return result;
}
*/

bool
NativeToJavaBridge::RequestSystem( lua_State *L, const char *actionName, int optionsIndex )
{
	NativeTrace trace( "NativeToJavaBridge::RequestSystem" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = false;
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callRequestSystem", "(Lcom/ansca/corona/CoronaRuntime;JLjava/lang/String;I)Z");
		if (mid != NULL)
		{
			jstringParam actionNameJ(bridge.getEnv(), actionName);
			result = bridge.getEnv()->CallStaticBooleanMethod(
							bridge.getClass(), mid, fCoronaRuntime, (jlong)(uintptr_t)L, actionNameJ.getValue(), optionsIndex);
			HandleJavaExceptionUsing(L);
		}
	}
	return (bool)result;
}

void
NativeToJavaBridge::Ping()
{
	CallVoidMethod( "ping" );
}

int
NativeToJavaBridge::LoadFile( lua_State *L, const char *fileName )
{
	NativeTrace trace( "NativeToJavaBridge::LoadFile" );
	int result = 0;
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
			bridge.getClass(), "callLoadFile", "(Lcom/ansca/corona/CoronaRuntime;JLjava/lang/String;)I");

		if ( mid != NULL ) {
			jstringParam fileNameJ( bridge.getEnv(), fileName );
			if ( fileNameJ.isValid() ) {
				result = bridge.getEnv()->CallStaticIntMethod(
					bridge.getClass(), mid, fCoronaRuntime, (jlong)(uintptr_t)L, fileNameJ.getValue() );
				HandleJavaExceptionUsing(L);
			}
		}
	}

	return result;	
}

int
NativeToJavaBridge::LoadClass( lua_State *L, const char *libName, const char *className )
{
	NativeTrace trace( "NativeToJavaBridge::LoadClass" );
	int result = 0;
	
	// 2 bigger than the length for the terminating null and the underscore if needed
	char libraryName[strlen(libName)+2];
	const char * hasNative = strstr(libName, ".native.");


	if (hasNative) {
		// Copy everything before native, hasNative is the position where it first finds .native, libName is where the char array starts
		// add 1 for the dot before native
		strncpy(libraryName, libName, hasNative - libName + 1);

		// Add in an underscore
		libraryName[(hasNative-libName) + 1] = '_';

		// Copy everything after native
		strncpy(libraryName + (hasNative - libName + 2), libName + (hasNative - libName + 1), strlen(libName) - (hasNative - libName));
	} else {
		strncpy(libraryName, libName, strlen(libName) + 1);
	}

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
			bridge.getClass(), "callLoadClass", "(Lcom/ansca/corona/CoronaRuntime;JLjava/lang/String;Ljava/lang/String;)I");

		if ( mid != NULL ) {
			jstringParam libNameJ( bridge.getEnv(), libraryName );
			jstringParam classNameJ( bridge.getEnv(), className );
			if ( libNameJ.isValid() && classNameJ.isValid() ) {
				result = bridge.getEnv()->CallStaticIntMethod(
					bridge.getClass(), mid, fCoronaRuntime, (jlong)(uintptr_t)L, libNameJ.getValue(), classNameJ.getValue());
				HandleJavaExceptionUsing(L);
			}
		}
	}

	return result;
}

void
NativeToJavaBridge::OnRuntimeLoaded(lua_State *L)
{
	CallLongMethod("callOnRuntimeLoaded", (jlong)(uintptr_t)L);
}

void
NativeToJavaBridge::OnRuntimeWillLoadMain()
{
	CallVoidMethod("callOnRuntimeWillLoadMain");
}

void
NativeToJavaBridge::OnRuntimeStarted()
{
	CallVoidMethod("callOnRuntimeStarted");
}

void
NativeToJavaBridge::OnRuntimeSuspended()
{
	CallVoidMethod("callOnRuntimeSuspended");
}

void
NativeToJavaBridge::OnRuntimeResumed()
{
	CallVoidMethod("callOnRuntimeResumed");
}

void
NativeToJavaBridge::OnRuntimeExiting()
{
	fRuntime = NULL;
}

void
NativeToJavaBridge::OnAudioEnabled()
{
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callOnAudioEnabled", "()V" );
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid );
		}
	}
}

int
NativeToJavaBridge::InvokeLuaErrorHandler(lua_State *L)
{
	NativeTrace trace( "NativeToJavaBridge::InvokeLuaErrorHandler" );

	// Flag that a Lua error has occurred.
	// fHasLuaErrorOccurred = true;

	// Invoke the Lua error handler.
	int result = 0;
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), "callInvokeLuaErrorHandler", "(J)I");
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticIntMethod(bridge.getClass(), mid, (jlong)(uintptr_t)L);
		}
	}
	return result;
}

void
NativeToJavaBridge::PushLaunchArgumentsToLuaTable(lua_State *L)
{
	NativeTrace trace( "NativeToJavaBridge::PushLaunchArgumentsToLuaTable" );
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
					bridge.getClass(), "callPushLaunchArgumentsToLuaTable", "(Lcom/ansca/corona/CoronaRuntime;J)V");
		if (mid != NULL)
		{
			bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, (jlong)(uintptr_t)L);
		}
	}
}

void
NativeToJavaBridge::PushApplicationOpenArgumentsToLuaTable(lua_State *L)
{
	NativeTrace trace( "NativeToJavaBridge::PushApplicationOpenArgumentsToLuaTable" );
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
					bridge.getClass(), "callPushApplicationOpenArgumentsToLuaTable", "(Lcom/ansca/corona/CoronaRuntime;J)V");
		if (mid != NULL)
		{
			bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, (jlong)(uintptr_t)L);
		}
	}
}

#if 0
enum JNI_JAVATYPE { JNI_UNKNOWN, JNI_BYTEARRAY, JNI_SHORTARRAY, JNI_INTARRAY, JNI_LONGARRAY };


JNI_JAVATYPE jni_type(JNIEnv *jni,jobject object)
{
    // get java array classes
    jclass jbyte_class  = jni->FindClass("[B");
    jclass jshort_class = jni->FindClass("[S");
    jclass jint_class   = jni->FindClass("[I");
    jclass jlong_class  = jni->FindClass("[J");
    
    // return array type id
    if (jni->IsInstanceOf(object,jbyte_class))  return JNI_BYTEARRAY;
    if (jni->IsInstanceOf(object,jshort_class)) return JNI_SHORTARRAY;
    if (jni->IsInstanceOf(object,jint_class))   return JNI_INTARRAY;
    if (jni->IsInstanceOf(object,jlong_class))  return JNI_LONGARRAY;

    // unknown type
    return JNI_UNKNOWN;
}
#endif

bool
NativeToJavaBridge::GetRawAsset( const char * assetName, Rtt::Data<char> & data )
{
	// Validate.
	if (!assetName)
	{
		return false;
	}

	NativeTrace trace( "NativeToJavaBridge::GetRawAsset" );
	bool result = false;

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		// Fetch the package file the given asset is contained in and its compression information.
		AndroidZipFileEntry zipFileEntry(data.Allocator());
		bool wasAssetFileFound = GetAssetFileLocation(assetName, zipFileEntry);
		if (wasAssetFileFound && (zipFileEntry.GetByteCountInPackage() > 0))
		{
			// The asset file was located. Fetch its bytes.
			if (zipFileEntry.IsCompressed())
			{
				// Asset is compressed. Decompress it in Java.
				NativeTrace trace( "NativeToJavaBridge::GetBytesFromFile" );
				jstringParam assetNameJ(bridge.getEnv(), assetName);
				jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
						"callGetBytesFromFile", "(Ljava/lang/String;)[B");
				if (mid)
				{
					jobject jo = bridge.getEnv()->CallStaticObjectMethod(
										bridge.getClass(), mid, assetNameJ.getValue());
					// HandleJavaException();
					if (jo)
					{
						jbyteArrayResult bytesJ( bridge.getEnv(), (jbyteArray) jo );
						data.Set( (const char *) bytesJ.getValues(), bytesJ.getLength() );
						bytesJ.release();
						result = true;
						bridge.getEnv()->DeleteLocalRef(jo);
					}
				}
			}
			else
			{
				// Asset is not compressed. Fetch its bytes from within the package file directly here.
				int fileDescriptor = open(zipFileEntry.GetPackageFilePath(), O_RDONLY);
				if (fileDescriptor >= 0)
				{
					data.SetLength(zipFileEntry.GetByteCountInPackage());
					lseek(fileDescriptor, zipFileEntry.GetByteOffsetInPackage(), SEEK_SET);
					ssize_t readResult = read(fileDescriptor, data.Get(), zipFileEntry.GetByteCountInPackage());
					result = (readResult >= 0);
					close(fileDescriptor);
				}
			}
		}
	}

	return result;
}

bool
NativeToJavaBridge::GetRawAssetExists( const char * assetName )
{
	NativeTrace trace( "NativeToJavaBridge::GetRawAssetExists");

	bool result = false;

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() ) {
		trace.Trace();

		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callGetRawAssetExists", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;)Z" );

		if ( mid != NULL ) {
			trace.Trace();

			jstringParam assetNameJ( bridge.getEnv(), assetName );

			if ( assetNameJ.isValid() ) {
				result = (bool) bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, assetNameJ.getValue() );
				HandleJavaException();
// #ifdef Rtt_DEBUG
// 				__android_log_print(ANDROID_LOG_INFO, "Corona", "NativeToJavaBridge::GetRawAssetExists call returned %d", (int) result);
// #endif
			}
		}
	}

	return result;
}

bool
NativeToJavaBridge::GetCoronaResourceFileExists( const char * assetName )
{
	NativeTrace trace( "NativeToJavaBridge::GetCoronaResourceFileExists");

	bool result = false;

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() ) {
		trace.Trace();

		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callGetCoronaResourceFileExists", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;)Z" );

		if ( mid != NULL ) {
			trace.Trace();

			jstringParam assetNameJ( bridge.getEnv(), assetName );

			if ( assetNameJ.isValid() ) {
				result = (bool) bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, assetNameJ.getValue() );
				HandleJavaException();
			}
		}
	}

	return result;
}

bool
NativeToJavaBridge::GetAssetFileLocation(const char *assetName, AndroidZipFileEntry &zipFileEntry)
{
	NativeTrace trace( "NativeToJavaBridge::GetAssetFileLocation");

	bool wasAssetFound = false;
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callGetAssetFileLocation", "(Ljava/lang/String;J)Z");
		if (mid)
		{
			jstringParam assetNameJ(bridge.getEnv(), assetName);
			jboolean result = bridge.getEnv()->CallStaticBooleanMethod(
						bridge.getClass(), mid, assetNameJ.getValue(), (jlong)(uintptr_t)(&zipFileEntry));
			// HandleJavaException();
			wasAssetFound = result ? true : false;
		}
	}
	return wasAssetFound;
}

void
NativeToJavaBridge::SetTimer( int milliseconds )
{
	CallIntMethod( "callSetTimer", milliseconds );
	HandleJavaException();
}

void
NativeToJavaBridge::CancelTimer()
{
	CallVoidMethod( "callCancelTimer" );
	HandleJavaException();
}

void
NativeToJavaBridge::CallStringMethod( const char * method, const char * param ) const
{
	NativeTrace trace( method );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), method, "(Ljava/lang/String;)V" );
		if ( mid != NULL )
		{
			jstringParam paramJ( bridge.getEnv(), param );
			if ( paramJ.isValid() )
			{
				bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, paramJ.getValue() );
			}
		}
	}
}

void
NativeToJavaBridge::CallIntMethod( const char * method, int param ) const
{
	NativeTrace trace( method );
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), method, "(ILcom/ansca/corona/CoronaRuntime;)V" );
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, (jint) param, fCoronaRuntime );
		}
	}
}

void
NativeToJavaBridge::CallLongMethod( const char * method, long param ) const
{
	NativeTrace trace( method );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), method, "(JLcom/ansca/corona/CoronaRuntime;)V" );
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, (jlong) param, fCoronaRuntime );
		}
	}
}

void
NativeToJavaBridge::CallDoubleMethod( const char * method, double param ) const
{
	NativeTrace trace( method );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), method, "(DLcom/ansca/corona/CoronaRuntime;)V" );
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, (jdouble) param, fCoronaRuntime );
		}
	}
}

void
NativeToJavaBridge::CallFloatMethod( const char * method, float param ) const
{
	NativeTrace trace( method );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), method, "(FLcom/ansca/corona/CoronaRuntime;)V" );
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, (jfloat) param, fCoronaRuntime );
		}
	}
}

void
NativeToJavaBridge::CallVoidMethod( const char * method ) const
{
//	NativeTrace trace( method );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), method, "(Lcom/ansca/corona/CoronaRuntime;)V" );
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime );
		}
	}
}

void
NativeToJavaBridge::LoadSound( uintptr_t id, const char * name, bool eventSound )
{
	const char * methodName;
	
	if ( !name )
	{
		return;
	}
	
	if ( eventSound )
		methodName = "callLoadEventSound";
	else
		methodName = "callLoadSound";

	NativeTrace trace( methodName );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
							bridge.getClass(), methodName, "(Lcom/ansca/corona/CoronaRuntime;JLjava/lang/String;)V" );
		if ( mid != NULL )
		{
			jstringParam nameJ( bridge.getEnv(), name );
			if ( nameJ.isValid() )
			{
				bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, (jlong)id, nameJ.getValue()  );
				HandleJavaException();
			}
		}
	}
}

void
NativeToJavaBridge::PlaySound( uintptr_t id, const char * name, bool loop )
{
	if ( !name )
	{
		return;
	}
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
							bridge.getClass(), "callPlaySound", "(Lcom/ansca/corona/CoronaRuntime;JLjava/lang/String;Z)V" );
		if ( mid != NULL )
		{
			jstringParam nameJ( bridge.getEnv(), name );
			if ( nameJ.isValid() )
			{
				bridge.getEnv()->CallStaticVoidMethod(
						bridge.getClass(), mid, fCoronaRuntime, (jlong)id, nameJ.getValue(), (jboolean) loop);
				HandleJavaException();
			}
		}
	}
}

void
NativeToJavaBridge::StopSound( uintptr_t id )
{
	CallLongMethod( "callStopSound", id );
	HandleJavaException();
}

void
NativeToJavaBridge::PauseSound( uintptr_t id )
{
	CallLongMethod( "callPauseSound", id );
	HandleJavaException();
}

void
NativeToJavaBridge::ResumeSound( uintptr_t id )
{
	CallLongMethod( "callResumeSound", id );
	HandleJavaException();
}

bool
NativeToJavaBridge::RenderText(
	const char *text, const char *fontName, float fontSize, bool isBold,
	int wrapWidth, int clipWidth, int clipHeight, const char *alignment, AndroidImageData &result, Rtt::Real &baselineOffset)
{
	NativeTrace trace( "NativeToJavaBridge::RenderText" );

	jboolean wasRendered = JNI_FALSE;
	jclassInstance bridge(GetJNIEnv(), kNativeToJavaBridge);
	if (bridge.isValid())
	{
		jmethodID methodId = bridge.getEnv()->GetStaticMethodID(
					bridge.getClass(), "callRenderText",
					"(Lcom/ansca/corona/CoronaRuntime;JLjava/lang/String;Ljava/lang/String;FZIIILjava/lang/String;[F)Z");
		if (methodId)
		{
			jfloatArray outBaselineOffset = bridge.getEnv()->NewFloatArray(1);
			jstringParam textJ( bridge.getEnv(), text );
			jstringParam fontNameJ( bridge.getEnv(), fontName );
			jstringParam alignmentJ( bridge.getEnv(), alignment );
			if (textJ.isValid())
			{
				jfloat jFontSize = fontSize;
				wasRendered = bridge.getEnv()->CallStaticBooleanMethod(
								bridge.getClass(), methodId, fCoronaRuntime, (jlong)(uintptr_t)(&result),
								textJ.getValue(), fontNameJ.getValue(),
								jFontSize, isBold, wrapWidth, clipWidth, clipHeight, alignmentJ.getValue(), outBaselineOffset);
				HandleJavaException();
			}
			jfloat* offsetArrJ = bridge.getEnv()->GetFloatArrayElements(outBaselineOffset, NULL);
			if(offsetArrJ != NULL)
			{
				baselineOffset = *offsetArrJ;
			}
			bridge.getEnv()->ReleaseFloatArrayElements( outBaselineOffset, offsetArrJ, 0 );
			bridge.getEnv()->DeleteLocalRef( outBaselineOffset );
		}
	}
	return wasRendered ? true : false;
}

void 
NativeToJavaBridge::GetSafeAreaInsetsPixels(Rtt::Real &top, Rtt::Real &left, Rtt::Real &bottom, Rtt::Real &right)
{
	top = left = bottom = right = 0;
	NativeTrace trace( "NativeToJavaBridge::GetSafeAreaInsetsPixels" );
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() ) 
	{
		jmethodID methodId = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
								"callGetSafeAreaInsetPixels", "(Lcom/ansca/corona/CoronaRuntime;)[F" );
		if (methodId)
		{
			jobject objArray = bridge.getEnv()->CallStaticObjectMethod( bridge.getClass(), methodId, fCoronaRuntime );
			jfloatArray * jfArray = reinterpret_cast< jfloatArray* >( & objArray );
			if(objArray == NULL) return;
			jsize len = bridge.getEnv()->GetArrayLength( *jfArray );
			float* data = bridge.getEnv()->GetFloatArrayElements( *jfArray, 0 );
			if ( len == 4 )
			{
				top 	= data [ 0 ];
				left 	= data [ 1 ];
				right 	= data [ 2 ];
				bottom  = data [ 3 ];
			}
			bridge.getEnv()->ReleaseFloatArrayElements( *jfArray, data, 0 );
			bridge.getEnv()->DeleteLocalRef( *jfArray );
			HandleJavaException();
		}
	}
}

bool
NativeToJavaBridge::LoadImage(
	const char *filePath, AndroidImageData &imageData, bool convertToGrayscale,
	int maxWidth, int maxHeight, bool loadImageInfoOnly)
{
	NativeTrace trace( "NativeToJavaBridge::LoadImage" );

	bool wasLoaded = false;
	jclassInstance bridge(GetJNIEnv(), kNativeToJavaBridge);
	if (bridge.isValid())
	{
		jmethodID methodId = bridge.getEnv()->GetStaticMethodID(
								bridge.getClass(), "callLoadBitmap", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;JZIIZ)Z");
		if (methodId)
		{
			jstringParam filePathJ(bridge.getEnv(), filePath);
			if (filePathJ.isValid())
			{
				jboolean javaBooleanResult;
				javaBooleanResult = bridge.getEnv()->CallStaticBooleanMethod(
											bridge.getClass(), methodId, fCoronaRuntime, filePathJ.getValue(),
											(jlong)(uintptr_t)(&imageData),
											convertToGrayscale ? JNI_TRUE : JNI_FALSE,
											maxWidth, maxHeight,
											loadImageInfoOnly ? JNI_TRUE : JNI_FALSE);
				HandleJavaException();
				wasLoaded = javaBooleanResult ? true : false;
			}
		}
	}
	return wasLoaded;
}

void 
NativeToJavaBridge::PlayVideo( uintptr_t id, const char * url, bool mediaControlsEnabled )
{
	if ( !url )
		return;

	NativeTrace trace( "NativeToJavaBridge::PlayVideo" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callPlayVideo", "(Lcom/ansca/corona/CoronaRuntime;JLjava/lang/String;Z)V" );
		if ( mid != NULL )
		{
			jstringParam paramJ( bridge.getEnv(),  url );
			if ( paramJ.isValid() )
			{
				bridge.getEnv()->CallStaticVoidMethod(
						bridge.getClass(), mid, fCoronaRuntime, (jlong)id, paramJ.getValue(), (jboolean)mediaControlsEnabled);
				HandleJavaException();
			}
		}
	}
}

bool 
NativeToJavaBridge::CanOpenUrl( const char * url )
{
	NativeTrace trace( "NativeToJavaBridge::CanOpenUrl" );

	jclassInstance bridge(GetJNIEnv(), kNativeToJavaBridge);
	jboolean result = false;
	if (bridge.isValid() && url)
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
				bridge.getClass(), "callCanOpenUrl", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;)Z");
		if (mid)
		{
			jstringParam urlJ(bridge.getEnv(), url);
			result = bridge.getEnv()->CallStaticBooleanMethod(bridge.getClass(), mid, fCoronaRuntime, urlJ.getValue());
			HandleJavaException();
		}
	}
	return (bool)result;
}

bool 
NativeToJavaBridge::OpenUrl( const char * url )
{
	NativeTrace trace( "NativeToJavaBridge::OpenUrl" );

	jclassInstance bridge(GetJNIEnv(), kNativeToJavaBridge);
	jboolean result = false;
	if (bridge.isValid() && url)
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
							bridge.getClass(), "callOpenUrl", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;)Z");
		if (mid)
		{
			jstringParam urlJ(bridge.getEnv(), url);
			result = bridge.getEnv()->CallStaticBooleanMethod(bridge.getClass(), mid, fCoronaRuntime, urlJ.getValue());
			HandleJavaException();
		}
	}
	return (bool)result;
}

void
NativeToJavaBridge::SetIdleTimer( bool enabled )
{
	NativeTrace trace( "NativeToJavaBridge::SetIdleTimer" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() )
	{
		jmethodID mid;
		
		mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callSetIdleTimer", "(Lcom/ansca/corona/CoronaRuntime;Z)V" );
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, (jboolean)enabled );
			HandleJavaException();
		}
	}
}

bool
NativeToJavaBridge::GetIdleTimer() const
{
	NativeTrace trace( "NativeToJavaBridge::GetIdleTimer" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = true;
	if ( bridge.isValid() )
	{
		jmethodID mid;
		
		mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callGetIdleTimer", "(Lcom/ansca/corona/CoronaRuntime;)Z" );
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime );
			HandleJavaException();
		}
	}
	return result;
}

void
NativeToJavaBridge::SetStatusBarMode( int mode )
{
	NativeTrace trace( "NativeToJavaBridge::SetStatusBarMode" );

	CallIntMethod( "callSetStatusBarMode", mode );
	HandleJavaException();
}

int
NativeToJavaBridge::GetStatusBarMode()
{
	int result = 0;
	
	NativeTrace trace( "NativeToJavaBridge::GetStatusBarMode" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callGetStatusBarMode", "(Lcom/ansca/corona/CoronaRuntime;)I" );
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticIntMethod( bridge.getClass(), mid, fCoronaRuntime );
			HandleJavaException();
		}
	}
	return result;
}

int
NativeToJavaBridge::GetStatusBarHeight()
{
	int result = 0;
	
	NativeTrace trace( "NativeToJavaBridge::GetStatusBarHeight" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callGetStatusBarHeight", "(Lcom/ansca/corona/CoronaRuntime;)I" );
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticIntMethod( bridge.getClass(), mid, fCoronaRuntime );
			HandleJavaException();
		}
	}
	return result;
}

void
NativeToJavaBridge::ShowNativeAlert( const char * title, const char * msg, const char ** labels, int numLabels, Rtt::LuaResource * resource )
{
	NativeTrace trace( "NativeToJavaBridge::ShowNativeAlert" );

	if ( !title || !msg )
	{
		return;
	}
	
	if ( !labels )
	{
		numLabels = 0;
	}
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() ) {
		
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callShowNativeAlert", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)V" );
		
		if ( mid != NULL ) {
			jstringParam titleJ( bridge.getEnv(), title );
			jstringParam msgJ( bridge.getEnv(), msg );
			jstringArrayParam labelsJ( bridge.getEnv(), numLabels );
			
			if ( labelsJ.isValid() && titleJ.isValid() && msgJ.isValid() ) {
				for ( int i = 0; i < numLabels; i++ ) {
					labelsJ.setElement( i, labels[i] );
				}
				
				bridge.getEnv()->CallStaticVoidMethod(
						bridge.getClass(), mid, fCoronaRuntime, titleJ.getValue(), msgJ.getValue(), labelsJ.getValue());
				HandleJavaException();
				
				fAlertCallbackResource = resource;
			}
		}
	}
}

void
NativeToJavaBridge::CancelNativeAlert( int which )
{
	CallIntMethod( "callCancelNativeAlert", which );
	HandleJavaException();
}

void
NativeToJavaBridge::AlertCallback( int which , bool cancelled)
{
	if ( fAlertCallbackResource != NULL ) {
		Rtt::LuaLibNative::AlertComplete( *fAlertCallbackResource, which, cancelled );

		Rtt_DELETE( fAlertCallbackResource );
		fAlertCallbackResource = NULL;
	}
}

void
NativeToJavaBridge::ShowTrialAlert()
{
	CallVoidMethod( "callShowTrialAlert" );
}

void
NativeToJavaBridge::ShowNativeActivityIndicator()
{
	NativeTrace trace( "NativeToJavaBridge::ShowNativeActivityIndicator" );
	
	CallVoidMethod( "callShowNativeActivityIndicator" );
	HandleJavaException();
}

void
NativeToJavaBridge::CloseNativeActivityIndicator()
{
	NativeTrace trace( "NativeToJavaBridge::CloseNativeActivityIndicator" );
	
	CallVoidMethod( "callCloseNativeActivityIndicator" );
	HandleJavaException();
}

bool
NativeToJavaBridge::HasMediaSource( int mediaSourceType )
{
	NativeTrace trace( "NativeToJavaBridge::HasMediaSource" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = true;
	if ( bridge.isValid() )
	{
		jmethodID mid;
		mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callHasMediaSource", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, mediaSourceType );
			HandleJavaException();
		}
	}
	return result;
}

bool
NativeToJavaBridge::HasAccessToMediaSource( int mediaSourceType )
{
	NativeTrace trace( "NativeToJavaBridge::HasAccessToMediaSource" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = true;
	if ( bridge.isValid() )
	{
		jmethodID mid;
		mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callHasAccessToMediaSource", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, mediaSourceType );
			HandleJavaException();
		}
	}
	return result;
}

void
NativeToJavaBridge::ShowImagePicker( int imageSourceType, const char *destinationFilePath )
{
	NativeTrace trace( "NativeToJavaBridge::ShowImagePicker" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
							   bridge.getClass(), "callShowImagePicker", "(Lcom/ansca/corona/CoronaRuntime;ILjava/lang/String;)V" );
		if ( mid != NULL )
		{
			jstringParam destinationFilePathJ( bridge.getEnv(), destinationFilePath );
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, imageSourceType, destinationFilePathJ.getValue() );
			HandleJavaException();
		}
	}
}

void 
NativeToJavaBridge::ShowVideoPicker( int videoSourceType, int maxTime, int quality )
{
	NativeTrace trace( "NativeToJavaBridge::ShowVideoPicker" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
							   bridge.getClass(), "callShowVideoPicker", "(Lcom/ansca/corona/CoronaRuntime;III)V" );
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, videoSourceType, maxTime, quality );
			HandleJavaException();
		}
	}
}

bool
NativeToJavaBridge::CanShowPopup( const char *name )
{
	NativeTrace trace( "NativeToJavaBridge::CanShowPopup" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = false;
	if ( bridge.isValid() && name )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
						bridge.getClass(), "callCanShowPopup", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;)Z" );
		if ( mid != NULL )
		{
			jstringParam nameJ( bridge.getEnv(), name );
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, nameJ.getValue() );
			HandleJavaException();
		}
	}
	return (bool)result;
}

void
NativeToJavaBridge::ShowSendMailPopup( NativeToJavaBridge::DictionaryRef dictionaryOfSettings, Rtt::LuaResource *resource )
{
	NativeTrace trace( "NativeToJavaBridge::ShowSendMailPopup" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
						bridge.getClass(), "callShowSendMailPopup", "(Lcom/ansca/corona/CoronaRuntime;Ljava/util/HashMap;)V" );
		if ( mid != NULL )
		{
			if (!fPopupClosedEventResource)
			{
				fPopupClosedEventResource = resource;
			}
			jobject hashMapObject = NULL;
			if (dictionaryOfSettings)
			{
				hashMapObject = ((jHashMapParam*)dictionaryOfSettings)->getHashMapObject();
			}
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, hashMapObject );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::ShowSendSmsPopup( NativeToJavaBridge::DictionaryRef dictionaryOfSettings, Rtt::LuaResource *resource )
{
	NativeTrace trace( "NativeToJavaBridge::ShowSendSmsPopup" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
						bridge.getClass(), "callShowSendSmsPopup", "(Lcom/ansca/corona/CoronaRuntime;Ljava/util/HashMap;)V" );
		if ( mid != NULL )
		{
			if (!fPopupClosedEventResource)
			{
				fPopupClosedEventResource = resource;
			}
			jobject hashMapObject = NULL;
			if (dictionaryOfSettings)
			{
				hashMapObject = ((jHashMapParam*)dictionaryOfSettings)->getHashMapObject();
			}
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, hashMapObject );
			HandleJavaException();
		}
	}
}

bool
NativeToJavaBridge::ShowAppStorePopup( NativeToJavaBridge::DictionaryRef dictionaryOfSettings, Rtt::LuaResource *resource )
{
	NativeTrace trace( "NativeToJavaBridge::ShowAppStorePopup" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = false;
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
						bridge.getClass(), "callShowAppStorePopup", "(Lcom/ansca/corona/CoronaRuntime;Ljava/util/HashMap;)Z" );
		if ( mid != NULL )
		{
			if (!fPopupClosedEventResource)
			{
				fPopupClosedEventResource = resource;
			}
			jobject hashMapObject = NULL;
			if (dictionaryOfSettings)
			{
				hashMapObject = ((jHashMapParam*)dictionaryOfSettings)->getHashMapObject();
			}
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, hashMapObject );
			HandleJavaException();
		}
	}
	return (bool)result;
}

void
NativeToJavaBridge::ShowRequestPermissionsPopup( NativeToJavaBridge::DictionaryRef dictionaryOfSettings, Rtt::LuaResource *resource )
{
	NativeTrace trace( "NativeToJavaBridge::ShowRequestPermissionsPopup" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
						bridge.getClass(), "callShowRequestPermissionsPopup", "(Lcom/ansca/corona/CoronaRuntime;Ljava/util/HashMap;)V" );
		if ( mid != NULL )
		{
			if (!fPopupClosedEventResource)
			{
				fPopupClosedEventResource = resource;
			}
			jobject hashMapObject = NULL;
			if (dictionaryOfSettings)
			{
				hashMapObject = ((jHashMapParam*)dictionaryOfSettings)->getHashMapObject();
			}
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, hashMapObject );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::RaisePopupClosedEvent( const char *popupName, bool wasCanceled )
{
	if (fPopupClosedEventResource)
	{
		Rtt::LuaLibNative::PopupClosed(*fPopupClosedEventResource, popupName, wasCanceled);
		Rtt_DELETE( fPopupClosedEventResource );
		fPopupClosedEventResource = NULL;
	}
}

void
NativeToJavaBridge::DisplayUpdate()
{
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callDisplayUpdate", "(Lcom/ansca/corona/CoronaRuntime;)V" );
		
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::GetString( const char *method, Rtt::String *outValue )
{
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	NativeTrace trace( method );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
														    method, "(Lcom/ansca/corona/CoronaRuntime;)Ljava/lang/String;" );
		
		if ( mid != NULL ) {
			jobject jo = bridge.getEnv()->CallStaticObjectMethod( bridge.getClass(), mid, fCoronaRuntime );
			HandleJavaException();
			if (jo)
			{
				jstringResult jstr( bridge.getEnv() );
				jstr.setString( (jstring) jo );
				if ( jstr.isValidString() )
				{
					outValue->Set( jstr.getUTF8() );
				}
			}
		}
	}
}

void
NativeToJavaBridge::GetStringWithInt( const char *method, int intParameter, Rtt::String *outValue )
{
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	NativeTrace trace( method );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			method, "(ILcom/ansca/corona/CoronaRuntime;)Ljava/lang/String;" );
		
		if ( mid != NULL ) {
			jobject jo = bridge.getEnv()->CallStaticObjectMethod( bridge.getClass(), mid, intParameter, fCoronaRuntime );
			HandleJavaException();
			if (jo)
			{
				jstringResult jstr( bridge.getEnv() );
				jstr.setString( (jstring) jo );
				if ( jstr.isValidString() )
				{
					outValue->Set( jstr.getUTF8() );
				}
			}
		}
	}	
}

void
NativeToJavaBridge::GetStringWithLong( const char *method, long longParameter, Rtt::String *outValue )
{
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	NativeTrace trace( method );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(),
															method, "(JLcom/ansca/corona/CoronaRuntime;)Ljava/lang/String;" );

		if ( mid != NULL ) {
			jobject jo = bridge.getEnv()->CallStaticObjectMethod( bridge.getClass(), mid, (jlong)longParameter, fCoronaRuntime );
			HandleJavaException();
			if (jo)
			{
				jstringResult jstr( bridge.getEnv() );
				jstr.setString( (jstring) jo );
				if ( jstr.isValidString() )
				{
					outValue->Set( jstr.getUTF8() );
				}
			}
		}
	}
}

void
NativeToJavaBridge::GetManufacturerName( Rtt::String *outValue )
{
	GetString( "callGetManufacturerName", outValue );
	HandleJavaException();
}

void
NativeToJavaBridge::GetModel( Rtt::String *outValue )
{
	GetString( "callGetModel", outValue );
	HandleJavaException();
}

void
NativeToJavaBridge::GetName( Rtt::String *outValue )
{
	GetString( "callGetName", outValue );
	HandleJavaException();
}

void
NativeToJavaBridge::GetUniqueIdentifier( int t, Rtt::String *outValue )
{
	GetStringWithInt( "callGetUniqueIdentifier", t, outValue );
	HandleJavaException();
}

void
NativeToJavaBridge::GetPlatformVersion( Rtt::String *outValue )
{
	GetString( "callGetPlatformVersion", outValue );
	HandleJavaException();
}

void
NativeToJavaBridge::GetProductName( Rtt::String *outValue )
{
	GetString( "callGetProductName", outValue );
	HandleJavaException();
}

float
NativeToJavaBridge::GetDefaultFontSize()
{
	NativeTrace trace("NativeToJavaBridge::GetDefaultFontSize");

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	float result = 16.0f;
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), "callGetDefaultFontSize", "(Lcom/ansca/corona/CoronaRuntime;)F");
		if (mid != NULL)
		{
			result = (float)bridge.getEnv()->CallStaticFloatMethod(bridge.getClass(), mid, fCoronaRuntime);
			HandleJavaException();
		}
	}
	return result;
}

int
NativeToJavaBridge::GetDefaultTextFieldPaddingInPixels()
{
	NativeTrace trace("NativeToJavaBridge::GetDefaultTextFieldPaddingInPixels");

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	int result = 10;
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(),
				"callGetDefaultTextFieldPaddingInPixels", "(Lcom/ansca/corona/CoronaRuntime;)I");
		if (mid != NULL)
		{
			result = (int)bridge.getEnv()->CallStaticIntMethod(bridge.getClass(), mid, fCoronaRuntime);
			HandleJavaException();
		}
	}
	return result;
}

int
NativeToJavaBridge::GetApproximateScreenDpi()
{
	NativeTrace trace( "NativeToJavaBridge::GetApproximateScreenDpi" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	int result = 0;
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callGetApproximateScreenDpi", "()I" );
		if (mid != NULL)
		{
			result = (int)bridge.getEnv()->CallStaticIntMethod(bridge.getClass(), mid);
			HandleJavaException();
		}
	}
	return result;

}

int
NativeToJavaBridge::PushSystemInfoToLua( lua_State *L, const char *key )
{
	NativeTrace trace( "NativeToJavaBridge::PushSystemInfoToLua" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	int result = 0;
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callPushSystemInfoToLua", "(Lcom/ansca/corona/CoronaRuntime;JLjava/lang/String;)I" );
		if (mid != NULL)
		{
			jstringParam keyJ(bridge.getEnv(), key);
			result = (int)bridge.getEnv()->CallStaticIntMethod(
							bridge.getClass(), mid, fCoronaRuntime, (jlong)(uintptr_t)L, keyJ.getValue());
			HandleJavaExceptionUsing(L);
		}
	}
	return result;
}

void
NativeToJavaBridge::GetPreference( int category, Rtt::String *outValue )
{
	GetStringWithInt( "callGetPreference", category, outValue );
	HandleJavaException();
}

Rtt::Preference::ReadValueResult
NativeToJavaBridge::GetPreference( const char* keyName )
{
	// Fetch a reference to this feature's Java method.
	jmethodID getPreferenceMethodId = NULL;
	jclassInstance bridge(GetJNIEnv(), kNativeToJavaBridge);
	if (bridge.isValid())
	{
		getPreferenceMethodId = bridge.getEnv()->GetStaticMethodID(
				bridge.getClass(), "callGetPreference", "(Ljava/lang/String;)Ljava/lang/Object;" );
	}
	if (!getPreferenceMethodId)
	{
		return Rtt::Preference::ReadValueResult::FailedWith("JNI bridge failure.");
	}

	// Attempt to read the given preference key.
	jstringParam javaKeyName(bridge.getEnv(), keyName);
	jobject javaObjectResult = bridge.getEnv()->CallStaticObjectMethod(
				bridge.getClass(), getPreferenceMethodId, javaKeyName.getValue());
	HandleJavaException();

	// If the Java method returned null, then the preference was not found.
	if (!javaObjectResult)
	{
		return Rtt::Preference::ReadValueResult::kPreferenceNotFound;
	}

	// An object was returned.
	// Use reflection to determine if we've received a valid preference value type.
	{
		// Is it a string?
		jclassInstance javaValueType(GetJNIEnv(), "java/lang/String");
		if (GetJNIEnv()->IsInstanceOf(javaObjectResult, javaValueType.getClass()))
		{
			jstringResult javaStringValue(bridge.getEnv());
			javaStringValue.setString((jstring)javaObjectResult);
			if (javaStringValue.isValidString())
			{
				return Rtt::Preference::ReadValueResult::SucceededWith(javaStringValue.getUTF8());
			}
			return Rtt::Preference::ReadValueResult::SucceededWith("");
		}
	}
	{
		// Is it a boolean?
		jclassInstance javaValueType(GetJNIEnv(), "java/lang/Boolean");
		if (GetJNIEnv()->IsInstanceOf(javaObjectResult, javaValueType.getClass()))
		{
			jmethodID methodId = javaValueType.getEnv()->GetMethodID(
					javaValueType.getClass(), "booleanValue", "()Z");
			if (!methodId)
			{
				return Rtt::Preference::ReadValueResult::FailedWith("Failed to extract value from Java 'Boolean' object.");
			}
			bool value = javaValueType.getEnv()->CallBooleanMethod(javaObjectResult, methodId) ? true : false;
			HandleJavaException();
			return Rtt::Preference::ReadValueResult::SucceededWith(value);
		}
	}
	{
		// Is it a 32-bit signed integer?
		jclassInstance javaValueType(GetJNIEnv(), "java/lang/Integer");
		if (GetJNIEnv()->IsInstanceOf(javaObjectResult, javaValueType.getClass()))
		{
			jmethodID methodId = javaValueType.getEnv()->GetMethodID(
					javaValueType.getClass(), "intValue", "()I");
			if (!methodId)
			{
				return Rtt::Preference::ReadValueResult::FailedWith("Failed to extract value from Java 'Integer' object.");
			}
			int value = (int)javaValueType.getEnv()->CallIntMethod(javaObjectResult, methodId);
			HandleJavaException();
			return Rtt::Preference::ReadValueResult::SucceededWith(value);
		}
	}
	{
		// Is it a 64-bit signed integer?
		jclassInstance javaValueType(GetJNIEnv(), "java/lang/Long");
		if (GetJNIEnv()->IsInstanceOf(javaObjectResult, javaValueType.getClass()))
		{
			jmethodID methodId = javaValueType.getEnv()->GetMethodID(
					javaValueType.getClass(), "longValue", "()J");
			if (!methodId)
			{
				return Rtt::Preference::ReadValueResult::FailedWith("Failed to extract value from Java 'Long' object.");
			}
			S64 value = (S64)javaValueType.getEnv()->CallLongMethod(javaObjectResult, methodId);
			HandleJavaException();
			return Rtt::Preference::ReadValueResult::SucceededWith(value);
		}
	}
	{
		// Is it a single precision float?
		jclassInstance javaValueType(GetJNIEnv(), "java/lang/Float");
		if (GetJNIEnv()->IsInstanceOf(javaObjectResult, javaValueType.getClass()))
		{
			jmethodID methodId = javaValueType.getEnv()->GetMethodID(
					javaValueType.getClass(), "longValue", "()F");
			if (!methodId)
			{
				return Rtt::Preference::ReadValueResult::FailedWith("Failed to extract value from Java 'Float' object.");
			}
			float value = (float)javaValueType.getEnv()->CallFloatMethod(javaObjectResult, methodId);
			HandleJavaException();
			return Rtt::Preference::ReadValueResult::SucceededWith(value);
		}
	}

	// If a Java exception object was returned, then an error occurred.
	// Return a failure result to the caller using the exception object's message.
	{
		jclassInstance javaThrowableType(GetJNIEnv(), "java/lang/Throwable");
		if (GetJNIEnv()->IsInstanceOf(javaObjectResult, javaThrowableType.getClass()))
		{
			jmethodID methodId = javaThrowableType.getEnv()->GetMethodID(
					javaThrowableType.getClass(), "getMessage", "()Ljava/lang/String;");
			if (!methodId)
			{
				return Rtt::Preference::ReadValueResult::FailedWith("Failed to fetch message from Java 'Exception' object.");
			}
			javaObjectResult = javaThrowableType.getEnv()->CallObjectMethod(javaObjectResult, methodId);
			HandleJavaException();
			jstringResult javaExceptionMessage(bridge.getEnv());
			if (javaObjectResult)
			{
				javaExceptionMessage.setString((jstring)javaObjectResult);
			}
			if (javaExceptionMessage.isValidString())
			{
				return Rtt::Preference::ReadValueResult::FailedWith(javaExceptionMessage.getUTF8());
			}
			else
			{
				return Rtt::Preference::ReadValueResult::FailedWith("Unknown Java exception error occurred.");
			}
		}
	}

	// An unknown Java object type was returned. Return a failure result.
	return Rtt::Preference::ReadValueResult::FailedWith("Received unknown/unsupported Java value type.");
}

Rtt::OperationResult
NativeToJavaBridge::SetPreferences( const Rtt::PreferenceCollection& collection )
{
	// Fetch a reference to this feature's Java method.
	jmethodID setPreferencesMethodId = NULL;
	jclassInstance bridge(GetJNIEnv(), kNativeToJavaBridge);
	if (bridge.isValid())
	{
		setPreferencesMethodId = bridge.getEnv()->GetStaticMethodID(
				bridge.getClass(), "callSetPreferences", "(Ljava/util/HashMap;)Ljava/lang/String;" );
	}
	if (!setPreferencesMethodId)
	{
		return Rtt::OperationResult::FailedWith("JNI bridge failure.");
	}

	// Set up a collection of all value types supported by Android's SharedPreferences feature.
	Rtt::PreferenceValue::TypeSet supportedTypes;
	supportedTypes.Add(Rtt::PreferenceValue::kTypeBoolean);
	supportedTypes.Add(Rtt::PreferenceValue::kTypeSignedInt32);
	supportedTypes.Add(Rtt::PreferenceValue::kTypeSignedInt64);
	supportedTypes.Add(Rtt::PreferenceValue::kTypeFloatSingle);
	supportedTypes.Add(Rtt::PreferenceValue::kTypeString);

	// Copy the given native preference key-value pairs to a Java HashMap collection.
	jHashMapParam hashMap(GetJNIEnv());
	for (int index = 0; index < collection.GetCount(); index++)
	{
		// Fetch the next preference in the native collection.
		Rtt::Preference* preferencePointer = collection.GetByIndex(index);
		if (!preferencePointer || !preferencePointer->GetKeyName())
		{
			continue;
		}

		// Attempt to convert the given preference value to a type supported by Android's SharedPreferences feature.
		Rtt::PreferenceValue preferenceValue = preferencePointer->GetValue();
		Rtt::ValueResult<Rtt::PreferenceValue> conversionResult = preferenceValue.ToClosestValueTypeIn(supportedTypes);
		if (conversionResult.HasFailed())
		{
			return Rtt::OperationResult::FailedWith(conversionResult.GetUtf8MessageAsSharedPointer());
		}
		preferenceValue = conversionResult.GetValue();

		// Add the native preference key-value pair to the Java HashMap.
		switch (preferenceValue.GetType())
		{
			case Rtt::PreferenceValue::kTypeBoolean:
				hashMap.put(preferencePointer->GetKeyName(), preferenceValue.ToBoolean().GetValue());
				break;
			case Rtt::PreferenceValue::kTypeSignedInt32:
				hashMap.put(preferencePointer->GetKeyName(), preferenceValue.ToSignedInt32().GetValue());
				break;
			case Rtt::PreferenceValue::kTypeSignedInt64:
				hashMap.put(preferencePointer->GetKeyName(), preferenceValue.ToSignedInt64().GetValue());
				break;
			case Rtt::PreferenceValue::kTypeFloatSingle:
				hashMap.put(preferencePointer->GetKeyName(), preferenceValue.ToFloatSingle().GetValue());
				break;
			case Rtt::PreferenceValue::kTypeString:
			default:
				hashMap.put(preferencePointer->GetKeyName(), preferenceValue.ToString().GetValue()->c_str());
				break;
		}
	}

	// Write the given preferences to Android's SharedPreferences feature.
	jobject javaObjectResult = bridge.getEnv()->CallStaticObjectMethod(
				bridge.getClass(), setPreferencesMethodId, hashMap.getHashMapObject());
	HandleJavaException();

	// If a string object was returned, then an error occurred.
	// The returned string provides the error message.
	if (javaObjectResult)
	{
		jstringResult javaStringValue(bridge.getEnv());
		javaStringValue.setString((jstring)javaObjectResult);
		if (javaStringValue.isValidString())
		{
			return Rtt::OperationResult::FailedWith(javaStringValue.getUTF8());
		}
		return Rtt::OperationResult::FailedWith("Unknown error occurred.");
	}

	// Java returned null. This means the operation was a success!
	return Rtt::OperationResult::kSucceeded;
}

Rtt::OperationResult
NativeToJavaBridge::DeletePreferences( const char** keyNameArray, size_t keyNameCount )
{
	// Validate arguments.
	if (!keyNameArray || (keyNameCount <= 0))
	{
		return Rtt::OperationResult::FailedWith("Key name array is null or empty.");
	}

	// Fetch a reference to this feature's Java method.
	jmethodID methodId = NULL;
	jclassInstance bridge(GetJNIEnv(), kNativeToJavaBridge);
	if (bridge.isValid())
	{
		methodId = bridge.getEnv()->GetStaticMethodID(
				bridge.getClass(), "callDeletePreferences", "([Ljava/lang/String;)Ljava/lang/String;" );
	}
	if (!methodId)
	{
		return Rtt::OperationResult::FailedWith("JNI bridge failure.");
	}

	// Copy the given native strings to a Java string array.
	jstringArrayParam javaStringArray(bridge.getEnv(), keyNameCount);
	for (size_t index = 0; index < keyNameCount; index++)
	{
		javaStringArray.setElement((int)index, keyNameArray[index]);
	}

	// Attempt to delete the given keys from Android's SharedPreferences.
	jobject javaObjectResult = bridge.getEnv()->CallStaticObjectMethod(
				bridge.getClass(), methodId, javaStringArray.getValue());
	HandleJavaException();

	// If a string object was returned, then an error occurred.
	// The returned string provides the error message.
	if (javaObjectResult)
	{
		jstringResult javaStringValue(bridge.getEnv());
		javaStringValue.setString((jstring)javaObjectResult);
		if (javaStringValue.isValidString())
		{
			return Rtt::OperationResult::FailedWith(javaStringValue.getUTF8());
		}
		return Rtt::OperationResult::FailedWith("Unknown error occurred.");
	}

	// Java returned null. This means the operation was a success!
	return Rtt::OperationResult::kSucceeded;
}

void
NativeToJavaBridge::GetSystemProperty( const char *name, Rtt::String *outValue )
{
	// Validate arguments.
	if (!name || !outValue)
	{
		return;
	}

	// Fetch the requested property from the Java "System" class.
	jclassInstance bridge(GetJNIEnv(), "java/lang/System");
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
							bridge.getClass(), "getProperty", "(Ljava/lang/String;)Ljava/lang/String;" );
		if (mid != NULL) {
			jstringParam nameJ(bridge.getEnv(), name);
			jobject jo = bridge.getEnv()->CallStaticObjectMethod(bridge.getClass(), mid, nameJ.getValue());
			HandleJavaException();
			if (jo)
			{
				jstringResult jstr(bridge.getEnv());
				jstr.setString((jstring)jo);
				if (jstr.isValidString())
				{
					outValue->Set(jstr.getUTF8());
				}
			}
		}
	}	
}

long
NativeToJavaBridge::GetUptimeInMilliseconds()
{
	// Call the SystemClock.uptimeMillis() static method in Java.
	long result = 0;
	jclassInstance bridge(GetJNIEnv(), "android/os/SystemClock");
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), "uptimeMillis", "()J");
		if (mid != NULL)
		{
			result = (long)bridge.getEnv()->CallStaticLongMethod(bridge.getClass(), mid);
			HandleJavaException();
		}
	}
	return result;
}

void
NativeToJavaBridge::MakeLowerCase(Rtt::String *stringToConvert)
{
	// Validate.
	if (!stringToConvert || stringToConvert->IsEmpty())
	{
		return;
	}

	// Convert the given string to lower case via the String.toLowerCase() Java method.
	// The reason we do this is because Java strings will correctly convert unicode strings.
	jclassInstance bridge(GetJNIEnv(), "java/lang/String");
	if (bridge.isValid())
	{
		jmethodID methodId = bridge.getEnv()->GetMethodID(
								bridge.getClass(), "toLowerCase", "()Ljava/lang/String;");
		if (methodId)
		{
			jstringParam javaString(bridge.getEnv(), stringToConvert->GetString());
			jobject resultObject = bridge.getEnv()->CallObjectMethod(javaString.getObject(), methodId);
			if (resultObject)
			{
				jstringResult lowerCaseJavaString(bridge.getEnv());
				lowerCaseJavaString.setString((jstring)resultObject);
				if (lowerCaseJavaString.isValidString())
				{
					stringToConvert->Set(lowerCaseJavaString.getUTF8());
				}
			}
		}
	}
}

void
NativeToJavaBridge::Vibrate(const char * hapticType, const char* hapticStyle)
{
	HandleJavaException();
	NativeTrace trace( "NativeToJavaBridge::Vibrate" );


	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {

		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(),
															"callVibrate", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;Ljava/lang/String;)V" );

		if ( mid != NULL ) {
			jstringParam hapticTypeJ( bridge.getEnv(), hapticType );
			jstringParam hapticStyleJ( bridge.getEnv(), hapticStyle );

			bridge.getEnv()->CallStaticVoidMethod(
					bridge.getClass(), mid, fCoronaRuntime, hapticTypeJ.getValue(), hapticStyleJ.getValue());
			HandleJavaException();

		}
	}
}

void
NativeToJavaBridge::SetAccelerometerInterval( int frequency )
{
	CallIntMethod( "callSetAccelerometerInterval", frequency );
	HandleJavaException();
}

void
NativeToJavaBridge::SetGyroscopeInterval( int frequency )
{
	CallIntMethod( "callSetGyroscopeInterval", frequency );
	HandleJavaException();
}

bool
NativeToJavaBridge::HasAccelerometer()
{
	NativeTrace trace( "NativeToJavaBridge::HasAccelerometer" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = true;
	if ( bridge.isValid() )
	{
		jmethodID mid;
		mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callHasAccelerometer", "(Lcom/ansca/corona/CoronaRuntime;)Z" );
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime );
			HandleJavaException();
		}
	}
	return result;
}

bool
NativeToJavaBridge::HasGyroscope()
{
	NativeTrace trace( "NativeToJavaBridge::HasGyroscope" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = true;
	if ( bridge.isValid() )
	{
		jmethodID mid;
		mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callHasGyroscope", "(Lcom/ansca/corona/CoronaRuntime;)Z" );
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime );
			HandleJavaException();
		}
	}
	return result;
}

bool
NativeToJavaBridge::HasHeadingHardware()
{
	NativeTrace trace( "NativeToJavaBridge::HasHeadingHardware" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = true;
	if ( bridge.isValid() )
	{
		jmethodID mid;
		mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callHasHeadingHardware", "(Lcom/ansca/corona/CoronaRuntime;)Z" );
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime );
			HandleJavaException();
		}
	}
	return result;
}

void 
NativeToJavaBridge::SetEventNotification( int eventType, bool enable )
{
	NativeTrace trace( "NativeToJavaBridge::SetEventNotification" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(),
			"callSetEventNotification", 
			"(Lcom/ansca/corona/CoronaRuntime;IZ)V" );
		
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, (jint) eventType, (jboolean) enable );
			HandleJavaException();
		}
	}
}

bool
NativeToJavaBridge::GetFonts( Rtt::StringArray & outFonts )
{
	bool result = false;

	NativeTrace trace( "NativeToJavaBridge::GetFonts" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() ) {
		jmethodID mid =  bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callGetFonts", 
			"()[Ljava/lang/String;" );

		if ( mid != NULL ) {

			jobject resultArray;

			resultArray =  bridge.getEnv()->CallStaticObjectMethod( bridge.getClass(), mid );
			HandleJavaException();

			if ( resultArray != NULL )
			{
				jstringArrayResult jstrings( bridge.getEnv(), (jobjectArray) resultArray );
				jsize arraySize = jstrings.getLength();
				outFonts.Reserve( arraySize );
				jstringResult jstr( bridge.getEnv() );
				
				for ( int i = 0; i < arraySize; i++ )
				{
					jstr.setString( jstrings.getString( i ) );
					outFonts.SetElement( i, jstr.getUTF8() );
				}
				
				result = true;
				bridge.getEnv()->DeleteLocalRef(resultArray);
			}
		}
	}
	
	return result;	
}

Rtt::FontMetricsMap
NativeToJavaBridge::GetFontMetrics ( const char *fontName, float fontSize, bool isBold )
{
	NativeTrace trace( "NativeToJavaBridge::GetFontMetrics" );

	Rtt::FontMetricsMap jfMetrics;
	jclassInstance bridge(GetJNIEnv(), kNativeToJavaBridge);
	if (bridge.isValid())
	{
		jmethodID methodId = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), "callGetFontMetrics", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;FZ)[F");
		if (methodId)
		{
			jstringParam fontNameJ( bridge.getEnv(), fontName );
			jfloat jFontSize = fontSize;
			jobject objArray = bridge.getEnv()->CallStaticObjectMethod( bridge.getClass(), methodId, fCoronaRuntime, fontNameJ.getValue(), jFontSize, isBold );
			jfloatArray * jfArray = reinterpret_cast< jfloatArray* >( & objArray );
			jsize len = bridge.getEnv()->GetArrayLength( *jfArray );
			float* data = bridge.getEnv()->GetFloatArrayElements( *jfArray, 0 );
			if ( len == 4 )
			{
				jfMetrics [ "ascent" ]  = data [ 0 ];
				jfMetrics [ "descent" ] = data [ 1 ];
				jfMetrics [ "leading" ] = data [ 2 ];
				jfMetrics [ "height" ]  = data [ 3 ];
			}
			bridge.getEnv()->ReleaseFloatArrayElements( *jfArray, data, 0 );
			bridge.getEnv()->DeleteLocalRef( *jfArray );
			HandleJavaException();
		}
	}
	return jfMetrics;
}

void
NativeToJavaBridge::SetLocationAccuracy( double meters )
{
	CallDoubleMethod( "callSetLocationAccuracy", meters );
	HandleJavaException();
}

void
NativeToJavaBridge::SetLocationThreshold( double meters )
{
	CallDoubleMethod( "callSetLocationThreshold", meters );
	HandleJavaException();
}

void 
NativeToJavaBridge::SetVolume( uintptr_t id, float volume )
{
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callSetVolume", "(Lcom/ansca/corona/CoronaRuntime;JF)V" );
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, (jlong)id, volume );
			HandleJavaException();
		}
	}
}

float 
NativeToJavaBridge::GetVolume( uintptr_t id ) const
{
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jfloat result = 0.0f;
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callGetVolume", "(Lcom/ansca/corona/CoronaRuntime;J)F" );
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticFloatMethod( bridge.getClass(), mid, fCoronaRuntime, (jlong)id );
			HandleJavaException();
		}
	}
	return result;
}

int
NativeToJavaBridge::TextFieldCreate( int id, int left, int top, int width, int height, int isSingleLine )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldCreate" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	int result = 0;
	
	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callTextFieldCreate", "(Lcom/ansca/corona/CoronaRuntime;IIIIIZ)I" );
		
		if ( mid != NULL ) {
			result = bridge.getEnv()->CallStaticIntMethod( bridge.getClass(), mid, 
				fCoronaRuntime, id, left, top, width, height, (jboolean) isSingleLine );
			HandleJavaException();
		}
	}
	
	return result;
}

void
NativeToJavaBridge::TextFieldSetReturnKey( int id, const char * keyType )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldSetReturnKey" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(),
			"callTextFieldSetReturnKey", "(Lcom/ansca/corona/CoronaRuntime;ILjava/lang/String;)V");

		if ( mid != NULL ) {
			jstringParam textJ( bridge.getEnv(), keyType );
			if ( textJ.isValid() ) {
				bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, textJ.getValue() );
				HandleJavaException();
			}	
		}
	}
}

void
NativeToJavaBridge::TextFieldSetSelection( int id, int startPosition, int endPosition )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldSetSelection" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(),
			"callTextFieldSetSelection", "(Lcom/ansca/corona/CoronaRuntime;III)V");

		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, startPosition, endPosition );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::TextFieldSetPlaceholder( int id, const char * placeholder )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldSetReturnKey" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(),
			"callTextFieldSetPlaceholder", "(Lcom/ansca/corona/CoronaRuntime;ILjava/lang/String;)V");

		if ( mid != NULL ) {
			jstringParam textJ( bridge.getEnv(), placeholder );
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, textJ.getValue() );
			HandleJavaException();
		}
	}		
}

void
NativeToJavaBridge::TextFieldSetColor( int id, int r, int g, int b, int a )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldSetColor" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callTextFieldSetColor", "(Lcom/ansca/corona/CoronaRuntime;IIIII)V" );
		
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, r, g, b, a );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::TextFieldSetText( int id, const char * text )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldSetText" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callTextFieldSetText", "(Lcom/ansca/corona/CoronaRuntime;ILjava/lang/String;)V" );
		
		if ( mid != NULL ) {
			jstringParam textJ( bridge.getEnv(), text );

			if ( textJ.isValid() ) {
				bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, textJ.getValue() );
				HandleJavaException();
			}
		}
	}
}

void
NativeToJavaBridge::TextFieldSetSize( int id, float fontSize )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldSetSize" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callTextFieldSetSize", "(Lcom/ansca/corona/CoronaRuntime;IF)V" );
		
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, fontSize );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::TextFieldSetFont( int id, const char * fontName, float fontSize, bool isBold )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldSetFont" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callTextFieldSetFont", "(Lcom/ansca/corona/CoronaRuntime;ILjava/lang/String;FZ)V" );

		if ( mid != NULL ) {
			if ( fontName == NULL )
				fontName = "";
			
			jstringParam fontNameJ( bridge.getEnv(), fontName );

			if ( fontNameJ.isValid() ) {
				bridge.getEnv()->CallStaticVoidMethod(
						bridge.getClass(), mid, fCoronaRuntime, id, fontNameJ.getValue(), fontSize, isBold);
				HandleJavaException();
			}
		}
	}
}

void
NativeToJavaBridge::TextFieldSetAlign( int id, const char * align )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldSetAlign" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callTextFieldSetAlign", "(Lcom/ansca/corona/CoronaRuntime;ILjava/lang/String;)V" );
		
		if ( mid != NULL ) {
			jstringParam alignJ( bridge.getEnv(), align );

			if ( alignJ.isValid() ) {
				bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, alignJ.getValue() );
				HandleJavaException();
			}
		}
	}
}

void
NativeToJavaBridge::TextFieldSetSecure( int id, bool isSecure )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldSetSecure" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callTextFieldSetSecure", "(Lcom/ansca/corona/CoronaRuntime;IZ)V" );
		
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, isSecure );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::TextFieldSetInputType( int id, const char * inputType )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldSetInputType" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callTextFieldSetInputType", "(Lcom/ansca/corona/CoronaRuntime;ILjava/lang/String;)V" );
		
		if ( mid != NULL ) {
			jstringParam inputTypeJ( bridge.getEnv(), inputType );

			if ( inputTypeJ.isValid() ) {
				bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, inputTypeJ.getValue() );
				HandleJavaException();
			}
		}
	}
}

void
NativeToJavaBridge::TextFieldSetEditable( int id, bool isEditable )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldSetEditable" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
														   "callTextFieldSetEditable", "(Lcom/ansca/corona/CoronaRuntime;IZ)V" );
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, isEditable );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::TextFieldGetColor( int id, int * r, int * g, int * b, int * a )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldGetColor" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	*r = *g = *b = *a = 0;

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callTextFieldGetColor", "(Lcom/ansca/corona/CoronaRuntime;I)[I" );
		
		if ( mid != NULL ) {
			jobject jo = bridge.getEnv()->CallStaticObjectMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
			if (jo)
			{
				jintArrayResult array( bridge.getEnv(), (jintArray) jo );
				if ( array.isValid() && array.getLength() == 4 ) {
					const int * values = (const int *) array.getValues();
					*r = values[0];
					*g = values[1];
					*b = values[2];
					*a = values[3];
				}
				array.release();
				bridge.getEnv()->DeleteLocalRef(jo);
			}
		}
	}
}

void
NativeToJavaBridge::TextFieldGetText( int id, Rtt::String * text )
{
	GetStringWithInt( "callTextFieldGetText", id, text );
	HandleJavaException();
}

void
NativeToJavaBridge::TextFieldGetPlaceholder( int id, Rtt::String * placeholder )
{
	GetStringWithInt( "callTextFieldGetPlaceholder", id, placeholder );
	HandleJavaException();
}

float
NativeToJavaBridge::TextFieldGetSize( int id )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldGetSize" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callTextFieldGetSize", "(Lcom/ansca/corona/CoronaRuntime;I)F" );
		
	if ( mid != NULL )
	{
		jfloat size = bridge.getEnv()->CallStaticFloatMethod( bridge.getClass(), mid, fCoronaRuntime, id );
		HandleJavaException();
		return size;
	}
	return 0.0f;
}

void
NativeToJavaBridge::TextFieldGetFont( int id, Rtt::String * font, float * fontSize )
{
}

void
NativeToJavaBridge::TextFieldGetAlign( int id, Rtt::String * align )
{
	GetStringWithInt( "callTextFieldGetAlign", id, align );
	HandleJavaException();
}

bool
NativeToJavaBridge::TextFieldGetSecure( int id )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldGetSecure" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callTextFieldGetSecure", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		
		if ( mid != NULL ) {
			jboolean result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
			return result;
		}
	}
	
	return false;
}

void
NativeToJavaBridge::TextFieldGetInputType( int id, Rtt::String * inputType )
{
	GetStringWithInt( "callTextFieldGetInputType", id, inputType );
	HandleJavaException();
}

bool
NativeToJavaBridge::TextFieldIsSingleLine( int id )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldIsSingleLine" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
														   "callTextFieldIsSingleLine", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		if ( mid != NULL ) {
			jboolean result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
			return result;
		}
	}
	return true;
}

bool
NativeToJavaBridge::TextFieldIsEditable( int id )
{
	NativeTrace trace( "NativeToJavaBridge::TextFieldIsEditable" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
														   "callTextFieldIsEditable", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		if ( mid != NULL ) {
			jboolean result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
			return result;
		}
	}
	return true;
}

void
NativeToJavaBridge::DisplayObjectDestroy( int id )
{
	NativeTrace trace( "NativeToJavaBridge::DisplayObjectDestroy" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
							bridge.getClass(), "callDisplayObjectDestroy", "(Lcom/ansca/corona/CoronaRuntime;I)V" );
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::DisplayObjectSetVisible( int id, bool visible )
{
	NativeTrace trace( "NativeToJavaBridge::DisplayObjectSetVisible" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callDisplayObjectSetVisible", "(Lcom/ansca/corona/CoronaRuntime;IZ)V" );
		
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, visible );
			HandleJavaException();
		}
	}
}

void 
NativeToJavaBridge::DisplayObjectSetAlpha( int id, float alpha )
{
	NativeTrace trace( "NativeToJavaBridge::DisplayObjectSetAlpha" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callDisplayObjectSetAlpha", "(Lcom/ansca/corona/CoronaRuntime;IF)V" );
		
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, alpha );
			HandleJavaException();
		}
	}
}

void 
NativeToJavaBridge::DisplayObjectSetBackground( int id, bool bg )
{
	NativeTrace trace( "NativeToJavaBridge::DisplayObjectSetBackground" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callDisplayObjectSetBackground", "(Lcom/ansca/corona/CoronaRuntime;IZ)V" );
		
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, bg );
			HandleJavaException();
		}
	}
}

bool 
NativeToJavaBridge::DisplayObjectGetVisible( int id )
{
	NativeTrace trace( "NativeToJavaBridge::DisplayObjectGetVisible" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callDisplayObjectGetVisible", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		
		if ( mid != NULL ) {
			jboolean result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
			return result;
		}
	}
	
	return false;
}

float 
NativeToJavaBridge::DisplayObjectGetAlpha( int id )
{
	NativeTrace trace( "NativeToJavaBridge::DisplayObjectGetAlpha" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callDisplayObjectGetAlpha", "(Lcom/ansca/corona/CoronaRuntime;I)F" );
		
		if ( mid != NULL ) {
			jfloat result = bridge.getEnv()->CallStaticFloatMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
			return result;
		}
	}
	
	return 0;
}

bool
NativeToJavaBridge::DisplayObjectGetBackground( int id )
{
	NativeTrace trace( "NativeToJavaBridge::DisplayObjectGetBackground" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = false;

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callDisplayObjectGetBackground", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		
		if ( mid != NULL ) {
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
		}
	}
	
	return result;
}

void
NativeToJavaBridge::DisplayObjectSetFocus( int id, bool focus )
{
	NativeTrace trace( "NativeToJavaBridge::DisplayObjectSetFocus" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callDisplayObjectSetFocus", "(Lcom/ansca/corona/CoronaRuntime;IZ)V" );
		
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, focus );
			HandleJavaException();
		}
	}
}

void 
NativeToJavaBridge::DisplayObjectUpdateScreenBounds( int id, int x, int y, int width, int height )
{
	NativeTrace trace( "NativeToJavaBridge::DisplayObjectUpdateScreenBounds" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callDisplayObjectUpdateScreenBounds", "(Lcom/ansca/corona/CoronaRuntime;IIIII)V" );
		
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, x, y, width, height );
			HandleJavaException();
		}
	}
}

bool NativeToJavaBridge::DisplayObjectSetNativeProperty(int id, const char key[], lua_State *L, int valueIndex)
{
    bool ret = false;
	NativeTrace trace( "NativeToJavaBridge::DisplayObjectSetNativeProperty" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
	    jstringParam textJ( bridge.getEnv(), key );
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(),
			"callDisplayObjectSetNativeProperty", "(Lcom/ansca/corona/CoronaRuntime;ILjava/lang/String;JI)Z" );

		if ( mid != NULL ) {
			ret = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, id, textJ.getValue(), (jlong)(uintptr_t)L, valueIndex );
			HandleJavaException();
		}
	}
	return ret;
}

bool
NativeToJavaBridge::RecordStart( uintptr_t id, const char * file )
{
	NativeTrace trace( "NativeToJavaBridge::RecordStart" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = false;
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callRecordStart", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;J)Z" );
		
		if ( mid != NULL )
		{
			if ( file == NULL )
			{
				file = "";
			}

			jstringParam paramJ( bridge.getEnv(),  file );

			if ( paramJ.isValid() )
			{
				result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, paramJ.getValue(), (jlong)id );
				HandleJavaException();
			}
		}
	}

	return result;
}

void 
NativeToJavaBridge::RecordStop( uintptr_t id )
{
	CallLongMethod( "callRecordStop", id );
	HandleJavaException();
}

bool 
NativeToJavaBridge::RecordGetBytes( uintptr_t id, Rtt::Data<char> & data )
{
	bool result = false;
	
	NativeTrace trace( "NativeToJavaBridge::RecordGetBytes" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callRecordGetBytes", "(Lcom/ansca/corona/CoronaRuntime;J)Ljava/nio/ByteBuffer;" );
		jmethodID mid2 = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callRecordGetCurrentByteCount", "(Lcom/ansca/corona/CoronaRuntime;J)I" );
		int numBytes = 0;

		if ( mid != NULL ) {
			jobject jo = bridge.getEnv()->CallStaticObjectMethod( bridge.getClass(), mid, fCoronaRuntime, (jlong)id );
			HandleJavaException();
			if ( jo && mid2 != NULL ) {
				numBytes = bridge.getEnv()->CallStaticIntMethod( bridge.getClass(), mid2, fCoronaRuntime, (jlong)id );
				HandleJavaException();
			}

			void * address = NULL;
			
			if ( jo )
			{
				address = bridge.getEnv()->GetDirectBufferAddress( jo );
			
				if ( address && numBytes > 0 ) {
					data.Set( (const char *) address, numBytes );
					RecordReleaseCurrentBuffer( id );
					result = true;
				}
				bridge.getEnv()->DeleteLocalRef( jo );
			}
		}
	}

	return result;
}

void 
NativeToJavaBridge::RecordReleaseCurrentBuffer( uintptr_t id )
{
	CallLongMethod( "callRecordReleaseCurrentBuffer", (jlong)id );
	HandleJavaException();
}

void 
NativeToJavaBridge::RecordCallback( uintptr_t id, int status )
{
	Rtt::LuaLibMedia::RecordCallback( id, status );
}

void 
NativeToJavaBridge::SoundEndCallback( uintptr_t id )
{
	Rtt::LuaLibMedia::SoundEndCallback( id );
}

void 
NativeToJavaBridge::VideoEndCallback( uintptr_t id )
{
	Rtt::LuaLibMedia::VideoEndCallback( id );
}

void
NativeToJavaBridge::WebViewCreate(
	int id, int left, int top, int width, int height, bool isPopup, bool autoCancelEnabled)
{
	NativeTrace trace( "NativeToJavaBridge::WebViewCreate" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callWebViewCreate", "(Lcom/ansca/corona/CoronaRuntime;IIIIIZZ)V" );
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, 
				fCoronaRuntime, id, left, top, width, height, (jboolean)isPopup, (jboolean)autoCancelEnabled );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::WebViewRequestLoadUrl( int id, const char * url )
{
	NativeTrace trace( "NativeToJavaBridge::WebViewRequestLoadUrl" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callWebViewRequestLoadUrl", "(Lcom/ansca/corona/CoronaRuntime;ILjava/lang/String;)V" );
		
		if ( mid != NULL )
		{
			jstringParam textJ( bridge.getEnv(), url );
			if ( textJ.isValid() )
			{
				bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, textJ.getValue() );
				HandleJavaException();
			}
		}
	}
}

void
NativeToJavaBridge::WebViewRequestReload( int id )
{
	NativeTrace trace( "NativeToJavaBridge::WebViewRequestReload" );
	CallIntMethod( "callWebViewRequestReload", id );
	HandleJavaException();
}

void
NativeToJavaBridge::WebViewRequestStop( int id )
{
	NativeTrace trace( "NativeToJavaBridge::WebViewRequestStop" );
	CallIntMethod( "callWebViewRequestStop", id );
	HandleJavaException();
}

void
NativeToJavaBridge::WebViewRequestGoBack( int id )
{
	NativeTrace trace( "NativeToJavaBridge::WebViewRequestGoBack" );
	CallIntMethod( "callWebViewRequestGoBack", id );
	HandleJavaException();
}

void
NativeToJavaBridge::WebViewRequestGoForward( int id )
{
	NativeTrace trace( "NativeToJavaBridge::WebViewRequestGoForward" );
	CallIntMethod( "callWebViewRequestGoForward", id );
	HandleJavaException();
}

void
NativeToJavaBridge::WebViewRequestDeleteCookies( int id )
{
	NativeTrace trace( "NativeToJavaBridge::WebViewRequestDeleteCookies" );
	CallIntMethod( "callWebViewRequestDeleteCookies", id );
	HandleJavaException();
}

void
NativeToJavaBridge::VideoViewCreate(
	int id, int left, int top, int width, int height)
{
	NativeTrace trace( "NativeToJavaBridge::VideoViewCreate" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() ) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callVideoViewCreate", "(Lcom/ansca/corona/CoronaRuntime;IIIII)V" );
		if ( mid != NULL ) {
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, 
				fCoronaRuntime, id, left, top, width, height );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::VideoViewLoad( int id, const char * source )
{
	NativeTrace trace( "NativeToJavaBridge::VideoViewLoad" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callVideoViewLoad", "(Lcom/ansca/corona/CoronaRuntime;ILjava/lang/String;)V" );
		
		if ( mid != NULL )
		{
			jstringParam textJ( bridge.getEnv(), source );
			if ( textJ.isValid() )
			{
				bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, textJ.getValue() );
				HandleJavaException();
			}
		}
	}
}

void
NativeToJavaBridge::VideoViewPlay( int id )
{
	NativeTrace trace( "NativeToJavaBridge::VideoViewPlay" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callVideoViewPlay", "(Lcom/ansca/corona/CoronaRuntime;I)V" );
		
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::VideoViewPause( int id )
{
	NativeTrace trace( "NativeToJavaBridge::VideoViewPause" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callVideoViewPause", "(Lcom/ansca/corona/CoronaRuntime;I)V" );
		
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::VideoViewSeek( int id, int seekTo )
{
	NativeTrace trace( "NativeToJavaBridge::VideoViewSeek" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callVideoViewSeek", "(Lcom/ansca/corona/CoronaRuntime;II)V" );
		
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, seekTo );
			HandleJavaException();
		}
	}
}

int
NativeToJavaBridge::VideoViewGetCurrentTime( int id )
{
	NativeTrace trace( "NativeToJavaBridge::VideoViewGetCurrentTime" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	int result = 0;

	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callVideoViewGetCurrentTime", "(Lcom/ansca/corona/CoronaRuntime;I)I" );
		
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticIntMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
		}
	}
	return result;
}

int
NativeToJavaBridge::VideoViewGetTotalTime( int id )
{
	NativeTrace trace( "NativeToJavaBridge::VideoViewGetTotalTime" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	int result = 0;

	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callVideoViewGetTotalTime", "(Lcom/ansca/corona/CoronaRuntime;I)I" );
		
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticIntMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
		}
	}
	return result;
}

bool
NativeToJavaBridge::VideoViewGetIsMuted( int id )
{
	NativeTrace trace( "NativeToJavaBridge::VideoViewGetIsMuted" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	bool result = false;

	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callVideoViewGetIsMuted", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
		}
	}
	return result;
}

void
NativeToJavaBridge::VideoViewMute( int id, bool mute )
{
	NativeTrace trace( "NativeToJavaBridge::VideoViewMute" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callVideoViewMute", "(Lcom/ansca/corona/CoronaRuntime;IZ)V" );
		
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, mute );
			HandleJavaException();
		}
	}
}

bool
NativeToJavaBridge::VideoViewGetIsTouchTogglesPlay( int id )
{
	NativeTrace trace( "NativeToJavaBridge::VideoViewGetIsTouchTogglesPlay" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	bool result = false;

	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callVideoViewGetIsTouchTogglesPlay", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
		}
	}
	return result;
}

void
NativeToJavaBridge::VideoViewTouchTogglesPlay( int id, bool toggle )
{
	NativeTrace trace( "NativeToJavaBridge::VideoViewTouchTogglesPlay" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );

	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callVideoViewTouchTogglesPlay", "(Lcom/ansca/corona/CoronaRuntime;IZ)V" );
		
		if ( mid != NULL )
		{
			bridge.getEnv()->CallStaticVoidMethod( bridge.getClass(), mid, fCoronaRuntime, id, toggle );
			HandleJavaException();
		}
	}
}

bool
NativeToJavaBridge::VideoViewGetIsPlaying( int id )
{
	NativeTrace trace( "NativeToJavaBridge::VideoViewGetIsPlaying" );

	bool result = false;
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(),
				"callVideoViewGetIsPlaying", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		if ( mid != NULL )
		{
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, id );
			HandleJavaException();
		}
	}
	return result;
}

void
NativeToJavaBridge::MapViewCreate(int id, int left, int top, int width, int height)
{
	NativeTrace trace( "NativeToJavaBridge::MapViewCreate" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callMapViewCreate", "(Lcom/ansca/corona/CoronaRuntime;IIIII)V");
		if (mid != NULL)
		{
			bridge.getEnv()->CallStaticVoidMethod(
					bridge.getClass(), mid, fCoronaRuntime, id, left, top, width, height);
			HandleJavaException();
		}
	}
}

bool
NativeToJavaBridge::MapViewIsCurrentLocationVisible(int id)
{
	NativeTrace trace( "NativeToJavaBridge::MapViewIsCurrentLocationVisible" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = false;
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callMapViewIsCurrentLocationVisible", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		if (mid != NULL)
		{
			result = bridge.getEnv()->CallStaticBooleanMethod(bridge.getClass(), mid, fCoronaRuntime, id);
			HandleJavaException();
		}
	}
	return result;
}

int
NativeToJavaBridge::MapViewPushCurrentLocationToLua(int id, lua_State *L)
{
	NativeTrace trace( "NativeToJavaBridge::MapViewPushCurrentLocationToLua" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	int result = 0;
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callMapViewPushCurrentLocationToLua", "(Lcom/ansca/corona/CoronaRuntime;IJ)I" );
		if (mid != NULL)
		{
			result = (int)bridge.getEnv()->CallStaticIntMethod(bridge.getClass(), mid, fCoronaRuntime, id, (jlong)(uintptr_t)L);
			HandleJavaExceptionUsing(L);
		}
	}
	return result;
}

bool
NativeToJavaBridge::MapViewIsScrollEnabled(int id)
{
	NativeTrace trace( "NativeToJavaBridge::MapViewIsScrollEnabled" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = false;
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callMapViewIsScrollEnabled", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		if (mid != NULL)
		{
			result = bridge.getEnv()->CallStaticBooleanMethod(bridge.getClass(), mid, fCoronaRuntime, id);
			HandleJavaException();
		}
	}
	return result;
}

void
NativeToJavaBridge::MapViewSetScrollEnabled(int id, bool enabled)
{
	NativeTrace trace( "NativeToJavaBridge::MapViewSetScrollEnabled" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
				"callMapViewSetScrollEnabled", "(Lcom/ansca/corona/CoronaRuntime;IZ)V" );
		if (mid != NULL)
		{
			bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, id, enabled);
			HandleJavaException();
		}
	}
}

bool
NativeToJavaBridge::MapViewIsZoomEnabled(int id)
{
	NativeTrace trace( "NativeToJavaBridge::MapViewIsZoomEnabled" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = false;
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callMapViewIsZoomEnabled", "(Lcom/ansca/corona/CoronaRuntime;I)Z" );
		if (mid != NULL)
		{
			result = bridge.getEnv()->CallStaticBooleanMethod(bridge.getClass(), mid, fCoronaRuntime, id);
			HandleJavaException();
		}
	}
	return result;
}

void
NativeToJavaBridge::MapViewSetZoomEnabled(int id, bool enabled)
{
	NativeTrace trace( "NativeToJavaBridge::MapViewSetZoomEnabled" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
				"callMapViewSetZoomEnabled", "(Lcom/ansca/corona/CoronaRuntime;IZ)V" );
		if (mid != NULL)
		{
			bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, id, enabled);
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::MapViewGetType(int id, Rtt::String *mapTypeResult)
{
	if (!mapTypeResult)
	{
		return;
	}

	NativeTrace trace( "NativeToJavaBridge::MapViewGetType" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
				"callMapViewGetType", "(Lcom/ansca/corona/CoronaRuntime;I)Ljava/lang/String;" );
		if (mid)
		{
			jobject objectResult = bridge.getEnv()->CallStaticObjectMethod(bridge.getClass(), mid, fCoronaRuntime, id);
			HandleJavaException();
			if (objectResult)
			{
				jstringResult stringResult(bridge.getEnv());
				stringResult.setString((jstring)objectResult);
				if (stringResult.isValidString())
				{
					mapTypeResult->Set(stringResult.getUTF8());
				}
			}
		}
	}
}

void
NativeToJavaBridge::MapViewSetType(int id, const char *mapTypeName)
{
	NativeTrace trace( "NativeToJavaBridge::MapViewSetType" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(),
				"callMapViewSetType", "(Lcom/ansca/corona/CoronaRuntime;ILjava/lang/String;)V" );
		if (mid)
		{
			jstringParam mapTypeNameJ(bridge.getEnv(), mapTypeName);
			bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, id, mapTypeNameJ.getValue());
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::MapViewSetCenter(int id, double latitude, double longitude, bool isAnimated)
{
	NativeTrace trace( "NativeToJavaBridge::MapViewSetCenter" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(),
				"callMapViewSetCenter", "(Lcom/ansca/corona/CoronaRuntime;IDDZ)V" );
		if (mid != NULL)
		{
			bridge.getEnv()->CallStaticVoidMethod(
					bridge.getClass(), mid, fCoronaRuntime, id, (jdouble)latitude, (jdouble)longitude, (jboolean)isAnimated);
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::MapViewSetRegion(
	int id, double latitude, double longitude, double latitudeSpan, double longitudeSpan, bool isAnimated)
{
	NativeTrace trace( "NativeToJavaBridge::MapViewSetRegion" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(),
				"callMapViewSetRegion", "(Lcom/ansca/corona/CoronaRuntime;IDDDDZ)V" );
		if (mid != NULL)
		{
			bridge.getEnv()->CallStaticVoidMethod(
					bridge.getClass(), mid, fCoronaRuntime, id, (jdouble)latitude, (jdouble)longitude,
					(jdouble)latitudeSpan, (jdouble)longitudeSpan, (jboolean)isAnimated);
			HandleJavaException();
		}
	}
}

int
NativeToJavaBridge::MapViewAddMarker(
	int id, double latitude, double longitude, const char *title, const char *subtitle, int listener, const char *fileNameWithPath)
{
	int result = 0;
	NativeTrace trace( "NativeToJavaBridge::MapViewAddMarker" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callMapViewAddMarker", "(Lcom/ansca/corona/CoronaRuntime;IDDLjava/lang/String;Ljava/lang/String;ILjava/lang/String;)I" );
		if (mid != NULL)
		{
			jstringParam titleJ(bridge.getEnv(), title);
			jstringParam subtitleJ(bridge.getEnv(), subtitle);
			jstringParam fileNameWithPathJ(bridge.getEnv(), fileNameWithPath);

			result = bridge.getEnv()->CallStaticIntMethod(
					bridge.getClass(), mid, fCoronaRuntime, id, (jdouble)latitude, (jdouble)longitude,
					titleJ.getValue(), subtitleJ.getValue(), listener, fileNameWithPathJ.getValue());
			HandleJavaException();
		}
	}
	return result;
}

void
NativeToJavaBridge::MapViewRemoveMarker(int id, int markerId)
{
	int result = 0;
	NativeTrace trace( "NativeToJavaBridge::MapViewRemoveMarker" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callMapViewRemoveMarker", "(Lcom/ansca/corona/CoronaRuntime;II)V" );
		if (mid != NULL)
		{
			bridge.getEnv()->CallStaticVoidMethod(
					bridge.getClass(), mid, fCoronaRuntime, id, markerId);
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::MapViewRemoveAllMarkers(int id)
{
	NativeTrace trace( "NativeToJavaBridge::MapViewRemoveAllMarkers" );
	CallIntMethod( "callMapViewRemoveAllMarkers", id );
	HandleJavaException();
}

bool
NativeToJavaBridge::SaveImageToPhotoLibrary( const char *fileName )
{
	NativeTrace trace( "NativeToJavaBridge::SaveImageToPhotoLibrary" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	jboolean result = false;
	if ( bridge.isValid() && fileName )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
							bridge.getClass(), "callSaveImageToPhotoLibrary", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;)Z" );
		if ( mid != NULL )
		{
			jstringParam fileNameJ( bridge.getEnv(), fileName );
			result = bridge.getEnv()->CallStaticBooleanMethod( bridge.getClass(), mid, fCoronaRuntime, fileNameJ.getValue() );
			HandleJavaException();
		}
	}
	return (bool)result;
}

bool
NativeToJavaBridge::SaveBitmap( const Rtt::PlatformBitmap * bitmap, const char * path, int quality )
{
	NativeTrace trace( "NativeToJavaBridge::SaveBitmap" );
#ifdef Rtt_DEBUG
	if ( bitmap == NULL || bitmap->GetBits( NULL ) == NULL )
	{
		__android_log_print(ANDROID_LOG_INFO, "Corona", "= NativeToJavaBridge::SaveBitmap NULL");
		return false;
	}
#endif

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	bool result = false;
	
	if ( bridge.isValid() )
	{
		jmethodID mid;
		
		mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), "callSaveBitmap", "(Lcom/ansca/corona/CoronaRuntime;[IIIILjava/lang/String;)Z" );
		
		if ( path == NULL )
		{
			path = "";
		}

		if ( mid != NULL )
		{
			jstringParam pathJ( bridge.getEnv(), path );
			if ( pathJ.isValid() )
			{
				int width = bitmap->Width();
				int height = bitmap->Height();
				
				jintArrayParam array( bridge.getEnv(), width * height);
				
				if ( array.isValid() )
				{
					if ( width > 0 )
					{
						array.setArray( (const int *) bitmap->GetBits( NULL ), 0, width * height );
					}
				}

				result = bridge.getEnv()->CallStaticBooleanMethod(
							bridge.getClass(), mid, fCoronaRuntime, array.getValue(), width, height, quality, pathJ.getValue());
				HandleJavaException();
			}
		}
	}

	return result;
}

int 
NativeToJavaBridge::CryptoGetDigestLength( const char * algorithm )
{
	NativeTrace trace( "NativeToJavaBridge::CryptoGetDigestLength" );
	int result = 0;

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callCryptoGetDigestLength", "(Ljava/lang/String;)I" );
		if ( mid != NULL )
		{
			jstringParam algorithmJ( bridge.getEnv(), algorithm );
			if ( algorithmJ.isValid() )
			{
				result = bridge.getEnv()->CallStaticIntMethod( bridge.getClass(), mid, algorithmJ.getValue() );
				HandleJavaException();
			}
		}
	}
		
	return result;
}

void 
NativeToJavaBridge::CryptoCalculateDigest( const char * algorithm, const Rtt::Data<const char> & data, U8 * digest )
{
	NativeTrace trace( "NativeToJavaBridge::CryptoCalculateDigest" );
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
							bridge.getClass(), "callCryptoCalculateDigest", "(Ljava/lang/String;[B)[B" );
		if ( mid != NULL )
		{
			jstringParam algorithmJ( bridge.getEnv(), algorithm );
			jbyteArrayParam dataJ( bridge.getEnv(), data.GetLength() );
			dataJ.setArray( data.Get(), 0, data.GetLength() );

			jobject jo = bridge.getEnv()->CallStaticObjectMethod(
							bridge.getClass(), mid, algorithmJ.getValue(), dataJ.getValue() );
			HandleJavaException();
			if (jo)
			{
				jbyteArrayResult jbytes( bridge.getEnv(), (jbyteArray) jo );
				memcpy( digest, (const char *) jbytes.getValues(), jbytes.getLength() );
				jbytes.release();
				bridge.getEnv()->DeleteLocalRef(jo);
			}
		}
	}
}

void 
NativeToJavaBridge::CryptoCalculateHMAC( const char * algorithm, const Rtt::Data<const char> & key, const Rtt::Data<const char> & data, 
	U8 * digest )
{
	NativeTrace trace( "NativeToJavaBridge::CryptoCalculateHMAC" );
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
				"callCryptoCalculateHMAC", "(Ljava/lang/String;[B[B)[B" );
		if ( mid != NULL )
		{
			jstringParam algorithmJ( bridge.getEnv(), algorithm );
			jbyteArrayParam keyJ( bridge.getEnv(), key.GetLength() );
			keyJ.setArray( key.Get(), 0, key.GetLength() );
			jbyteArrayParam dataJ( bridge.getEnv(), data.GetLength() );
			dataJ.setArray( data.Get(), 0, data.GetLength() );

			jobject jo = bridge.getEnv()->CallStaticObjectMethod(
							bridge.getClass(), mid, algorithmJ.getValue(), keyJ.getValue(), dataJ.getValue() );
			HandleJavaException();
			if (jo)
			{
				jbyteArrayResult jbytes( bridge.getEnv(), (jbyteArray) jo );
				memcpy( digest, (const char *) jbytes.getValues(), jbytes.getLength() );
				jbytes.release();
				bridge.getEnv()->DeleteLocalRef(jo);
			}
		}
	}
}

bool
NativeToJavaBridge::WebPopupShouldLoadUrl( int id, const char * url )
{
	Rtt::PlatformWebPopup * owner = (Rtt::PlatformWebPopup *) id;
	
	bool result = owner->ShouldLoadUrl( url );
	
	return result;
}

bool
NativeToJavaBridge::WebPopupDidFailLoadUrl( int id, const char * url, const char * msg, int code )
{
	Rtt::PlatformWebPopup * owner = (Rtt::PlatformWebPopup *) id;
	
	bool result = owner->DidFailLoadUrl( url, msg, code );
	
	return result;
}

void
NativeToJavaBridge::ExternalizeResource( const char * assetName, Rtt::String * result )
{
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	NativeTrace trace( "NativeToJavaBridge::ExternalizeResource" );

	if ( bridge.isValid() )
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
			"callExternalizeResource", "(Ljava/lang/String;Lcom/ansca/corona/CoronaRuntime;)Ljava/lang/String;" );
		jstringParam assetNameJ( bridge.getEnv(), assetName );
		
		if ( mid != NULL && assetNameJ.isValid() )
		{
			jobject jo = bridge.getEnv()->CallStaticObjectMethod( bridge.getClass(), mid, assetNameJ.getValue(), fCoronaRuntime );
			HandleJavaException();
			if ( jo )
			{
				jstringResult jstr( bridge.getEnv() );
				jstr.setString( (jstring) jo );
				if ( jstr.isValidString() )
				{
					result->Set( jstr.getUTF8() );
				}
			}
		}
	}
}


#ifdef Rtt_FLURRY
void
NativeToJavaBridge::FlurryInit( const char * applicationKey ) const
{
	if ( !applicationKey )
		return;
	
	CallStringMethod( "callFlurryInit", applicationKey );
	HandleJavaException();
}

void
NativeToJavaBridge::FlurryEvent( const char * eventId ) const
{
	if ( !eventId )
		return;
	
	CallStringMethod( "callFlurryEvent", eventId );
	HandleJavaException();
}

#endif

void
NativeToJavaBridge::RequestNearestAddressFromCoordinates( lua_State *L )
{
	NativeTrace trace( "NativeToJavaBridge::RequestNearestAddressFromCoordinates" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callRequestNearestAddressFromCoordinates", "(Lcom/ansca/corona/CoronaRuntime;J)V");
		if (mid)
		{
			bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, (jlong)(uintptr_t)L);
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::RequestLocationAsync( lua_State *L)
{
	NativeTrace trace( "NativeToJavaBridge::RequestLocationAsync" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge);
	if (bridge.isValid()) {
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), 
				"callRequestLocationAsync", "(Lcom/ansca/corona/CoronaRuntime;J)V");
		if (mid)
		{
			bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, (jlong)(uintptr_t)L);
			HandleJavaException();
		}
	}
}

int
NativeToJavaBridge::PushLocationNameCoordinatesToLua( const char *locationName, lua_State *L )
{
	NativeTrace trace( "NativeToJavaBridge::PushLocationNameCoordinatesToLua" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	int result = 0;
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID( bridge.getClass(), 
				"callPushLocationNameCoordinatesToLua", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;J)I" );
		if (mid)
		{
			jstringParam locationNameJ(bridge.getEnv(), locationName);
			result = (int)bridge.getEnv()->CallStaticIntMethod(
						bridge.getClass(), mid, fCoronaRuntime, locationNameJ.getValue(), (jlong)(uintptr_t)L);
			HandleJavaExceptionUsing(L);
		}
	}
	return result;
}

void
NativeToJavaBridge::StoreInit( const char *storeName )
{
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	NativeTrace trace( "NativeToJavaBridge::StoreInit" );
	
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
								bridge.getClass(), "callStoreInit", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;)V");
		jstringParam storeNameJ(bridge.getEnv(), storeName);
		bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, storeNameJ.getValue());
		HandleJavaException();
	}
}

void
NativeToJavaBridge::StorePurchase( const char *productName )
{
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	NativeTrace trace( "NativeToJavaBridge::StorePurchase" );
	
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
								bridge.getClass(), "callStorePurchase", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;)V");
		jstringParam productNameJ(bridge.getEnv(), productName);
		bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, productNameJ.getValue());
		HandleJavaException();
	}
}

void
NativeToJavaBridge::StoreFinishTransaction( const char *transactionStringId )
{
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	
	NativeTrace trace( "NativeToJavaBridge::StoreFinishTransaction" );
	
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
								bridge.getClass(), "callStoreFinishTransaction", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;)V");
		jstringParam transactionStringIdJ(bridge.getEnv(), transactionStringId);
		bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, transactionStringIdJ.getValue());
		HandleJavaException();
	}
}

void
NativeToJavaBridge::StoreRestoreCompletedTransactions()
{
	NativeTrace trace( "NativeToJavaBridge::StoreRestoreCompletedTransactions" );
	
	CallVoidMethod( "callStoreRestoreCompletedTransactions" );
	HandleJavaException();
}

void
NativeToJavaBridge::GetAvailableStoreNames( Rtt::PtrArray<Rtt::String> &storeNames )
{
	NativeTrace trace( "NativeToJavaBridge::GetAvailableStoreNames" );
	
	// Fetch store names from the Java side of Corona.
	bool didReceiveStrings = false;
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid =  bridge.getEnv()->GetStaticMethodID(
								bridge.getClass(), "callGetAvailableStoreNames", "()[Ljava/lang/String;");
		jobject resultArray = bridge.getEnv()->CallStaticObjectMethod(bridge.getClass(), mid);
		HandleJavaException();
		if (resultArray)
		{
			jstringArrayResult jstrings(bridge.getEnv(), (jobjectArray)resultArray);
			jsize arraySize = jstrings.getLength();
			if (arraySize > 0)
			{
				// Copy the received Java strings to the given string array.
				// String objects allocated here will be automatically deleted by the given array.
				storeNames.Reserve(arraySize);
				jstringResult javaString(bridge.getEnv());
				for (int index = 0; index < arraySize; index++)
				{
					javaString.setString(jstrings.getString(index));
					storeNames.Append(new Rtt::String(storeNames.Allocator(), javaString.getUTF8()));
				}
				didReceiveStrings = true;
			}
			bridge.getEnv()->DeleteLocalRef(resultArray);
		}
	}
	
	// If strings were not received, then clear the given array.
	// Doing this last is an optimization. No sense in deleting the array's memory if we don't have to.
	if (!didReceiveStrings)
	{
		storeNames.Empty();
	}
}

void
NativeToJavaBridge::GetTargetedStoreName( Rtt::String *outValue )
{
	GetString( "callGetTargetedStoreName", outValue );
	HandleJavaException();
}

int
NativeToJavaBridge::NotificationSchedule( lua_State *L, int index )
{
	NativeTrace trace( "NativeToJavaBridge::NotificationSchedule" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	int notificationId = -1;
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
				bridge.getClass(), "callNotificationSchedule", "(Lcom/ansca/corona/CoronaRuntime;JI)J");
		if (mid)
		{
			notificationId = (int)bridge.getEnv()->CallStaticIntMethod(
									bridge.getClass(), mid, fCoronaRuntime, (jlong)(uintptr_t)L, index);
			HandleJavaExceptionUsing(L);
		}
	}
	return notificationId;
}

void
NativeToJavaBridge::NotificationCancel( int id )
{
	NativeTrace trace( "NativeToJavaBridge::NotificationCancel" );
	
	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(), "callNotificationCancel", "(I)V");
		if (mid)
		{
			bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, id);
			HandleJavaException();
		}
	}
}

void
NativeToJavaBridge::NotificationCancelAll()
{
	NativeTrace trace( "NativeToJavaBridge::NotificationCancelAll" );
	CallVoidMethod( "callNotificationCancelAll" );
	HandleJavaException();
}

void
NativeToJavaBridge::GooglePushNotificationsRegister( const char *projectNumber )
{
	NativeTrace trace( "NativeToJavaBridge::GooglePushNotificationsRegister" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
								bridge.getClass(), "callGooglePushNotificationsRegister", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;)V");
		jstringParam projectNumberJ(bridge.getEnv(), projectNumber);
		bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, projectNumberJ.getValue());
		HandleJavaException();
	}
}

void
NativeToJavaBridge::SetGoogleMapsAPIKey( const char *mapsKey )
{
	NativeTrace trace( "NativeToJavaBridge::SetGoogleMapsAPIKey" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
								bridge.getClass(), "callGoogleSetMapsAPIKey", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;)V");
		jstringParam mapsKeyJ(bridge.getEnv(), mapsKey);
		bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, mapsKeyJ.getValue());
		HandleJavaException();
	}
}

void
NativeToJavaBridge::SetSystemUiVisibility( const char *visibility )
{
	NativeTrace trace( "NativeToJavaBridge::SetSystemUiVisibility" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
								bridge.getClass(), "callSetSystemUiVisibility", "(Lcom/ansca/corona/CoronaRuntime;Ljava/lang/String;)V");
		jstringParam visibilityJ(bridge.getEnv(), visibility);
		bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, visibilityJ.getValue());
		HandleJavaException();
	}
}

void
NativeToJavaBridge::GetSystemUiVisibility( Rtt::String * visibility )
{
	NativeTrace trace( "NativeToJavaBridge::GetSystemUiVisibility" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
								bridge.getClass(), "callGetSystemUiVisibility", "(Lcom/ansca/corona/CoronaRuntime;)Ljava/lang/String;");
		jobject jo = bridge.getEnv()->CallStaticObjectMethod(bridge.getClass(), mid, fCoronaRuntime);
		HandleJavaException();

		if ( jo )
		{
			jstringResult jstr( bridge.getEnv() );
			jstr.setString( (jstring) jo );
			if ( jstr.isValidString() )
			{
				visibility->Set( jstr.getUTF8() );
			}
		}

	}
}

void
NativeToJavaBridge::SetNavigationBarColor( double red, double green, double blue )
{
	NativeTrace trace( "NativeToJavaBridge::SetNavigationBarColor" );

	jclassInstance bridge( GetJNIEnv(), kNativeToJavaBridge );
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(
								bridge.getClass(), "callSetNavigationBarColor", "(Lcom/ansca/corona/CoronaRuntime;DDD)V");
		bridge.getEnv()->CallStaticVoidMethod(bridge.getClass(), mid, fCoronaRuntime, (jdouble)red, (jdouble)green, (jdouble)blue);
		HandleJavaException();
	}
}

void
NativeToJavaBridge::GooglePushNotificationsUnregister()
{
	NativeTrace trace( "NativeToJavaBridge::GooglePushNotificationsUnregister" );
	CallVoidMethod( "callGooglePushNotificationsUnregister" );
	HandleJavaException();
}

void
NativeToJavaBridge::FetchInputDevice(int coronaDeviceId)
{
	NativeTrace trace( "NativeToJavaBridge::FetchInputDevice" );
	CallIntMethod( "callFetchInputDevice", coronaDeviceId );
}

void
NativeToJavaBridge::FetchAllInputDevices()
{
	NativeTrace trace( "NativeToJavaBridge::FetchAllInputDevices" );
	CallVoidMethod( "callFetchAllInputDevices" );
}

void
NativeToJavaBridge::VibrateInputDevice(int coronaDeviceId)
{
	NativeTrace trace( "NativeToJavaBridge::VibrateInputDevice" );
	CallIntMethod( "callVibrateInputDevice", coronaDeviceId );
}

void
NativeToJavaBridge::ShowSplashScreen()
{
	NativeTrace trace( "NativeToJavaBridge::ShowSplashScreen" );
	CallVoidMethod( "callShowSplashScreen" );
	HandleJavaException();
}

void NativeToJavaBridge::SetRuntime(Rtt::Runtime *runtime)
{
	fRuntime = runtime;
}

/**
 * Base64 decodes a given byte array
 * @param payloadData The char object to decode
 * @param data The object that will be populated with the correctly decoded data
 * @return true if decoding was successful else false
 */
bool
NativeToJavaBridge::DecodeBase64( const Rtt::Data<const char> & payloadData, Rtt::Data<char> & data )
{
	JNIEnv *env = GetJNIEnv();
	jclass base64 = env->FindClass("android/util/Base64");
	jmethodID mid = env->GetStaticMethodID(base64, "decode", "([BI)[B");
	if ( mid != NULL )
	{
		jbyteArrayParam payloadJ( env, payloadData.GetLength() );
		payloadJ.setArray( payloadData.Get(), 0, payloadData.GetLength() );
		jobject jo = env->CallStaticObjectMethod(base64, mid, payloadJ.getValue(), 0);

		if (jo)
		{
			jbyteArrayResult bytesJ( env, (jbyteArray) jo );
			data.Set( (const char *) bytesJ.getValues(), bytesJ.getLength() );
			env->DeleteLocalRef(base64);
			return true;
		}
	}
	return false;
}

bool
NativeToJavaBridge::Check( const Rtt::Data<const char> & publicKey, const Rtt::Data<const char> & signature, const Rtt::Data<const char> & payloadData)
{
	NativeTrace trace( "NativeToJavaBridge::Check" );

	bool result = false;
	JNIEnv *env = GetJNIEnv();

	// byte[] decodedKey = Base64.Decode(publicKey, Base64.DEFAULT);
	Rtt::Data<char> decodedKey(publicKey.Allocator());
	if (!DecodeBase64(publicKey, decodedKey))
	{
		return false;
	}
	jbyteArrayParam decodedKeyJ( env, decodedKey.GetLength() );
	decodedKeyJ.setArray( decodedKey.Get(), 0, decodedKey.GetLength() );

	// byte[] decodedSignature = Base64.Decode(Signature, Base64.DEFAULT);
	Rtt::Data<char> decodedSignature(signature.Allocator());
	if (!DecodeBase64(signature, decodedSignature))
	{
		return false;
	}
	jbyteArrayParam decodedSignatureJ( env, decodedSignature.GetLength() );
	decodedSignatureJ.setArray( decodedSignature.Get(), 0, decodedSignature.GetLength() );

	// byte[] payload = payloadData;
	jbyteArrayParam dataJ( env, payloadData.GetLength() );
	dataJ.setArray( payloadData.Get(), 0, payloadData.GetLength() );

	// X509EncodedKeySpec publicKeySpec = new X509EncodedKeySpec(decodedKey);
	jclass X509EncodedKeySpecClass = env->FindClass("java/security/spec/X509EncodedKeySpec");
	jmethodID X509EncodedKeySpecConstructor = env->GetMethodID(X509EncodedKeySpecClass, "<init>", "([B)V");
	jobject X509EncodedKeySpecObject = env->NewObject(X509EncodedKeySpecClass, X509EncodedKeySpecConstructor, decodedKeyJ.getValue());

	// KeyFactory keyFactory = KeyFactory.getInstance("RSA");
	jclass KeyFactoryClass = env->FindClass("java/security/KeyFactory");
	jmethodID KeyFactoryMid = env->GetStaticMethodID(KeyFactoryClass, "getInstance", "(Ljava/lang/String;)Ljava/security/KeyFactory;");
	jstringParam encryption(env, "RSA");
	jobject KeyFactoryObject = env->CallStaticObjectMethod(KeyFactoryClass, KeyFactoryMid, encryption.getValue());

	// PublicKey public = keyFactory.generatePublic(publicKeySpec);
	jmethodID KeyFactoryMid1 = env->GetMethodID(KeyFactoryClass, "generatePublic", "(Ljava/security/spec/KeySpec;)Ljava/security/PublicKey;");
	jobject PublicKey = env->CallObjectMethod(KeyFactoryObject, KeyFactoryMid1, X509EncodedKeySpecObject);

	// Signature signature = Signature.getInstance("SHA1withRSA");
	jclass SignatureClass = env->FindClass("java/security/Signature");
	jmethodID SignatureMid = env->GetStaticMethodID(SignatureClass, "getInstance", "(Ljava/lang/String;)Ljava/security/Signature;");
	jstringParam sigencryption(env, "SHA1withRSA");
	jobject SignatureObject = env->CallStaticObjectMethod(SignatureClass, SignatureMid, sigencryption.getValue());
	
	// signature.initVerify(publicKey);
	jmethodID initVerifyMid = env->GetMethodID(SignatureClass, "initVerify", "(Ljava/security/PublicKey;)V");
	env->CallVoidMethod(SignatureObject, initVerifyMid, PublicKey);

	// signature.update(payload);
	jmethodID updateMid = env->GetMethodID(SignatureClass, "update", "([B)V");
	env->CallVoidMethod(SignatureObject, updateMid, dataJ.getValue());

	// bool verified = signature.verify(decodedSignature);
	jmethodID verifyMid = env->GetMethodID(SignatureClass, "verify", "([B)Z");
	jboolean verified = env->CallBooleanMethod(SignatureObject, verifyMid, decodedSignatureJ.getValue());

	// This shouldn't be necessary because all objects returned by jni functions are local references.
	// Local references are automatically freed once the method exits.
	env->DeleteLocalRef(X509EncodedKeySpecClass);
	env->DeleteLocalRef(X509EncodedKeySpecObject);
	env->DeleteLocalRef(KeyFactoryClass);
	env->DeleteLocalRef(KeyFactoryObject);
	env->DeleteLocalRef(PublicKey);
	env->DeleteLocalRef(SignatureClass);
	env->DeleteLocalRef(SignatureObject);

	return verified;
}

void
NativeToJavaBridge::getAudioOutputSettings(std::vector<int>& settings)
{
	NativeTrace trace("NativeToJavaBridge::getAudioOutputSettings");

	// Fetch the JNI environment.
	JNIEnv *jniEnvironmentPointer = GetJNIEnv();
	if (!jniEnvironmentPointer)
	{
		return;
	}

	jclassInstance bridge(GetJNIEnv(), kNativeToJavaBridge);
	if (bridge.isValid())
	{
		jmethodID mid = bridge.getEnv()->GetStaticMethodID(bridge.getClass(),	"getAudioOutputSettings", "()Ljava/lang/String;");
		if (mid != NULL)
		{
			jobject objectResult = bridge.getEnv()->CallStaticObjectMethod(bridge.getClass(), mid);
			if (objectResult)
			{
				jstringResult stringResult(jniEnvironmentPointer);
				stringResult.setString((jstring)objectResult);
				if (stringResult.isValidString())
				{
					const char* res = stringResult.getUTF8();
					char *token, *str, *tofree;
					tofree = str = strdup(res);  // We own str's memory now.
					while ((token = strsep(&str, ",")))
					{
						settings.push_back(atoi(token));
					}
					free(tofree);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------
