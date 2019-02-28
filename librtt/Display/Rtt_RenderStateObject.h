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

#ifndef _Rtt_RenderStateObject_H__
#define _Rtt_RenderStateObject_H__

#include "Core/Rtt_Array.h"
#include "Core/Rtt_Real.h"
#include "Display/Rtt_DisplayObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class CustomCommand;

// ----------------------------------------------------------------------------

class RenderStateObject : public DisplayObject
{
	Rtt_CLASS_NO_COPIES( RenderStateObject )

	public:
		typedef DisplayObject Super;

		typedef enum _UsedKind
		{
			kBoolean,
			kBoolean4,
			kInt,
			kInt3,
			kReal,
			kReal2,
			kReal4,

			kNumKinds
		}
		UsedKind;

		typedef enum _BooleanState : int
		{
			kCullFaceEnable,
			kDepthMask,
			kDepthTestEnable,
			kDitherEnable,
			kScissorEnable,
			kStencilEnable,

			kNumBooleanStates
		}
		BooleanState;

		typedef enum _BooleanState4 : int
		{
			kColorMask,

			kNumBoolean4States
		}
		Boolean4State;

		typedef enum _IntState : int
		{
			kClearStencil,
			kCullFace,
			kDepthFunc,
			kStencilMask,

			kNumIntStates
		}
		IntState;

		typedef enum _Int3State
		{
			kStencilFunc,
			kStencilOp,

			kNumInt3States
		}
		Int3State;

		typedef enum _RealState : int
		{
			kClearDepth,

			kNumRealStates
		}
		RealState;

		typedef enum _Real2State : int
		{
			kDepthRange,

			kNumReal2States
		}
		Real2State;

		typedef enum _Real4State : int
		{
			kScissor,
			kViewport,

			kNumReal4States
		}
		Real4State;

	private:
		union UsedValue {
			bool bvalues[4];
			int ivalue;
			Real rvalues[4];
		};

		struct StateCommands {
			CustomCommand* func;
			CustomCommand* cleanup;
			UsedValue value;
			UsedKind kind;
			int state;
		};

		StateCommands* Find( UsedKind kind, int state );

	public:
		static RenderStateObject* NewRenderState( Rtt_Allocator* );

	public:
		RenderStateObject( Rtt_Allocator* );
		~RenderStateObject();

	public:
		// MDrawable
		virtual bool UpdateTransform( const Matrix& parentToDstSpace );
		virtual void Prepare( const Display& display );
//		virtual void Translate( Real deltaX, Real deltaY );
		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBounds( Rect& rect ) const;

		virtual bool CanCull() const { return false; }

	public:
		virtual const LuaProxyVTable& ProxyVTable() const;

	public:
		void Clear();

		void SetBooleanState( BooleanState state, bool b );
		void SetBoolean4State( Boolean4State state, bool b1, bool b2, bool b3, bool b4 );

		void SetIntState( IntState state, int i );
		void SetInt3State( Int3State state, int i1, int i2, int i3 );

		void SetRealState( RealState state, Real r );
		void SetReal2State( Real2State state, Real r1, Real r2, Real r3 );
		void SetReal4State( Real4State state, Real r1, Real r2, Real r3, Real r4 );
    
	private:
		Array<StateCommands> fCommands;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_RenderStateObject_H__
