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

#pragma region Forward Declarations
namespace Interop { namespace UI {
	class UIComponentCollection;
	class UIComponent;
} }

#pragma endregion


namespace Interop { namespace UI {

/// <summary>Read-only container which wraps a mutable UIComponentCollection.</summary>
/// <remarks>This class was designed to be returned by value cheaply.</remarks>
class ReadOnlyUIComponentCollection
{
	public:
		/// <summary>Creates an empty read-only collection.</summary>
		ReadOnlyUIComponentCollection();

		/// <summary>Creates a read-only collection wrapping the give mutable UIComponent collection.</summary>
		/// <param name="collectionPointer">
		///  <para>Pointer to the mutable UIComponent collection to be wrapped.</para>
		///  <para>Can be null. In which case, this read-only container will always be empty.</para>
		/// </param>
		ReadOnlyUIComponentCollection(const UIComponentCollection* collectionPointer);

		/// <summary>Creates a read-only collection wrapping the give mutable UIComponent collection.</summary>
		/// <param name="collection">The mutable UIComponent collection to be wrapped.</param>
		ReadOnlyUIComponentCollection(const UIComponentCollection& collection);

		/// <summary>
		///  <para>Destroys this read-only collection container.</para>
		///  <para>This does not delete the given UIComponentCollection or its UIComponent objects.</para>
		/// </summary>
		virtual ~ReadOnlyUIComponentCollection();

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

	private:
		/// <summary>
		///  <para>Pointer to the mutable UIComponent collection this read-only container wraps.</para>
		///  <para>Set to null if this read-only wrapper does not wrap a collection.</para>
		/// </summary>
		const UIComponentCollection* fCollectionPointer;
};

} }	// namespace Interop::UI
