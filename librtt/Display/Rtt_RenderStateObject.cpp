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

#include "Display/Rtt_Display.h"
#include "Display/Rtt_RenderStateObject.h"
#include "Renderer/Rtt_CommandBuffer.h"
#include "Renderer/Rtt_Renderer.h"
#include "Rtt_LuaProxyVTable.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

RenderStateObject*
RenderStateObject::NewRenderState( Rtt_Allocator* pAllocator )
{
	return Rtt_NEW( pAllocator, RenderStateObject( pAllocator ) );
}

// ----------------------------------------------------------------------------

RenderStateObject::RenderStateObject( Rtt_Allocator* allocator )
:	Super(),
	fAllocator( allocator ),
	fCommands( allocator )
{
    SetObjectDesc("RenderStateObject"); // for introspection
}

RenderStateObject::~RenderStateObject()
{
}

S32
RenderStateObject::IndexForStencilFunc( const char *name )
{
	return CommandBuffer::GetStencilFuncForName( name );
}

S32
RenderStateObject::IndexForStencilOpAction( const char *name )
{
	return CommandBuffer::GetStencilOpActionForName( name );
}

const char *
RenderStateObject::StencilFuncForIndex( S32 index )
{
	return CommandBuffer::GetNameForStencilFunc( index );
}

const char *
RenderStateObject::StencilOpActionForIndex( S32 index )
{
	return CommandBuffer::GetNameForStencilOpAction( index );
}

bool
RenderStateObject::UpdateTransform( const Matrix& parentToDstSpace )
{
	return false;
}

void
RenderStateObject::Prepare( const Display& display )
{
//	Super::Prepare( display );

	const Renderer& renderer = display.GetRenderer();

	for (S32 i = 0, length = fCommands.Length(); i < length; ++i)
	{
		StateCommands& command = fCommands[i];

		command.func->Prepare( renderer );
	}
}

/*
void
RenderStateObject::Translate( Real dx, Real dy )
{
}
*/

void
RenderStateObject::Draw( Renderer& renderer ) const
{
	CommandStack* stack = renderer.GetCommandStack();

	Rtt_ASSERT( stack );

	for (S32 i = 0, length = fCommands.Length(); i < length; ++i)
	{
		const StateCommands& command = fCommands[i];

		if (command.func->GetFlags( renderer ) & CustomCommand::kDirtyFlag)
		{
			command.func->Render( renderer );

			if (command.cleanup && !stack->Contains( *command.cleanup,
				[]( const CustomCommand& c1, const CustomCommand& c2)
				{
					return c1.GetUserdata() == c2.GetUserdata();
				}
			) )
			{
				stack->Push( command.cleanup );
			}
		}
	}
}

void
RenderStateObject::GetSelfBounds( Rect& rect ) const
{
	rect.SetEmpty();
}

const LuaProxyVTable&
RenderStateObject::ProxyVTable() const
{
	return LuaRenderStateObjectProxyVTable::Constant();
}

void
RenderStateObject::Clear()
{
	for (S32 i = 0, length = fCommands.Length(); i < length; ++i)
	{
		StateCommands& command = fCommands[i];

		Rtt_DELETE( command.func );
		Rtt_DELETE( command.cleanup );
	}

	fCommands.Clear();
}

const RenderStateObject::StateCommands *
RenderStateObject::Find( UsedKind kind, int state ) const
{
	const StateCommands* commands = fCommands.ReadAccess();

	for (S32 i = 0, length = fCommands.Length(); i < length; ++i)
	{
		if (commands[i].kind == kind && commands[i].state == state)
		{
			return &commands[i];
		}
	}

	return NULL;
}

RenderStateObject::StateCommands *
RenderStateObject::Find( UsedKind kind, int state )
{
	StateCommands* commands = fCommands.WriteAccess();

	for (S32 i = 0, length = fCommands.Length(); i < length; ++i)
	{
		if (commands[i].kind == kind && commands[i].state == state)
		{
			return &commands[i];
		}
	}

	return NULL;
}

BooleanCommand::BooleanCommand( const bool* bools, bool* prev, PreRenderFunc preRender, RenderFunc render, FlagFunc flags )
:	Super(),
	fBools( bools ),
	fPrev( prev ),
	fPreRender( preRender ),
	fRender( render ),
	fGetFlags( flags )
{
}

int 
BooleanCommand::GetFlags( const Renderer& renderer )
{
	return fGetFlags( renderer, fBools, fPrev );
}

