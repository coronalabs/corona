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

#include <string>


namespace Interop {

/// <summary>
///  <para>Provides easy access to information about this application and its executable.</para>
///  <para>You cannot create instances of this class. You can only access its static functions.</para>
/// </summary>
class ApplicationServices
{
	private:
		/// <summary>Constructor made private to prevent instances from being made.</summary>
		ApplicationServices();

	public:
		/// <summary>
		///  <para>
		///   Determines if the currently running application is a Corona application,
		///   such as the Corona Simulator or Corona Shell.
		///  </para>
		///  <para>
		///   A Corona Win32 desktop app and the Corona Shell both use the same Corona library. The Corona library
		///   can determine that it's okay to use Corona Simulator installed features by calling this function,
		///   such as the Corona Shell using the same sandbox directory as the simulator.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns true if this is a Corona application, such as the Corona Simulator or Corona Shell.</para>
		///  <para>Returns false if this is a normal Corona Win32 desktop app.</para>
		/// </returns>
		static bool IsCoronaSdkApp();

		/// <summary>
		///  <para>Fetches the currently running application's EXE file name, including its *.exe extension.</para>
		///  <para>The returned string does not include the directory path.</para>
		/// </summary>
		/// <returns>Returns the EXE file name, including the extension.</returns>
		static const wchar_t* GetExeFileName();

		/// <summary>
		///  <para>Fetches the currently running application's EXE file name, excluding its file extension.</para>
		///  <para>The returned string does not include the directory path.</para>
		/// </summary>
		/// <returns>Returns the EXE file name, excluding the file extension.</returns>
		static const wchar_t* GetExeFileNameWithoutExtension();

		/// <summary>
		///  <para>Fetches the absolute path to the directory the application EXE is running in.</para>
		///  <para>Does not include the application file name. Only the directory it resides in.</para>
		/// </summary>
		/// <returns>Returns an absolute path to the directory the application EXE file is in.</returns>
		static const wchar_t* GetDirectoryPath();

		/// <summary>Fetches the product name string from the application EXE file's property/version info table.</summary>
		/// <returns>
		///  <para>Returns the product name from the EXE file's property/version info.</para>
		///  <para>Returns an empty string if a product name string was not found in the EXE.</para>
		/// </returns>
		static const wchar_t* GetProductName();

		/// <summary>Fetches the company name string from the application EXE file's property/version info table.</summary>
		/// <returns>
		///  <para>Returns the company name from the EXE file's property/version info.</para>
		///  <para>Returns an empty string if a company name string was not found in the EXE.</para>
		/// </returns>
		static const wchar_t* GetCompanyName();

		/// <summary>Fetches the file version string from the application EXE file's property/version info table.</summary>
		/// <returns>
		///  <para>Returns the file version string from the EXE file's property/version info.</para>
		///  <para>Returns an empty string if file version was not found in the EXE.</para>
		/// </returns>
		static const wchar_t* GetFileVersionString();

		/// <summary>Fetches the copyright string from the application EXE file's property/version info table.</summary>
		/// <returns>
		///  <para>Returns the copyright string from the EXE file's property/version info.</para>
		///  <para>Returns an empty string if the copyright string was not found in the EXE.</para>
		/// </returns>
		static const wchar_t* GetCopyrightString();

	private:
		/// <summary>
		///  <para>Fetches the property/version info from the application's EXE file.</para>
		///  <para>Fetched information will be stored to this class' static member variables.</para>
		///  <para>Optimized to only fetch information on the first call. Subsquent calls will no-op.</para>
		/// </summary>
		static void FetchVersionInfo();


		/// <summary>Product name fetched from the EXE file's version info table.</summary>
		static std::wstring sProductName;

		/// <summary>Company name fetched from the EXE file's version info table.</summary>
		static std::wstring sCompanyName;

		/// <summary>File version string fetched from the EXE file's version info table.</summary>
		static std::wstring sFileVersionString;

		/// <summary>Copyright string fetched from the EXE file's version info table.</summary>
		static std::wstring sCopyrightString;
};

}	// namespace Interop
