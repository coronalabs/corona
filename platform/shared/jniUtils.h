/*
 * C++ utility classes for passing values back and forth in JNI
 */
 
#ifndef _H_jniUtil_
#define _H_jniUtil_

#if defined(Rtt_ANDROID_ENV) || defined(Rtt_WIN_ENV)
	#include <jni.h>   
	// $JAVA_HOME/include and $JAVA_HOME/include/win32

    #ifdef Rtt_DEBUG
		#include <stdio.h>
	#endif

#else
	// OSX
	#include <JavaVM/jni.h>
#endif

#if defined(Rtt_ANDROID_ENV) && defined(Rtt_DEBUG)
	#include <android/log.h>
#endif

class jniEnv {
	JNIEnv *	myEnv;
	
public:
	jniEnv( JNIEnv *e )
	{
		myEnv = e;
	}
	
	inline JNIEnv *	getEnv() const
	{
		return myEnv;
	}
};

template <typename T> class jobjectRef : public jniEnv
{
	bool myLocalRef;
	T myObject;
	
public:
	inline jobjectRef<T>( JNIEnv *env ) : jniEnv( env ), myLocalRef( true )
	{
	}
	
	inline jobjectRef<T>( JNIEnv *env, T o ) : jniEnv( env ), myObject( o ), myLocalRef( true )
	{
	}
	
	inline ~jobjectRef<T>()
	{
		// http://java.sun.com/docs/books/jni/html/fldmeth.html#26254
		if ( myObject != NULL && myLocalRef )
	    	getEnv()->DeleteLocalRef( myObject );
	}
	
	inline void setObject( T o )
	{
		myObject = o;
	}
	
	inline T getObject() const 
	{
		return myObject;
	}
	
	// Specifies the object is not locally created but instead resides in the VM
	// (is passed as a parameter to us)
	inline void setNotLocal()
	{
		myLocalRef = false;
	}

	inline bool isValid() const
	{
		return myObject != NULL;
	}
};

class jclassInstance : public jobjectRef<jclass> {
	
public:
	jclassInstance( JNIEnv *env, const char * className ) : jobjectRef<jclass>( env )
	{
		if ( env != NULL ) {
			setObject( env->FindClass( className ) );
		}
	}

	inline jclass getClass() const
	{
		return getObject();
	}
};

class jstringParam : public jobjectRef<jstring> {
private:
	const char *	mySource;

public:	
	jstringParam( JNIEnv *env, const char * str ) : jobjectRef<jstring>( env ), mySource( str )
	{
		// TODO: do conversion using NewString, because NewStringUTF does not use standard UTF8
		// See http://en.wikipedia.org/wiki/Java_Native_Interface
		setObject((mySource != NULL) ? env->NewStringUTF(mySource) : NULL);
	}

	inline jstring getValue() const
	{
		return getObject();
	}
};

class jstringArrayParam : public jobjectRef<jobjectArray> {
	
public:	
	jstringArrayParam( JNIEnv *env, size_t length ) : jobjectRef<jobjectArray>( env )
	{
		setObject( env->NewObjectArray( (jsize) length, env->FindClass( "java/lang/String" ), NULL ) );
	}
	
	void setElement( int i, const char * value )
	{
		jstring jstr = getEnv()->NewStringUTF( value );

		getEnv()->SetObjectArrayElement( getObject(), i, jstr );
	}
	
	inline jobjectArray getValue() const
	{
		return getObject();
	}
};

class jintArrayParam : public jobjectRef<jintArray> {

public:
	jintArrayParam( JNIEnv *env, size_t length ) : jobjectRef<jintArray>( env )
	{
		setObject( env->NewIntArray( (jsize) length ) );
	}
	
	~jintArrayParam()
	{
	}
	
	void setArray( const int * src, int start, int count )
	{
		getEnv()->SetIntArrayRegion( getObject(), start, count, (const jint *) src );
	}
	
	inline jintArray getValue() const
	{
		return getObject();
	}	
};

class jbyteArrayParam : public jobjectRef<jbyteArray>  {

public:
	jbyteArrayParam( JNIEnv *env, size_t length ) : jobjectRef<jbyteArray>( env )
	{
		setObject( env->NewByteArray( (jsize) length ) );
	}
	
