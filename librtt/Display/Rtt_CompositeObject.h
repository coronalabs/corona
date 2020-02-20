//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_CompositeObject_H__
#define _Rtt_CompositeObject_H__

#include "Display/Rtt_DisplayObject.h"

#include "Renderer/Rtt_RenderData.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class FrameBufferObject;
class Shader;

// ----------------------------------------------------------------------------

class CompositeObject : public DisplayObject
{
	Rtt_CLASS_NO_COPIES( CompositeObject )

	public:
		typedef DisplayObject Super;
		typedef CompositeObject Self;

	public:
		typedef enum _Input
		{
			kForeground,
			kBackground,

			kNumInputs
		}
		Input;

	public:
		CompositeObject( Rtt_Allocator* pAllocator );
		virtual ~CompositeObject();

	public:
		// MDrawable
		virtual bool UpdateTransform( const Matrix& parentToDstSpace );
		virtual void Prepare( const Display& display );
		virtual void Draw( Renderer& renderer ) const;

	public:
		virtual bool CanCull() const;

	protected:
		virtual void DidInsert( bool childParentChanged );
		virtual void DidRemove();

	public:
		void SetInput( Input input, DisplayObject *child );
		DisplayObject *ReleaseInput( Input input );

		void SetBlend( RenderTypes::BlendType newValue );
		RenderTypes::BlendType GetBlend() const;

		const Shader *GetShader() const { return fShader; }
		void SetShader( const Shader *newValue );

	private:
		RenderData fData;
		const Shader *fShader;
		DisplayObject *fInputs[kNumInputs];
		FrameBufferObject *fFBOs[kNumInputs];
		mutable bool fDirty;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_CompositeObject_H__
