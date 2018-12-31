README for JNLua

For information about JNLua, please visit http://code.google.com/p/jnlua/.


* Release 0.9.4

- Added a javavm module, allowing to create a Java VM from Lua.

- Added a 'new' method to interfaces in the default Java reflector. The method
accepts a table providing the methods of the interface and returns a proxy
that implements the interface.

- Refactored the error handling in the JNLua native library.


* Relesae 0.9.3 (2012-01-12)

- Corrected an issue where the native library would not properly catch Lua
errors, leading to uncontrolled transitions between Java code and native code.

- Corrected an issue where the native library would exit incorrectly from the
lua_tojavafunction function.


* Release 0.9.2 (2012-01-05)

- Made the behavior of library open methods more consistent. The openLib method
of LuaState is now properly documented to leave the opened library on the
stack; the openLibs method of LuaState now removes libraries that are pushed
onto the stack; the open method of JavaModule now leaves the Java module on
the stack.

- Corrected an issue where the type method in LuaState would not return null for
non-valid stack indexes.

- Corrected an issue where the default converter would not properly handle
undefined stack indexes.

- Corrected an issue where the setJavaReflector method in LuaState would allow
a null value to be set.


* Release 0.9.1 Beta (2010-04-05)

- Added NativeSupport for more explicit control over the native library
loading process.

- Migrated build system to Maven.
 

* Release 0.9.0 Beta (2008-10-27)
  
- Initial public release.
