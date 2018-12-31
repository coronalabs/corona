//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ApplePlatform_H__
#define _Rtt_ApplePlatform_H__

#include "Core/Rtt_Types.h"
#include "Rtt_MPlatform.h"

#include "Rtt_AppleCrypto.h"

#include <objc/objc.h>

#include <map>
#include <string>

// ----------------------------------------------------------------------------

@class NSData;
@class NSDictionary;
@class NSMutableDictionary;
@class NSString;
@class NSArray;
struct ALmixer_Data;

// ----------------------------------------------------------------------------

namespace Rtt
{

struct KeyValuePair;

class AppleAudioPlayer;
class RenderingStream;

// ----------------------------------------------------------------------------

class ApplePlatform : public MPlatform
{
	Rtt_CLASS_NO_COPIES( ApplePlatform )

	public:
		typedef ApplePlatform Self;

	// TODO: Move these to a LuaAppleBridge utils section
	public:
		// Converts value at 'index' to corresponding Obj-C value
		static id ToValue( lua_State *L, int index );

		// Creates a dictionary from the Lua table at index 't'
		// TODO: Rename (it returns an autoreleased object which contradicts the 'Create' naming convention)
		static NSMutableDictionary* CreateDictionary( lua_State *L, int t );

		// Copies from NSDictionary into Lua table at 'index'
		static void CopyDictionary( lua_State *L, int index, NSDictionary *src );

		// Creates a Lua table at top of stack and copies in entries from 'src'
		static int CreateAndPushTable( lua_State *L, NSDictionary *src );

		static bool CreateAndPushArray( lua_State *L, NSArray *array );

		// Pushes the 'value' on the Lua stack. Returns false if cannot map 'value' to a Lua type.
		static bool Push( lua_State *L, id value );

	public:
		ApplePlatform();
		virtual ~ApplePlatform();

	public:
		void SetResourceDirectory( NSString *resourcePath );
        virtual void SetProjectResourceDirectory( NSString *projectResourcePath );
        virtual void SetSkinResourceDirectory( NSString *skinResourcePath );

	protected:
		NSString *GetResourceDirectory() const { return fResourcePath; }

	// MPlatform
	public:
		virtual Rtt_Allocator& GetAllocator() const;
		virtual RenderingStream* CreateRenderingStream( bool antialias ) const;
		virtual PlatformSurface* CreateOffscreenSurface( const PlatformSurface& parentDevice ) const;
		virtual PlatformTimer* CreateTimerWithCallback( MCallback& callback ) const;
		virtual PlatformBitmap* CreateBitmap( const char *filePath, bool convertToGrayscale ) const;
		virtual PlatformBitmap* CreateBitmapMask( const char str[], const PlatformFont& font, Real w, Real h, const char alignment[], Real& baselineOffset ) const;
		virtual FontMetricsMap GetFontMetrics( const PlatformFont& font ) const;
		virtual const MCrypto& GetCrypto() const;

		virtual void GetPreference( Category category, Rtt::String * value ) const;
		virtual Preference::ReadValueResult GetPreference( const char* categoryName, const char* keyName ) const;
		virtual OperationResult SetPreferences( const char* categoryName, const PreferenceCollection& collection ) const;
		virtual OperationResult DeletePreferences( const char* categoryName, const char** keyNameArray, U32 keyNameCount ) const;

		virtual void HttpPost( const char* url, const char* key, const char* value ) const;
		virtual bool HttpDownload( const char* url, String& result, String& errorMesg, const std::map<std::string, std::string>& headers ) const;
		virtual bool HttpDownloadFile( const char* url, const char *filename, String& errorMesgOut, const std::map<std::string, std::string>& headers ) const;


		virtual PlatformEventSound * CreateEventSound( const ResourceHandle<lua_State> &handle, const char* filePath ) const;
		virtual void ReleaseEventSound( PlatformEventSound * soundID ) const;
		virtual void PlayEventSound( PlatformEventSound * soundID ) const;

		virtual PlatformAudioRecorder * CreateAudioRecorder( const ResourceHandle<lua_State> & handle, const char * filePath ) const;

		virtual PlatformAudioPlayer* GetAudioPlayer( const ResourceHandle<lua_State> & handle ) const;

		virtual int GetStatusBarHeight() const;
        virtual int GetTopStatusBarHeightPixels()  const;
        virtual int GetBottomStatusBarHeightPixels() const;
    