void 
BooleanCommand::Render( Renderer& renderer )
{
	if (fPreRender && fPrev)
	{
		fPreRender( renderer, fPrev );
	}

	fRender( renderer, fBools );
}

RenderStateObject::StateCommands *
RenderStateObject::AddCommands ( UsedKind kind, int state )
{
	StateCommands newCommand;

	newCommand.kind = kind;
	newCommand.state = state;
	newCommand.func = newCommand.cleanup = NULL;

	fCommands.Append( newCommand );

	return &fCommands[fCommands.Length() - 1];
}

RenderStateObject::StateCommands *
RenderStateObject::AddBooleanCommands( UsedKind kind, int state, BooleanCommand::PreRenderFunc preRender, BooleanCommand::RenderFunc render, BooleanCommand::FlagFunc flags )
{
	StateCommands* commands = AddCommands( kind, state );

	commands->func = Rtt_NEW( fAllocator, BooleanCommand( commands->value.bvalues, commands->previous.bvalues, preRender, render, flags ) );
	commands->cleanup = Rtt_NEW( fAllocator, BooleanCommand( commands->previous.bvalues, NULL, NULL, render, flags ) );

	commands->cleanup->SetUserdata( render );

	return commands;
}

void
RenderStateObject::SetBooleanState( BooleanState state, bool b )
{
	StateCommands* commands = Find( kBoolean, state );

	if (!commands)
	{
		BooleanCommand::FlagFunc flags = []( const Renderer&, const bool* bools1, const bool* bools2 ) {
			int flags = 0;

			if (memcmp( bools1, bools2, sizeof( bool ) ) != 0)
			{
				flags |= CustomCommand::kDirtyFlag;
			}

			return flags;
		};

		BooleanCommand::PreRenderFunc preRender = NULL;
		BooleanCommand::RenderFunc render = NULL;

		switch (state)
		{
		case kCullFaceEnable:
			break;
		case kDepthMask:
			break;
		case kDepthTestEnable:
			break;
		case kScissorEnable:
			preRender = []( Renderer& renderer, bool* prev )
			{
				prev[0] = renderer.GetScissorEnabled();
			};

			render = []( Renderer& renderer, const bool* bools )
			{
				renderer.SetScissorEnabled( bools[0] );
			};

			break;
		case kStencilEnable:
			preRender = []( Renderer& renderer, bool* prev )
			{
				prev[0] = renderer.GetStencilEnabled();
			};

			render = []( Renderer& renderer, const bool* bools )
			{
				renderer.SetStencilEnabled( bools[0] );
			};

			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
		}

		if (render)
		{
			commands = AddBooleanCommands( kBoolean, state, preRender, render, flags );
		}
	}

	if (commands)
	{
		commands->value.bvalues[0] = b;
	}
}

bool
RenderStateObject::GetBooleanState( BooleanState state, bool& b ) const
{
	const StateCommands* commands = Find( kBoolean, state );

	if (commands)
	{
		b = commands->value.bvalues[0];

		return true;
	}

	return false;
}

void
RenderStateObject::SetBoolean4State( Boolean4State state, bool b1, bool b2, bool b3, bool b4 )
{
	StateCommands* commands = Find( kBoolean4, state );

	if (!commands)
	{
		BooleanCommand::FlagFunc flags = []( const Renderer&, const bool* bools1, const bool* bools2 ) {
			int flags = 0;

			if (memcmp( bools1, bools2, sizeof( bool ) * 4U ) != 0)
			{
				flags |= CustomCommand::kDirtyFlag;
			}

			return flags;
		};
		
		BooleanCommand::PreRenderFunc preRender = NULL;
		BooleanCommand::RenderFunc render = NULL;

		switch (state)
		{
		case kColorMask:
			preRender = []( Renderer& renderer, bool* prev )
			{
				renderer.GetColorMask( prev[0], prev[1], prev[2], prev[3] );
			};

			render = []( Renderer& renderer, const bool* bools )
			{
				renderer.SetColorMask( bools[0], bools[1], bools[2], bools[3] );
			};

			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
		}

		if (render)
		{
			commands = AddBooleanCommands( kBoolean4, state, preRender, render, flags );
		}
	}

	if (commands)
	{
		commands->value.bvalues[0] = b1;
		commands->value.bvalues[1] = b2;
		commands->value.bvalues[2] = b3;
		commands->value.bvalues[3] = b4;
	}
}

bool
RenderStateObject::GetBoolean4State( Boolean4State state, bool& b1, bool& b2, bool& b3, bool& b4 ) const
{
	const StateCommands* commands = Find( kBoolean4, state );

	if (commands)
	{
		b1 = commands->value.bvalues[0];
		b2 = commands->value.bvalues[1];
		b3 = commands->value.bvalues[2];
		b4 = commands->value.bvalues[3];

		return true;
	}

	return false;
}


