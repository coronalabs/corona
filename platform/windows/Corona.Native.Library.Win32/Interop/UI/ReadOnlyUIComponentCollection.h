//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
