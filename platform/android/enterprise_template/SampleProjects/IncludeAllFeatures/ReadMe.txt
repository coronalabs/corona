--- Sample Project:  IncludeAllFeatures
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
--- Summary
--------------------------------------------------------------------------------

This project demonstrates how to include all optional libraries that the Corona supports.
This enables the following features in Corona Lua APIs:
- Facebook
- Flurry
- InMobi
- inneractive

Note:
Calling a Lua API for a feature whose library is not included can cause an error or crash to occur.


--------------------------------------------------------------------------------
--- Points of Interest
--------------------------------------------------------------------------------

- You will need to get the jar and resource files from CoronaEnterprisePlugins.xxxx.xxxx.tgz file.

- Remember to update your "AndroidManifest.xml" file to include any settings that are required by the included libraries.


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
