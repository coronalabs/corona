/*
 * $Id: jnlua.c 155 2012-10-05 22:12:54Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <jni.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* Include uintptr_t */
#ifdef LUA_WIN
#include <stddef.h>
#define JNLUA_THREADLOCAL static __declspec(thread)
#endif
#ifdef LUA_USE_POSIX
#include <stdint.h>
#define JNLUA_THREADLOCAL static __thread
#endif

/* ---- Definitions ---- */
#define JNLUA_APIVERSION 2
#define JNLUA_JNIVERSION JNI_VERSION_1_6
#define JNLUA_JAVASTATE "jnlua.JavaState"
#define JNLUA_OBJECT "jnlua.Object"
#define JNLUA_MINSTACK LUA_MINSTACK
#define JNLUA_ENV(env)				/* Ansca: Changed to no-op. No longer stores JNIEnv pointers to be thread safe. */
#define JNLUA_PCALL(L, nargs, nresults) {\
	int status = lua_pcall(L, (nargs), (nresults), 0);\
	if (status != 0) {\
		throw(L, status);\
	}\
}
#define lua_absindex(L, index) (index > 0 || index <= LUA_REGISTRYINDEX) ? index : lua_gettop(L) + index + 1
#define thread_env get_jni_env()	/* Ansca: No longer a global variable. Now fetches a thread safe JNIEnv from VM. */

/* ---- Types ---- */
/* Structure for reading and writing Java streams. */
typedef struct StreamStruct  {
	jobject stream;
	jbyteArray byte_array;
	jbyte* bytes;
	jboolean is_copy;
} Stream;

/* ---- JNI helpers ---- */
static jclass referenceclass(JNIEnv *env, const char *className);
static jbyteArray newbytearray(jsize length);
static const char *getstringchars(jstring string);
static void releasestringchars(jstring string, const char *chars);

/* ---- Java state operations ---- */
static lua_State *getluastate(jobject javastate);
static void setluastate(jobject javastate, lua_State *L);
static lua_State *getluathread(jobject javastate);
static void setluathread(jobject javastate, lua_State *L);

/* ---- Checks ---- */
static int validindex(lua_State *L, int index);
static int checkstack(lua_State *L, int space);
static int checkindex(lua_State *L, int index);
static int checkrealindex(lua_State *L, int index);
static int checktype(lua_State *L, int index, int type);
static int checknelems(lua_State *L, int n);
static int checknotnull (void *object);
static int checkarg(int cond, const char *msg);
static int checkstate(int cond, const char *msg);
static int check(int cond, jthrowable throwable_class, const char *msg);

/* ---- Java objects and functions ---- */
static void pushjavaobject(lua_State *L, jobject object);
static jobject tojavaobject(lua_State *L, int index, jclass class);
static jstring tostring(lua_State *L, int index);
static int gcjavaobject(lua_State *L);
static int calljavafunction(lua_State *L);

/* ---- Error handling ---- */
static int messagehandler(lua_State *L);
static int isrelevant(lua_Debug *ar);
static void throw(lua_State *L, int status);

/* ---- Stream adapters ---- */
static const char *readhandler(lua_State *L, void *ud, size_t *size);
static int writehandler(lua_State *L, const void *data, size_t size, void *ud);

/* ---- Variables ---- */
static jclass luastate_class = NULL;
static jfieldID luastate_id = 0;
static jfieldID luathread_id = 0;
static jclass javafunction_interface = NULL;
static jmethodID invoke_id = 0;
static jclass luaruntimeexception_class = NULL;
static jmethodID luaruntimeexception_id = 0;
static jmethodID setluaerror_id = 0;
static jclass luasyntaxexception_class = NULL;
static jmethodID luasyntaxexception_id = 0;
static jclass luamemoryallocationexception_class = NULL;
static jmethodID luamemoryallocationexception_id = 0;
static jclass luamessagehandlerexception_class = NULL;
static jmethodID luamessagehandlerexception_id = 0;
static jclass luastacktraceelement_class = NULL;
static jmethodID luastacktraceelement_id = 0;
static jclass luaerror_class = NULL;
static jmethodID luaerror_id = 0;
static jmethodID setluastacktrace_id = 0;
static jclass nullpointerexception_class = NULL;
static jclass illegalargumentexception_class = NULL;
static jclass illegalstateexception_class = NULL;
static jclass error_class = NULL;
static jclass inputstream_class = NULL;
static jmethodID read_id = 0;
static jclass outputstream_class = NULL;
static jmethodID write_id = 0;
static jclass ioexception_class = NULL;
static int initialized = 0;
static JavaVM *java_vm = NULL;			/* Ansca: New global variable. Used to fetch a thread safe JNIEnv. */

/* Ansca: Fetches a JNIEnv pointer from the Java virtual machine that will work for the current thread. */
JNIEnv* get_jni_env()
{
	JNIEnv *env = NULL;
	if (java_vm)
	{
		(*java_vm)->GetEnv(java_vm, (void**)&env, JNLUA_JNIVERSION);
	}
	return env;
}

/* --- Version --- */
/* lua_version() */
JNIEXPORT jstring JNICALL Java_com_naef_jnlua_LuaState_lua_1version(JNIEnv *env, jobject obj) {
	const char *luaVersion;
	
	luaVersion = LUA_VERSION;
	if (strncmp(luaVersion, "Lua ", 4) == 0) {
		luaVersion += 4;
	}
	return (*env)->NewStringUTF(env, luaVersion); 
}

/* ---- Life cycle ---- */
/*
 * lua_newstate()
 */
JNLUA_THREADLOCAL jobject newstate_obj;
static int newstate_protected (lua_State *L) {
	jobject *ref;
	
	/* Set the Java state in the Lua state. */
	/* Ansca: Original code stored this object as a "weak reference", which did not pin the object in memory
	 *        on the Java side and caused random crashes. Changed to a "global reference" to pin it in memory.
	 */
	ref = lua_newuserdata(L, sizeof(jobject));
	lua_createtable(L, 0, 1);
	lua_pushboolean(L, 0); /* non-weak global reference */
	lua_pushcclosure(L, gcjavaobject, 1);
	lua_setfield(L, -2, "__gc");
	*ref = (*thread_env)->NewGlobalRef(thread_env, newstate_obj);
	if (!*ref) {
		lua_pushliteral(L, "JNI error: NewWeakGlobalRef() failed setting up Lua state");
		return lua_error(L);
	}
	lua_setmetatable(L, -2);
	lua_setfield(L, LUA_REGISTRYINDEX, JNLUA_JAVASTATE);
	
	/*
	 * Create the meta table for Java objects and return it. Population will
	 * be finished on the Java side.
	 */
	luaL_newmetatable(L, JNLUA_OBJECT);
	lua_pushboolean(L, 0);
	lua_setfield(L, -2, "__metatable");
	lua_pushboolean(L, 0); /* non-weak global reference */
	lua_pushcclosure(L, gcjavaobject, 1);
	lua_setfield(L, -2, "__gc");
	return 1;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1newstate (JNIEnv *env, jobject obj, int apiversion, jlong existing) {
	lua_State *L;
	
	/* Initialized? */
	if (!initialized) {
		return;
	}
	
	/* API version? */
	if (apiversion != JNLUA_APIVERSION) {
		return;
	}

	/* Create or attach to Lua state. */
	L = !existing ? luaL_newstate() : (lua_State *) (uintptr_t) existing;
	if (!L) {
		return;
	}
	
	/* Setup Lua state. */
	JNLUA_ENV(env);
	if (checkstack(L, JNLUA_MINSTACK)) {
		newstate_obj = obj;
		lua_pushcfunction(L, newstate_protected);
		JNLUA_PCALL(L, 0, 1);
	}
	if ((*env)->ExceptionCheck(env)) {
		if (!existing) {
			lua_close(L);
		}
		return;
	}
	
	/* Set the Lua state in the Java state. */
	setluathread(obj, L);
	setluastate(obj, L);
}

/* lua_close() */
static int close_protected (lua_State *L) {
	/* Unset the Java state in the Lua state. */
	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, JNLUA_JAVASTATE);
	
	return 0;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1close (JNIEnv *env, jobject obj, jboolean ownstate) {
	lua_State *L, *T;
	lua_Debug ar;

	JNLUA_ENV(env);
	L = getluastate(obj);
	if (ownstate) {
		/* Can close? */
		T = getluathread(obj);
		if (L != T || lua_getstack(L, 0, &ar)) {
			return;
		}
		
		/* Unset the Lua state in the Java state. */
		setluastate(obj, NULL);
		setluathread(obj, NULL);
		
		/* Close Lua state. */
		lua_close(L);
	} else {
		/* Can close? */
		if (!lua_checkstack(L, JNLUA_MINSTACK)) {
			return;
		}
		
		/* Cleanup Lua state. */
		lua_pushcfunction(L, close_protected);
		JNLUA_PCALL(L, 0, 0);
		if ((*env)->ExceptionCheck(env)) {
			return;
		}
		
		/* Unset the Lua state in the Java state. */
		setluastate(obj, NULL);
		setluathread(obj, NULL);
		
		/* Unset environment. */
		JNLUA_ENV(NULL);
	}
}

/* lua_gc() */
JNLUA_THREADLOCAL int gc_what;
JNLUA_THREADLOCAL int gc_data;
JNLUA_THREADLOCAL int gc_result;
static int gc_protected (lua_State *L) {
	gc_result = lua_gc(L, gc_what, gc_data);
	return 0;
}
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1gc (JNIEnv *env, jobject obj, jint what, jint data) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if(checkstack(L, JNLUA_MINSTACK)) {
		gc_what = what;
		gc_data = data;
		lua_pushcfunction(L, gc_protected);
		JNLUA_PCALL(L, 0, 0);
	}
	return (jint) gc_result;
}

