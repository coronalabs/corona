// ----------------------------------------------------------------------------
// 
// PixelFormatSet.h
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

#include "PixelFormat.h"
#include <collection.h>


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

/// <summary>Stores a collection of unique pixel formats. Does not allow duplicate pixel formats to be added.</summary>
public ref class PixelFormatSet sealed :
	public Windows::Foundation::Collections::IIterable<PixelFormat^>
{
	public:
		/// <summary>Creates an empty pixel format set.</summary>
		PixelFormatSet();

		/// <summary>Adds the given pixel format to the set, if not done already.</summary>
		/// <param name="item">
		///  <para>The pixel format to be added.</para>
		///  <para>Will not be added if it already exists in the set.</para>
		///  <para>Will be ignored if null. Null references cannot be added to the set.</para>
		/// </param>
		void Add(PixelFormat^ item);

		/// <summary>Adds a collection of pixel formats into this set.</summary>
		/// <param name="items">
		///  Collection of pixel formats to be added to this set. If any pixel formats in this collection already exist
		///  in this set, then they are not duplicated in the set. Null references are ignored.
		/// </param>
		void AddRange(Windows::Foundation::Collections::IIterable<PixelFormat^>^ items);

		/// <summary>Removes the given pixel format from the set by reference.</summary>
		/// <param name="item">The pixel format to be removed from the set, if it exists.</param>
		/// <returns>
		///  <para>Returns true if the pixel format was removed from the set.</para>
		///  <para>Returns false if the given pixel format was not found in the set or if given null.</para>
		/// </returns>
		bool Remove(PixelFormat^ item);

		/// <summary>Removes all pixel formats from the set, making it empty.</summary>
		void Clear();

		/// <summary>Determines if the given pixel format exists in the set.</summary>
		/// <param name="item">The pixel format to search for in the set.</param>
		/// <returns>
		///  <para>Returns true if the given pixel format exists in the set.</para>
		///  <para>Returns false if the pixel format was not found or if given null.</para>
		/// </returns>
		bool Contains(PixelFormat^ item);

		/// <summary>Gets the number of pixel formats contained in the set.</summary>
		/// <value>
		///  <para>The number of pixel formats contained in this set.</para>
		///  <para>Will be zero if this set is empty.</para>
		/// </value>
		property int Count { int get(); }

		/// <summary>Gets an enumerator that can be used to iterate through all of the items in this collection.</summary>
		/// <returns>Returns an enumerator that can be used by a foreach loop.</returns>
		virtual Windows::Foundation::Collections::IIterator<PixelFormat^>^ First();

	private:
		/// <summary>Collection storing all pixel format objects.</summary>
		Platform::Collections::Vector<PixelFormat^>^ fCollection;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