	void setArray( const char * src, int start, int count )
	{
		getEnv()->SetByteArrayRegion( getObject(), start, count, (const jbyte *) src );
	}
	
	inline jbyteArray getValue() const
	{
		return getObject();
	}
};

class jBooleanParam : public jclassInstance
{
private:
	jobjectRef<jobject> myBoolean;
	
public:
	jBooleanParam( JNIEnv * env, bool value ) : jclassInstance( env, "java/lang/Boolean" ), myBoolean( env )
	{
		jmethodID methodId = env->GetStaticMethodID(getClass(), "valueOf", "(Z)Ljava/lang/Boolean;");
		myBoolean.setObject(env->CallStaticObjectMethod(getClass(), methodId, value));
	}
	
	jobject getValue()
	{
		return myBoolean.getObject();
	}
};

class jByteParam : public jclassInstance
{
private:
	jobjectRef<jobject> myByte;
	
public:
	jByteParam( JNIEnv * env, char value ) : jclassInstance( env, "java/lang/Byte" ), myByte( env )
	{
		jmethodID methodId = env->GetStaticMethodID(getClass(), "valueOf", "(B)Ljava/lang/Byte;");
		myByte.setObject(env->CallStaticObjectMethod(getClass(), methodId, value));
	}
	
	jobject getValue()
	{
		return myByte.getObject();
	}
};

class jIntegerParam : public jclassInstance
{
private:
	jobjectRef<jobject> myInteger;
	
public:
	jIntegerParam( JNIEnv * env, int value ) : jclassInstance( env, "java/lang/Integer" ), myInteger( env )
	{
		jmethodID methodId = env->GetStaticMethodID(getClass(), "valueOf", "(I)Ljava/lang/Integer;");
		myInteger.setObject(env->CallStaticObjectMethod(getClass(), methodId, value));
	}
	
	jobject getValue()
	{
		return myInteger.getObject();
	}
};

class jLongParam : public jclassInstance
{
private:
	jobjectRef<jobject> myLong;
	
public:
	jLongParam( JNIEnv * env, jlong value ) : jclassInstance( env, "java/lang/Integer" ), myLong( env )
	{
		jmethodID methodId = env->GetStaticMethodID(getClass(), "valueOf", "(J)Ljava/lang/Long;");
		myLong.setObject(env->CallStaticObjectMethod(getClass(), methodId, value));
	}
	
	jobject getValue()
	{
		return myLong.getObject();
	}
};

class jFloatParam : public jclassInstance
{
private:
	jobjectRef<jobject> myFloat;
	
public:
	jFloatParam( JNIEnv * env, float value ) : jclassInstance( env, "java/lang/Float" ), myFloat( env )
	{
		jmethodID methodId = env->GetStaticMethodID(getClass(), "valueOf", "(F)Ljava/lang/Float;");
		myFloat.setObject(env->CallStaticObjectMethod(getClass(), methodId, value));
	}
	
	jobject getValue()
	{
		return myFloat.getObject();
	}
};

class jDoubleParam : public jclassInstance
{
private:
	jobjectRef<jobject> myDouble;
	
public:
	jDoubleParam( JNIEnv * env, double value ) : jclassInstance( env, "java/lang/Double" ), myDouble( env )
	{
		jmethodID methodId = env->GetStaticMethodID(getClass(), "valueOf", "(D)Ljava/lang/Double;");
		myDouble.setObject(env->CallStaticObjectMethod(getClass(), methodId, value));
	}
	
	jobject getValue()
	{
		return myDouble.getObject();
	}
};

class jFile : public jclassInstance {
private:
	jobjectRef<jobject> myFile;
	
public:
	jFile( JNIEnv * env, const char * path ) : jclassInstance( env, "java/io/File" ), myFile( env )
	{
		jstringParam jPath(env, path);
		jmethodID constructor = env->GetMethodID(getClass(), "<init>", "(Ljava/lang/String;)V");
		myFile.setObject(env->NewObject(getClass(), constructor, jPath.getValue()));
	}