IntCommand::IntCommand( const S32* ints, S32* prev, PreRenderFunc preRender, RenderFunc render, FlagFunc flags )
:	Super(),
	fInts( ints ),
	fPrev( prev ),
	fPreRender( preRender ),
	fRender( render ),
	fGetFlags( flags )
{
}

int 
IntCommand::GetFlags( const Renderer& renderer )
{
	return fGetFlags( renderer, fInts, fPrev );
}

void 
IntCommand::Render( Renderer& renderer )
{
	if (fPreRender && fPrev)
	{
		fPreRender( renderer, fPrev );
	}

	fRender( renderer, fInts );
}

RenderStateObject::StateCommands *
RenderStateObject::AddIntCommands( UsedKind kind, int state, IntCommand::PreRenderFunc preRender, IntCommand::RenderFunc render, IntCommand::FlagFunc flags )
{
	StateCommands* commands = AddCommands( kind, state );

	commands->func = Rtt_NEW( fAllocator, IntCommand( commands->value.ivalues, commands->previous.ivalues, preRender, render, flags ) );

	if (preRender)
	{
		commands->cleanup = Rtt_NEW( fAllocator, IntCommand( commands->previous.ivalues, NULL, NULL, render, flags ) );
	}

	commands->cleanup->SetUserdata( render );

	return commands;
}

void
RenderStateObject::SetIntState( IntState state, S32 i )
{
	StateCommands* commands = Find( kInt, state );

	if (!commands)
	{
		IntCommand::FlagFunc flags = []( const Renderer&, const S32* ints1, const S32* ints2 ) {
			int flags = 0;

			if (memcmp( ints1, ints2, sizeof( S32 ) ) != 0)
			{
				flags |= CustomCommand::kDirtyFlag;
			}

			return flags;
		};
		
		IntCommand::PreRenderFunc preRender = NULL;
		IntCommand::RenderFunc render = NULL;

		switch (state)
		{
		case kClearStencil:
			// this is a command posing as a property, so not balanced with any pre-render and cleanup

			render = []( Renderer& renderer, const S32* ints )
			{
				renderer.ClearStencil( ints[0] );
			};

			break;
		case kCullFace:
			break;
		case kDepthFunc:
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
		}

		if (render)
		{
			commands = AddIntCommands( kInt, state, preRender, render, flags );
		}
	}

	if (commands)
	{
		commands->value.ivalues[0] = i;
	}
}

bool
RenderStateObject::GetIntState( IntState state, S32& i ) const
{
	const StateCommands* commands = Find( kInt, state );

	if (commands)
	{
		i = commands->value.ivalues[0];

		return true;
	}

	return false;
}

void
RenderStateObject::SetInt2UintState( Int2UintState state, S32 i1, S32 i2, U32 u )
{
	StateCommands* commands = Find( kInt2Uint, state );

	if (!commands)
	{
		IntCommand::FlagFunc flags = []( const Renderer&, const S32* ints1, const S32* ints2 ) {
			int flags = 0;

			if (memcmp( ints1, ints2, sizeof( S32 ) * 3U ) != 0)
			{
				flags |= CustomCommand::kDirtyFlag;
			}

			return flags;
		};

		IntCommand::PreRenderFunc preRender = NULL;
		IntCommand::RenderFunc render = NULL;

		switch (state)
		{
		case kStencilFunc:
			preRender = []( Renderer& renderer, S32* prev )
			{
				renderer.GetStencilFunc( prev[0], prev[1], *reinterpret_cast<U32*>(&prev[2]) );
			};

			render = []( Renderer& renderer, const S32* ints )
			{
				renderer.SetStencilFunc( ints[0], ints[1], *reinterpret_cast<const U32*>(&ints[2]) );
			};

			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
		}

		if (render)
		{
			commands = AddIntCommands( kInt2Uint, state, preRender, render, flags );
		}
	}

	if (commands)
	{
		commands->value.ivalues[0] = i1;
		commands->value.ivalues[1] = i2;
		commands->value.uvalues[2] = u;
	}
}

bool
RenderStateObject::GetInt2UintState( Int2UintState state, S32& i1, S32& i2, U32& u ) const
{
	const StateCommands* commands = Find( kInt2Uint, state );

	if (commands)
	{
		i1 = commands->value.ivalues[0];
		i2 = commands->value.ivalues[1];
		u = commands->value.uvalues[2];

		return true;
	}

	return false;
}

