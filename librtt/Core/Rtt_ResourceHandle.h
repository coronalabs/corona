//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_ResourceHandle_H
#define Rtt_ResourceHandle_H

#include "Core/Rtt_Types.h"
#include "Core/Rtt_Macros.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// Base class for handling mechanics and reduce code bloat
// Use ResourceHandle and ResourceHandleOwner instead
class BaseResourceHandle
{
	public:
		typedef BaseResourceHandle Self;

	public:
		BaseResourceHandle();
		BaseResourceHandle( const Self& rhs );
		~BaseResourceHandle();
		Self& operator=( const Self& rhs );

	protected:
		void Initialize( Rtt_Allocator* pAllocator );

	public:
		bool IsValid() const;

	protected:
		void Invalidate();

	private:
		void Retain( RefCount *pCount );
		void Release();

	protected:
		RefCount *fCount;
};

// ----------------------------------------------------------------------------

// There are times when resources have particular lifetimes which are well-defined
// in scope making smart pointers a poor choice in managing them. This becomes
// especially problematic when many objects require a reference to this resource,
// but do own them. Those objects would need to be notified when that resource 
// becomes invalid. The typical solution is often heavyweight, using the Observer
// pattern to notify all objects that the resource is invalid.
// 
// This class is a lightweight alternative. The idea is to eliminate notification
// overhead by letting the observers detect invalidity in a just-in-time manner.
// The key is for the actual owner to create a ResourceHandleOwner object and
// let all "observer" objects hold onto a ResourceHandle object. The owner can
// Invalidate() the handle while the observers should check that the handle
// IsValid() before dereferencing the resource.
// 
template < typename T >
class ResourceHandle : public BaseResourceHandle
{
	public:
		typedef BaseResourceHandle Super;

	// ResourceHandle instances can only be copied.
	// The resource owner should wrap a resource via ResourceHandleOwner
	// and then only expose them as ResourceHandle's to any object that
	// needs a reference to it.
	protected:
		ResourceHandle( T& resource );

	public:
		T* Dereference() const;

	protected:
		T& fResource;
};

template < typename T >
ResourceHandle< T >::ResourceHandle( T& resource )
:	Super(),
	fResource( resource )
{
}

template < typename T >
T*
ResourceHandle< T >::Dereference() const
{
	T *result = NULL;

	if ( Super::IsValid() )
	{
		result = & fResource;
	}

	return result;
}

// ----------------------------------------------------------------------------

// Controls the validity of all ResourceHandle objects
template < typename T >
class ResourceHandleOwner : public ResourceHandle< T >
{
	public:
		typedef ResourceHandle< T > Super;
		typedef ResourceHandleOwner< T > Self;

	public:
		// Actual resource is owned outside of the ResourceHandle mechanism.
		ResourceHandleOwner( Rtt_Allocator* pAllocator, T& resource );
		~ResourceHandleOwner();

	private:
		// Prevent copies. Only one instance allowed!
		ResourceHandleOwner( const Self& rhs );
		const Self& operator=( const Self& rhs );

	Rtt_CLASS_NO_DYNAMIC_ALLOCATION
};

template < typename T >
ResourceHandleOwner< T >::ResourceHandleOwner( Rtt_Allocator* pAllocator, T& resource )
:	Super( resource )
{
	Super::Initialize( pAllocator );
}

template < typename T >
ResourceHandleOwner< T >::~ResourceHandleOwner()
{
	Super::Invalidate();
}

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_ResourceHandle_H