/* ---- Registration ---- */
JNLUA_THREADLOCAL int openlib_lib;
static int openlib_protected (lua_State *L) {
	lua_CFunction openfunc;
	const char *libname;
	
	switch (openlib_lib) {
	case 0:
		openfunc = luaopen_base;
		libname = "";
		break;
	case 1:
		openfunc = luaopen_table;
		libname = LUA_TABLIBNAME;
		break;
	case 2:
		openfunc = luaopen_io;
		libname = LUA_IOLIBNAME;
		break;
	case 3:
		openfunc = luaopen_os;
		libname = LUA_OSLIBNAME;
		break;
	case 4:
		openfunc = luaopen_string;
		libname = LUA_STRLIBNAME;
		break;
	case 5:
		openfunc = luaopen_math;
		libname = LUA_MATHLIBNAME;
		break;
	case 6:
		openfunc = luaopen_debug;
		libname = LUA_DBLIBNAME;
		break;
	case 7:
		openfunc = luaopen_package;
		libname = LUA_LOADLIBNAME;
		break;
	default:
		return 0;
	} 
	lua_pushcfunction(L, openfunc);
	lua_pushstring(L, libname);
	lua_call(L, 1, 0);
	return 0;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1openlib (JNIEnv *env, jobject obj, jint lib) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checkarg(lib >= 0 && lib <= 7, "illegal library")) {
		openlib_lib = lib;
		lua_pushcfunction(L, openlib_protected);
		JNLUA_PCALL(L, 0, 0);
	}
}

/* ---- Load and dump ---- */
/* lua_load() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1load (JNIEnv *env, jobject obj, jobject inputStream, jstring chunkname) {
	lua_State *L;
	const char *chunkname_utf = NULL;
	Stream stream = { inputStream, NULL, NULL, 0 };
	int status;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& (chunkname_utf = getstringchars(chunkname))
			&& (stream.byte_array = newbytearray(1024))) {
		status = lua_load(L, readhandler, &stream, chunkname_utf);
		if (status != 0) {
			throw(L, status);
		}
	}
	if (stream.bytes) {
		(*env)->ReleaseByteArrayElements(env, stream.byte_array, stream.bytes, JNI_ABORT);
	}
	if (stream.byte_array) {
		(*env)->DeleteLocalRef(env, stream.byte_array);
	}
	if (chunkname_utf) {
		releasestringchars(chunkname, chunkname_utf);
	}
}

/* lua_dump() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1dump (JNIEnv *env, jobject obj, jobject outputStream) {
	lua_State *L;
	Stream stream = { outputStream, NULL, NULL, 0 };

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checknelems(L, 1)
			&& (stream.byte_array = newbytearray(1024))) {
		lua_dump(L, writehandler, &stream);
	}
	if (stream.bytes) {
		(*env)->ReleaseByteArrayElements(env, stream.byte_array, stream.bytes, JNI_ABORT);
	}
	if (stream.byte_array) {
		(*env)->DeleteLocalRef(env, stream.byte_array);
	}
}

/* ---- Call ---- */
/* lua_pcall() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1pcall (JNIEnv *env, jobject obj, jint nargs, jint nresults) {
	lua_State *L;
	int index, status;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkarg(nargs >= 0, "illegal argument count")
			&& checknelems(L, nargs + 1)
			&& checkarg(nresults >= 0 || nresults == LUA_MULTRET, "illegal return count")
			&& (nresults == LUA_MULTRET || checkstack(L, nresults - (nargs + 1)))) {
		index = lua_absindex(L, -nargs - 1);
		lua_pushcfunction(L, messagehandler);
		lua_insert(L, index);
		status = lua_pcall(L, nargs, nresults, index);
		lua_remove(L, index);
		if (status != 0) {
			throw(L, status);
		}
	}
}

/* ---- Global ---- */
/* lua_getglobal() */
JNLUA_THREADLOCAL const char *getglobal_name;
static int getglobal_protected (lua_State *L) {
	lua_getglobal(L, getglobal_name);
	return 1;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1getglobal (JNIEnv *env, jobject obj, jstring name) {
	lua_State *L;

	getglobal_name = NULL;
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& (getglobal_name = getstringchars(name))) {
		lua_pushcfunction(L, getglobal_protected);
		JNLUA_PCALL(L, 0, 1);
	}
	if (getglobal_name) {
		releasestringchars(name, getglobal_name);
	}
}

/* lua_setglobal() */
JNLUA_THREADLOCAL const char *setglobal_name;
static int setglobal_protected (lua_State *L) {
	lua_setglobal(L, setglobal_name);
	return 0;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1setglobal (JNIEnv *env, jobject obj, jstring name) {
	lua_State *L;

	setglobal_name = NULL;
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checknelems(L, 1)
			&& (setglobal_name = getstringchars(name))) {
		lua_pushcfunction(L, setglobal_protected);
		lua_insert(L, -2);
		JNLUA_PCALL(L, 1, 0);
	}
	if (setglobal_name) {
		releasestringchars(name, setglobal_name);
	}
}

/* ---- Stack push ---- */
/* lua_pushboolean() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1pushboolean (JNIEnv *env, jobject obj, jint b) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)) {
		lua_pushboolean(L, b);
	}
}

/* lua_pushbytearray() */
JNLUA_THREADLOCAL jbyte *pushbytearray_b;
JNLUA_THREADLOCAL jsize pushbytearray_count;
static int pushbytearray_protected (lua_State *L) {
	lua_pushlstring(L, pushbytearray_b, pushbytearray_count);
	return 1;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1pushbytearray (JNIEnv *env, jobject obj, jbyteArray ba, jint byteCount) {
	lua_State *L;
	jsize arrayLength;
	
	pushbytearray_b = NULL;
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& (pushbytearray_b = (*env)->GetByteArrayElements(env, ba, NULL))) {
		arrayLength = (*env)->GetArrayLength(env, ba);
		if (byteCount > arrayLength) {
			byteCount = arrayLength;
		}
		pushbytearray_count = byteCount;
		lua_pushcfunction(L, pushbytearray_protected);
		JNLUA_PCALL(L, 0, 1);
	}
	if (pushbytearray_b) {
		(*env)->ReleaseByteArrayElements(env, ba, pushbytearray_b, JNI_ABORT);
	}
}

/* lua_pushinteger() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1pushinteger (JNIEnv *env, jobject obj, jint n) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)) {
		lua_pushinteger(L, n);
	}
}

/* lua_pushjavafunction() */
JNLUA_THREADLOCAL jobject pushjavafunction_f;
static int pushjavafunction_protected (lua_State *L) {
	pushjavaobject(L, pushjavafunction_f);
	lua_pushcclosure(L, calljavafunction, 1);
	return 1;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1pushjavafunction (JNIEnv *env, jobject obj, jobject f) {
	lua_State *L;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checknotnull(f)) {
		pushjavafunction_f = f;
		lua_pushcfunction(L, pushjavafunction_protected);
		JNLUA_PCALL(L, 0, 1);
	}
}

