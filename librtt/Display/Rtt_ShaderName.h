//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShaderName_H__
#define _Rtt_ShaderName_H__

#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderTypes.h"
#include <string>

// ----------------------------------------------------------------------------

namespace Rtt
{

class Program;

// ----------------------------------------------------------------------------

class ShaderName
{
	public:
		ShaderName();
		ShaderName( ShaderTypes::Category category, const char *name );
		ShaderName( const char *fullyQuallifiedName);

	public:
		ShaderTypes::Category GetCategory() const { return fCategory; }
		const char *GetName() const { return fName.c_str(); }

	public:
		bool operator==( const ShaderName& rhs ) const;
		bool operator!=( const ShaderName& rhs ) const { return ! ( *this == rhs ); }

	private:
		ShaderTypes::Category fCategory;
		std::string fName;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderName_H__
