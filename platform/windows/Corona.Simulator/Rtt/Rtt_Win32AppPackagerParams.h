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

#pragma once

#include "Core\Rtt_Build.h"
#include "Core\Rtt_String.h"
#include "Rtt_PlatformAppPackager.h"


namespace Rtt
{

class Runtime;

/// <summary>Settings needed to do a Win32 desktop app build via the Win32AppPackager class.</summary>
class Win32AppPackagerParams : public AppPackagerParams
{
	public:
		/// <summary>
		///  <para>Settings to be provided to the Win32AppPackagerParams constructor.</para>
		///  <para>The Win32AppPackagerParams class has no setters for these parameters.</para>
		/// </summary>
		struct CoreSettings
		{
			const char* AppName;
			const char* VersionString;
			const char* SourceDirectoryPath;
			const char* DestinationDirectoryPath;
		};

		/// <summary>Creates a new parameters object to be used by a Win32AppPackagerParams class.</summary>
		/// <param name="settings">Immutable settings to initialize the parameters object with.</param>
		Win32AppPackagerParams(const Win32AppPackagerParams::CoreSettings& settings);

		/// <summary>Destroys this object's allocated resources.</summary>
		virtual ~Win32AppPackagerParams();

		/// <summary>
		///  <para>Gets the name of EXE file to build, excluding the directory path.</para>
		///  <para>This parameter is required. The build will fail if null or empty.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the name of the EXE file to be built.</para>
		///  <para>Returns null if not assigned yet.</para>
		/// </returns>
		const char* GetExeFileName() const;

		/// <summary>
		///  <para>Sets the name of the EXE file to be built, excluding the directory path.</para>
		///  <para>This parameter is required. The build will fail if null or empty.</para>
		/// </summary>
		/// <param name="value">The name of the exe file, excluding the directory path. Can be null or empty string.</param>
		void SetExeFileName(const char* value);

		/// <summary>Gets the company name to be written to the built EXE file's properties.</summary>
		/// <returns>
		///  <para>Returns the company name string to be written to the EXE file's properties.</para>
		///  <para>Returns null if not assigned yet.</para>
		/// </returns>
		const char* GetCompanyName() const;

		/// <summary>Sets the company name to be written to the built EXE file's properties.</summary>
		/// <param name="value">The company name. Can be null or empty string.</param>
		void SetCompanyName(const char* value);

		/// <summary>Gets the copyright string to be written to the built EXE file's properties.</summary>
		/// <returns>
		///  <para>Returns the copyright string to be written to the EXE file's properties.</para>
		///  <para>Returns null if not assigned yet.</para>
		/// </returns>
		const char* GetCopyrightString() const;

		/// <summary>Sets the copyright string to be written to the built EXE file's properties.</summary>
		/// <param name="value">The copyright string. Can be null or empty string.</param>
		void SetCopyrightString(const char* value);

		/// <summary>Gets the application description to be written to the built EXE file's properties.</summary>
		/// <returns>
		///  <para>Returns the application description to be written to the EXE file's properties.</para>
		///  <para>Returns null if not assigned yet.</para>
		/// </returns>
		const char* GetAppDescription() const;

		/// <summary>Sets the application description to be written to the built EXE file's properties.</summary>
		/// <param name="value">The application description. Can be null or empty string.</param>
		void SetAppDescription(const char* value);

		/// <summary>
		///  <para>Gets a pointer to the Corona runtime associated with the project that is being built.</para>
		///  <para>The runtime is needed to acquire and unzip its plugins for local builds.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to the Corona runtime associated with the project being built.</para>
		///  <para>Returns null if not assigned yet.</para>
		/// </returns>
		Runtime* GetRuntime() const;

		/// <summary>
		///  <para>Sets a pointer to the Corona runtime associated with the project that is being built.</para>
		///  <para>The runtime is needed to acquire and unzip its plugins for local builds.</para>
		/// </summary>
		/// <param name="value">Pointer to the Corona runtime. Can be null.</param>
		void SetRuntime(Runtime* value);

	private:
		Rtt::String fExeFileName;
		Rtt::String fCompanyName;
		Rtt::String fCopyrightString;
		Rtt::String fAppDescription;
		Rtt::Runtime* fRuntimePointer;
};

} // namespace Rtt
