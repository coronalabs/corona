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

#if Rtt_WIN_ENV // TODO: RenderStateObject REMOVE ME!
#include "Renderer/Rtt_CustomCommand.h"

#include <stddef.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

CustomCommand::CustomCommand()
:	fNext( NULL ),
	fUserdata( NULL )
{
}

CustomCommand::~CustomCommand()
{
}

CommandStack::CommandStack()
:	fTop( NULL )
{
}

CommandStack::~CommandStack()
{
	while (!IsEmpty())
	{
		Pop();
	}
}

void
CommandStack::Push( CustomCommand* command )
{
	command->SetNextCommand( fTop );

	fTop = command;
}

CustomCommand *
CommandStack::Pop()
{
	CustomCommand* top = fTop;

	if (top)
	{
		fTop = top->GetNextCommand();

		top->SetNextCommand( NULL );
	}

	return top;
}

bool
CommandStack::Contains( const CustomCommand& ref, CommandPredicate predicate ) const
{
	for (CustomCommand* command = fTop; command; command = command->GetNextCommand())
	{
		if (predicate( ref, *command ))
		{
			return true;
		}
	}

	return false;
}

bool
CommandStack::IsEmpty() const
{
	return NULL == fTop;
}

// ----------------------------------------------------------------------------

} // namespace Rtt
#endif // TODO: RenderStateObject REMOVE ME!
// ----------------------------------------------------------------------------