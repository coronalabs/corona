//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShaderBinaryVersions_H__
#define _Rtt_ShaderBinaryVersions_H__

#include "Renderer/Rtt_Program.h"

// Forward declarations.
namespace Rtt
{
	class ShaderBinary;
}
struct Rtt_Allocator;


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/// Stores multiple compiled versions of a shader in binary form.
/// <p>
/// The versions of a shader this class stores match the Program::Version enum, such as kMaskCount0, kMaskCount1, etc.
class ShaderBinaryVersions
{
	private:
		/// Default constructor made private to force the caller to pass an allocator the other construtor.
		ShaderBinaryVersions() { }

	public:
		/// Creates a new shader binary container.
		/// @param allocatorPointer Allocator needed to create the shader's byte buffer. Cannot be null.
		ShaderBinaryVersions(Rtt_Allocator *allocatorPointer);

		/// Destroys this object's allocated resources.
		virtual ~ShaderBinaryVersions();

		/// Gets the allocator this object uses to create the byte buffers for all of the shader binaries.
		/// @returns Pointer to the allocator this object uses.
		Rtt_Allocator* GetAllocator();

		/// Gets a compiled shader binary for the given version.
		/// @param version The version of shader such as Program::kMaskCount0, Program::kMaskCount1, etc.
		/// @returns Returns a pointer to the specified version of shader.
		///          <p>
		///          Returns NULL if given an invalid Program::kNumVersions version type or
		///          if an allocator was not assigned to this object during construction.
		ShaderBinary* Get(Program::Version version);

		/// Copies all of the given shader binaries to this object.
		/// @param shaderVersionsPointer Pointer to a collection of compiled shaders.
		/// @returns Returns true if the copy was successful.
		///          <p>
		///          Returns false if given a null argument.
		bool CopyFrom(ShaderBinaryVersions *shaderVersionsPointer);

		/// Frees the memory used by all shader versions contained by this object.
		/// <p>
		/// This effectively calls the Free() method for each ShaderBinary stored by this object.
		void FreeAllShaders();

	private:
		Rtt_Allocator *fAllocatorPointer;
		ShaderBinary *fShaderBinaryPointerArray[Program::kNumVersions];
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_ShaderBinaryVersions_H__
