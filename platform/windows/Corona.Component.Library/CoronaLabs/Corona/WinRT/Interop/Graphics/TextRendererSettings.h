//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

// Forward declarations.
ref class FontSettings;
ref class HorizontalAlignment;


/// <summary>Provides the configuration for an ITextRenderer implemented object.</summary>
public ref class TextRendererSettings sealed
{
	public:
		/// <summary>Creates a new configuration for a text renderer.</summary>
		TextRendererSettings();

		/// <summary>
		///  Gets the modifiable font settings to be used such as font name, point size, and the bold/italic styles.
		/// </summary>
		/// <value>The font settings to be applied by the text renderer.</value>
		property CoronaLabs::Corona::WinRT::Interop::Graphics::FontSettings^ FontSettings
		{
			CoronaLabs::Corona::WinRT::Interop::Graphics::FontSettings^ get();
		}

		/// <summary>Gets or sets the text alignment the renderer will use.</summary>
		/// <value>
		///  <para>The text alignment such as Left, Center, or Right.</para>
		///  <para>Setting this to null will cause an exception to be thrown.</para>
		/// </value>
		property CoronaLabs::Corona::WinRT::Interop::Graphics::HorizontalAlignment^ HorizontalAlignment
		{
			CoronaLabs::Corona::WinRT::Interop::Graphics::HorizontalAlignment^ get();
			void set(CoronaLabs::Corona::WinRT::Interop::Graphics::HorizontalAlignment^ value);
		}

		/// <summary>Gets or sets the width in pixels of the text block. This is where text wrapping occurs.</summary>
		/// <value>
		///  <para>The number of pixels wide the text can be until it is wrapped.</para>
		///  <para>
		///   Set to zero if text wrapping is disabled.
		///   The final rendered width will be automatically set based on the size of the text.
		///  </para>
		/// </value>
		property int BlockWidth { int get(); void set(int value); }

		/// <summary>Gets or sets the height in pixels of the rendered text block.</summary>
		/// <value>
		///  <para>The number of pixels high the text can be.</para>
		///  <para>Set to zero if the text block height will be set automatically based on the rendered text.</para>
		/// </value>
		property int BlockHeight { int get(); void set(int value); }

		/// <summary>
		///  Gets or sets the maximum number of pixels wide the rendered text can be until it gets clipped.
		/// </summary>
		/// <remarks>If you are rendering text in Direct3D, then this width should be set to the maximum texture size.</remarks>
		/// <value>
		///  <para>The number of pixels wide until the rendered text gets clipped.</para>
		///  <para>Set to zero if clipping is disabled, meaning that the renderer will draw text as wide as needed.</para>
		/// </value>
		property int ClipWidth { int get(); void set(int value); }

		/// <summary>
		///  Gets or sets the maximum number of pixels high the rendered text can be until it gets clipped.
		/// </summary>
		/// <remarks>If you are rendering text in Direct3D, then this height should be set to the maximum texture size.</remarks>
		/// <value>
		///  <para>The number of pixels high until the rendered text gets clipped.</para>
		///  <para>Set to zero if clipping is disabled, meaning that the renderer will draw text as tall as needed.</para>
		/// </value>
		property int ClipHeight { int get(); void set(int value); }

		/// <summary>Gets or sets the text to be renderered.</summary>
		/// <value>The text to be rendered. Can be null or empty string.</value>
		property Platform::String^ Text { Platform::String^ get(); void set(Platform::String^ value); }

	private:
		CoronaLabs::Corona::WinRT::Interop::Graphics::FontSettings^ fFontSettings;
		CoronaLabs::Corona::WinRT::Interop::Graphics::HorizontalAlignment^ fHorizontalAlignment;
		int fBlockWidth;
		int fBlockHeight;
		int fClipWidth;
		int fClipHeight;
		Platform::String^ fText;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