/* lua_pushjavaobject() */
JNLUA_THREADLOCAL jobject pushjavaobject_object;
static int pushjavaobject_protected (lua_State *L) {
	pushjavaobject(L, pushjavaobject_object);
	return 1;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1pushjavaobject (JNIEnv *env, jobject obj, jobject object) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checknotnull(object)) {
		pushjavaobject_object = object;
		lua_pushcfunction(L, pushjavaobject_protected);
		JNLUA_PCALL(L, 0, 1);
	}
}

/* lua_pushnil() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1pushnil (JNIEnv *env, jobject obj) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)) {
		lua_pushnil(L);
	}
}

/* lua_pushnumber() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1pushnumber (JNIEnv *env, jobject obj, jdouble n) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)) {
		lua_pushnumber(L, n);
	}
}

/* lua_pushstring() */
JNLUA_THREADLOCAL const char *pushstring_s;
JNLUA_THREADLOCAL jsize pushstring_length;
static int pushstring_protected (lua_State *L) {
	lua_pushlstring(L, pushstring_s, pushstring_length);
	return 1;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1pushstring (JNIEnv *env, jobject obj, jstring s) {
	lua_State *L;
	
	pushstring_s = NULL;
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& (pushstring_s = getstringchars(s))) {
		pushstring_length = (*env)->GetStringUTFLength(env, s);
		lua_pushcfunction(L, pushstring_protected);
		JNLUA_PCALL(L, 0, 1);
	}
	if (pushstring_s) {
		releasestringchars(s, pushstring_s);
	}
}

/* ---- Stack type test ---- */
/* lua_isboolean() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1isboolean (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index)) {
		return 0;
	}
	return (jint) lua_isboolean(L, index);
}

/* lua_iscfunction() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1iscfunction (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	lua_CFunction c_function = NULL;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index)) {
		return 0;
	}
	c_function = lua_tocfunction(L, index);
	return (jint) (c_function != NULL && c_function != calljavafunction);
}

/* lua_isfunction() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1isfunction (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index)) {
		return 0;
	}
	return (jint) lua_isfunction(L, index);
}

/* lua_isjavafunction() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1isjavafunction (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index)) {
		return 0;
	}
	return (jint) (lua_tocfunction(L, index) == calljavafunction);
}

/* lua_isjavaobject() */
JNLUA_THREADLOCAL int isjavaobject_result;
static int isjavaobject_protected (lua_State *L) {
	isjavaobject_result = tojavaobject(L, 1, NULL) != NULL;
	return 0;
}
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1isjavaobject (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index)) {
		return 0;
	}
	if (checkstack(L, JNLUA_MINSTACK)) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, isjavaobject_protected);
		lua_pushvalue(L, index);
		JNLUA_PCALL(L, 1, 0);
	}
	return (jint) isjavaobject_result;
}

/* lua_isnil() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1isnil (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index)) {
		return 0;
	}
	return (jint) lua_isnil(L, index);
}

/* lua_isnone() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1isnone (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;

	JNLUA_ENV(env);
	L = getluathread(obj);
	return (jint) !validindex(L, index);
}

/* lua_isnoneornil() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1isnoneornil (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index)) {
		return 1;
	}
	return (jint) lua_isnil(L, index);
}

/* lua_isnumber() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1isnumber (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index)) {
		return 0;
	}
	return (jint) lua_isnumber(L, index);
}

/* lua_isstring() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1isstring (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index)) {
		return 0;
	}
	return (jint) lua_isstring(L, index);
}

/* lua_istable() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1istable (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index)) {
		return 0;
	}
	return lua_istable(L, index);
}

/* lua_isthread() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1isthread (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index)) {
		return 0;
	}
	return (jint) lua_isthread(L, index);
}

/* ---- Stack query ---- */
/* lua_equal() */
JNLUA_THREADLOCAL int equal_result;
static int equal_protected (lua_State *L) {
	equal_result = lua_equal(L, 1, 2);
	return 0;
}
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1equal (JNIEnv *env, jobject obj, jint index1, jint index2) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index1) || !validindex(L, index2)) {
		return (jint) 0;
	}
	if (checkstack(L, JNLUA_MINSTACK)) {
		index1 = lua_absindex(L, index1);
		index2 = lua_absindex(L, index2);
		lua_pushcfunction(L, equal_protected);
		lua_pushvalue(L, index1);
		lua_pushvalue(L, index2);
		JNLUA_PCALL(L, 2, 0);
	}
	return (jint) equal_result;
}

/* lua_lessthan() */
JNLUA_THREADLOCAL int lessthan_result;
static int lessthan_protected (lua_State *L) {
	lessthan_result = lua_lessthan(L, 1, 2);
	return 0;
}
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1lessthan (JNIEnv *env, jobject obj, jint index1, jint index2) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index1) || !validindex(L, index2)) {
		return (jint) 0;
	}
	if (checkstack(L, JNLUA_MINSTACK)) {
		index1 = lua_absindex(L, index1);
		index2 = lua_absindex(L, index2);
		lua_pushcfunction(L, lessthan_protected);
		lua_pushvalue(L, index1);
		lua_pushvalue(L, index2);
		JNLUA_PCALL(L, 2, 0);
	}
	return (jint) lessthan_result;
}

/* lua_objlen() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1objlen (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	size_t result = 0;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkindex(L, index)) {
		result = lua_objlen(L, index);
	}
	return (jint) result;
}

/* lua_rawequal() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1rawequal (JNIEnv *env, jobject obj, jint index1, jint index2) {
	lua_State *L;
	int result = 0;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkindex(L, index1) 
			&& checkindex(L, index2)) {
		result = lua_rawequal(L, index1, index2);
	}
	return (jint) result;
}

/* lua_toboolean() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1toboolean (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	int result = 0;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkindex(L, index)) {
		result = lua_toboolean(L, index);
	}
	return (jint) result;
}

/* lua_tobytearray() */
JNLUA_THREADLOCAL const char *tobytearray_result;
JNLUA_THREADLOCAL size_t tobytearray_length;
static int tobytearray_protected (lua_State *L) {
	tobytearray_result = lua_tolstring(L, 1, &tobytearray_length);
	return 0;
}
JNIEXPORT jbyteArray JNICALL Java_com_naef_jnlua_LuaState_lua_1tobytearray (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	jbyteArray ba;
	jbyte *b;

	tobytearray_result = NULL;
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checkindex(L, index)) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, tobytearray_protected);
		lua_pushvalue(L, index);
		JNLUA_PCALL(L, 1, 0);
	}
	if (!tobytearray_result) {
		return NULL;
	}
	ba = (*env)->NewByteArray(env, (jsize) tobytearray_length);
	if (!ba) {
		return NULL;
	}
	b = (*env)->GetByteArrayElements(env, ba, NULL);
	if (!b) {
		return NULL;
	}
	memcpy(b, tobytearray_result, tobytearray_length);
	(*env)->ReleaseByteArrayElements(env, ba, b, 0);
	return ba;
}

/* lua_tointeger() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1tointeger (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	lua_Integer result = 0;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkindex(L, index)) {
		result = lua_tointeger(L, index);
	}
	return (jint) result;
}

/* lua_tojavafunction() */
JNLUA_THREADLOCAL jobject tojavafunction_result;
static int tojavafunction_protected (lua_State *L) {
	tojavafunction_result = NULL;
	if (lua_tocfunction(L, 1) == calljavafunction) {
		if (lua_getupvalue(L, 1, 1)) {
			tojavafunction_result = tojavaobject(L, -1, javafunction_interface);
		}
	}
	return 0;
}
JNIEXPORT jobject JNICALL Java_com_naef_jnlua_LuaState_lua_1tojavafunction (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checkindex(L, index)) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, tojavafunction_protected);
		lua_pushvalue(L, index);
		JNLUA_PCALL(L, 1, 0);
	}
	return tojavafunction_result;
}