void
RenderStateObject::SetInt3State( Int3State state, S32 i1, S32 i2, S32 i3 )
{
	StateCommands* commands = Find( kInt3, state );

	if (!commands)
	{
		IntCommand::FlagFunc flags = []( const Renderer&, const S32* ints1, const S32* ints2 ) {
			int flags = 0;

			if (memcmp( ints1, ints2, sizeof( S32 ) * 3U ) != 0)
			{
				flags |= CustomCommand::kDirtyFlag;
			}

			return flags;
		};
		
		IntCommand::PreRenderFunc preRender = NULL;
		IntCommand::RenderFunc render = NULL;

		switch (state)
		{
		case kStencilOp:
			preRender = []( Renderer& renderer, S32* prev )
			{
				renderer.GetStencilOp( prev[0], prev[1], prev[2] );
			};

			render = []( Renderer& renderer, const S32* ints )
			{
				renderer.SetStencilOp( ints[0], ints[1], ints[2] );
			};

			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
		}

		if (render)
		{
			commands = AddIntCommands( kInt3, state, preRender, render, flags );
		}
	}

	if (commands)
	{
		commands->value.ivalues[0] = i1;
		commands->value.ivalues[1] = i2;
		commands->value.ivalues[2] = i3;
	}
}

bool
RenderStateObject::GetInt3State( Int3State state, S32& i1, S32& i2, S32& i3 ) const
{
	const StateCommands* commands = Find( kInt3, state );

	if (commands)
	{
		i1 = commands->value.ivalues[0];
		i2 = commands->value.ivalues[1];
		i3 = commands->value.ivalues[2];

		return true;
	}

	return false;
}

void
RenderStateObject::SetInt4State( Int4State state, S32 i1, S32 i2, S32 i3, S32 i4 )
{
	StateCommands* commands = Find( kInt4, state );

	if (!commands)
	{
		IntCommand::FlagFunc flags = []( const Renderer&, const S32* ints1, const S32* ints2 ) {
			int flags = 0;

			if (memcmp( ints1, ints2, sizeof( S32 ) * 4U ) != 0)
			{
				flags |= CustomCommand::kDirtyFlag;
			}

			return flags;
		};
		
		IntCommand::PreRenderFunc preRender = NULL;
		IntCommand::RenderFunc render = NULL;

		switch (state)
		{
		case kScissor:
			preRender = []( Renderer& renderer, S32* prev )
			{
				renderer.GetScissor( prev[0], prev[1], prev[2], prev[3] );
			};

			render = []( Renderer& renderer, const S32* ints )
			{
				renderer.SetScissor( ints[0], ints[1], ints[2], ints[3] );
			};

			break;
		case kViewport:
			preRender = []( Renderer& renderer, S32* prev )
			{
				renderer.GetViewport( prev[0], prev[1], prev[2], prev[3] );
			};

			render = []( Renderer& renderer, const S32* ints )
			{
				renderer.SetViewport( ints[0], ints[1], ints[2], ints[3] );
			};

			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
		}

		if (render)
		{
			commands = AddIntCommands( kInt4, state, preRender, render, flags );
		}
	}

	if (commands)
	{
		commands->value.ivalues[0] = i1;
		commands->value.ivalues[1] = i2;
		commands->value.ivalues[2] = i3;
		commands->value.ivalues[3] = i4;
	}
}

bool
RenderStateObject::GetInt4State( Int4State state, S32& i1, S32& i2, S32& i3, S32& i4 ) const
{
	const StateCommands* commands = Find( kInt4, state );

	if (commands)
	{
		i1 = commands->value.ivalues[0];
		i2 = commands->value.ivalues[1];
		i3 = commands->value.ivalues[2];
		i4 = commands->value.ivalues[3];

		return true;
	}

	return false;
}

RealCommand::RealCommand( const Real* reals, Real* prev, PreRenderFunc preRender, RenderFunc render, FlagFunc flags )
:	Super(),
	fReals( reals ),
	fPrev( prev ),
	fPreRender( preRender ),
	fRender( render ),
	fGetFlags( flags )
{
}

int 
RealCommand::GetFlags( const Renderer& renderer )
{
	return fGetFlags( renderer, fReals, fPrev );
}

void 
RealCommand::Render( Renderer& renderer )
{
	if (fPreRender && fPrev)
	{
		fPreRender( renderer, fPrev );
	}

	fRender( renderer, fReals );
}

