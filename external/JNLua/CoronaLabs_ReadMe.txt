
The JNLua source code was modified by Corona Labs to do the following:

1) Fixed Random Crash
Source file "src/main/c/jnlua.c" was modified to fix a random crasher on Android caused by the LuaState Java object it was trying to access not being pinned in memory.  The original implementation was only holding a "weak global reference" to that object by calling NewWeakGlobalRef() on that object, but Google's documentation states that you must call NewGlobalRef() on that object to pin it in memory.

2) Modified LuaState Java Class to Accept Existing lua_State
The original LuaState Java code (src/main/java/com/naef/jnlua/LuaState.java) did not allow you to pass in an existing lua_State that was created in C/C++.  A LuaState constructor was modified from private to public to allow passing in the memory address of an existing lua_State struct that was created by the Corona Runtime on the C/C++ side.  The rest of the JNLua code was already designed to handle a lua_State that it does not own so that it won't ever call lua_close() on it.

3) Removed Java Bean Support
Source file "src/main/java/com/naef/jnlua/DefaultJavaReflector.java" was modified to remove Java Bean support since Android does not support it.  This would have caused an exception when attempting to create a LuaState Java object.  The Java Bean code was commented out.

4) Added New Build Scripts
The following files were added to the root JNLua directory by Corona Labs:
- build.xml
- build.sh
Running the "build.sh" shell script will compile the JNLua source code and create a "JNLua.jar" in the bin directory.  Requires the JDK (Java Development Kit) and Ant (included with the JDK) to work.

5) Added Byte Array Support
Added ability to push a Java byte array as a Lua string and to retrieve a Lua string as a Java byte array.  Added new functions to source files "src/main/c/jnlua.c" and "src/main/java/com/naef/jnlua/LuaState.java" that are similar to the official JNLua 0.9.5 version, except Corona Labs' implementation passes a byte array via LuaState.pushString(byte[]) instead, which makes it more obvious how it works. Also, this implementation allows byte arrays to passed as Java objects without automatic conversion to Lua string.
