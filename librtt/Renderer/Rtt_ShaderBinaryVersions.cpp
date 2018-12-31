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

#include "Renderer/Rtt_ShaderBinaryVersions.h"
#include "Renderer/Rtt_ShaderBinary.h"
#include "Core/Rtt_Config.h"
#include "Core/Rtt_Allocator.h"


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

ShaderBinaryVersions::ShaderBinaryVersions(Rtt_Allocator *allocatorPointer)
:	fAllocatorPointer(allocatorPointer)
{
	for (int index = 0; index < Program::kNumVersions; index++)
	{
		if (fAllocatorPointer)
		{
			fShaderBinaryPointerArray[index] = Rtt_NEW(fAllocatorPointer, ShaderBinary(fAllocatorPointer));
		}
		else
		{
			fShaderBinaryPointerArray[index] = NULL;
		}
	}
}

ShaderBinaryVersions::~ShaderBinaryVersions()
{
	ShaderBinary *shaderBinaryPointer;

	for (int index = 0; index < Program::kNumVersions; index++)
	{
		shaderBinaryPointer = fShaderBinaryPointerArray[index];
		if (shaderBinaryPointer)
		{
			Rtt_DELETE(shaderBinaryPointer);
		}
	}
}


// ----------------------------------------------------------------------------
// Public Functions
// ----------------------------------------------------------------------------

Rtt_Allocator* ShaderBinaryVersions::GetAllocator()
{
	return fAllocatorPointer;
}

ShaderBinary* ShaderBinaryVersions::Get(Program::Version version)
{
	return (version < Program::kNumVersions) ? fShaderBinaryPointerArray[version] : NULL;
}

bool ShaderBinaryVersions::CopyFrom(ShaderBinaryVersions *shaderVersionsPointer)
{
	ShaderBinary *shaderBinaryPointer;
	bool wasCopied = false;

	// Validate argument.
	if (!shaderVersionsPointer)
	{
		return false;
	}

	// Copy the given shader binaries to this object.
	for (int index = 0; index < Program::kNumVersions; index++)
	{
		shaderBinaryPointer = fShaderBinaryPointerArray[index];
		if (shaderBinaryPointer)
		{
			wasCopied |= shaderBinaryPointer->CopyFrom(shaderVersionsPointer->Get((Program::Version)index));
		}
	}
	return wasCopied;
}

void ShaderBinaryVersions::FreeAllShaders()
{
	ShaderBinary *shaderBinaryPointer;

	// Delete all of the shaders' byte buffers.
	for (int index = 0; index < Program::kNumVersions; index++)
	{
		shaderBinaryPointer = fShaderBinaryPointerArray[index];
		if (shaderBinaryPointer)
		{
			shaderBinaryPointer->Free();
		}
	}
}

} // namespace Rtt
