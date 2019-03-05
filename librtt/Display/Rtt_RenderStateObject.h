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
#include "Renderer/Rtt_CustomCommand.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class BooleanCommand : public CustomCommand {
public:
	typedef CustomCommand Super;
	typedef void (*PreRenderFunc)( Renderer&, bool* );
	typedef void (*RenderFunc)( Renderer&, const bool* );
	typedef int (*FlagFunc)( const Renderer&, const bool*, const bool* );

	BooleanCommand( const bool* bools, bool* prev, PreRenderFunc preRender, RenderFunc render, FlagFunc flags );

	virtual int GetFlags( const Renderer& renderer );
	virtual void Render( Renderer& renderer );

private:
	const bool* fBools;
	mutable bool* fPrev;
	PreRenderFunc fPreRender;
	RenderFunc fRender;
	FlagFunc fGetFlags;
};

class IntCommand : public CustomCommand {
public:
	typedef CustomCommand Super;
	typedef void (*PreRenderFunc)( Renderer&, S32* );
	typedef void (*RenderFunc)( Renderer&, const S32* );
	typedef int (*FlagFunc)( const Renderer&, const S32*, const S32* );

	IntCommand( const S32* ints, S32* prev, PreRenderFunc preRender, RenderFunc render, FlagFunc flags );

	virtual int GetFlags( const Renderer& renderer );
	virtual void Render( Renderer& renderer );

private:
	const S32* fInts;
	mutable S32* fPrev;
	PreRenderFunc fPreRender;
	RenderFunc fRender;
	FlagFunc fGetFlags;
};

class RealCommand : public CustomCommand {
public:
	typedef CustomCommand Super;
	typedef void (*PreRenderFunc)( Renderer&, Real* );
	typedef void (*RenderFunc)( Renderer&, const Real* );
	typedef int (*FlagFunc)( const Renderer&, const Real*, const Real* );

	RealCommand( const Real* reals, Real* prev, PreRenderFunc preRender, RenderFunc render, FlagFunc flags );

	virtual int GetFlags( const Renderer& renderer );
	virtual void Render( Renderer& renderer );

private:
	const Real* fReals;
	mutable Real* fPrev;
	PreRenderFunc fPreRender;
	RenderFunc fRender;
	FlagFunc fGetFlags;
};

class UintCommand : public CustomCommand {
public:
	typedef CustomCommand Super;
	typedef void (*PreRenderFunc)( Renderer&, U32* );
	typedef void (*RenderFunc)( Renderer&, const U32* );
	typedef int (*FlagFunc)( const Renderer&, const U32*, const U32* );

	UintCommand( const U32* ints, U32* prev, PreRenderFunc preRender, RenderFunc render, FlagFunc flags );

	virtual int GetFlags( const Renderer& renderer );
	virtual void Render( Renderer& renderer );

private:
	const U32* fUints;
	mutable U32* fPrev;
	PreRenderFunc fPreRender;
	RenderFunc fRender;
	FlagFunc fGetFlags;
};

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
			kInt2Uint,
			kInt3,
			kInt4,
			kReal,
			kReal2,
			// kReal16 / 32? (matrices)
			kUint
		}
		UsedKind;

		typedef enum _BooleanState : int
		{
			kCullFaceEnable,
			kDepthMask,
			kDepthTestEnable,
			kScissorEnable,
			kStencilEnable
		}
		BooleanState;

		typedef enum _BooleanState4 : int
		{
			kColorMask
		}
		Boolean4State;

		typedef enum _IntState : int
		{
			kClearStencil,
			kCullFace,
			kDepthFunc
		}
		IntState;

		typedef enum _Int2UintState : int
		{
			kStencilFunc
		}
		Int2UintState;

		typedef enum _Int3State
		{
			kStencilOp
		}
		Int3State;

		typedef enum _Int4State
		{
			kScissor,
			kViewport
		}
		Int4State;

		typedef enum _RealState : int
		{
			kClearDepth
		}
		RealState;

		typedef enum _Real2State : int
		{
			kDepthRange
		}
		Real2State;

		typedef enum _UintState
		{
			kStencilMask
		}
		UintState;

	private:
		union UsedValue {
			bool bvalues[4];
			S32 ivalues[4];
			Real rvalues[2];
			U32 uvalues[3];
		};

		struct StateCommands {
			CustomCommand* func;
			CustomCommand* cleanup;
			UsedValue value;
			UsedValue previous;
			UsedKind kind;
			int state;
		};

		const StateCommands* Find( UsedKind kind, int state ) const;
		StateCommands* Find( UsedKind kind, int state );

	public:
		static RenderStateObject* NewRenderState( Rtt_Allocator* );

	public:
		RenderStateObject( Rtt_Allocator* );
		~RenderStateObject();

		static S32 IndexForStencilFunc( const char *name );
		static S32 IndexForStencilOpAction( const char *name );
		static const char *StencilFuncForIndex( S32 index );
		static const char *StencilOpActionForIndex( S32 index );

	public:
		// MDrawable
		virtual bool UpdateTransform( const Matrix& parentToDstSpace );
		virtual void Prepare( const Display& display );
