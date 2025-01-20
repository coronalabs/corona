//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_AndroidSupportTools_H
#define Rtt_AndroidSupportTools_H


#ifdef __cplusplus
extern "C"
{
#endif
	#include "lua.h"
#ifdef __cplusplus
}
#endif

// Compile the below function in C unless were compiling in Visual Studio.
// Visual Studio 2008 does not support the "bool" type in C, so it must compile as C++ instead.
#ifdef __cplusplus
#	if !defined(_MSC_VER) || (_MSC_VER >= 1800)
		extern "C" {
#	endif
#endif


#if !defined(_MSC_VER) || (_MSC_VER >= 1800)
#	include <stdbool.h>
#endif


/**
 * Creates a new Lua state ready to use with the support functions listed in this file.
 * This Lua state provides functionality related to supporting Android.
 * The Lua state currently includes the standard library, LPeg, and LuaFileSystem.
 * @param filepath The path (presumably absolute path) and file of the AndroidValidation.lua script.
 * So an example is "/Applications/Corona Simulator.app/Contents/Resources/AndroidValidation.lua".
 * This is required so platforms can provide the proper path to the bundled Lua file.
 * @return Returns the initialized lua_State.
 * @see Rtt_AndroidSupportTools_CloseLuaState
 */
lua_State* Rtt_AndroidSupportTools_NewLuaState(const char* filepath);

/**
 * Closes the lua_State created by Rtt_AndroidSupportTools_NewLuaState.
 * Essentially a call to lua_close(), but this API is provided for cleaner design.
 * @param L The lua_State
 * @see Rtt_AndroidSupportTools_NewLuaState
 */
void Rtt_AndroidSupportTools_CloseLuaState(lua_State* L);

/**
 * Checks if a provided string is a valid Android package name.
 * Android (mostly) follows Java rules for package names. 
 * There are multiple places in Android (such as package identifiers and file names) where names must conform to these rules.
 * 
 * @param L An initialized lua_State.
 * @param packagename The package name to test. Note that this function should also work for file names (without paths).
 * @return True if a valid package name, false otherwise. The Lua stack will remain balanced.
 */
bool Rtt_AndroidSupportTools_IsAndroidPackageName(lua_State* L, const char* packagename);

/**
 * OBSOLETE: This now always returns true.
 * Checks Res files to make sure they don't conflict with Java reserved words.
 * Android does weird things and makes Java classes out of files. This will break
 * things if a filename is a Java reserved word.
 * @param L An initialized lua_State.
 * @param rootdir The path to the directory of files you want to validate. The algorithm traverses subdirectories.
 * @return Returns true if all the files are valid, false if an error. The Lua stack is not cleared and will contain a list of problem files.
 * @note This function always leaves 2 elements on the stack, the boolean return and a table containing a list of problem files.
 */
bool Rtt_AndroidSupportTools_ValidateFilesForJavaKeywordsInDirectory(lua_State* L, const char* rootdir);

/**
 * OBSOLETE: This now always returns true.
 * Checks Res files to make sure they don't have duplicated basenames.
 * Android does weird things and makes Java classes out of files. This will break
 * things if multiple files have the same basename (defined as up to the first period, not the last period).
 * @param L An initialized lua_State.
 * @param rootdir The path to the directory of files you want to validate. The algorithm traverses subdirectories.
 * @return Returns true if all the files are valid, false if an error. The Lua stack is not cleared and will contain a list of problem files.
 * @note This function always leaves 2 elements on the stack, the boolean return and a table containing a list of problem files.
 */
bool Rtt_AndroidSupportTools_ValidateFilesForDuplicateBaseNamesInDirectory(lua_State* L, const char* rootdir);

/**
 * OBSOLETE: This now always returns true.
 * Checks Res files to make sure they don't exist in subdirectories.
 * Android does weird things and makes Java classes out of files. This will break
 * things if they are in subdirectories.
 * @param L An initialized lua_State.
 * @param rootdir The path to the directory of files you want to validate. The algorithm traverses subdirectories.
 * @return Returns true if all the files are valid, false if an error. The Lua stack is not cleared and will contain a list of problem files.
 * @note This function always leaves 2 elements on the stack, the boolean return and a table containing a list of problem files.
 */
bool Rtt_AndroidSupportTools_ValidateResFilesForForbiddenSubdirectories(lua_State* L, const char* rootdir);
	
/**
 * OBSOLETE: This now always returns true.
 * Checks Lua files to make sure they don't exist in subdirectories.
 * We currently don't support Lua files in subdirectories so we need to check for it.
 * This is true for Android and iOS.
 * @param L An initialized lua_State.
 * @param rootdir The path to the directory of files you want to validate. The algorithm traverses subdirectories.
 * @return Returns true if all the files are valid, false if an error. The Lua stack is not cleared and will contain a list of problem files.
 * @note This function always leaves 2 elements on the stack, the boolean return and a table containing a list of problem files.
 */
bool Rtt_CommonSupportTools_ValidateLuaFilesForForbiddenSubdirectories(lua_State* L, const char* rootdir);

			
/**
 * Checks if a provided string is some subset of ASCII that we know survives our server build process.
 * Current character range is  [A-Za-z0-9_' ]. 
 * This is only temporary until we fix the real problem. (Indications suggest we need to fix stuff on the server.)
 * 
 * @param L An initialized lua_State.
 * @param name The string to test. N
 * @return True if a valid package name, false otherwise. The Lua stack will remain balanced.
 */
bool Rtt_CommonSupportTools_ValidateNameForRestrictedASCIICharacters(lua_State* L, const char* name);

#ifdef __cplusplus
#	if !defined(_MSC_VER) || (_MSC_VER >= 1800)
		}
#	endif
#endif

#endif /* end Rtt_AndroidSupportTools_H */
