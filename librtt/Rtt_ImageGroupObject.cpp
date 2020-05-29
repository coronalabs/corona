//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#if 0

#include "Core/Rtt_Build.h"

#include "Rtt_ImageGroupObject.h"

#include "Rtt_Color.h"
#include "Rtt_DisplayList.h"
#include "Rtt_ImageSheet.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_StageObject.h"
#include "Rtt_VertexArray.h"
#include "Rtt_VertexArrayStream.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

bool
ImageGroupObject::IsCompatibleParent(
	CompositeObject *parent,
	const AutoPtr< ImageSheet >& sheet )
{
	bool result = true;

	if ( parent
		 && ( & parent->ProxyVTable() == & LuaImageGroupObjectProxyVTable::Constant() ) )
	{
		ImageGroupObject *imageGroup = (ImageGroupObject*)parent;
		result = ( imageGroup->GetSheet() == sheet );
	}

	return result;
}

// ----------------------------------------------------------------------------

const U8 kAttributeNumComponents[] =
{
	2,	// kPositionAttribute
	2,	// kTextureCoordAttribute
	4	// kColorAttribute
};

const VertexArray::Attribute kAttributeTypes[] =
{
	VertexArray::kPositionAttribute,
	VertexArray::kTextureCoordAttribute,
	VertexArray::kColorAttribute
};

const ArrayTuple::Type kAttributeComponentTypes[] =
{
	ArrayTuple::kFloat,
	ArrayTuple::kFloat,
	ArrayTuple::kUnsignedByte,
};

const int kNumAttributes = sizeof( kAttributeNumComponents ) / sizeof( kAttributeNumComponents[0] );

ImageGroupObject::ImageGroupObject(
	Rtt_Allocator *pAllocator,
	StageObject *canvas,
	const AutoPtr< ImageSheet >& sheet )
:	Super( pAllocator, canvas ),
	fSheet( sheet ),
	fPaint( Paint::NewBitmap( pAllocator, sheet ) ),
	fArray( pAllocator, kAttributeComponentTypes, kAttributeNumComponents, kAttributeTypes, kNumAttributes, Rendering::kTriangle )
{
	SetProperty( kIsUsingVertexArray, true );

	// ImageGroup has paint to set texture
	// Each child object also has paint that specifies which frame in sheet
	// to use for proper texture coord generation
}

ImageGroupObject::~ImageGroupObject()
{
	Rtt_DELETE( fPaint );
}

// Optimizations:
// * Remove BuildStageBounds() from groups (lazily build)
//   + requires changes to HitTestStream so always traverse children of group (no rect intersect test)
// * Add RenderingStream::GetScreenContentBounds() and during Build(), update kIsVisibleOnscreen
// * Modify VertexArrayStream to use Set instead of Append
//   + requires concept of current index
// * HitTestStream of ImageGroupObject --- not efficient unless bypasses rendering optimization
//    and calls children Draw().  OR: Modify VertexArrayStream() so it's a struct of arrays instead
//    of an array of structs.
void
ImageGroupObject::Build()
{
	Super::Build();

	// Only transform vertices if the object is hit-testable
	if ( ShouldHitTest() )
	{
		// Don't bother optimizing invalidation. 
		// The real cost is paid for by the overhead of culling offscreen children, 
		// O(# children). At this point, we're dealing with O(# objects on-screen).
		fArray.Empty();

		const Matrix& xform = GetSrcToDstMatrix();

		VertexArrayStream stream( & xform, fArray );
		Super::Draw( stream );
	}
}

// Translate is the only operation where we can update all data _without_
// a rebuild. All other ops will trigger a rebuild
void
ImageGroupObject::Translate( Real dx, Real dy )
{
//	Rtt_ASSERT_NOT_IMPLEMENTED();
	Super::Translate( dx, dy );
}

void
ImageGroupObject::Draw( RenderingStream& rStream ) const
{
	if ( ShouldDraw() )
	{
		rStream.BeginPaint( fPaint );

		// Submit all vertices at once
		rStream.Submit( fArray );

		rStream.EndPaint();
	}
}

const LuaProxyVTable&
ImageGroupObject::ProxyVTable() const
{
	return LuaImageGroupObjectProxyVTable::Constant();
}

void
ImageGroupObject::DidInsert( bool childParentChanged )
{
	Invalidate( true );
}

void
ImageGroupObject::DidRemove()
{
	Invalidate( true );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif