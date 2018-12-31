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

	public:
		// MDrawable
		virtual bool UpdateTransform( const Matrix& parentToDstSpace );
		virtual void Prepare( const Display& display );
		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBounds( Rect& rect ) const;

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
