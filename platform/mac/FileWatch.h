//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _FileWatch_H_
#define _FileWatch_H_

#ifdef __cplusplus
extern "C" {
#endif

FSEventStreamRef FileWatch_StartMonitoringFolder(NSString* folderPath,
												 FSEventStreamCallback callbackFunction,
												 FSEventStreamContext* callbackUserData);

void FileWatch_StopMonitoringFolder(FSEventStreamRef eventStream);

void FolderChangeNotificationDelegate_LuaFolderChangedCallbackFunction(ConstFSEventStreamRef eventStream,
																	   void *clientCallBackInfo,
																	   size_t numEvents,
																	   void *eventPaths,
																	   const FSEventStreamEventFlags eventFlags[],
																	   const FSEventStreamEventId eventIds[]);

#ifdef __cplusplus
}
#endif

#endif //_FileWatch_H_