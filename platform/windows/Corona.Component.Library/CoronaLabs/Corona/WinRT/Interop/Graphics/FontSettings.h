// ----------------------------------------------------------------------------
// 
// FontSettings.h
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


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

/// <summary>Stores the configuration for one font such as its font name, point size, and bold/italic style.</summary>
public ref class FontSettings sealed
{
	public:
		/// <summary>Creates a new font settings object.</summary>
		FontSettings();

		/// <summary>Copies the given settings to this object.</summary>
		/// <param name="settings">The settings to be copied.</param>
		void CopyFrom(FontSettings^ settings);

		/// <summary>Gets or sets the path to a font file to use.</summary>
		/// <value>
		///  <para>The name and path to the font file to use.</para>
		///  <para>Set to null or empty string to use the system's installed font.</para>
		/// </value>
		property Platform::String^ FilePath { Platform::String^ get(); void set(Platform::String^ value); }

		/// <summary>Gets or sets the font family name.</summary>
		/// <value>
		///  <para>The name of the font family.</para>
		///  <para>Set to null or empty string if the font name has not been assigned.</para>
		/// </value>
		property Platform::String^ FamilyName { Platform::String^ get(); void set(Platform::String^ value); }

		/// <summary>Gets or sets the whether or not the font should be rendered using a "bold" style.</summary>
		/// <value>Set to true if the font is set up to render text as bold. Set to false if not.</value>
		property bool IsBold { bool get(); void set(bool value); }

		/// <summary>Gets or sets the whether or not the font should be rendered using an "italic" style.</summary>
		/// <value>Set to true if the font is set up to render text as italic. Set to false if not.</value>
		property bool IsItalic { bool get(); void set(bool value); }

		/// <summary>Gets or sets the point size of the font.</summary>
		/// <value>
		///  <para>The point size the font will render text with.</para>
		///  <para>Cannot be set to less than 1.0.</para>
		/// </value>
		property float PointSize { float get(); void set(float value); }

		/// <summary>Gets a string describing the configuration of this object.</summary>
		/// <returns>Returns a string describing this object's configuration.</returns>
		Platform::String^ ToString();

	private:
		Platform::String^ fFilePath;
		Platform::String^ fFamilyName;
		bool fIsBold;
		bool fIsItalic;
		float fPointSize;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
