
RC Edit

Version:
	0.1.0

Downloaded From:
   https://github.com/atom/rcedit

Summary:
Open source code used to change the resources and icon of a precompiled Windows EXE file.
Used by the Windows Corona Simulator when doing local Win32 app builds.

The source code was modified by Corona Labs for the following reasons:
- Fixed to support unicode paths. Especially with the Load() function.
- Modified SetIcon() to "replace" the first icon found in the module instead of injecting a new one with a resource ID of 1.
