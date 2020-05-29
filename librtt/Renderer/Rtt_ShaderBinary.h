//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShaderBinary_H__
#define _Rtt_ShaderBinary_H__

#include "Core/Rtt_Types.h"

// Forward declarations.
struct Rtt_Allocator;


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/// Stores one compiled shader as a byte array.
class ShaderBinary
{
	public:
		/// Creates a new shader binary container.
		/// @param allocatorPointer Allocator needed to create the shader's byte buffer. Cannot be null.
		ShaderBinary(Rtt_Allocator *allocatorPointer);

		/// Destroys this object's allocated resources.
		virtual ~ShaderBinary();

		/// Gets the allocator this object uses to create the shader binary's byte buffer.
		/// @returns Pointer to the allocator this object uses.
		Rtt_Allocator* GetAllocator();

		/// Fetches a pointer to the compiled shader's binary.
		/// @returns Returns a pointer to the first byte in the shader binary's byte array.
		///          <p>
		///          Returns null if this object is not storing a shader binary.
		const U8* GetBytes();

		/// Gets the number of bytes the shader binary consumes.
		/// @returns Returns the number of bytes the shader binary uses.
		///          <p>
		///          Returns zero if this object is not storing a shader binary.
		size_t GetByteCount();

		/// Replaces this object's binary with the given shader's binary.
		/// <p>
		/// If the given object does not contain a binary, then this object's binary will be freed to match.
		/// @param binaryPointer Pointer to the shader binary to be copied from.
		/// @returns Returns true if the binary was successfully copied to this object.
		///          <p>
		///          Returns false if the given null or if this object was not given an allocator.
		bool CopyFrom(const ShaderBinary *binaryPointer);

		/// Replaces this object's binary with the given one.
		/// @param byteBufferPointer Pointer to the byte array to be copied.
		/// @param byteCount Number of bytes to copy from the given byte buffer.
		/// @returns Returns true if the given binary was successfully copied to this object.
		///          <p>
		///          Returns false if given a null pointer, zero byte count, or if this object does not have an allocator.
		bool CopyFrom(const U8 *byteBufferPointer, const size_t byteCount);

		/// Decodes the given hexadecimal string to binary form and stores it in this object.
		/// @param hexString Null terminated string providing a binary in hexadecimal string form.
		///                  <p>
		///                  The only valid characters this string can use is 0-9, A-F, a-f, and null.
		///                  <p>
		///                  An empty string is valid and will clear this object's binary.
		///                  <p>
		///                  A null pointer is not valid and will cause this function to return false.
		/// @returns Returns true if successfully decoded the string and copied the binary to this object.
		///          <p>
		///          Returns false if given null, if given an invalid string, or if this object does not have an allocator.
		bool CopyFromHexadecimalString(const char *hexString);

		/// Deletes the shader binary this object is storing.
		/// <p>
		/// This causes this object's GetBytes() to return null and GetByteCount() to return zero.
		/// <p>
		/// It is safe to call this function if the binary has already been freed.
		void Free();

	private:
		/// Converts the given hexadecimal character to its equivalent value.
		/// For example:
		/// - Character '0' would be converted to 0x00 (0 in decimal form)
		/// - Character '1' would be converted to 0x01 (1 in decimal form)
		/// - Character 'A' would be converted to 0x0A (10 in decimal form)
		/// - Character 'a' would be converted to 0x0A (10 in decimal form)
		/// @param hexCharacter Hexadecimal character such as '0'-'9', 'A'-'F', or 'a'-'f'.
		/// @param valuePointer Pointer to the byte value that will receive the converted result.
		/// @return Returns true if the given character was successfully converted.
		///         <p>
		///         Returns false if the given character was not a valid hexadecimal character
		///         or if the given valuePointer was null.
		static bool ConvertHexCharacterToByte(char hexCharacter, U8 *valuePointer);

		Rtt_Allocator *fAllocatorPointer;
		U8 *fByteBufferPointer;
		size_t fByteCount;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderBinary_H__
