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

#include "TouchTracker.h"
#include <collection.h>


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

/// <summary>Stores a collection of TouchTracker objects.</summary>
public ref class TouchTrackerCollection sealed :
	public Windows::Foundation::Collections::IIterable<TouchTracker^>
{
	public:
		/// <summary>Creates an empty collection used to store TouchTracker objects.</summary>
		TouchTrackerCollection();

		/// <summary>Adds the given touch tracker to the collection, if not done already.</summary>
		/// <param name="item">
		///  <para>The touch tracker to be added.</para>
		///  <para>Will not be added if the given object reference already exists in the collection.</para>
		///  <para>Will be ignored if null. Null references cannot be added to the set.</para>
		/// </param>
		void Add(TouchTracker^ item);

		/// <summary>Fetches a touch tracker from this collection by its unique pointer ID.</summary>
		/// <param name="id">Unique integer ID assigned to the touch tracker.</param>
		/// <returns>
		///  <para>Returns a touch tracker matching the given ID.</para>
		///  <para>Returns null if the given ID was not found in the collection.</para>
		/// </returns>
		TouchTracker^ GetByPointerId(int id);

		/// <summary>Removes the given touch tracker from the collection by reference.</summary>
		/// <param name="item">The touch tracker to be removed from the collection, if it exists.</param>
		/// <returns>
		///  <para>Returns true if the touch tracker was removed from the collection.</para>
		///  <para>Returns false if the given touch tracker was not found or if given null.</para>
		/// </returns>
		bool Remove(TouchTracker^ item);

		/// <summary>Removes all touch trackers from the collection, making it empty.</summary>
		void Clear();

		/// <summary>Determines if the given touch tracker reference exists in the collection.</summary>
		/// <param name="item">The touch tracker to search for by reference.</param>
		/// <returns>
		///  <para>Returns true if the given touch tracker exists in the collection.</para>
		///  <para>Returns false if the touch tracker was not found or if given null.</para>
		/// </returns>
		bool Contains(TouchTracker^ item);

		/// <summary>Determines if a touch tracker assigned the given pointer ID exists in the collection.</summary>
		/// <param name="id">The unique pointer ID of the touch tracker to search for.</param>
		/// <returns>
		///  <para>Returns true if a touch tracker having the given ID was found in the collection.</para>
		///  <para>Returns false if the given ID was not found in the collection.</para>
		/// </returns>
		bool ContainsPointerId(int id);

		/// <summary>Gets the number of touch trackers contained in the collection.</summary>
		/// <value>
		///  <para>The number of touch trackers contained in this collection.</para>
		///  <para>Will be zero if this collection is empty.</para>
		/// </value>
		property int Count { int get(); }

		/// <summary>Resets all touch trackers in the collection.</summary>
		void ResetAll();

		/// <summary>Gets an enumerator that can be used to iterate through all of the items in this collection.</summary>
		/// <returns>Returns an enumerator that can be used by a foreach loop.</returns>
		virtual Windows::Foundation::Collections::IIterator<TouchTracker^>^ First();

	private:
		Platform::Collections::Vector<TouchTracker^>^ fCollection;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input
