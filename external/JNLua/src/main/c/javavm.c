/*
 * $Id: javavm.c 131 2012-01-23 20:25:29Z andre@naef.com $
 * Provides the Java VM module. See LICENSE.txt for license terms.
 */

#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <lauxlib.h>
#ifdef LUA_WIN
#include <stddef.h>
#pragma warning(disable : 4996)
#endif
#ifdef LUA_USE_POSIX
#include <stdint.h>
#endif
#include "javavm.h"

/*
 * Java VM parameters.
 */
#define JAVAVM_METATABLE "javavm.metatable"
#define JAVAVM_VM "javavm.vm"
#define JAVAVM_MAXOPTIONS 128
#define JAVAVM_JNIVERSION JNI_VERSION_1_6

/*
 * VM record.
 */
typedef struct vm_rec {
	JavaVM *vm;
	jobject luastate;
	int num_options;
	JavaVMOption options[JAVAVM_MAXOPTIONS];
} vm_rec;

/*
 * Raises an error from JNI.
 */
static int error (lua_State *L, JNIEnv *env, const char *msg) {
	jthrowable throwable;
	jclass throwable_class;
	jmethodID tostring_id;
	jstring string;
	const char *extramsg = NULL;
	
	throwable = (*env)->ExceptionOccurred(env);
	if (throwable) {
		throwable_class = (*env)->GetObjectClass(env, throwable);
		if ((tostring_id = (*env)->GetMethodID(env, throwable_class, "toString", "()Ljava/lang/String;"))) {
			string = (*env)->CallObjectMethod(env, throwable, tostring_id);
			if (string) {
				extramsg = (*env)->GetStringUTFChars(env, string, NULL);
			}
		}
	}
	if (extramsg) {
		lua_pushfstring(L, "%s (%s)", msg, extramsg);
		(*env)->ReleaseStringUTFChars(env, string, extramsg);
	} else {
		lua_pushstring(L, msg);
	}
	return luaL_error(L, lua_tostring(L, -1));
}

/*
 * Releases a VM.
 */
static int release_vm (lua_State *L) {
	vm_rec *vm;
	jclass luastate_class;
	jmethodID close_id;
	JNIEnv *env;
	int res;
	int i;

	/* Get VM */
	vm = luaL_checkudata(L, 1, JAVAVM_METATABLE);
	
	/* Already released? */
	if (!vm->vm) {
		return 0;
	}
	
	/* Check thread */
	if ((*vm->vm)->GetEnv(vm->vm, (void **) &env, JAVAVM_JNIVERSION) != JNI_OK) {
		return luaL_error(L, "invalid thread");
	}
	
	/* Close the Lua state in the Java VM */
	if (vm->luastate) {
		luastate_class = (*env)->GetObjectClass(env, vm->luastate);
		if (!(close_id = (*env)->GetMethodID(env, luastate_class, "close", "()V"))) {
			return error(L, env, "close method not found");
		}
		(*env)->CallVoidMethod(env, vm->luastate, close_id);
		(*env)->DeleteGlobalRef(env, vm->luastate);
		vm->luastate = NULL;
	}
	
	/* Destroy the Java VM */
	res = (*vm->vm)->DestroyJavaVM(vm->vm);
	if (res < 0) {
		return luaL_error(L, "error destroying Java VM: %d", res);
	}
	vm->vm = NULL;

	/* Free options */
	for (i = 0; i < vm->num_options; i++) {
		free(vm->options[i].optionString);
		vm->options[i].optionString = NULL;
	}
	vm->num_options = 0;
	
	return 0;
}

/*
 * Returns a string representation of a VM.
 */
static int tostring_vm (lua_State *L) {
	vm_rec *vm;
	int i;
	
	vm = luaL_checkudata(L, 1, JAVAVM_METATABLE);
	lua_pushfstring(L, "Java VM (%p)", vm->vm);
	luaL_checkstack(L, vm->num_options, NULL);
	for (i = 0; i < vm->num_options; i++) {
		lua_pushfstring(L, "\n\t%s", vm->options[i].optionString);
	}
	lua_concat(L, vm->num_options + 1);
	return 1;
}

/*
 * Creates a VM.
 */
