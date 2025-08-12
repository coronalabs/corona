//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShapeObject.h"

#include "Core/Rtt_AutoPtr.h"
#include "Display/Rtt_ImageSheetPaint.h"
#include "Display/Rtt_ClosedPath.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Rtt_LuaProxyVTable.h"

#include "Renderer/Rtt_Renderer.h"

#include "Display/Rtt_BitmapMask.h"
#include "Display/Rtt_GroupObject.h"
#include "Display/Rtt_ImageFrame.h"
#include "Display/Rtt_ImageSheet.h"
#include "Rtt_LuaUserdataProxy.h"
#include "Rtt_Profiling.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ShapeObject::ShapeObject( ClosedPath* path )
:	Super(),
	fFillData(),
	fStrokeData(),
	fPath( path ),
	fFillShader( NULL ),
	fStrokeShader( NULL )
{
	Rtt_ASSERT( fPath );

	fPath->SetObserver( this );

    SetObjectDesc( "ShapeObject" );
}

ShapeObject::~ShapeObject()
{
	Rtt_DELETE( fPath );
}

bool
ShapeObject::UpdateTransform( const Matrix& parentToDstSpace )
{
	bool shouldUpdate = Super::UpdateTransform( parentToDstSpace );

	SUMMED_TIMING( sut, "ShapeObject: post-Super::UpdateTransform" );

	if ( shouldUpdate )
	{
		fPath->Invalidate( ClosedPath::kFill | ClosedPath::kStroke );
	}

	return shouldUpdate;
}

void
ShapeObject::Prepare( const Display& display )
{
	Super::Prepare( display );

	SUMMED_TIMING( sp, "ShapeObject: post-Super::Prepare" );

	if ( ShouldPrepare() )
	{
		// Vertices
		Rtt_ASSERT( fPath );

		fPath->SetStrokeData( & fStrokeData );
		{
			// NOTE: We need to update paint *prior* to geometry
			// b/c in the case of image sheets, the paint needs to be updated
			// in order for the texture coordinates to be updated.
			if ( ! IsValid( kPaintFlag ) )
			{
				fPath->UpdatePaint( fFillData );
				SetValid( kPaintFlag );
			}

			if ( ! IsValid( kGeometryFlag ) )
			{
				const Matrix& xform = GetSrcToDstMatrix();
				fPath->Update( fFillData, xform );
				SetValid( kGeometryFlag );
			}

			if ( ! IsValid( kColorFlag ) )
			{
				fPath->UpdateColor( fFillData, AlphaCumulative() );
				SetValid( kColorFlag );
			}

			if ( ! IsValid( kProgramDataFlag ) )
			{
				SetValid( kProgramDataFlag );
			}
		}
		fPath->SetStrokeData( NULL );

		// Program
		if ( ! IsValid( kProgramFlag ) )
		{
			Rect bounds;
			fPath->GetSelfBounds( bounds );
			int w = Rtt_RealToInt( bounds.Width() );
			int h = Rtt_RealToInt( bounds.Height() );

			ShaderFactory& factory = display.GetShaderFactory();

			Paint *fill = fPath->GetFill();
			if ( fill )
			{
				Shader *shader = fill->GetShader(factory);
				ShaderResource::ProgramMod mod = GetProgramMod();
				shader->Prepare( fFillData, w, h, mod );
				fFillShader = shader;

//				shader->Log("", false);
//				const Shader *shader = factory.FindOrLoad( fill->GetShaderName() );
//				Program *program = const_cast< Program * >( shader->GetProgram() );
//				fFillData.fProgram = program;
			}

			Paint *stroke = fPath->GetStroke();
			if ( stroke )
			{
				Shader *shader = stroke->GetShader(factory);
				ShaderResource::ProgramMod mod = GetProgramMod();
				shader->Prepare( fStrokeData, w, h, mod );
				fStrokeShader = shader;
//				const Shader *shader = factory.FindOrLoad( stroke->GetShaderName() );
//				Program *program = const_cast< Program * >( shader->GetProgram() );
//				fStrokeData.fProgram = program;
			}

			SetValid( kProgramFlag );
		}
	}
}