	jobject getAbsolutePath()
	{
		jmethodID mid = getEnv()->GetMethodID(getClass(), "getAbsolutePath", "()Ljava/lang/String;");
		return getEnv()->CallObjectMethod( myFile.getObject(), mid );
	}
	
	jobject getValue()
	{
		return myFile.getObject();
	}
};

class jHashMapParam : public jclassInstance
{
private:
	jobjectRef<jobject> myHashMap;
	jmethodID myPutMid;
	
public:
	jHashMapParam( JNIEnv * env ) : jclassInstance( env, "java/util/HashMap" ), myHashMap( env )
	{
		jmethodID constructor = env->GetMethodID(getClass(), "<init>", "()V"); 
		
		myHashMap.setObject( env->NewObject(getClass(), constructor) );
		
		myPutMid = env->GetMethodID(getClass(), "put", 
									"(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
	}
	
	void put( const char * key, const char * value )
	{
		jstringParam jKey( getEnv(), key );
		jstringParam jValue( getEnv(), value );
		getEnv()->CallObjectMethod(myHashMap.getObject(), myPutMid, jKey.getValue(), jValue.getValue() );
	}
	
	void put( const char * key, bool value )
	{
		jstringParam jKey( getEnv(), key );
		jBooleanParam jValue( getEnv(), value );
		getEnv()->CallObjectMethod(myHashMap.getObject(), myPutMid, jKey.getValue(), jValue.getValue() );
	}
	
	void put( const char * key, int value )
	{
		jstringParam jKey( getEnv(), key );
		jIntegerParam jValue( getEnv(), value );
		getEnv()->CallObjectMethod(myHashMap.getObject(), myPutMid, jKey.getValue(), jValue.getValue() );
	}
	
	void put( const char * key, jlong value )
	{
		jstringParam jKey( getEnv(), key );
		jLongParam jValue( getEnv(), value );
		getEnv()->CallObjectMethod(myHashMap.getObject(), myPutMid, jKey.getValue(), jValue.getValue() );
	}
	
	void put( const char * key, float value )
	{
		jstringParam jKey( getEnv(), key );
		jFloatParam jValue( getEnv(), value );
		getEnv()->CallObjectMethod(myHashMap.getObject(), myPutMid, jKey.getValue(), jValue.getValue() );
	}
	
	void put( const char * key, double value )
	{
		jstringParam jKey( getEnv(), key );
		jDoubleParam jValue( getEnv(), value );
		getEnv()->CallObjectMethod(myHashMap.getObject(), myPutMid, jKey.getValue(), jValue.getValue() );
	}
	
	void put( const char * key, jobject value )
	{
		jstringParam jKey( getEnv(), key );
		getEnv()->CallObjectMethod(myHashMap.getObject(), myPutMid, jKey.getValue(), value );
	}
	
	jobject getHashMapObject() const
	{
		return myHashMap.getObject();
	}
};

/*
class jAntProject : public jclassInstance {
	jobjectRef<jobject> myProject;

public:
	jAntProject( JNIEnv * env ) : jclassInstance( env, "org/apache/tools/ant/Project" ), myProject( env )
	{
		jmethodID constructor = env->GetMethodID(getClass(), "<init>", "()V"); 

		myProject.setObject( env->NewObject(getClass(), constructor) );
	}

	jobject getAntProject() const
	{
		return myProject.getObject();
	}

	void setUserProperty( const char * property, const char * value )
	{
		jmethodID mid = getEnv()->GetMethodID(getClass(), "setUserProperty", "(Ljava/lang/String;Ljava/lang/String)V"); 
		jstringParam jProperty( getEnv(), property );
		jstringParam jValue( getEnv(), value );

		getEnv()->CallVoidMethod(myProject.getObject(), mid, jProperty.getValue(), jValue.getValue() );
	}

	void setProperty( const char * property, const char * value )
	{
		jmethodID mid = getEnv()->GetMethodID(getClass(), "setProperty", "(Ljava/lang/String;Ljava/lang/String)V"); 
		jstringParam jProperty( getEnv(), property );
		jstringParam jValue( getEnv(), value );

		getEnv()->CallVoidMethod(myProject.getObject(), mid, jProperty.getValue(), jValue.getValue() );
	}

	void init()
	{
		jmethodID mid = getEnv()->GetMethodID(getClass(), "init", "()V"); 

		getEnv()->CallVoidMethod(myProject.getObject(), mid );
	}

	int executeTarget( const char * target )
	{
		jmethodID mid = getEnv()->GetMethodID(getClass(), "executeTarget", "(Ljava/lang/String)I"); 
		jstringParam jTarget( getEnv(), target );

		return getEnv()->CallIntMethod( myProject.getObject(), mid, jTarget.getValue() );
	}
};

class jAntProjectHelper : public jclassInstance {
	jobjectRef<jobject> myProjectHelper;
public:
	jAntProjectHelper( JNIEnv * env ) : jclassInstance( env, "org/apache/tools/ant/ProjectHelper" ), myProjectHelper( env )
	{
		jmethodID mid = env->GetStaticMethodID( getClass(), "getProjectHelper", "()V" );
		myProjectHelper.setObject( env->CallStaticObjectMethod( getClass(), mid ) );
	}

	jobject getProjectHelper() const
	{
		return myProjectHelper.getObject();
	}

	void parse( jobject project, const char * buildFilePath )
	{
		jstringParam jBuildFilePath( getEnv(), buildFilePath );
		jFile file( getEnv(), buildFilePath );

		jmethodID mid = getEnv()->GetMethodID(getClass(), "parse", "(;Ljava/lang/String)V"); 

		getEnv()->CallVoidMethod(myProjectHelper.getObject(), mid, file.getObject() );
	}
};
*/

// TODO: use     env->GetByteArrayRegion(array, 0, len, buffer);
// see http://android.git.kernel.org/?p=platform/dalvik.git;a=blob_plain;f=docs/jni-tips.html;hb=HEAD#Native_Libraries
class jbyteArrayResult : public jobjectRef<jbyteArray> {
	jbyte *			myValues;

public:	

