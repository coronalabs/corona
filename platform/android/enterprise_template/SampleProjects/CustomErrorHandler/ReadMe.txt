--- Sample Project:  CustomErrorHandler
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--- Summary
--------------------------------------------------------------------------------

This project demonstrates how to capture Lua errors and uncaught Java exception on the Corona runtime thread.


--------------------------------------------------------------------------------
--- Points of Interest
--------------------------------------------------------------------------------

- The Android "Application" class is overridden in this application to set the custom Lua error handler and to set the default uncaught exception handler.  See the "CoronaApplication.java" file.

- Note that you can only set an uncaught exception handler to one particular thread at a time.  To catch uncaught exceptions from the Corona runtime's thread, which is the same thread that Lua runs in, then you should set it in the CoronaRuntimeListener's onLoaded() method which is called on the Corona runtime's thread.

- See the "LuaErrorHandlerFunction.java" file on how to code a custom Lua error handler and how to retrieve a Lua stack trace.

- See the "UncaughtExceptionHandler.java" file on how to code an uncaught exception handler.


--------------------------------------------------------------------------------
--- Build Requirements
--------------------------------------------------------------------------------

- Java Development Kit 6  (a.k.a.: JDK 6)
- Android SDK revision 14 or higher, supporting API Level 15 or higher.
  * Note that Corona supports at least API Level 8.
  * API Level 15 is recommended to avoid compatibility mode on newest operating systems.


--------------------------------------------------------------------------------
--- How to Build
--------------------------------------------------------------------------------

See script file "build.sh" on how to build this sample project via the Apache Ant command line utility.
