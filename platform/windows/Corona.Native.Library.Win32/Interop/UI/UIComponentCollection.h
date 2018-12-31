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

#include <list>


#pragma region Forward Declarations
namespace Interop { namespace UI {
	class UIComponent;
} }

#pragma endregion


namespace Interop { namespace UI {

/// <summary>Stores a collection of UIComponent pointers.</summary>
class UIComponentCollection
{
	public:
		/// <summary>Creates an empty UIComponent collection.</summary>
		UIComponentCollection();

		/// <summary>Creates a new collection containing a copy of the given collection's UIComponent pointers.</summary>
		/// <param name="collection">The collection to be copied.</param>
		UIComponentCollection(const UIComponentCollection& collection);

		/// <summary>
		///  <para>Destroys this collection object.</para>
		///  <para>This does not destroy the UIComponents stored in this collection.</para>
		/// </summary>
		virtual ~UIComponentCollection();

		/// <summary>Adds the given UIComponent pointer to the end of the collection.</summary>
		/// <param name="componentPointer">
		///  <para>Pointer to the UIComponent to be added to the collection.</para>
		///  <para>Ignored if the given pointer was already added to the collection. (Must be a unique instance.)</para>
		///  <para>Ignored if given a null pointer. (This collection does not support adding null pointers.)</para>
		/// </param>
		void Add(UIComponent* componentPointer);

		/// <summary>Adds the given collection's UIComponent pointers to this collection.</summary>
		/// <param name="collection">
		///  <para>Collection of UIComponent pointers to be added to the end of this collection.</para>
		///  <para>Pointers already added to this collection will be ignored. (ie: Already added.)</para>
		/// </param>
		void Add(const UIComponentCollection& collection);

		/// <summary>Removes a UIComponent from this collection by its memory address.</summary>
		/// <param name="componentPointer">Pointer to the UIComponent to be removed.</param>
		/// <returns>
		///  <para>Returns true if the referenced UIComponent was successfully removed.</para>
		///  <para>Returns false if the referenced UIComponent was not found or if given null.</para>
		/// </returns>
		bool Remove(UIComponent* componentPointer);

		/// <summary>Removes a UIComponent from this collection by its zero based index.</summary>
		/// <param name="index">Zero based index of the UIComponent to be removed.</param>
		/// <returns>
		///  <para>Returns true if the indexed UIComponent was successfully removed.</para>
		///  <para>Returns false if given an invalid index.</para>
		/// </returns>
		bool RemoveByIndex(int index);

		/// <summary>Removes all UIComponents from the collection.</summary>
		void Clear();

		/// <summary>Gets the number of UIComponents stored in this collection.</summary>
		/// <returns>
		///  <para>Returns the number of UIComponents stored in this collection.</para>
		///  <para>Returns zero if this collection is empty.</para>
		/// </returns>
		int GetCount() const;

		/// <summary>Fetches a UIComponent from the collection by its zero based index.</summary>
		/// <param name="index">Zero based index of the UIComponent to fetch.</param>
		/// <returns>
		///  <para>Returns a pointer to the indexed UIComponent.</para>
		///  <para>Returns null if given an invalid index.</para>
		/// </returns>
		UIComponent* GetByIndex(int index) const;

		/// <summary>Determines if the given UIComponent exists in the collection.</summary>
		/// <param name="componentPointer">Pointer to the UIComponent to search for.</param>
		/// <returns>
		///  <para>Returns true if the UIComponent was found in the collection.</para>
		///  <para>Returns false if not found or if given a null pointer.</para>
		/// </returns>
		bool Contains(UIComponent* componentPointer) const;

		/// <summary>
		///  Clears this collection and copies the UIComponents from the given collection in the same order.
		/// </summary>
		/// <param name="collection">The collection to copy from.</param>
		/// <returns>Returns a UIComponent collection which exactly matches the given collection.</returns>
		UIComponentCollection& operator=(const UIComponentCollection& collection);

	private:
		/// <summary>Stores a collection of UIComponent pointers.</summary>
		std::list<UIComponent*> fCollection;
};

} }	// namespace Interop::UI