static int create_vm (lua_State *L) {
	vm_rec *vm;
	int i;
	const char *option;
	JavaVMInitArgs vm_args;
	int res;
	JNIEnv *env;
	jclass luastate_class, library_class;
	jmethodID init_id, openlib_id;
	jfieldID java_id;
	jobject luastate, java;

	/* Check for existing VM */
	lua_getfield(L, LUA_REGISTRYINDEX, JAVAVM_VM);
	if (!lua_isnil(L, -1)) {
		return luaL_error(L, "VM already created");
	}
	lua_pop(L, 1);
	
	/* Create VM */
	vm = lua_newuserdata(L, sizeof(vm_rec));
	memset(vm, 0, sizeof(vm_rec));
	luaL_getmetatable(L, JAVAVM_METATABLE);
	lua_setmetatable(L, -2);
	
	/* Process options */
	vm->num_options = lua_gettop(L) - 1;
	if (vm->num_options > JAVAVM_MAXOPTIONS) {
		return luaL_error(L, "%d options limit, got %d", JAVAVM_MAXOPTIONS, vm->num_options);
	}
	for (i = 1; i <= vm->num_options; i++) {
		option = luaL_checkstring(L, i);
		if (strcmp(option, "vfprintf") == 0
				|| strcmp(option, "exit") == 0
				|| strcmp(option, "abort") == 0) {
			return luaL_error(L, "unsupported option '%s'", option);
		}
		vm->options[i - 1].optionString = strdup(option);
		if (!vm->options[i - 1].optionString) {
			return luaL_error(L, "out of memory");
		}
	}
	
	/* Create Java VM */
	vm_args.version = JAVAVM_JNIVERSION;
	vm_args.options = vm->options;
	vm_args.nOptions = vm->num_options;
	vm_args.ignoreUnrecognized = JNI_TRUE;
	res = JNI_CreateJavaVM(&vm->vm, (void**) &env, &vm_args);
	if (res < 0) {
		return luaL_error(L, "error creating Java VM: %d", res);
	}
	
	/* Create a LuaState in the Java VM */
	if (!(luastate_class = (*env)->FindClass(env, "com/naef/jnlua/LuaState"))
			|| !(init_id = (*env)->GetMethodID(env, luastate_class, "<init>", "(J)V"))) {
		return error(L, env, "LuaState not found");
	}
	luastate = (*env)->NewObject(env, luastate_class, init_id, (jlong) (uintptr_t) L);
	if (!luastate) {
		return error(L, env, "error creating LuaState");
	}
	vm->luastate = (*env)->NewGlobalRef(env, luastate);
	if (!vm->luastate) {
		return luaL_error(L, "error referencing LuaState");
	}
	
	/* Load the Java module */
	if (!(library_class  = (*env)->FindClass(env, "com/naef/jnlua/LuaState$Library"))
			|| !(openlib_id = (*env)->GetMethodID(env, luastate_class, "openLib", "(Lcom/naef/jnlua/LuaState$Library;)V"))
			|| !(java_id = (*env)->GetStaticFieldID(env, library_class, "JAVA", "Lcom/naef/jnlua/LuaState$Library;"))
			|| !(java = (*env)->GetStaticObjectField(env, library_class, java_id))) {
		return error(L, env, "Java module not found");
	}
	(*env)->CallVoidMethod(env, luastate, openlib_id, java);
	if ((*env)->ExceptionCheck(env)) {
		return error(L, env, "error loading Java module");
	}
	
	/* Store VM */
	lua_pushvalue(L, -1);
	lua_setfield(L, LUA_REGISTRYINDEX, JAVAVM_VM);
	
	return 1;
}

/*
 * Destroys the VM.
 */
static int destroy_vm (lua_State *L) {
	/* Release VM, if any */
	lua_pushcfunction(L, release_vm);
	lua_getfield(L, LUA_REGISTRYINDEX, JAVAVM_VM);
	if (lua_isnil(L, -1)) {
		/* No VM to destroy */
		lua_pushboolean(L, 0);
		return 1;
	}
	lua_call(L, 1, 0);
	
	/* Clear VM */
	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, JAVAVM_VM);
	
	/* Success */
	lua_pushboolean(L, 1);
	return 1;
}

/*
 * Returns the VM, if any.
 */
static int get_vm (lua_State *L) {
	lua_getfield(L, LUA_REGISTRYINDEX, JAVAVM_VM);
	return 1;
}

/*
 * Java VM module functions.
 */
static const luaL_Reg functions[] = {
	{ "create", create_vm },
	{ "destroy", destroy_vm },
	{ "get", get_vm },
	{ NULL, NULL }
};

/*
 * Exported functions.
 */ 
 
LUALIB_API int luaopen_javavm (lua_State *L) {
	/* Create module */
	luaL_register(L, lua_tostring(L, -1), functions);
	
	/* Create metatable */
	luaL_newmetatable(L, JAVAVM_METATABLE);
	lua_pushcfunction(L, release_vm);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, tostring_vm);
	lua_setfield(L, -2, "__tostring");
	lua_pop(L, 1);
	
	return 1;
}
