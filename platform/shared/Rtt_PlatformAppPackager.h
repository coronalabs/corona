//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformAppPackager_H__
#define _Rtt_PlatformAppPackager_H__

#include "Core/Rtt_Array.h"
#include "Core/Rtt_String.h"
#include "Rtt_TargetDevice.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class DeviceBuildData;
class LuaContext;
class MPlatform;
class MPlatformServices;
#if !defined( Rtt_NO_GUI )
	class Runtime;
#endif

// ----------------------------------------------------------------------------

class AppPackagerParams
{
	private:
		String fAppName;
		String fVersion;
		String fIdentity;
		String fProvisionFile;
		String fSrcDir;
		String fDstDir;
		String fSdkRoot;
		String fCustomBuildId;
		String fProductId;
		String fAppPackage;
		String fCertType;
		String fTargetAppStoreName;
		String fBuildMessage;
		String fCustomTemplate;
		TargetDevice::Platform fTargetPlatform;
		S32 fTargetVersion;
		S32 fTargetDevice;
		bool fIsStripDebug;
		String fBuildSettingsPath;
		bool fIncludeBuildSettings;
		mutable DeviceBuildData *fDeviceBuildData;
		bool fIncludeFusePlugins;
		bool fUsesMonetization;
		bool fLiveBuild;
        bool fIncludeStandardResources = true;
		String fCoronaUser;

	public:
		AppPackagerParams( const char* appName,
			const char* version,
			const char* identity,
			const char* provisionFile,
			const char* srcDir,
			const char* dstDir,
			const char* sdkRoot,
			TargetDevice::Platform targetPlatform,
			const char * targetAppStoreName,
			S32 targetVersion,
			S32 targetDevice,
		    const char * customBuildId,
			const char * productId,
			const char * appPackage,
			bool isDistributionCert );
		virtual ~AppPackagerParams();
	
		const char * GetAppName() const { return fAppName.GetString(); }
		const char * GetVersion() const { return fVersion.GetString(); }
		const char * GetIdentity() const { return fIdentity.GetString(); }
		const char * GetProvisionFile() const { return fProvisionFile.GetString(); }
		const char * GetSrcDir() const { return fSrcDir.GetString(); }
		const char * GetDstDir() const { return fDstDir.GetString(); }
		const char * GetSdkRoot() const { return fSdkRoot.GetString(); }
		const char * GetTargetAppStoreName() const { return fTargetAppStoreName.GetString(); }
		void SetCustomTemplate( const char * newValue ) { fCustomTemplate.Set( newValue ); }
		const char * GetCustomTemplate() const { return fCustomTemplate.IsEmpty() ? "" : fCustomTemplate.GetString(); }
		TargetDevice::Platform GetTargetPlatform() const { return fTargetPlatform; }
		S32 GetTargetVersion() const { return fTargetVersion; }
		S32 GetTargetDevice() const { return fTargetDevice; }
		const char * GetCustomBuildId() const { return (fCustomBuildId.IsEmpty() ? NULL : fCustomBuildId.GetString()); }
		const char * GetProductId() const { return fProductId.GetString(); }
		const char * GetAppPackage() const { return fAppPackage.GetString(); }
		const char * GetCertType() const { return fCertType.GetString(); }
		bool IsStripDebug() const { return fIsStripDebug; }
		void SetStripDebug( bool newValue ) { fIsStripDebug = newValue; }
		void SetBuildMessage( const char * newValue ) { fBuildMessage.Set( newValue ); }
		const char * GetBuildMessage( ) { return fBuildMessage.GetString(); }
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
		bool UsesMonetization() const { return fUsesMonetization; }
		void SetUsesMonetization( bool newValue ) { fUsesMonetization = newValue; }
		bool IncludeFusePlugins() const { return fIncludeFusePlugins; }
		void SetIncludeFusePlugins( bool newValue ) { fIncludeFusePlugins = newValue; }
#endif

		bool IsLiveBuild() const { return fLiveBuild; }
		void SetLiveBuild( bool newValue ) { fLiveBuild = newValue; }
  
        bool IncludeStandardResources() const { return fIncludeStandardResources; }
		void SetIncludeStandardResources( bool newValue ) { fIncludeStandardResources = newValue; }

		const char * GetCoronaUser() const { return fCoronaUser.GetString(); }
		void SetCoronaUser(const char* user) { fCoronaUser.Set(user); }

	public:
		void SetBuildSettingsPath( const char *path ) { fBuildSettingsPath.Set( path ); }
		const char *GetBuildSettingsPath() { return fBuildSettingsPath.GetString(); }
		void SetIncludeBuildSettings( bool value ) { fIncludeBuildSettings = value; }
		bool IncludeBuildSettings() const { return fIncludeBuildSettings; }
		DeviceBuildData& GetDeviceBuildData( const MPlatform& platform, const MPlatformServices& services ) const;

	public:
		virtual void Print();

	public:
		// Optimize productId based on module dependency
		void InitializeProductId( U32 modules );
};

class PlatformAppPackager
{
	public:
	enum {
		kNoError = 0,
		kBuildError,
		kLocalPackagingError,
	};
	public:
		PlatformAppPackager( const MPlatformServices& services, TargetDevice::Platform targetPlatform );
		virtual ~PlatformAppPackager();

	public:
		const MPlatformServices& GetServices() const { return fServices; }

	public:
		static bool mkdir( const char *sDir );
		static bool rmdir( const char *sDir );

		// TODO: caller should make dstDir a unique directory
		virtual int Build( AppPackagerParams * params, const char* tmpDirBase );

		virtual bool VerifyConfiguration() const;