/* lua_tojavaobject() */
JNLUA_THREADLOCAL jobject tojavaobject_result;
static int tojavaobject_protected (lua_State *L) {
	tojavaobject_result = tojavaobject(L, 1, NULL);
	return 0;
}
JNIEXPORT jobject JNICALL Java_com_naef_jnlua_LuaState_lua_1tojavaobject (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checkindex(L, index)) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, tojavaobject_protected);
		lua_pushvalue(L, index);
		JNLUA_PCALL(L, 1, 0);
	}
	return tojavaobject_result;
}

/* lua_tonumber() */
JNIEXPORT jdouble JNICALL Java_com_naef_jnlua_LuaState_lua_1tonumber (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	lua_Number result = 0.0;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkindex(L, index)) {
		result = lua_tonumber(L, index);
	}
	return (jdouble) result;
}

/* lua_topointer() */
JNIEXPORT jlong JNICALL Java_com_naef_jnlua_LuaState_lua_1topointer (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	const void *result = NULL;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkindex(L, index)) {
		result = lua_topointer(L, index);
	}
	return (jlong) (uintptr_t) result;
}

/* lua_tostring() */
JNLUA_THREADLOCAL const char *tostring_result;
static int tostring_protected (lua_State *L) {
	tostring_result = lua_tostring(L, 1);
	return 0;
}
JNIEXPORT jstring JNICALL Java_com_naef_jnlua_LuaState_lua_1tostring (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;

	tostring_result = NULL;
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checkindex(L, index)) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, tostring_protected);
		lua_pushvalue(L, index);
		JNLUA_PCALL(L, 1, 0);
	}
	return tostring_result ? (*env)->NewStringUTF(env, tostring_result) : NULL;
}

/* lua_type() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1type (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (!validindex(L, index)) {
		return LUA_TNONE;
	}
	return (jint) lua_type(L, index);
}

/* ---- Stack operations ---- */
/* lua_concat() */
JNLUA_THREADLOCAL int concat_n;
static int concat_protected (lua_State *L) {
	lua_concat(L, concat_n);
	return 1;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1concat (JNIEnv *env, jobject obj, jint n) {
	lua_State *L;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checkarg(n >= 0, "illegal count")
			&& checknelems(L, n)) {
		concat_n = n;
		lua_pushcfunction(L, concat_protected);
		lua_insert(L, -n - 1);
		JNLUA_PCALL(L, n, 1);
	}
}

/* lua_gettop() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1gettop (JNIEnv *env, jobject obj) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	return (jint) lua_gettop(L);
}

/* lua_insert() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1insert (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkrealindex(L, index)) {
		lua_insert(L, index);
	}
}

/* lua_pop() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1pop (JNIEnv *env, jobject obj, jint n) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkarg(n >= 0 && n <= lua_gettop(L), "illegal count")) {
		lua_pop(L, n);
	}
}

/* lua_pushvalue() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1pushvalue (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checkindex(L, index)) {
		lua_pushvalue(L, index);
	}
}

/* lua_remove() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1remove (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;

	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkrealindex(L, index)) {
		lua_remove(L, index);
	}
}

/* lua_replace() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1replace (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkindex(L, index)
			&& checknelems(L, 1)) {
		lua_replace(L, index);
	}
}

/* lua_settop() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1settop (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if ((index >= 0 && (index <= lua_gettop(L) || checkstack(L, index - lua_gettop(L))))
			|| (index < 0 && checkrealindex(L, index))) {
		lua_settop(L, index);
	}
}

/* ---- Table ---- */
/* lua_createtable() */
JNLUA_THREADLOCAL int createtable_narr;
JNLUA_THREADLOCAL int createtable_nrec;
static int createtable_protected (lua_State *L) {
	lua_createtable(L, createtable_narr, createtable_nrec);
	return 1;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1createtable (JNIEnv *env, jobject obj, jint narr, jint nrec) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checkarg(narr >= 0, "illegal array count")
			&& checkarg(nrec >= 0, "illegal record count")) {
		createtable_narr = narr;
		createtable_nrec = nrec;
		lua_pushcfunction(L, createtable_protected);
		JNLUA_PCALL(L, 0, 1);
	}
}

/* lua_findtable() */
JNLUA_THREADLOCAL const char *findtable_fname;
JNLUA_THREADLOCAL int findtable_szhint;
JNLUA_THREADLOCAL const char *findtable_result;
static int findtable_protected (lua_State *L) {
	findtable_result = luaL_findtable(L, 1, findtable_fname, findtable_szhint);
	return findtable_result ? 0 : 1;
}
JNIEXPORT jstring JNICALL Java_com_naef_jnlua_LuaState_lua_1findtable (JNIEnv *env, jobject obj, jint index, jstring fname, int szhint) {
	lua_State *L;
	
	findtable_fname = NULL;
	findtable_result = NULL;
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checkindex(L, index)
			&& (findtable_fname = getstringchars(fname))
			&& checkarg(szhint >= 0, "illegal size hint")) {
		findtable_szhint = szhint;
		index = lua_absindex(L, index);
		lua_pushcfunction(L, findtable_protected);
		lua_pushvalue(L, index);
		JNLUA_PCALL(L, 1, LUA_MULTRET);
	}
	if (findtable_fname) {
		releasestringchars(fname, findtable_fname);
	}
	return findtable_result ? (*env)->NewStringUTF(env, findtable_result) : NULL;
}

/* lua_getfield() */
JNLUA_THREADLOCAL const char *getfield_k;
static int getfield_protected (lua_State *L) {
	lua_getfield(L, 1, getfield_k);
	return 1;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1getfield (JNIEnv *env, jobject obj, jint index, jstring k) {
	lua_State *L;

	getfield_k = NULL;
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, index, LUA_TTABLE)
			&& (getfield_k = getstringchars(k))) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, getfield_protected);
		lua_pushvalue(L, index);
		JNLUA_PCALL(L, 1, 1);
	}
	if (getfield_k) {
		releasestringchars(k, getfield_k);
	}
}

/* lua_gettable() */
static int gettable_protected (lua_State *L) {
	lua_gettable(L, 1);
	return 1;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1gettable (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, index, LUA_TTABLE)) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, gettable_protected);
		lua_insert(L, -2);
		lua_pushvalue(L, index);
		lua_insert(L, -2);
		JNLUA_PCALL(L, 2, 1);
	}
}

/* lua_newtable() */
static int newtable_protected (lua_State *L) {
	lua_newtable(L);
	return 1;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1newtable (JNIEnv *env, jobject obj) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)) {
		lua_pushcfunction(L, newtable_protected);
		JNLUA_PCALL(L, 0, 1);
	}
}

/* lua_next() */
JNLUA_THREADLOCAL int next_result;
static int next_protected (lua_State *L) {
	next_result = lua_next(L, 1);
	return next_result ? 2 : 0;
}
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1next (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, index, LUA_TTABLE)) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, next_protected);
		lua_insert(L, -2);
		lua_pushvalue(L, index);
		lua_insert(L, -2);
		JNLUA_PCALL(L, 2, LUA_MULTRET);
	}
	return (jint) next_result;
}

/* lua_rawget() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1rawget (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checktype(L, index, LUA_TTABLE)) {
		lua_rawget(L, index);
	}
}

/* lua_rawgeti() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1rawgeti (JNIEnv *env, jobject obj, jint index, jint n) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, index, LUA_TTABLE)) {
		lua_rawgeti(L, index, n);
	}
}

/* lua_rawset() */
static int rawset_protected (lua_State *L) {
	lua_rawset(L, 1);
	return 0;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1rawset (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, index, LUA_TTABLE)
			&& checknelems(L, 2)) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, rawset_protected);
		lua_insert(L, -3);
		lua_pushvalue(L, index);
		lua_insert(L, -3);
		JNLUA_PCALL(L, 3, 0);
	}
}

/* lua_rawseti() */
JNLUA_THREADLOCAL int rawseti_n;
static int rawseti_protected (lua_State *L) {
	lua_rawseti(L, 1, rawseti_n);
	return 0;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1rawseti (JNIEnv *env, jobject obj, jint index, jint n) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, index, LUA_TTABLE)) {
		rawseti_n = n;
		index = lua_absindex(L, index);
		lua_pushcfunction(L, rawseti_protected);
		lua_insert(L, -2);
		lua_pushvalue(L, index);
		lua_insert(L, -2);
		JNLUA_PCALL(L, 2, 0);
	}
}

