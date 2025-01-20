//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#if 0

#ifndef __Rtt_ImageGroupObject__
#define __Rtt_ImageGroupObject__

#include "Rtt_GroupObject.h"

#include "Core/Rtt_AutoPtr.h"
#include "Rtt_VertexArray.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class BitmapPaint;
class ImageSheet;

// ----------------------------------------------------------------------------

class ImageGroupObject : public GroupObject
{
	public:
		typedef GroupObject Super;
		typedef ImageGroupObject Self;

	public:
		static bool IsCompatibleParent(
			CompositeObject *parent, const AutoPtr< ImageSheet >& sheet );

	public:
		ImageGroupObject(
			Rtt_Allocator *pAllocator,
			StageObject *canvas,
			const AutoPtr< ImageSheet >& sheet );
		virtual ~ImageGroupObject();

	public:
		// MDrawable
		virtual void Build();
		virtual void Translate( Real dx, Real dy );
		virtual void Draw( RenderingStream& rStream ) const;

	public:
		virtual const LuaProxyVTable& ProxyVTable() const;

	protected:
		virtual void DidInsert( bool childParentChanged );
		virtual void DidRemove();

	public:
		const AutoPtr< ImageSheet >& GetSheet() const { return fSheet; }

	private:
		AutoPtr< ImageSheet > fSheet;
		BitmapPaint *fPaint;
		VertexArray fArray;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_ImageGroupObject__

#endif