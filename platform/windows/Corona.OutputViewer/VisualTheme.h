//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
