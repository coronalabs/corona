//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