/* lua_settable() */
static int settable_protected (lua_State *L) {
	lua_settable(L, 1);
	return 0;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1settable (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, index, LUA_TTABLE)
			&& checknelems(L, 2)) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, settable_protected);
		lua_insert(L, -3);
		lua_pushvalue(L, index);
		lua_insert(L, -3);
		JNLUA_PCALL(L, 3, 0);
	}
}

/* lua_setfield() */
JNLUA_THREADLOCAL const char *setfield_k;
static int setfield_protected (lua_State *L) {
	lua_setfield(L, 1, setfield_k);
	return 0;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1setfield (JNIEnv *env, jobject obj, jint index, jstring k) {
	lua_State *L;
	
	setfield_k = NULL;
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, index, LUA_TTABLE)
			&& (setfield_k = getstringchars(k))) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, setfield_protected);
		lua_insert(L, -2);
		lua_pushvalue(L, index);
		lua_insert(L, -2);
		JNLUA_PCALL(L, 2, 0);
	}
	if (setfield_k) {
		releasestringchars(k, setfield_k);
	}
}

/* ---- Metatable ---- */
/* lua_getmetatable() */
JNIEXPORT int JNICALL Java_com_naef_jnlua_LuaState_lua_1getmetatable (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	int result = 0;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (lua_checkstack(L, JNLUA_MINSTACK)
			&& checkindex(L, index)) {
		result = lua_getmetatable(L, index);
	}
	return (jint) result;
}

/* lua_setmetatable() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1setmetatable (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	int result = 0;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkindex(L, index)
			&& checknelems(L, 1)
			&& checkarg(lua_type(L, -1) == LUA_TTABLE || lua_type(L, -1) == LUA_TNIL, "illegal type")) {
		result = lua_setmetatable(L, index);
	}
	return (jint) result;
}

/* lua_getmetafield() */
JNLUA_THREADLOCAL const char *getmetafield_k;
JNLUA_THREADLOCAL int getmetafield_result;
static int getmetafield_protected (lua_State *L) {
	getmetafield_result = luaL_getmetafield(L, 1, getmetafield_k);
	return getmetafield_result ? 1 : 0;
}
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1getmetafield (JNIEnv *env, jobject obj, jint index, jstring k) {
	lua_State *L;
	
	getmetafield_k = NULL;
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checkindex(L, index)
			&& (getmetafield_k = getstringchars(k))) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, getmetafield_protected);
		lua_pushvalue(L, index);
		JNLUA_PCALL(L, 1, LUA_MULTRET);
	}
	if (getmetafield_k) {
		releasestringchars(k, getmetafield_k);
	}
	return (jint) getmetafield_result;
}

/* ---- Function environment ---- */
/* lua_getfenv() */
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1getfenv (JNIEnv *env, jobject obj, jint index) {
	lua_State* L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checkindex(L, index)) {
		lua_getfenv(L, index);
	}
}

/* lua_setfenv() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1setfenv (JNIEnv *env, jobject obj, jint index) {
	lua_State* L;
	int result = 0;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkindex(L, index)
			&& checktype(L, -1, LUA_TTABLE)) {
		result = lua_setfenv(L, index);
	}
	return (jint) result;
}

/* ---- Thread ---- */
/* lua_newthread() */
static int newthread_protected (lua_State *L) {
	lua_State *T;
	
	T = lua_newthread(L);
	lua_insert(L, 1);
	lua_xmove(L, T, 1);
	return 1;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1newthread (JNIEnv *env, jobject obj) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, -1, LUA_TFUNCTION)) {
		lua_pushcfunction(L, newthread_protected);
		lua_insert(L, -2);
		JNLUA_PCALL(L, 1, 1);
	}
}

/* lua_resume() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1resume (JNIEnv *env, jobject obj, jint index, jint nargs) {
	lua_State *L, *T;
	int status;
	int nresults = 0;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checktype(L, index, LUA_TTHREAD)
			&& checkarg(nargs >= 0, "illegal argument count")
			&& checknelems(L, nargs + 1)) {
		T = lua_tothread(L, index);
		if (checkstack(T, nargs)) {
			lua_xmove(L, T, nargs);
			status = lua_resume(T, nargs);
			switch (status) {
			case 0:
			case LUA_YIELD:
				nresults = lua_gettop(T);
				if (checkstack(L, nresults)) {
					lua_xmove(T, L, nresults);
				}
				break;
			default:
				throw(L, status);
			}
		}
	}
	return (jint) nresults;
}

/* lua_status() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1status (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	int result = 0;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checktype(L, index, LUA_TTHREAD)) {
		result = lua_status(lua_tothread(L, index));
	}
	return (jint) result;	
}

/* lua_yield() */
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1yield (JNIEnv *env, jobject obj, int nresults) {
	lua_State *L;
	int result = 0;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkarg(nresults >= 0, "illegal return count")
			&& checknelems(L, nresults)
			&& checkstate(L != getluastate(obj), "not in a thread")) {
		result = lua_yield(L, nresults);
	}
	return (jint) result;
}

/* ---- Reference ---- */
/* lua_ref() */
JNLUA_THREADLOCAL int ref_result;
static int ref_protected (lua_State *L) {
	ref_result = luaL_ref(L, 1);
	return 0;
}
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1ref (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, index, LUA_TTABLE)) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, ref_protected);
		lua_insert(L, -2);
		lua_pushvalue(L, index);
		lua_insert(L, -2);
		JNLUA_PCALL(L, 2, 0);
	}
	return (jint) ref_result;
}

/* lua_unref() */
JNLUA_THREADLOCAL int unref_ref;
static int unref_protected (lua_State *L) {
	luaL_unref(L, 1, unref_ref);
	return 0;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1unref (JNIEnv *env, jobject obj, jint index, jint ref) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, index, LUA_TTABLE)) {
		unref_ref = ref;
		index = lua_absindex(L, index);
		lua_pushcfunction(L, unref_protected);
		lua_pushvalue(L, index);
		JNLUA_PCALL(L, 1, 0);
	}
}

/* ---- Function arguments ---- */
/* Returns the current function name. */
JNLUA_THREADLOCAL const char *funcname_result;
static int funcname_protected (lua_State *L) {
	lua_Debug ar;
	
	if (lua_getstack(L, 1, &ar) && lua_getinfo(L, "n", &ar)) {
		funcname_result = ar.name;
	}
	return 0;
}
JNIEXPORT jstring JNICALL Java_com_naef_jnlua_LuaState_lua_1funcname (JNIEnv *env, jobject obj) {
	lua_State* L;
	
	funcname_result = NULL;
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)) {
		lua_pushcfunction(L, funcname_protected);
		JNLUA_PCALL(L, 0, 0);
	}
	return funcname_result ? (*env)->NewStringUTF(env, funcname_result) : NULL;
}

/* Returns the effective argument number, adjusting for methods. */
JNLUA_THREADLOCAL int narg_result;
static int narg_protected (lua_State *L) {
	lua_Debug ar;
	
	if (lua_getstack(L, 1, &ar) && lua_getinfo(L, "n", &ar)) {
		if (ar.namewhat && strcmp(ar.namewhat, "method") == 0) {
			narg_result--;
		}
	}
	return 0;
}
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1narg (JNIEnv *env, jobject obj, jint narg) {
	lua_State *L;
	
	narg_result = narg;
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)) {
		lua_pushcfunction(L, narg_protected);
		JNLUA_PCALL(L, 0, 0);
	}
	return (jint) narg_result;
}

/* ---- Optimization ---- */
/* lua_tablesize() */
JNLUA_THREADLOCAL int tablesize_result;
static int tablesize_protected (lua_State *L) {
	int count = 0;
	
	lua_pushnil(L);
	while (lua_next(L, -2)) {
		lua_pop(L, 1);
		count++;
	}
	tablesize_result = count;
	return 0;
}
JNIEXPORT jint JNICALL Java_com_naef_jnlua_LuaState_lua_1tablesize (JNIEnv *env, jobject obj, jint index) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, index, LUA_TTABLE)) {
		index = lua_absindex(L, index);
		lua_pushcfunction(L, tablesize_protected);
		lua_pushvalue(L, index);
		JNLUA_PCALL(L, 1, 0);
	}
	return (jint) tablesize_result;
}