//		virtual void Translate( Real deltaX, Real deltaY );
		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBounds( Rect& rect ) const;

		virtual bool CanCull() const { return false; }
		virtual bool CanHitTest() const { return false; }

	public:
		virtual const LuaProxyVTable& ProxyVTable() const;

	public:
		void Clear();

		void SetBooleanState( BooleanState state, bool b );
		void SetBoolean4State( Boolean4State state, bool b1, bool b2, bool b3, bool b4 );

		void SetIntState( IntState state, S32 i );
		void SetInt2UintState( Int2UintState state, S32 i1, S32 i2, U32 u );
		void SetInt3State( Int3State state, S32 i1, S32 i2, S32 i3 );
		void SetInt4State( Int4State state, S32 i1, S32 i2, S32 i3, S32 i4 );

		void SetRealState( RealState state, Real r );
		void SetReal2State( Real2State state, Real r1, Real r2 );
    
		void SetUintState( UintState state, U32 i );

		bool GetBooleanState( BooleanState state, bool& b ) const;
		bool GetBoolean4State( Boolean4State state, bool& b1, bool& b2, bool& b3, bool& b4 ) const;

		bool GetIntState( IntState state, S32& i ) const;
		bool GetInt2UintState( Int2UintState state, S32& i1, S32& i2, U32& u ) const;
		bool GetInt3State( Int3State state, S32& i1, S32& i2, S32& i3 ) const;
		bool GetInt4State( Int4State state, S32& i1, S32& i2, S32& i3, S32& i4 ) const;

		bool GetRealState( RealState state, Real& r ) const;
		bool GetReal2State( Real2State state, Real& r1, Real& r2 ) const;
    
		bool GetUintState( UintState state, U32& i ) const;

		void Remove( UsedKind kind, int state );

	private:
		StateCommands* AddCommands( UsedKind kind, int state );
		StateCommands* AddBooleanCommands( UsedKind kind, int state, BooleanCommand::PreRenderFunc preRender, BooleanCommand::RenderFunc render, BooleanCommand::FlagFunc flags );
		StateCommands* AddIntCommands( UsedKind kind, int state, IntCommand::PreRenderFunc preRender, IntCommand::RenderFunc render, IntCommand::FlagFunc flags );
		StateCommands* AddRealCommands( UsedKind kind, int state, RealCommand::PreRenderFunc preRender, RealCommand::RenderFunc render, RealCommand::FlagFunc flags );
		StateCommands* AddUintCommands( UsedKind kind, int state, UintCommand::PreRenderFunc preRender, UintCommand::RenderFunc render, UintCommand::FlagFunc flags );
	private:
		Rtt_Allocator* fAllocator;
		Array<StateCommands> fCommands;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_RenderStateObject_H__