	jbyteArrayResult( JNIEnv *env, jbyteArray array ) : jobjectRef<jbyteArray>( env, array ), myValues( NULL )
	{
		setNotLocal();
	}

	~jbyteArrayResult()
	{
		release();
	}

	void init()
	{
		if ( myValues == NULL ) {
			myValues = getEnv()->GetByteArrayElements( getObject(), NULL );
		}
	}
	
	const jbyte * getValues()
	{
		init();
		return myValues;
	}
	
	inline size_t getLength() const
	{
		return getEnv()->GetArrayLength( getObject() );
	}

	void release()
	{
		if ( myValues != NULL )
		{
			getEnv()->ReleaseByteArrayElements( getObject(), myValues, 0 );
			myValues = NULL;
		}
	}
};

// TODO: use     env->GetByteArrayRegion(array, 0, len, buffer);
// see http://android.git.kernel.org/?p=platform/dalvik.git;a=blob_plain;f=docs/jni-tips.html;hb=HEAD#Native_Libraries
class jintArrayResult : public jobjectRef<jintArray> {
	jint *			myValues;

public:	

	jintArrayResult( JNIEnv *env, jintArray array ) : jobjectRef<jintArray>( env, array ), myValues( NULL )
	{
		setNotLocal();
	}

	~jintArrayResult()
	{
		release();
	}

	void init()
	{
		if ( myValues == NULL ) {
			myValues = getEnv()->GetIntArrayElements( getObject(), NULL );
		}
	}
	
	const jint * getValues()
	{
		init();
		return myValues;
	}
	
	inline size_t getLength() const
	{
		return getEnv()->GetArrayLength( getObject() );
	}
	
	void release()
	{
		if ( myValues != NULL ) {
			getEnv()->ReleaseIntArrayElements( getObject(), myValues, 0 );
			myValues = NULL;
		}
	}
};

class jstringResult : public jobjectRef<jstring> {
	const char *	myUTF8;

public:	
	jstringResult( JNIEnv *env, jstring jstr ) : jobjectRef<jstring>( env, jstr ), myUTF8( NULL )
	{
		setNotLocal();

	// TODO: do conversion using GetStringChars, because GetStringUTFChars does not use standard UTF8
	// See http://en.wikipedia.org/wiki/Java_Native_Interface
	
		if ( jstr != NULL )
			myUTF8 = getEnv()->GetStringUTFChars( jstr, NULL );
	}

