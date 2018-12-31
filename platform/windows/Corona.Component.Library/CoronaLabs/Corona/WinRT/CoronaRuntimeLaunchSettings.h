// ----------------------------------------------------------------------------
// 
// CoronaRuntimeLaunchSettings.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace Corona { namespace WinRT {

/// <summary>
///  Provides customizable settings that Corona should use on startup, such as directory paths to be used in Lua
///  and the Lua file to be executed on startup.
/// </summary>
public ref class CoronaRuntimeLaunchSettings sealed
{
	public:
		/// <summary>Creates a new settings object initialized to its defaults.</summary>
		CoronaRuntimeLaunchSettings();

		/// <summary>Copies the given settings to this object.</summary>
		/// <param name="settings">
		///  <para>Reference to the settings object to copy from.</para>
		///  <para>Will be ignored if set to null.</para>
		/// </param>
		void CopyFrom(CoronaRuntimeLaunchSettings^ settings);

		/// <summary>
		///  <para>Gets or sets the path Corona will use for its resource directory.</para>
		///  <para>This is the directory that Lua property "system.ResourceDirectory" maps to.</para>
		///  <para>By default, this is set to the application's read-only ".\Assets\Corona" directory.</para>
		///  <para>
		///   The caller can change this to a writable Application Data directory, which is useful if
		///   the Corona project files were downloaded.
		///  </para>
		/// </summary>
		/// <value>
		///  <para>The path to Corona's resource directory.</para>
		///  <para>
		///   Corona will automatically create the directory if it does not exist on startup.
		///   However, if the path is to a read-only Assets directory, then the directory must already exist.
		///  </para>
		///  <para>When setting this property, it is recommended to set it to an absolute path.</para>
		///  <para>
		///   Can be set to null or empty string, which will make Corona use its default path for this directory instead.
		///  </para>
		/// </value>
		property Platform::String^ ResourceDirectoryPath;

		/// <summary>
		///  <para>Gets or sets the path Corona will use for its documents directory.</para>
		///  <para>This is the directory that Lua property "system.DocumentsDirectory" maps to.</para>
		/// </summary>
		/// <value>
		///  <para>The path to Corona's documents directory.</para>
		///  <para>Corona will automatically create the directory if it does not exist on startup.</para>
		///  <para>When setting this property, it is recommended to set it to an absolute path.</para>
		///  <para>
		///   Can be set to null or empty string, which will make Corona use its default path for this directory instead.
		///  </para>
		/// </value>
		property Platform::String^ DocumentsDirectoryPath;

		/// <summary>
		///  <para>Gets or sets the path Corona will use for its temporary directory.</para>
		///  <para>This is the directory that Lua property "system.TemporaryDirectory" maps to.</para>
		///  <para>This directory's files will be automatically deleted if the system is running low on storage space.</para>
		/// </summary>
		/// <value>
		///  <para>The path to Corona's temporary directory.</para>
		///  <para>Corona will automatically create the directory if it does not exist on startup.</para>
		///  <para>When setting this property, it is recommended to set it to an absolute path.</para>
		///  <para>
		///   Can be set to null or empty string, which will make Corona use its default path for this directory instead.
		///  </para>
		/// </value>
		property Platform::String^ TemporaryDirectoryPath;

		/// <summary>
		///  <para>Gets or sets the path Corona will use for its caches directory.</para>
		///  <para>This is the directory that Lua property "system.CachesDirectory" maps to.</para>
		///  <para>This directory's files will be automatically deleted if the system is running low on storage space.</para>
		/// </summary>
		/// <value>
		///  <para>The path to Corona's caches directory.</para>
		///  <para>Corona will automatically create the directory if it does not exist on startup.</para>
		///  <para>When setting this property, it is recommended to set it to an absolute path.</para>
		///  <para>
		///   Can be set to null or empty string, which will make Corona use its default path for this directory instead.
		///  </para>
		/// </value>
		property Platform::String^ CachesDirectoryPath;

		/// <summary>
		///  <para>Gets or sets the directory path Corona will use for its internal features, such as analytics.</para>
		///  <para>This directory is not made available in Lua.</para>
		/// </summary>
		/// <value>
		///  <para>Directory path used by Corona to store files for its own internal usage, such as analytics.</para>
		///  <para>Corona will automatically create the directory if it does not exist on startup.</para>
		///  <para>When setting this property, it is recommended to set it to an absolute path.</para>
		///  <para>
		///   Can be set to null or empty string, which will make Corona use its default path for this directory instead.
		///  </para>
		/// </value>
		property Platform::String^ InternalDirectoryPath;

		/// <summary>
		///  <para>Gets or set a path to the file that the Corona runtime will begin execution on.</para>
		///  <para>Expected to be set to the path to a "main.lua" or "resource.car" file.</para>
		/// </summary>
		/// <value>
		///  <para>
		///   Path to a *.lua or *.car file that the Corona runtime will execute upon startup,
		///   such as a path to a "main.lua" or "resource.car" file
		///  </para>
		///  <para>
		///   Can be set to a file that is not in the "Resource" directory, which is useful if you plan on
		///   executing a Lua script that was downloaded form the Internet.
		///  </para>
		///  <para>
		///   Setting this path to null or empty string causes Corona to attempt to load a "main.lua" or "resource.car"
		///   file from the root of the "Resource" directory on startup.
		///  </para>
		/// </value>
		property Platform::String^ LaunchFilePath;
};

} } }	// namespace CoronaLabs::Corona::WinRT