/* lua_tablemove() */
JNLUA_THREADLOCAL int tablemove_from;
JNLUA_THREADLOCAL int tablemove_to;
JNLUA_THREADLOCAL int tablemove_count;
static int tablemove_protected (lua_State *L) {
	int from = tablemove_from, to = tablemove_to;
	int count = tablemove_count, i;
	
	if (from < to) {
		for (i = count - 1; i >= 0; i--) {
			lua_rawgeti(L, 1, from + i);
			lua_rawseti(L, 1, to + i);
		}
	} else if (from > to) {
		for (i = 0; i < count; i++) { 
			lua_rawgeti(L, 1, from + i);
			lua_rawseti(L, 1, to + i);
		}
	}
	return 0;
}
JNIEXPORT void JNICALL Java_com_naef_jnlua_LuaState_lua_1tablemove (JNIEnv *env, jobject obj, jint index, jint from, jint to, jint count) {
	lua_State *L;
	
	JNLUA_ENV(env);
	L = getluathread(obj);
	if (checkstack(L, JNLUA_MINSTACK)
			&& checktype(L, index, LUA_TTABLE)
			&& checkarg(count >= 0, "illegal count")) {
		tablemove_from = from;
		tablemove_to = to;
		tablemove_count = count;
		index = lua_absindex(L, index);
		lua_pushcfunction(L, tablemove_protected);
		lua_pushvalue(L, index);
		JNLUA_PCALL(L, 1, 0);
	}
}

