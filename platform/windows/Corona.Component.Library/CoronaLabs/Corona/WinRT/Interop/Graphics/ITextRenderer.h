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
ref class BitmapResult;
ref class BitmapInfoResult;
ref class TextRendererSettings;


/// <summary>Renders text to a grayscaled bitmap.</summary>
public interface class ITextRenderer
{
	/// <summary>Gets the configuration this renderer will use to render text to a bitmap.</summary>
	/// <value>The settings this renderer will use to render text to a bitmap.</value>
	property TextRendererSettings^ Settings { TextRendererSettings^ get(); }

	/// <summary>
	///  Determines the size of the bitmap this text renderer will produce when calling the CreateBitmap() method.
	/// </summary>
	/// <returns>
	///  <para>
	///   Returns a result object providing the bitmap information calculated by this text renderer if flagged successful.
	///  </para>
	///  <para>
	///   Returns a failure result object if there was an error, in which case the the result's Message property
	///   would provide details as to what went wrong.
	///  </para>
	/// </returns>
	BitmapInfoResult^ MeasureBitmap();

	/// <summary>Creates a new bitmap with text drawn to it according to the settings applied to this renderer.</summary>
	/// <returns>
	///  <para>Returns a result object which will provide a new grayscaled bitmap if flagged as successful.</para>
	///  <para>
	///   Returns a failure result object if there was an error, in which case the the result's Message property
	///   would provide details as to what went wrong.
	///  </para>
	/// </returns>
	BitmapResult^ CreateBitmap();
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