		virtual Real GetStandardFontSize() const;
		virtual PlatformFont* CreateFont( PlatformFont::SystemFont fontType, Rtt_Real size ) const;
		virtual PlatformFont* CreateFont( const char *fontName, Rtt_Real size ) const;

	public:
		virtual bool CanShowPopup( const char *name ) const;
		virtual bool ShowPopup( lua_State *L, const char *name, int optionsIndex ) const;
		virtual bool HidePopup( const char *name ) const;

	public:
		virtual void* CreateAndScheduleNotification( lua_State *L, int index ) const;
		virtual void ReleaseNotification( void *notificationId ) const;
		virtual void CancelNotification( void *notificationId ) const;

	public:
		virtual void FlurryInit( const char * applicationKey ) const;
		virtual void FlurryEvent( const char * eventId ) const;

	public:
		virtual void SetNativeProperty( lua_State *L, const char *key, int valueIndex ) const;
		virtual int PushNativeProperty( lua_State *L, const char *key ) const;
		virtual int PushSystemInfo( lua_State *L, const char *key ) const;

	private:
		void NetworkBaseRequest( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex, NSString *path ) const;

	public:
		void HttpRequest( NSString *url, NSString *method, NSDictionary *params,
			NSString *body, void (^block)( NSData* ) ) const;
		virtual void NetworkRequest( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex ) const;
		virtual void NetworkDownload( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex, const char *filename, MPlatform::Directory baseDir ) const;

		virtual PlatformReachability* NewReachability( const ResourceHandle<lua_State>& handle, PlatformReachability::PlatformReachabilityType type, const char* address ) const;
		virtual bool SupportsNetworkStatus() const;
	
	public:
		virtual void RaiseError( MPlatform::Error e, const char* reason ) const;
		virtual void PathForFile( const char* filename, Directory baseDir, U32 flags, String & result ) const;
		virtual bool FileExists( const char * filename ) const;
		virtual bool ValidateAssetFile(const char *assetFilename, const int assetSize) const;

	protected:
		NSString* PathForDir( const char *folderName, NSString *baseDir, bool createIfNotExist ) const;


	protected:
		virtual NSString *PathForResourceFile( const char *filename ) const;
		NSString *PathForSystemResourceFile( const char *filename ) const;
        NSString *PathForProjectResourceFile( const char* filename ) const;
        NSString *PathForSkinResourceFile( const char* filename ) const;
        void SetProjectResourceDirectory( const char* filename );
        void SetSkinResourceDirectory( const char* filename );
		NSString *PathForCoronaResourceFile( const char* filename ) const;
		virtual NSString *PathForDocumentsFile( const char *filename ) const;
		virtual NSString *PathForApplicationSupportFile( const char* filename ) const;
		virtual NSString *CachesParentDir() const;
		NSString *PathForCachesFile( const char *filename ) const;
		NSString *PathForSystemCachesFile( const char *filename ) const;
		virtual NSString *PathForTmpFile( const char *filename ) const;
		virtual NSString *PathForPluginsFile( const char *filename ) const;
		virtual NSString* PathForFile( const char* filename, NSString* baseDir ) const;

	public:
		virtual int SetSync( lua_State* L ) const;
		virtual int GetSync( lua_State* L ) const;
	
	protected:
		bool SetSynciCloud( NSString* filepath, bool value, NSString** errorstr ) const;
		bool GetSynciCloud( NSString* filepath, bool& value, NSString** errorstr ) const;

	public:
		virtual void BeginRuntime( const Runtime& runtime ) const;
		virtual void EndRuntime( const Runtime& runtime ) const;

		virtual PlatformExitCallback* GetExitCallback();
		virtual bool RequestSystem( lua_State *L, const char *actionName, int optionsIndex ) const;

		virtual void Suspend() const;
		virtual void Resume() const;

	public:
		void SetBundleClass( Class newValue );
#ifdef Rtt_AUTHORING_SIMULATOR
        virtual void SetCursorForRect(const char *cursorName, int x, int y, int width, int height) const;
#endif

	protected:
		Rtt_Allocator* fAllocator;
		NSString *fResourcePath;
        NSString *fProjectResourcePath;
        NSString *fSkinResourcePath;
		mutable AppleAudioPlayer* fAudioPlayer;
		mutable id fHttpPostDelegate;
		mutable id fCustomConnectionDelegate;
		AppleCrypto fCrypto;
		Class fClass;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ApplePlatform_H__
