//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShapeObject_H__
#define _Rtt_ShapeObject_H__

#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_ShaderResource.h"

#include "Core/Rtt_Real.h"
#include "Renderer/Rtt_RenderData.h"
#include "Renderer/Rtt_RenderTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class ClosedPath;
class LuaUserdataProxy;
class Paint;
class Shader;
class ShaderName;
class Shader;

// ----------------------------------------------------------------------------

// The ShapeObject is the base representation of all closed shapes drawn on
// the screen.
class ShapeObject : public DisplayObject
{
	Rtt_CLASS_NO_COPIES( ShapeObject )

	public:
		typedef ShapeObject Self;
		typedef DisplayObject Super;

	public:
		ShapeObject( ClosedPath *path );
		~ShapeObject();
	
		static bool IsShapeObject( const DisplayObject& object );

		const BitmapPaint* GetBitmapPaint() const;

	public:
		// MDrawable
		virtual bool UpdateTransform( const Matrix& parentToDstSpace );
		virtual void Prepare( const Display& display );
		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBounds( Rect& rect ) const;
		virtual bool GetTrimmedFrameOffset( Real & deltaX, Real & deltaY, bool force ) const;

	public:
		virtual bool HitTest( Real contentX, Real contentY );
		virtual void DidUpdateTransform( Matrix& srcToDst );
		virtual ShaderResource::ProgramMod GetProgramMod() const;

	public:
		virtual const LuaProxyVTable& ProxyVTable() const;
		virtual void SetSelfBounds( Real width, Real height );

	protected:
		virtual void DidSetMask( BitmapMask *mask, Uniform *uniform );
		const RenderData& GetFillData() const { return fFillData; }
		const Shader* GetFillShader() const { return fFillShader; }

	public:
		const ClosedPath& GetPath() const { return *fPath; }
		ClosedPath& GetPath() { return *fPath; }

	public:
		void SetFill( Paint* newValue );
		void SetFillColor( Color newValue );
		void SetStroke( Paint* newValue );
		void SetStrokeColor( Color newValue );

		U8 GetStrokeWidth() const;

		void SetInnerStrokeWidth( U8 newValue );
		U8 GetInnerStrokeWidth() const;

		void SetOuterStrokeWidth( U8 newValue );
		U8 GetOuterStrokeWidth() const;

	public:
		void SetBlend( RenderTypes::BlendType newValue );
		RenderTypes::BlendType GetBlend() const;

	private:
		RenderData fFillData;
		
		RenderData fStrokeData;
		ClosedPath *fPath;

		const Shader *fFillShader; // Temporary
		const Shader *fStrokeShader; // Temporary

//		mutable LuaUserdataProxy *fProxy;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShapeObject_H__
