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

#ifndef _Rtt_CustomCommand_H__
#define _Rtt_CustomCommand_H__

// ----------------------------------------------------------------------------

struct Rtt_Allocator;

namespace Rtt
{

class Renderer;

// ----------------------------------------------------------------------------

// STEVE CHANGE
class CustomCommand {
public:
	typedef enum _CommandFlags
	{
		kDirtyFlag				= 0x01,
		kHasDummyGeometryFlag	= 0x02,
		kReuseRenderDataFlag	= 0x04,
		kShouldDrawFlag			= 0x08 // for later use...
	}
	CommandFlags;

	CustomCommand();
	virtual ~CustomCommand();

	virtual int GetFlags( const Renderer& renderer ) = 0;
	virtual void Prepare( const Renderer& renderer ) = 0;
	virtual void Render( Renderer& renderer ) = 0;

	CustomCommand* GetNextCommand() const { return fNext; }
	void SetNextCommand( CustomCommand* next ) { fNext = next; }

private:
	CustomCommand* fNext;
};

class CommandStack {
public:
	CommandStack();
	~CommandStack();

	void Push( CustomCommand* command );
	CustomCommand* Pop();

	bool IsEmpty() const;

private:
	CustomCommand* fTop;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_CustomCommand_H__