RenderStateObject::StateCommands *
RenderStateObject::AddRealCommands( UsedKind kind, int state, RealCommand::PreRenderFunc preRender, RealCommand::RenderFunc render, RealCommand::FlagFunc flags )
{
	StateCommands* commands = AddCommands( kind, state );

	commands->func = Rtt_NEW( fAllocator, RealCommand( commands->value.rvalues, commands->previous.rvalues, preRender, render, flags ) );
	commands->cleanup = Rtt_NEW( fAllocator, RealCommand( commands->previous.rvalues, NULL, NULL, render, flags ) );

	commands->cleanup->SetUserdata( render );

	return commands;
}

void
RenderStateObject::SetRealState( RealState state, Real r )
{
	switch (state)
	{
	case kClearDepth:
		break;
	default:
		Rtt_ASSERT_NOT_REACHED();
	}
}

bool
RenderStateObject::GetRealState( RealState state, Real& r ) const
{
	const StateCommands* commands = Find( kReal, state );

	if (commands)
	{
		r = commands->value.rvalues[0];

		return true;
	}

	return false;
}

void
RenderStateObject::SetReal2State( Real2State state, Real r1, Real r2 )
{
	switch (state)
	{
	case kDepthRange:
		break;
	default:
		Rtt_ASSERT_NOT_REACHED();
	}
}

bool 
RenderStateObject::GetReal2State( Real2State state, Real& r1, Real& r2 ) const
{
	const StateCommands* commands = Find( kReal2, state );

	if (commands)
	{
		r1 = commands->value.rvalues[0];
		r2 = commands->value.rvalues[1];

		return true;
	}

	return false;
}

UintCommand::UintCommand( const U32* uints, U32* prev, PreRenderFunc preRender, RenderFunc render, FlagFunc flags )
:	Super(),
	fUints( uints ),
	fPrev( prev ),
	fPreRender( preRender ),
	fRender( render ),
	fGetFlags( flags )
{
}

int 
UintCommand::GetFlags( const Renderer& renderer )
{
	return fGetFlags( renderer, fUints, fPrev );
}

void 
UintCommand::Render( Renderer& renderer )
{
	if (fPreRender && fPrev)
	{
		fPreRender( renderer, fPrev );
	}

	fRender( renderer, fUints );
}

RenderStateObject::StateCommands *
RenderStateObject::AddUintCommands( UsedKind kind, int state, UintCommand::PreRenderFunc preRender, UintCommand::RenderFunc render, UintCommand::FlagFunc flags )
{
	StateCommands* commands = AddCommands( kind, state );

	commands->func = Rtt_NEW( fAllocator, UintCommand( commands->value.uvalues, commands->previous.uvalues, preRender, render, flags ) );
	commands->cleanup = Rtt_NEW( fAllocator, UintCommand( commands->previous.uvalues, NULL, NULL, render, flags ) );

	commands->cleanup->SetUserdata( render );

	return commands;
}

void
RenderStateObject::SetUintState( UintState state, U32 i )
{
	StateCommands* commands = Find( kUint, state );

	if (!commands)
	{
		UintCommand::FlagFunc flags = []( const Renderer&, const U32* uints1, const U32* uints2 ) {
			int flags = 0;

			if (memcmp( uints1, uints2, sizeof( U32 ) ) != 0)
			{
				flags |= CustomCommand::kDirtyFlag;
			}

			return flags;
		};
		
		UintCommand::PreRenderFunc preRender = NULL;
		UintCommand::RenderFunc render = NULL;

		switch (state)
		{
		case kStencilMask:
			preRender = []( Renderer& renderer, U32* prev )
			{
				prev[0] = renderer.GetStencilMask();
			};

			render = []( Renderer& renderer, const U32* uints )
			{
				renderer.SetStencilMask( uints[0] );
			};

			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
		}

		if (render)
		{
			commands = AddUintCommands( kUint, state, preRender, render, flags );
		}
	}

	if (commands)
	{
		commands->value.uvalues[0] = i;
	}
}
    
bool
RenderStateObject::GetUintState( UintState state, U32& i ) const
{
	const StateCommands* commands = Find( kUint, state );

	if (commands)
	{
		i = commands->value.uvalues[0];

		return true;
	}

	return false;
}

void
RenderStateObject::Remove( UsedKind kind, int state )
{
	StateCommands* commands = Find( kind, state );

	if (commands)
	{
		Rtt_DELETE( commands->func );
		Rtt_DELETE( commands->cleanup );

		S32 index = (S32)(commands - fCommands.WriteAccess());

		fCommands.Remove( index, 1, false );
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
