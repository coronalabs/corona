//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_VerifierData_H__
#define _Rtt_VerifierData_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class VerifierData
{
	public:
		enum _Constants
		{
			kModulusIndex = 0,
			kModulusLen = 16,
			kSignatureIndex = kModulusIndex + kModulusLen,
			kSignatureLen = 16,
			kExponentIndex = kSignatureIndex + kSignatureLen,
			kExponentLen = 2,
			kVerifierDataLen = kModulusLen + kSignatureLen + kExponentLen,
			kPaddingLen = 4,
		};

	public:
		static const volatile U32* ModulusBase( const volatile U32* baseDataAddress ) { return baseDataAddress + kModulusIndex; }
		static const volatile U32* SignatureBase( const volatile U32* baseDataAddress ) { return baseDataAddress + kSignatureIndex; }
		static const volatile U32* ExponentBase( const volatile U32* baseDataAddress ) { return baseDataAddress + kExponentIndex; }

		static U32* ModulusBase( U32* baseDataAddress ) { return baseDataAddress + kModulusIndex; }
		static U32* SignatureBase( U32* baseDataAddress ) { return baseDataAddress + kSignatureIndex; }
		static U32* ExponentBase( U32* baseDataAddress ) { return baseDataAddress + kExponentIndex; }
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_VerifierData_H__