void
ShapeObject::Draw( Renderer& renderer ) const
{
	if ( ShouldDraw() )
	{
		Rtt_ASSERT( fPath );

		SUMMED_TIMING( sd, "ShapeObject: Draw" );

		fPath->UpdateResources( renderer );

		if ( fPath->IsFillVisible() )
		{
			fFillShader->Draw( renderer, fFillData );
		}

		if ( fPath->IsStrokeVisible() )
		{
			fStrokeShader->Draw( renderer, fStrokeData );
		}
	}
	
}

void
ShapeObject::GetSelfBounds( Rect& rect ) const
{
	fPath->GetSelfBounds( rect );
}

bool
ShapeObject::GetTrimmedFrameOffset( Real & deltaX, Real & deltaY, bool force ) const
{
	const Paint *paint = GetPath().GetFill();
	if ( paint && paint->IsType( Paint::kImageSheet ) )
	{
		const ImageSheetPaint *bitmap = (const ImageSheetPaint *)paint;
		const AutoPtr< ImageSheet >& sheet = bitmap->GetSheet();
		if ( AutoPtr< ImageSheet >::Null() != sheet && (force || sheet->CorrectsTrimOffsets()) )
		{
			int index = bitmap->GetFrame(); Rtt_ASSERT( index >= 0 );
			const ImageFrame *frame = sheet->GetFrame( index );

			if ( frame->IsTrimmed() )
			{
				deltaX = frame->GetOffsetX();
				deltaY = frame->GetOffsetY();

				return true;
			}
		}
	}

	return false;
}

bool
ShapeObject::HitTest( Real contentX, Real contentY )
{
	Rtt_ASSERT( ShouldDraw() );
	Rtt_ASSERT( ShouldHitTest() );

	bool result = false;
	
	if ( fPath->HasFill()
		 && ( fPath->IsFillVisible() || IsHitTestable() ) )
	{
		Rtt_ASSERT( fFillData.fGeometry );
		result = fFillData.fGeometry->HitTest( contentX, contentY );
	}

	if ( ! result
		 && fPath->HasStroke()
		 && ( fPath->IsStrokeVisible() || IsHitTestable() ) )
	{
		Rtt_ASSERT( fStrokeData.fGeometry );
		result = fStrokeData.fGeometry->HitTest( contentX, contentY );
	}

	return result;
}

void
ShapeObject::DidUpdateTransform( Matrix& srcToDst )
{
	Real dx, dy;
	if (GetTrimmedFrameOffset( dx, dy, true ))
	{
		Matrix t;
		t.Translate( dx, dy );
		srcToDst.Concat( t );
	}
}

ShaderResource::ProgramMod
ShapeObject::GetProgramMod() const
{
	return ShaderResource::kDefault;
}

const LuaProxyVTable&
ShapeObject::ProxyVTable() const
{
	return LuaShapeObjectProxyVTable::Constant();
}

void
ShapeObject::SetSelfBounds( Real width, Real height )
{
	if ( GetPath().SetSelfBounds( width, height ) )
	{
		// Changing bounds should not invalidate the transform matrix
		Invalidate( kGeometryFlag | kStageBoundsFlag | kTransformFlag );
	}
	else
	{
		Super::SetSelfBounds( width, height );
	}
}

void
ShapeObject::DidSetMask( BitmapMask *mask, Uniform *uniform )
{
	Texture *maskTexture = ( mask ? mask->GetPaint()->GetTexture() : NULL );

	fFillData.fMaskTexture = maskTexture;
	fFillData.fMaskUniform = uniform;
	fStrokeData.fMaskTexture = maskTexture;
	fStrokeData.fMaskUniform = uniform;
}

