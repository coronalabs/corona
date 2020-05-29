//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LineObject_H__
#define _Rtt_LineObject_H__

#include "Core/Rtt_Real.h"
#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_OpenPath.h"
#include "Display/Rtt_Paint.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Paint;
class OpenPath;
class Shader;

// ----------------------------------------------------------------------------

class LineObject : public DisplayObject
{
	Rtt_CLASS_NO_COPIES( LineObject )

	public:
		typedef DisplayObject Super;

	public:
		static LineObject* NewLine( Rtt_Allocator*, Real xStart, Real yStart, Real xEnd, Real yEnd );

	public:
		LineObject( OpenPath* path );
		~LineObject();

	public:
		// MDrawable
		virtual bool UpdateTransform( const Matrix& parentToDstSpace );
		virtual void Prepare( const Display& display );
//		virtual void Translate( Real deltaX, Real deltaY );
		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBounds( Rect& rect ) const;

	public:
		virtual const LuaProxyVTable& ProxyVTable() const;

	public:
		// Controls whether the transform is offset by the anchor
		virtual void SetAnchorSegments( bool should_anchor );
		virtual bool ShouldOffsetWithAnchor() const;

	public:
		OpenPath& GetPath() { return * fPath; }
		const OpenPath& GetPath() const { return * fPath; }

		void SetStroke( Paint* newValue );
		void SetStrokeColor( Color newValue );

		Rtt_INLINE Real GetStrokeWidth() const { return fPath->GetWidth(); }
		void SetStrokeWidth( Real newValue );

		void Append( const Vertex2& p );

    public:
        void SetBlend( RenderTypes::BlendType newValue );
        RenderTypes::BlendType GetBlend() const;
    
	private:
		RenderData fStrokeData;
		Color fShaderColor;
		OpenPath* fPath;

		const Shader *fStrokeShader; // Temporary

		bool fAnchorSegments;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LineObject_H__