	jstringResult( JNIEnv *env ) : jobjectRef<jstring>( env, NULL ), myUTF8( NULL )
	{
	}

	~jstringResult()
	{
		release();
	}

	void setString( jstring jstr )
	{
		release();
		setObject( jstr );
		myUTF8 = getEnv()->GetStringUTFChars( jstr, NULL );
	}
	
	void release()
	{
		if ( myUTF8 != NULL ) 
		{
			getEnv()->ReleaseStringUTFChars( getObject(), myUTF8 );
			myUTF8 = NULL;
		}
	}
	
	inline const char * getUTF8() const
	{
		return myUTF8;
	}
	
	inline bool isValidString() const
	{
		return myUTF8 != NULL;
	}
};

class jobjectArrayResult : public jobjectRef<jobjectArray> {

public:	

	jobjectArrayResult( JNIEnv *env, jobjectArray array ) : jobjectRef<jobjectArray>( env, array )
	{
		setNotLocal();
	}

	jobject getElement( int index )
	{
		return getEnv()->GetObjectArrayElement( getObject(), index );
	}
	
	inline size_t getLength() const
	{
		return getEnv()->GetArrayLength( getObject() );
	}
};


class jstringArrayResult : public jobjectArrayResult {

public:	

	jstringArrayResult( JNIEnv *env, jobjectArray array ) : jobjectArrayResult( env, array )
	{
		setNotLocal();
	}

	jstring getString( int index )
	{
		jstring stringElement = (jstring) getElement( index );
		
		return stringElement;
	}
	
	bool getString( int index, jstringResult & jstr )
	{
		jstring stringElement = (jstring) getElement( index );
		
		jstr.setString( stringElement );
		
		return jstr.isValid();
	}
};

class NativeTrace {

	const char * myMessage;
	int myCount;
	
public:
	NativeTrace( const char * msg )
	{
#if defined( Rtt_ANDROID_ENV ) && defined( Rtt_DEBUG )
		myMessage = msg;
		myCount = 0;
		char buf[256];
		sprintf( buf, "> %s", msg );
		__android_log_print(ANDROID_LOG_INFO, "Corona", "%s", buf );
#endif
	}
	~NativeTrace()
	{
#if defined( Rtt_ANDROID_ENV ) && defined( Rtt_DEBUG )
		char buf[256];
		sprintf( buf, "< %s", myMessage );
		__android_log_print(ANDROID_LOG_INFO, "Corona", "%s", buf );
#endif
	}

	void Trace()
	{
#if defined( Rtt_ANDROID_ENV ) && defined( Rtt_DEBUG )
		char buf[256];
		sprintf( buf, "= %s %d", myMessage, myCount++ );
		__android_log_print(ANDROID_LOG_INFO, "Corona", "%s", buf );
#endif
	}
};

class JniException : public jniEnv
{
	jstringResult myExceptionString;
	
public:
	JniException( JNIEnv *env ) : jniEnv( env ), myExceptionString( env )
	{
	}
	
	bool CheckException()
	{
		if (getEnv()->ExceptionCheck()) 
		{
			jthrowable e = getEnv()->ExceptionOccurred();

			// have to clear the exception before JNI will work again.
			getEnv()->ExceptionClear();

			jclass eclass = getEnv()->GetObjectClass(e);
			jmethodID mid;
			
			mid = getEnv()->GetMethodID(eclass, "toString", "()Ljava/lang/String;");

			jstring jErrorMsg = (jstring) getEnv()->CallObjectMethod(e, mid);

			myExceptionString.setString( jErrorMsg );

			return true;
		}

		return false;
	}

	inline const char * getUTF8() const
	{
		return myExceptionString.getUTF8();
	}
	
};

#endif // _H_jniUtil_