        // Looks in build.settings, Preferences/Registry, AppSettings.lua
		const char * GetCustomBuildId() const;

		const char * GetErrorMesg() { return fErrorMesg.GetString(); }

		/**
		 * @return True if the build.settings read or if the file doesn't exist. False if there is an error (e.g. syntax, runtime) in the file.
		 */
		virtual bool ReadBuildSettings( const char * srcDir );

		static const char * EscapeStringForIOS( const char *input );
		static void EscapeStringForAndroid( const char *sourceString, String &targetString );
		static void EscapeFileName( const char *sourceString, String &targetString, bool unicodeAllowed = false );

#if !defined( Rtt_NO_GUI )
		/**
		 * Determines if the system has acquired all plugins required for the given Corona runtime's project.
		 * @param runtime Pointer to the Corona runtime to be queried.
		 * @param missingPluginsString Pointer to a string used by this function to copy a newline separated list of
		 *                             plugin names that were missing.
		 *                             Only applicable if this function returns false.
		 * @return Returns true if all plugins have been downloaded/found or if the runtime does not depend on any plugins.
		 *
		 *         Returns false if not all plugins have been downloaded/found or if given an invalid argument.
		 */
		static bool AreAllPluginsAvailable( Runtime *runtime, String *missingPluginsString = NULL );
#endif

	protected:
		virtual char* Prepackage( AppPackagerParams * params, const char* tmpDir );
		bool CompileScripts( AppPackagerParams * params, const char* tmpDir );

		/**
		 * Archives all files in a given directory tree to a "resource.car" file.
		 * The files to be archived are expected to be compiled *.lu files, but will archive any file under the directory.
		 * @param params Pointer to the build's package parameters. Cannot be null.
		 * @param sourceDirectoryPath Path to the directory containing the files to be archived.
		 *                            Will archive all files under subdirectories too.
		 * @param destinationFilePath The path and file name of the archive to be created.
		 *                            Expected to be named "<DirectoryPath>/resource.car".
		 * @return Returns true if successfully archived at least 1 file and the archive file was created.
		 *
		 *         Returns false if given invalid parameters, unable to find any source files, or if unable to
		 *         create the archive file at the destination path.
		 */
		bool ArchiveDirectoryTree(
				AppPackagerParams* params, const char* sourceDirectoryPath, const char* destinationFilePath );

		/** Settings info needed by the CopyDirectoryTree() function. */
		struct CopyDirectoryTreeSettings
		{
			/** Pointer to the build's package parameters. Cannot be null. */
			AppPackagerParams* ParamsPointer;

			/**
			 * Path to the directory containing the files to be copied. Cannot be null.
			 * Does not copy the base directory to the destination.
			 */
			const char* SourceDirectoryPath;

			/**
			 * The directory to copy the files to. Cannot be null.
			 * Will create the directory if it does not already exist.
			 */
			const char* DestinationDirectoryPath;

			/**
			 * Optional array of wildcard '*' file pattern matches to exclude from the copy.
			 * Typically set with the patterns configured in the "build.settings" file's "excludeFiles" table.
			 * Can be null, in which case, all files will be copied to the destination.
			 */
			const LightPtrArray<const char>* ExcludeFilePatternArray;
		};

		/**
		 * Copies all files in from a source directory and its subdirectories to a destination directory.
		 * @param settings Provides the source directory, destination directory, and optional file exclusion patterns.
		 * @return Returns true if all files were copied to the destination successfully.
		 *
		 *         Returns false if at least 1 file copy failed or if given invalid settings.
		 */
		bool CopyDirectoryTree( const CopyDirectoryTreeSettings &settings );

#if !defined( Rtt_NO_GUI )
		/**
		 * Unzips the given Corona runtime's downloaded plugins to the destination path.
		 * @param params Pointer to the build's package parameters. Cannot be null.
		 *
		 *               If an error occurs, then this object's GetBuildMessage() will provide a reason why.
		 * @param runtime Pointer to a Corona runtime whose plugins are to be unzipped.
		 * @param destinationDirectoryPath Path to the directory the plugins will be unzipped to. Cannot be null or empty.
		 *                                 The directory will be created by this method if it doesn't already exist.
		 * @return Returns true if all plugins have been unzipped to the destination directory
		 *         or if the runtime has no plugins to unzip.
		 *
		 *         Returns false if failed to unzip all plugins of if given invalid arguments.
		 *         The "params" argument's GetBuildMessage() will provide a reason why it failed.
		 */
		bool UnzipPlugins( AppPackagerParams *params, Runtime *runtime, const char *destinationDirectoryPath );
#endif

		int OpenBuildSettings( const char * srcDir );
		virtual void OnReadingBuildSettings( lua_State *L, int index );
        void ReadGlobalCustomBuildId();
        // TODO: possibly move this to another class
        void ReadAppSetting( const char *setting, String *result );
		void SetSplashImageFile( const char * newValue ) { fSplashImageFile.Set( newValue ); }
		const char * GetSplashImageFile( ) { return fSplashImageFile.GetString(); }

	public:
		static bool IsAppSettingsEmpty( const MPlatform& platform );

	protected:
		const MPlatformServices& fServices;
		lua_State *fVM;

		String fCustomBuildId;
        String fGlobalCustomBuildId;
        String fAppSettingsCustomBuildId;
		String fErrorMesg;
		String fSplashImageFile;
        bool fNeverStripDebugInfo;
		TargetDevice::Platform fTargetPlatform;
};

Rtt_EXPORT int Rtt_LuaCompile( lua_State *L, int numSources, const char** sources, const char* dstFile, int stripDebug );
	
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformAppPackager_H__