void
ShapeObject::SetFill( Paint* newValue )
{
	DirtyFlags flags = ( kPaintFlag | kProgramFlag );
	if ( Paint::ShouldInvalidateColor( fPath->GetFill(), newValue ) )
	{
		flags |= kColorFlag;
	}

	if ( newValue && NULL == fPath->GetFill() )
	{
		// When paint goes from NULL to non-NULL,
		// ensure geometry is prepared
		flags |= kGeometryFlag;
	}

	if ( (newValue && newValue->AsPaint(Paint::kGradient)) ||
		 (fPath->GetFill() && fPath->GetFill()->AsPaint(Paint::kGradient)) )
	{
		// Gradient use UVs for it's direction
		flags |= kGeometryFlag;
	}

	if ( (newValue         && newValue->AsPaint(Paint::kImageSheet)) ||
         (fPath->GetFill() && fPath->GetFill()->AsPaint(Paint::kImageSheet)) )
	{
		//UVs are all wrecked if it was ImageSheet 
		flags |= kGeometryFlag;
		fPath->Invalidate(ClosedPath::kFillSourceTexture);
	}
	
	Invalidate( flags );

	fPath->SetFill( newValue );

	DidChangePaint( fFillData );
}

void
ShapeObject::SetFillColor( Color newValue )
{
	Paint *paint = GetPath().GetFill();
	if ( paint )
	{
		Rtt_ASSERT( paint->GetObserver() == this );
		paint->SetColor( newValue );
		Invalidate( kGeometryFlag | kColorFlag );
	}
}

void
ShapeObject::SetStroke( Paint* newValue )
{
	DirtyFlags flags = ( kPaintFlag | kProgramFlag );
	if ( Paint::ShouldInvalidateColor( fPath->GetStroke(), newValue ) )
	{
		flags |= kColorFlag;
	}
	if ( newValue && NULL == fPath->GetStroke() )
	{
		// When paint goes from NULL to non-NULL,
		// ensure geometry is prepared
		flags |= kGeometryFlag;
	}
	Invalidate( flags );

	fPath->SetStroke( newValue );

	DidChangePaint( fStrokeData );
}

void
ShapeObject::SetStrokeColor( Color newValue )
{
	Paint *paint = GetPath().GetStroke();
	if ( paint )
	{
		Rtt_ASSERT( paint->GetObserver() == this );
		paint->SetColor( newValue );
		Invalidate( kGeometryFlag | kColorFlag );
	}
}

U8
ShapeObject::GetStrokeWidth() const
{
	return fPath->GetInnerStrokeWidth() + fPath->GetOuterStrokeWidth();
}

void
ShapeObject::SetInnerStrokeWidth( U8 newValue )
{
	fPath->SetInnerStrokeWidth( newValue );
	Super::Invalidate( kGeometryFlag | kStageBoundsFlag );
}

U8
ShapeObject::GetInnerStrokeWidth() const
{
	return fPath->GetInnerStrokeWidth();
}

void
ShapeObject::SetOuterStrokeWidth( U8 newValue )
{
	fPath->SetOuterStrokeWidth( newValue );
	Super::Invalidate( kGeometryFlag | kStageBoundsFlag );
}

U8
ShapeObject::GetOuterStrokeWidth() const
{
	return fPath->GetOuterStrokeWidth();
}

void
ShapeObject::SetBlend( RenderTypes::BlendType newValue )
{
	Paint *paint = fPath->GetFill();
	if ( paint )
	{
		Rtt_ASSERT( paint->GetObserver() == this );
		paint->SetBlend( newValue );
	}

	paint = fPath->GetStroke();
	if ( paint )
	{
		Rtt_ASSERT( paint->GetObserver() == this );
		paint->SetBlend( newValue );
	}
}

RenderTypes::BlendType
ShapeObject::GetBlend() const
{
	const Paint *paint = fPath->GetFill();

	// Either there's no stroke or (if there is one), it's blend type should match the fill's
	Rtt_ASSERT( ! fPath->GetStroke() || paint->GetBlend() == fPath->GetStroke()->GetBlend() );

    if (paint == NULL)
    {
        return RenderTypes::kNormal; // sensible default
    }
    else
    {
        return paint->GetBlend();
    }
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

