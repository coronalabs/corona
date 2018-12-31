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


/// <summary>
///  <para>Defines a UI visual theme type such as kSystem, kDark, and kBlue.</para>
///  <para>
///   You cannot create instances of this class.
///   Instead, you access pre-defined instances via this class' static constants or functions.
///  </para>
/// </summary>
class VisualTheme final
{
	private:
		/// <summary>Copy constructor made private to prevent copies from being made.</summary>
		VisualTheme(const VisualTheme& theme);

		/// <summary>Creates a new visual theme having the given unique IDs.</summary>
		/// <param name="resourceIntegerId">
		///  The Win32 resource integer ID associated with this theme such as ID_THEME_DARK, ID_THEME_BLUE, etc.
		/// </param>
		/// <param name="stringId">Unique string ID used to save the theme to the registry.</param>
		VisualTheme(UINT resourceIntegerId, const TCHAR* stringId);

		/// <summary>Destroys this object.</summary>
		virtual ~VisualTheme();

	public:
		/// <summary>
		///  Gets the Win32 resource integer ID associated with this theme such as ID_THEME_DARK, ID_THEME_BLUE, etc.
		/// </summary>
		/// <returns>Returns this theme's unique Win32 resource integer ID.</returns>
		UINT GetResourceIntegerId() const;

		/// <summary>
		///  Gets this theme's unique string ID intended to be used to save the selected theme to the registry.
		/// </summary>
		/// <returns>Returns this theme's unique string ID.</returns>
		const TCHAR* GetStringId() const;

		/// <summary>Visual theme using the Windows OS' default UI and color scheme.</summary>
		static const VisualTheme kSystem;

		/// <summary>A dark MS-DOS like visual theme.</summary>
		static const VisualTheme kDark;

		/// <summary>A blue MS Office like visual theme.</summary>
		static const VisualTheme kBlue;

		/// <summary>Fetches a visual theme matching the given Win32 resource integer ID.</summary>
		/// <param name="integerId">The unique Win32 resource ID such as ID_THEME_DARK, ID_THEME_BLUE, etc.</param>
		/// <returns>
		///  <para>Returns a pointer to one of this class' visual theme constants matching the given ID.</para>
		///  <para>Returns null if the given ID was not found.</para>
		/// </returns>
		static const VisualTheme* FromResourceIntegerId(UINT integerId);

		/// <summary>Fetches a visual theme matching the given string ID.</summary>
		/// <param name="stringId">The unique string ID of the theme such as "dark", "blue", etc.</param>
		/// <returns>
		///  <para>Returns a pointer to one of this class' visual theme constants matching the given ID.</para>
		///  <para>Returns null if the given ID was not found.</para>
		/// </returns>
		static const VisualTheme* FromStringId(const TCHAR* stringId);

	private:
		/// <summary>Assignment operator made private to make instances immutable.</summary>
		const VisualTheme& operator=(const VisualTheme&) {}


		/// <summary>The theme's unique Win32 resource ID such as ID_THEME_DARK, ID_THEME_BLUE, etc.</summary>
		UINT fResourceIntegerId;

		/// <summary>The theme's unique string ID such as "dark", "blue", etc.</summary>
		CString fStringId;
};
