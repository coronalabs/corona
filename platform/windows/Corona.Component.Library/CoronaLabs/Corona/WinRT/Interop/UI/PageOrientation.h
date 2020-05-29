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
#	include "Rtt_DeviceOrientation.h"
#pragma warning(default: Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE)


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop {
	ref class RelativeOrientation2D;
} } } }


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

/// <summary>Indicates a page's current orientation such as PortraitUpright, LandscapeRight, etc.</summary>
/// <remarks>
///  <para>
///   You cannot create instances of this class. Instead, you access pre-defined orientation objects
///   via this class' static properties much like you would with an enum.
///  </para>
///  <para>Instances of this class are immutable.</para>
/// </remarks>
public ref class PageOrientation sealed
{
	private:
		/// <summary>Creates a new orientation type with the given settings.</summary>
		/// <remarks>This is a private constructor used to create this class's pre-defined orientation objects.</remarks>
		/// <param name="coronaDeviceOrientation">Corona's DeviceOrientation::Type enum ID for this orientation type.</param>
		PageOrientation(Rtt::DeviceOrientation::Type coronaDeviceOrientation);

	public:
		/// <summary>Determines if this page orientation is set to one of the portrait types.</summary>
		/// <value>
		///  <para>Returns true if this orientation is either PortraitUpright or PortraitUpsideDown.</para>
		///  <para>Returns false if this orientation is either LandscapeRight, LandscapeLeft, or Unknown.</para>
		/// </value>
		property bool IsPortrait { bool get(); }

		/// <summary>Determines if this page orientation is set to one of the landscape types.</summary>
		/// <value>
		///  <para>Returns true if this orientation is either LandscapeRight or LandscapeLeft.</para>
		///  <para>Returns false if this orientation is either PortraitUpright, PortraitUpsideDown, or Unknown.</para>
		/// </value>
		property bool IsLandscape { bool get(); }

		/// <summary>
		///  Compares this orientation object with the given orientation object and returns a "RelativeOrientation2D"
		///  object that is relative to the given orientation.
		/// </summary>
		/// <param name="orientation">The orientation to compare this object's orientation with.</param>
		/// <returns>
		///  <para>Returns "Upright" if both orientations match.</para>
		///  <para>
		///   Returns "SidewaysRight" if this orientation object is 90 degrees counter-clockwise compared
		///   to the given orientation object.
		///  </para>
		///  <para>
		///   Returns "SidewaysLeft" if this orientation object is 90 degrees clockwise compared
		///   to the given orientation object.
		///  </para>
		///  <para>Returns "UpsideDown" if there is a 180 degree difference between the 2 orientation objects.</para>
		///  <para>Returns "Unknown" if either orientation object is unknown or null.</para>
		/// </returns>
		RelativeOrientation2D^ ToOrientationRelativeTo(PageOrientation^ orientation);

		/// <summary>Gets the name of this orientation.</summary>
		/// <returns>
		///  Returns the invariant (non-localized) name of this orientation, such as "portrait" or "landscapeRight".
		/// </returns>
		Platform::String^ ToString();

		/// <summary>Represents an unknown orientation.</summary>
		static property PageOrientation^ Unknown { PageOrientation^ get(); }

		/// <summary>An upright portrait orientation.</summary>
		static property PageOrientation^ PortraitUpright { PageOrientation^ get(); }

		/// <summary>A portrait orientation that is upside down.</summary>
		static property PageOrientation^ PortraitUpsideDown { PageOrientation^ get(); }

		/// <summary>
		///  Landscape orientation that was rotated 90 degrees counter-clockwise from a PortraitUpright orientation.
		/// </summary>
		static property PageOrientation^ LandscapeRight { PageOrientation^ get(); }

		/// <summary>
		///  Landscape orientation that was rotated 90 degrees clockwise from a PortraitUpright orientation.
		/// </summary>
		static property PageOrientation^ LandscapeLeft { PageOrientation^ get(); }

		/// <summary>
		///  Gets a read-only collection of all orientations provided by this class, including the "Unknown" type.
		/// </summary>
		/// <value>Read-only collection of all orientation types provided by this class.</value>
		static property Windows::Foundation::Collections::IIterable<PageOrientation^>^ Collection
		{
			Windows::Foundation::Collections::IIterable<PageOrientation^>^ get();
		}

	internal:
		/// <summary>Gets the unique ID for this orientation matching a Corona DeviceOrientation::Type enum constant.</summary>
		/// <remarks>This is an internal method that is only accessible to Corona Labs.</remarks>
		/// <value>
		///  <para>Unique ID matching a constant in Corona's DeviceOrientation::Type enum.</para>
		///  <para>This property will never return kFaceUp or kFaceDown.</para>
		/// </value>
		property Rtt::DeviceOrientation::Type CoronaDeviceOrientation { Rtt::DeviceOrientation::Type get(); }

	private:
		/// <summary>Gets a private mutable collection used to store all orientation types provided by this class.</summary>
		/// <remarks>
		///  This is used by this class' constructor to automatically store all pre-defined orientation objects
		///  to be used by this class' static From() methods to do the equivalent of a Java/.NET reflection lookup
		///  for pre-existing objects.
		/// </remarks>
		static property Platform::Collections::Vector<PageOrientation^>^ MutableCollection
		{
			Platform::Collections::Vector<PageOrientation^>^ get();
		}

		static const PageOrientation^ kUnknown;
		static const PageOrientation^ kPortraitUpright;
		static const PageOrientation^ kPortraitUpsideDown;
		static const PageOrientation^ kLandscapeRight;
		static const PageOrientation^ kLandscapeLeft;

		Rtt::DeviceOrientation::Type fCoronaDeviceOrientation;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
