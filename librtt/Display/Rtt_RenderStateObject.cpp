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

//#include "Core/Rtt_Build.h"

#include "Rtt_RenderStateObject.h"
/*
#include "Display/Rtt_Display.h"
#include "Display/Rtt_OpenPath.h"
#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderFactory.h"
*/
#include "Renderer/Rtt_Renderer.h"
#include "Rtt_LuaProxyVTable.h"

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
	fCommands( allocator )
{
    SetObjectDesc("RenderStateObject"); // for introspection
}

RenderStateObject::~RenderStateObject()
{
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
	S32 length = fCommands.Length();

	if (length)
	{
		// CheckAndDraw()...
	}

	CommandStack* stack = renderer.GetCommandStack();

	Rtt_ASSERT( stack );

	for (S32 i = 0; i < length; ++i)
	{
		const StateCommands& command = fCommands[i];

		// TODO: derive from something with Dirty() check
		// if !dirty, continue

		command.func->Render( renderer );

		if (command.cleanup)
		{
			stack->Push( command.cleanup );
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
	return LuaLineObjectProxyVTable::Constant(); // TODO: LuaRenderStateObjectProxyVTable
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

RenderStateObject::StateCommands *
RenderStateObject::Find( UsedKind kind, int state )
{
	StateCommands* commands = fCommands.WriteAccess();

	for (S32 i = 0, length; i < length; ++i)
	{
		if (commands[i].kind == kind && commands[i].state == state)
		{
			return &commands[i];
		}
	}

	return NULL;
}

void
RenderStateObject::SetBooleanState( BooleanState state, bool b )
{
	// TODO: find
	// if not there
		// allocate along with any cleanup
	// update
}

void
RenderStateObject::SetBoolean4State( Boolean4State state, bool b1, bool b2, bool b3, bool b4 )
{
}

void
RenderStateObject::SetIntState( IntState state, int i )
{
}

void
RenderStateObject::SetInt3State( Int3State state, int i1, int i2, int i3 )
{
}

void
RenderStateObject::SetRealState( RealState state, Real r )
{
}

void
RenderStateObject::SetReal2State( Real2State state, Real r1, Real r2, Real r3 )
{
}

void
RenderStateObject::SetReal4State( Real4State state, Real r1, Real r2, Real r3, Real r4 )
{
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
