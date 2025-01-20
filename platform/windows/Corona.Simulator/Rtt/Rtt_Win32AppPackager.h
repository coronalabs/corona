//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Build.h"
#include "Rtt_PlatformAppPackager.h"
#include <string>
#include <vector>


namespace Rtt
{

class Win32AppPackagerParams;

/// <summary>Class used to do Corona Win32 desktop app builds.</summary>
class Win32AppPackager : public PlatformAppPackager
{
	Rtt_CLASS_NO_COPIES(Win32AppPackager)

	public:
		/// <summary>Creates a new Win32 desktop app builder object.</summary>
		/// <param name="services">Platform services object needed to perform the build.</param>
		Win32AppPackager(const MPlatformServices& services);

		/// <summary>Destroys this object's allocated resources.</summary>
		virtual ~Win32AppPackager();

		/// <summary>Builds a Corona Win32 desktop app using the given configuration settings.</summary>
		/// <param name="paramsPointer">
		///  <para>Provides the packaging parameters needed to perform the build.</para>
		///  <para>Must be of type Win32AppPackageParams or else the build will fail.</para>
		///  <para>Cannot be null or else the build will fail.</para>
		/// </param>
		/// <param name="webSession">
		///  Session object needed to authorize and perform the build via the Corona Labs' web servers.
		/// </param>
		/// <param name="tempDirectoryPath">
		///  <para>Absolute path to a directory for the build system to use to keep temporary/intermediate build files.</para>
		///  <para>This directory will be created and deleted by this Build() method.</para>
		///  <para>Path cannot be null or empty or else the build will fail.</para>
		/// </param>
		/// <returns>
		///  <para>Returns 0 if the build succeeded.</para>
		///  <para>
		///   Returns an error code from the Rtt::WebServiceSession enum if the build failed. The given "paramPointer"
		///   argument's GetBuildMessage() will provide the reason why the build failed in this case.
		///  </para>
		/// </returns>
		virtual int Build(AppPackagerParams* paramsPointer, const char* tempDirectoryPath);

	protected:
		/// <summary>Internal build settings required by this class' DoLocalBuild() and DoRemoteBuild() methods.</summary>
		struct BuildSettings
		{
			Win32AppPackagerParams* ParamsPointer;
			const char* IntermediateDirectoryPath;
			const char* BinDirectoryPath;
		};

		/// <summary>
		///  Performs a local Win32 desktop app build using the app template installed with the Corona Simulator.
		/// </summary>
		/// <param name="buildSettings">Settings required to perform the build.</param>
		/// <returns>
		///  <para>Returns 0 if the build succeeded.</para>
		///  <para>
		///   Returns an error code from the Rtt::WebServiceSession enum if the build failed. The given "paramPointer"
		///   argument's GetBuildMessage() will provide the reason why the build failed in this case.
		///  </para>
		/// </returns>
		int DoLocalBuild(const BuildSettings& buildSettings);

		/// <summary>
		///  <para>Performs a remote build via Corona Labs' build servers.</para>
		///  <para>
		///   WARNING: This method is not currently supported and will trigger a build failure and an assert if used.
		///  </para>
		/// </summary>
		/// <param name="buildSettings">Settings required to perform the build.</param>
		/// <param name="webSession">Web session needed to perform the remote build.</param>
		/// <returns>
		///  <para>Returns 0 if the build succeeded.</para>
		///  <para>
		///   Returns an error code from the Rtt::WebServiceSession enum if the build failed. The given "paramPointer"
		///   argument's GetBuildMessage() will provide the reason why the build failed in this case.
		///  </para>
		/// </returns>
		int DoRemoteBuild(const BuildSettings& buildSettings);

		/// <summary>
		///  <para>Called while the base class is reading the "build.settings" file via the ReadBuildSettings() method.</para>
		///  <para>This is this class' opportunity to read other settings from the file.</para>
		/// </summary>
		/// <param name="luaStatePointer">Pointer to the Lua state that has loaded the build settings table.</param>
		/// <param name="index">The index to the "settings" table in the Lua stack.</param>
		virtual void OnReadingBuildSettings(lua_State *luaStatePointer, int index);

		/// <summary>Unzips the given zip file to the given destination directory.</summary>
		/// <param name="zipFilePath">Path to the zip file to be unzipped.</param>
		/// <param name="destinationPath">Path to the destination directory to unzip the file's contents to.</param>
		/// <returns>
		///  <para>Returns true if the zip file was unzipped to the destination successfully.</para>
		///  <para>Returns false if the unzip failed or if given invalid arguments.</para>
		/// </returns>
		bool UnzipFile(const wchar_t* zipFilePath, const wchar_t* destinationPath);

		/// <summary>Determines if the given directory or file paths are on the volume/drive.</summary>
		/// <param name="path1">The first path to be analyzed. Can be relative or absolute.</param>
		/// <param name="path2">The second path to be analyzed. Can be relative or absolute.</param>
		/// <returns>
		///  <para>Returns true if both paths reference the same volume/drive.</para>
		///  <para>Returns false if they reference different volumes or if given null/empty strings.</para>
		/// </returns>
		bool ArePathsOnSameVolume(const wchar_t* path1, const wchar_t* path2);

	private:
		/// <summary>
		///  <para>Stores a collection of "excludeFiles" patterns read from the "build.settings" file.</para>
		///  <para>Used by the Build() function to exclude Corona project files from the built Win32 app.</para>
		/// </summary>
		std::vector<std::string> fExcludeFilePatterns;
};

} // namespace Rtt