/* ---- JNI ---- */
/* Handles the loading of this library. */
JNIEXPORT jint JNICALL JNI_OnLoad (JavaVM *vm, void *reserved) {
	JNIEnv *env;
	
	/* Ansca: Store the Java VM pointer to a global. To be used to safely fetch a JNIEnv pointer for the current thread. */
	java_vm = vm;
	
	/* Get environment */
	env = get_jni_env();
	
	/* Lookup and pin classes, fields and methods */
	if (!(luastate_class = referenceclass(env, "com/naef/jnlua/LuaState"))
			|| !(luastate_id = (*env)->GetFieldID(env, luastate_class, "luaState", "J"))
			|| !(luathread_id = (*env)->GetFieldID(env, luastate_class, "luaThread", "J"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(javafunction_interface = referenceclass(env, "com/naef/jnlua/JavaFunction"))
			|| !(invoke_id = (*env)->GetMethodID(env, javafunction_interface, "invoke", "(Lcom/naef/jnlua/LuaState;)I"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(luaruntimeexception_class = referenceclass(env, "com/naef/jnlua/LuaRuntimeException"))
			|| !(luaruntimeexception_id = (*env)->GetMethodID(env, luaruntimeexception_class, "<init>", "(Ljava/lang/String;)V"))
			|| !(setluaerror_id = (*env)->GetMethodID(env, luaruntimeexception_class, "setLuaError", "(Lcom/naef/jnlua/LuaError;)V"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(luasyntaxexception_class = referenceclass(env, "com/naef/jnlua/LuaSyntaxException"))
			|| !(luasyntaxexception_id = (*env)->GetMethodID(env, luasyntaxexception_class, "<init>", "(Ljava/lang/String;)V"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(luamemoryallocationexception_class = referenceclass(env, "com/naef/jnlua/LuaMemoryAllocationException"))
			|| !(luamemoryallocationexception_id = (*env)->GetMethodID(env, luamemoryallocationexception_class, "<init>", "(Ljava/lang/String;)V"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(luamessagehandlerexception_class = referenceclass(env, "com/naef/jnlua/LuaMessageHandlerException"))
			|| !(luamessagehandlerexception_id = (*env)->GetMethodID(env, luamessagehandlerexception_class, "<init>", "(Ljava/lang/String;)V"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(luastacktraceelement_class = referenceclass(env, "com/naef/jnlua/LuaStackTraceElement"))
			|| !(luastacktraceelement_id = (*env)->GetMethodID(env, luastacktraceelement_class, "<init>", "(Ljava/lang/String;Ljava/lang/String;I)V"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(luaerror_class = referenceclass(env, "com/naef/jnlua/LuaError"))
			|| !(luaerror_id = (*env)->GetMethodID(env, luaerror_class, "<init>", "(Ljava/lang/String;Ljava/lang/Throwable;)V"))
			|| !(setluastacktrace_id = (*env)->GetMethodID(env, luaerror_class, "setLuaStackTrace", "([Lcom/naef/jnlua/LuaStackTraceElement;)V"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(nullpointerexception_class = referenceclass(env, "java/lang/NullPointerException"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(illegalargumentexception_class = referenceclass(env, "java/lang/IllegalArgumentException"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(illegalstateexception_class = referenceclass(env, "java/lang/IllegalStateException"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(error_class = referenceclass(env, "java/lang/Error"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(inputstream_class = referenceclass(env, "java/io/InputStream"))
			|| !(read_id = (*env)->GetMethodID(env, inputstream_class, "read", "([B)I"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(outputstream_class = referenceclass(env, "java/io/OutputStream"))
			|| !(write_id = (*env)->GetMethodID(env, outputstream_class, "write", "([BII)V"))) {
		return JNLUA_JNIVERSION;
	}
	if (!(ioexception_class = referenceclass(env, "java/io/IOException"))) {
		return JNLUA_JNIVERSION;
	}

	/* OK */
	initialized = 1;
	return JNLUA_JNIVERSION;
}

/* Handles the unloading of this library. */
JNIEXPORT void JNICALL JNI_OnUnload (JavaVM *vm, void *reserved) {
	JNIEnv *env;
	
	/* Get environment */
	env = get_jni_env();
	
	/* Free classes */
	if (luastate_class) {
		(*env)->DeleteGlobalRef(env, luastate_class);
	}
	if (javafunction_interface) {
		(*env)->DeleteGlobalRef(env, javafunction_interface);
	}
	if (luaruntimeexception_class) {
		(*env)->DeleteGlobalRef(env, luaruntimeexception_class);
	}
	if (luasyntaxexception_class) {
		(*env)->DeleteGlobalRef(env, luasyntaxexception_class);
	}
	if (luamemoryallocationexception_class) {
		(*env)->DeleteGlobalRef(env, luamemoryallocationexception_class);
	}
	if (luamessagehandlerexception_class) {
		(*env)->DeleteGlobalRef(env, luamessagehandlerexception_class);
	}
	if (luastacktraceelement_class) {
		(*env)->DeleteGlobalRef(env, luastacktraceelement_class);
	}
	if (luaerror_class) {
		(*env)->DeleteGlobalRef(env, luaerror_class);
	}
	if (nullpointerexception_class) {
		(*env)->DeleteGlobalRef(env, nullpointerexception_class);
	}
	if (illegalargumentexception_class) {
		(*env)->DeleteGlobalRef(env, illegalargumentexception_class);
	}
	if (illegalstateexception_class) {
		(*env)->DeleteGlobalRef(env, illegalstateexception_class);
	}
	if (error_class) {
		(*env)->DeleteGlobalRef(env, error_class);
	}
	if (inputstream_class) {
		(*env)->DeleteGlobalRef(env, inputstream_class);
	}
	if (outputstream_class) {
		(*env)->DeleteGlobalRef(env, outputstream_class);
	}
	if (ioexception_class) {
		(*env)->DeleteGlobalRef(env, ioexception_class);
	}
	
	/* Ansca: Release the pointer to the Java VM. */
	java_vm = NULL;
}

/* ---- JNI helpers ---- */
/* Finds a class and returns a new JNI global reference to it. */
static jclass referenceclass (JNIEnv *env, const char *className) {
	jclass clazz;
	
	clazz = (*env)->FindClass(env, className);
	if (!clazz) {
		return NULL;
	}
	return (*env)->NewGlobalRef(env, clazz);
}

/* Return a new JNI byte array. */
static jbyteArray newbytearray (jsize length) {
	jbyteArray array;
	
	array = (*thread_env)->NewByteArray(thread_env, length);
	if (!check(array != NULL, luamemoryallocationexception_class, "JNI error: NewByteArray() failed")) {
		return NULL;
	}
	return array;
}

/* Returns the  UTF chars of a string. */
static const char *getstringchars (jstring string) {
	const char *utf;

	if (!checknotnull(string)) {
		return NULL;
	}
	utf = (*thread_env)->GetStringUTFChars(thread_env, string, NULL);
	if (!check(utf != NULL, luamemoryallocationexception_class, "JNI error: GetStringUTFChars() failed")) {
		return NULL;
	}
	return utf;
}

/* Releaes the UTF chars of a string. */
static void releasestringchars (jstring string, const char *chars) {
	(*thread_env)->ReleaseStringUTFChars(thread_env, string, chars);
}

/* ---- Java state operations ---- */
/* Returns the Lua state from the Java state. */
static lua_State *getluastate (jobject javastate) {
	return (lua_State *) (uintptr_t) (*thread_env)->GetLongField(thread_env, javastate, luastate_id);
}

/* Sets the Lua state in the Java state. */
static void setluastate (jobject javastate, lua_State *L) {
	(*thread_env)->SetLongField(thread_env, javastate, luastate_id, (jlong) (uintptr_t) L);
}

/* Returns the Lua thread from the Java state. */
static lua_State *getluathread (jobject javastate) {
	return (lua_State *) (uintptr_t) (*thread_env)->GetLongField(thread_env, javastate, luathread_id);
}

/* Sets the Lua state in the Java state. */
static void setluathread (jobject javastate, lua_State *L) {
	(*thread_env)->SetLongField(thread_env, javastate, luathread_id, (jlong) (uintptr_t) L);
}

/* ---- Checks ---- */
/* Returns whether an index is valid. */
static int validindex (lua_State *L, int index) {
	int top;
	
	top = lua_gettop(L);
	if (index <= 0) {
		if (index > LUA_REGISTRYINDEX) {
			index = top + index + 1;
		} else {
			switch (index) {
			case LUA_REGISTRYINDEX:
			case LUA_ENVIRONINDEX:
			case LUA_GLOBALSINDEX:
				return 1;
			default:
				return 0; /* C upvalue access not needed, don't even validate */
			}
		}
	}
	return index >= 1 && index <= top;
}

/* Checks stack space. */
static int checkstack (lua_State *L, int space) {
	return check(lua_checkstack(L, space), illegalstateexception_class, "stack overflow");
}

/* Checks if an index is valid. */
static int checkindex (lua_State *L, int index) {
	return checkarg(validindex(L, index), "illegal index");
}
	
/* Checks if an index is valid, ignoring pseudo indexes. */
static int checkrealindex (lua_State *L, int index) {
	int top;
	
	top = lua_gettop(L);
	if (index <= 0) {
		index = top + index + 1;
	}
	return checkarg(index >= 1 && index <= top, "illegal index");
}

/* Checks the type of a stack value. */
static int checktype (lua_State *L, int index, int type) {
	return checkindex(L, index)
			&& checkarg(lua_type(L, index) == type, "illegal type");
}
	
/* Checks that there are at least n values on the stack. */
static int checknelems (lua_State *L, int n) {
	return checkstate(lua_gettop(L) >= n, "stack underflow");
}

/* Checks an argument for not-null. */ 
static int checknotnull (void *object) {
	return check(object != NULL, nullpointerexception_class, "null");
}

/* Checks an argument condition. */
static int checkarg (int cond, const char *msg) {
	return check(cond, illegalargumentexception_class, msg);
}

/* Checks a state condition. */
static int checkstate (int cond, const char *msg) {
	return check(cond, illegalstateexception_class, msg);
}

/* Checks a condition. */
static int check (int cond, jthrowable throwable_class, const char *msg) {
	if (cond) {
		return 1;
	}
	(*thread_env)->ThrowNew(thread_env, throwable_class, msg);
	return 0;
}

/* ---- Java objects and functions ---- */
/* Pushes a Java object on the stack. */
static void pushjavaobject (lua_State *L, jobject object) {
	jobject *user_data;
	
	user_data = (jobject *) lua_newuserdata(L, sizeof(jobject));
	luaL_getmetatable(L, JNLUA_OBJECT);
	*user_data = (*thread_env)->NewGlobalRef(thread_env, object);
	if (!*user_data) {
		lua_pushliteral(L, "JNI error: NewGlobalRef() failed pushing Java object");
		lua_error(L);
	}
	lua_setmetatable(L, -2);
}
	
/* Returns the Java object at the specified index, or NULL if such an object is unobtainable. */
static jobject tojavaobject (lua_State *L, int index, jclass class) {
	int result;
	jobject object;

	if (!lua_isuserdata(L, index)) {
		return NULL;
	}
	if (!lua_getmetatable(L, index)) {
		return NULL;
	}
	luaL_getmetatable(L, JNLUA_OBJECT);
	result = lua_rawequal(L, -1, -2);
	lua_pop(L, 2);
	if (!result) {
		return NULL;
	}
	object = *(jobject *) lua_touserdata(L, index);
	if (class) {
		if (!(*thread_env)->IsInstanceOf(thread_env, object, class)) {
			return NULL;
		}
	}
	return object;
}

/* Returns a Java string for a value on the stack. */
static jstring tostring (lua_State *L, int index) {
	jstring string;

	if (!luaL_callmeta(L, index, "__tostring")) {
		switch (lua_type(L, index)) {
		case LUA_TNUMBER:
		case LUA_TSTRING:
			lua_pushvalue(L, index);
			break;
		case LUA_TBOOLEAN:
			lua_pushstring(L, lua_toboolean(L, index) ? "true" : "false");
			break;
		case LUA_TNIL:
			lua_pushliteral(L, "nil");
			break;
		default:
			lua_pushfstring(L, "%s: %p", luaL_typename(L, index), lua_topointer(L, index));
		}
	}
	string = (*thread_env)->NewStringUTF(thread_env, lua_tostring(L, -1));
	lua_pop(L, 1);
	return string;
}

/* Finalizes Java objects. */
static int gcjavaobject (lua_State *L) {
	jobject obj;

	if (!thread_env) {
		/* Environment has been cleared as the Java VM was destroyed. Nothing to do. */
		return 0;
	}
	obj = *(jobject *) lua_touserdata(L, 1);
	if (lua_toboolean(L, lua_upvalueindex(1))) {
		(*thread_env)->DeleteWeakGlobalRef(thread_env, obj);
	} else {
		(*thread_env)->DeleteGlobalRef(thread_env, obj);
	}
	return 0;
}

/*
 * Checks if an unhandled Java exception has occurred.
 * If so, then this function will catch the exception and throw a Lua error.
 */
static void handlejavaexception(lua_State *L) {
	jmethodID toStringMethodId;
	jthrowable throwable;
	jobject luaerror;
	jstring javaString;
	const char *stringPointer;
	int hasPushedErrorMessage = 0;

	/* Check if a Java exception was thrown. If not, then there no error to handle. */
	if ((*thread_env)->ExceptionCheck(thread_env) == JNI_FALSE) {
		return;
	}

	/* A Java exception was thrown. Catch it here. */
	throwable = (*thread_env)->ExceptionOccurred(thread_env);
	(*thread_env)->ExceptionClear(thread_env);

	/* Push a string indicating the current position in the Lua script. */
	luaL_where(L, 1);

	/* Push the exception's error message and stack trace. */
	luaerror = (*thread_env)->NewObject(thread_env, luaerror_class, luaerror_id, NULL, throwable);
	if (luaerror) {
		toStringMethodId = (*thread_env)->GetMethodID(thread_env, luaerror_class, "toString", "()Ljava/lang/String;");
		if (toStringMethodId) {
			javaString = (*thread_env)->CallObjectMethod(thread_env, luaerror, toStringMethodId);
			if (javaString) {
				stringPointer = (*thread_env)->GetStringUTFChars(thread_env, javaString, 0);
				if (stringPointer) {
					lua_pushstring(L, stringPointer);
					hasPushedErrorMessage = 1;
				}
				(*thread_env)->ReleaseStringUTFChars(thread_env, javaString, stringPointer);
				(*thread_env)->DeleteLocalRef(thread_env, javaString);
			}
		}
	}
	if (!hasPushedErrorMessage) {
		lua_pushliteral(L, "Java exception occurred.");
		hasPushedErrorMessage = 1;
	}

	/* Concatenate the last two pushed strings into one. */
	lua_concat(L, 2);

	/* Throw a Lua error. */
	lua_error(L);
}

/* Calls a Java function. If an exception is reported, store it as the cause for later use. */
static int calljavafunction (lua_State *L) {
	jobject javastate, javafunction;
	lua_State *T;
	int nresults;
	
	/* Get Java state. */
	lua_getfield(L, LUA_REGISTRYINDEX, JNLUA_JAVASTATE);
	if (!lua_isuserdata(L, -1)) {
		/* Java state has been cleared as the Java VM was destroyed. Cannot call. */
		lua_pushliteral(L, "no Java state");
		return lua_error(L);
	}
	javastate = *(jobject *) lua_touserdata(L, -1);
	lua_pop(L, 1);
	
	/* Get Java function object. */
	lua_pushvalue(L, lua_upvalueindex(1));
	javafunction = tojavaobject(L, -1, javafunction_interface);
	lua_pop(L, 1);
	if (!javafunction) {
		/* Function was cleared from outside JNLua code. */
		lua_pushliteral(L, "no Java function");
		return lua_error(L);
	}
	
	/* Perform the call, handling coroutine situations. */
	T = getluathread(javastate);
	if (T == L) {
		nresults = (*thread_env)->CallIntMethod(thread_env, javafunction, invoke_id, javastate);
	} else {
		setluathread(javastate, L);
		nresults = (*thread_env)->CallIntMethod(thread_env, javafunction, invoke_id, javastate);
		setluathread(javastate, T);
	}
	
	/* Throw a Lua error if a Java exception occurred. */
	handlejavaexception(L);
	
	return nresults;
}

/* Handles Lua errors. */
static int messagehandler (lua_State *L) {
	int level, count;
	lua_Debug ar;
	jobjectArray luastacktrace;
	jstring name, source;
	jobject luastacktraceelement;
	jobject luaerror;
	jstring message;

	/* Count relevant stack frames */
	level = 1;
	count = 0;
	while (lua_getstack(L, level, &ar)) {
		lua_getinfo(L, "nSl", &ar);
		if (isrelevant(&ar)) {
			count++;
		}
		level++;
	}
	
	/* Create Lua stack trace as a Java LuaStackTraceElement[] */
	luastacktrace = (*thread_env)->NewObjectArray(thread_env, count, luastacktraceelement_class, NULL);
	if (!luastacktrace) {
		return 1;
	}
	level = 1;
	count = 0;
	while (lua_getstack(L, level, &ar)) {
		lua_getinfo(L, "nSl", &ar);
		if (isrelevant(&ar)) {
			name = ar.name ? (*thread_env)->NewStringUTF(thread_env, ar.name) : NULL;
			source = ar.source ? (*thread_env)->NewStringUTF(thread_env, ar.source) : NULL;
			luastacktraceelement = (*thread_env)->NewObject(thread_env, luastacktraceelement_class,	luastacktraceelement_id, name, source, ar.currentline);
			if (!luastacktraceelement) {
				return 1;
			}
			(*thread_env)->SetObjectArrayElement(thread_env, luastacktrace, count, luastacktraceelement);
			if ((*thread_env)->ExceptionCheck(thread_env)) {
				return 1;
			}
			count++;
		}
		level++;
	}
	
	/* Get or create the error object  */
	luaerror = tojavaobject(L, -1, luaerror_class);
	if (!luaerror) {
		message = tostring(L, -1);
		if (!(luaerror = (*thread_env)->NewObject(thread_env, luaerror_class, luaerror_id, message, NULL))) {
			return 1;
		}
	}
	(*thread_env)->CallVoidMethod(thread_env, luaerror, setluastacktrace_id, luastacktrace);
	
	/* Replace error */
	pushjavaobject(L, luaerror);
	return 1;
}

/* Processes a Lua activation record and returns whether it is relevant. */
static int isrelevant (lua_Debug *ar) {
	if (ar->name && strlen(ar->name) == 0) {
		ar->name = NULL;
	}
	if (ar->what && strcmp(ar->what, "C") == 0) {
		ar->source = NULL;
	}
	if (ar->source) {
		if (*ar->source == '=' || *ar->source == '@') {
			ar->source++;
		}
	}
	return ar->name || ar->source;
}

/* Handles Lua errors by throwing a Java exception. */
JNLUA_THREADLOCAL int throw_status;
static int throw_protected (lua_State *L) {
	jclass class;
	jmethodID id;
	jthrowable throwable;
	jobject luaerror;
	
	/* Determine the type of exception to throw. */
	switch (throw_status) {
	case LUA_ERRRUN:
		class = luaruntimeexception_class;
		id = luaruntimeexception_id;
		break;
	case LUA_ERRSYNTAX:
		class = luasyntaxexception_class;
		id = luasyntaxexception_id;
		break;
	case LUA_ERRMEM:
		class = luamemoryallocationexception_class;
		id = luamemoryallocationexception_id;
		break;
	case LUA_ERRERR:
		class = luamessagehandlerexception_class;
		id = luamessagehandlerexception_id;
		break;
	default:
		lua_pushfstring(L, "unknown Lua status %d", throw_status);
		return lua_error(L);
	}
	
	/* Create exception */
	throwable = (*thread_env)->NewObject(thread_env, class, id, tostring(L, 1));
	if (!throwable) {
		lua_pushliteral(L, "JNI error: NewObject() failed creating throwable");
		return lua_error(L);
	}
		
	/* Set the Lua error, if any. */
	luaerror = tojavaobject(L, 1, luaerror_class);
	if (luaerror && class == luaruntimeexception_class) {
		(*thread_env)->CallVoidMethod(thread_env, throwable, setluaerror_id, luaerror);
	}
	
	/* Throw */
	if ((*thread_env)->Throw(thread_env, throwable) < 0) {
		lua_pushliteral(L, "JNI error: Throw() failed");
		return lua_error(L);
	}
	
	return 0;
}
static void throw (lua_State *L, int status) {
	const char *message;
	
	if (checkstack(L, JNLUA_MINSTACK)) {
		throw_status = status;
		lua_pushcfunction(L, throw_protected);
		lua_insert(L, -2);
		if (lua_pcall(L, 1, 0, 0) != 0) {
			message = lua_tostring(L, -1);
			(*thread_env)->ThrowNew(thread_env, error_class, message ? message : "error throwing Lua exception");
		}
	}
}

/* ---- Stream adapters ---- */
/* Lua reader for Java input streams. */
static const char *readhandler (lua_State *L, void *ud, size_t *size) {
	Stream *stream;
	int read;

	stream = (Stream *) ud;
	read = (*thread_env)->CallIntMethod(thread_env, stream->stream, read_id, stream->byte_array);
	if ((*thread_env)->ExceptionCheck(thread_env)) {
		return NULL;
	}
	if (read == -1) {
		return NULL;
	}
	if (stream->bytes && stream->is_copy) {
		(*thread_env)->ReleaseByteArrayElements(thread_env, stream->byte_array, stream->bytes, JNI_ABORT);
		stream->bytes = NULL;
	}
	if (!stream->bytes) {
		stream->bytes = (*thread_env)->GetByteArrayElements(thread_env, stream->byte_array, &stream->is_copy);
		if (!stream->bytes) {
			(*thread_env)->ThrowNew(thread_env, ioexception_class, "JNI error: GetByteArrayElements() failed accessing IO buffer");
			return NULL;
		}
	}
	*size = (size_t) read;
	return (const char *) stream->bytes;
}

/* Lua writer for Java output streams. */
static int writehandler (lua_State *L, const void *data, size_t size, void *ud) {
	Stream *stream;

	stream = (Stream *) ud;
	if (!stream->bytes) {
		stream->bytes = (*thread_env)->GetByteArrayElements(thread_env, stream->byte_array, &stream->is_copy);
		if (!stream->bytes) {
			(*thread_env)->ThrowNew(thread_env, ioexception_class, "JNI error: GetByteArrayElements() failed accessing IO buffer");
			return 1;
		}
	}
	memcpy(stream->bytes, data, size);
	if (stream->is_copy) {
		(*thread_env)->ReleaseByteArrayElements(thread_env, stream->byte_array, stream->bytes, JNI_COMMIT);
	}
	(*thread_env)->CallVoidMethod(thread_env, stream->stream, write_id, stream->byte_array, 0, size);
	if ((*thread_env)->ExceptionCheck(thread_env)) {
		return 1;
	}
	return 0;
}
