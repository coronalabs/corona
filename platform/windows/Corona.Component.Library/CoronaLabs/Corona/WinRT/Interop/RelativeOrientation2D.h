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

#include <collection.h>
#pragma warning(disable: Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE)
#	include "Rtt_Build.h"
#	include "Rtt_DeviceOrientation.h"
#	include "Display/Rtt_PlatformBitmap.h"
#pragma warning(default: Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE)


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop {

/// <summary>
///  Provides relative orientations compatible with Corona such as Upright, SidewaysLeft, SidewaysRight, etc.
/// </summary>
/// <remarks>
///  <para>
///   You cannot create instances of this class. Instead, you access pre-defined orientation objects
///   via this class' static properties much like you would with an enum.
///  </para>
///  <para>Instances of this class are immutable.</para>
/// </remarks>
public ref class RelativeOrientation2D sealed
{
	private:
		/// <summary>Creates a new orientation type with the given settings.</summary>
		/// <remarks>This is a private constructor used to create this class's pre-defined orientation objects.</remarks>
		/// <param name="coronaDeviceOrientation">Corona's DeviceOrientation::Type enum ID for this orientation type.</param>
		/// <param name="coronaBitmapOrientation">Corona's BitmapPlatform::Orientation enum ID for this orientation type.</param>
		/// <param name="invariantName">Unique name to be returned by this orientation type's ToString() method.</param>
		/// <param name="degreesClockwise">
		///  <para>The rotation of this orientation in degrees in the clockwise direction.</para>
		///  <para>0 degrees would be upright. 90 degrees would be sideways right. 180 degrees would be upside down.</para>
		///  <para>Will throw an exception if given a value less than zero or greater or equal to 360.</para>
		/// </param>
		RelativeOrientation2D(
				Rtt::DeviceOrientation::Type coronaDeviceOrientation,
				Rtt::PlatformBitmap::Orientation coronaBitmapOrientation,
				Platform::String^ invariantName,
				int degreesClockwise);

	public:
		/// <summary>Gets the rotation of this orientation in degrees in the clockwise direction.</summary>
		/// <value>
		///  <para>Returns 0 if the orientation is upright or unknown.</para>
		///  <para>Returns 90 if the orientation is sideways right.</para>
		///  <para>Returns 180 if the orientation is upside down.</para>
		///  <para>Returns 270 if the orientation is sideways left.</para>
		/// </value>
		property int DegreesClockwise { int get(); }

		/// <summary>Gets the rotation of this orientation in degrees in the counter-clockwise direction.</summary>
		/// <value>
		///  <para>Returns 0 if the orientation is upright or unknown.</para>
		///  <para>Returns 90 if the orientation is sideways left.</para>
		///  <para>Returns 180 if the orientation is upside down.</para>
		///  <para>Returns 270 if the orientation is sideways right.</para>
		/// </value>
		property int DegreesCounterClockwise { int get(); }

		/// <summary>Determines if this orientation is either SidewaysLeft or SidewaysRight.</summary>
		/// <value>
		///  <para>Returns true if the orientation is SidewaysLeft or SidewaysRight.</para>
		///  <para>Returns false if the orientation is Upright, UpsideDown, or Unknown.</para>
		/// </value>
		property bool IsSideways { bool get(); }

		/// <summary>Gets the name of this orientation.</summary>
		/// <returns>Returns the invariant (non-localized) name of this orientation, such as "Upright" or "SidewaysRight".</returns>
		Platform::String^ ToString();

		/// <summary>Represents an unknown orientation.</summary>
		static property RelativeOrientation2D^ Unknown { RelativeOrientation2D^ get(); }

		/// <summary>Represents an orientation that is upright.</summary>
		static property RelativeOrientation2D^ Upright { RelativeOrientation2D^ get(); }

		/// <summary>Represents an orientation that is upside down.</summary>
		static property RelativeOrientation2D^ UpsideDown { RelativeOrientation2D^ get(); }

		/// <summary>Represents an orientation that is rotated 90 degrees clockwise.</summary>
		static property RelativeOrientation2D^ SidewaysRight { RelativeOrientation2D^ get(); }

		/// <summary>Represents an orientation that is rotated 90 degrees counter-clockwise.</summary>
		static property RelativeOrientation2D^ SidewaysLeft { RelativeOrientation2D^ get(); }

		/// <summary>Gets a read-only collection of all orientations provided by this class, including the "Unknown" type.</summary>
		/// <value>Read-only collection of all orientation types provided by this class.</value>
		static property Windows::Foundation::Collections::IIterable<RelativeOrientation2D^>^ Collection
				{ Windows::Foundation::Collections::IIterable<RelativeOrientation2D^>^ get(); }

	internal:
		/// <summary>Gets the unique ID for this orientation matching a Corona DeviceOrientation::Type enum constant.</summary>
		/// <remarks>This is an internal method that is only accessible to Corona Labs.</remarks>
		/// <value>
		///  <para>Unique ID matching a constant in Corona's DeviceOrientation::Type enum.</para>
		///  <para>This property will never return kFaceUp or kFaceDown since this is a 2D orientation object.</para>
		/// </value>
		property Rtt::DeviceOrientation::Type CoronaDeviceOrientation { Rtt::DeviceOrientation::Type get(); }

		/// <summary>Gets the unique ID for this orientation matching a Corona PlatformBitmap::Orientation enum constant.</summary>
		/// <remarks>This is an internal method that is only accessible to Corona Labs.</remarks>
		/// <value>
		///  <para>Unique ID matching a constant in Corona's PlatformBitmap::Orientation enum.</para>
		///  <para>If this orientation type is Unknown, then this returns the "kUp" constant.</para>
		/// </value>
		property Rtt::PlatformBitmap::Orientation CoronaBitmapOrientation { Rtt::PlatformBitmap::Orientation get(); }

		/// <summary>Fetches an orientation matching the given Corona device orientation constant.</summary>
		/// <remarks>This is an internal method that is only accessible to Corona Labs.</remarks>
		/// <param name="value">The requested orientation type such as kUpright, kSidewaysRight, etc.</param>
		/// <returns>
		///  <para>Returns an orientation object matching the given Corona constant.</para>
		///  <para>
		///   Returns null if a matching orientation object could not be found. For example, this function will always
		///   return null if given a kFaceUp or kFaceDown value since this orienation class only supports 2D orientation types.
		///  </para>
		/// </returns>
		static RelativeOrientation2D^ From(Rtt::DeviceOrientation::Type value);

		/// <summary>Fetches an orientation matching the given Corona bitmap orientation constant.</summary>
		/// <remarks>This is an internal method that is only accessible to Corona Labs.</remarks>
		/// <param name="value">The requested orientation type such as kUp, kRight, etc.</param>
		/// <returns>
		///  <para>Returns an orientation object matching the given Corona constant.</para>
		///  <para>Returns null if a matching orientation object could not be found.</para>
		/// </returns>
		static RelativeOrientation2D^ From(Rtt::PlatformBitmap::Orientation value);

	private:
		/// <summary>Gets a private mutable collection used to store all orientation types provided by this class.</summary>
		/// <remarks>
		///  This is used by this class' constructor to automatically store all pre-defined orientation objects
		///  to be used by this class' static From() methods to do the equivalent of a Java/.NET reflection lookup
		///  for pre-existing objects.
		/// </remarks>
		static property Platform::Collections::Vector<RelativeOrientation2D^>^ MutableCollection
				{ Platform::Collections::Vector<RelativeOrientation2D^>^ get(); }

		static const RelativeOrientation2D^ kUnknown;
		static const RelativeOrientation2D^ kUpright;
		static const RelativeOrientation2D^ kUpsideDown;
		static const RelativeOrientation2D^ kSidewaysRight;
		static const RelativeOrientation2D^ kSidewaysLeft;

		Rtt::DeviceOrientation::Type fCoronaDeviceOrientation;
		Rtt::PlatformBitmap::Orientation fCoronaBitmapOrientation;
		Platform::String^ fInvariantName;
		int fDegreesClockwise;
};

} } } }	// namespace CoronaLabs::Corona::WinRT::Interop
