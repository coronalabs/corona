//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShaderName.h"
#include "Display/Rtt_ShaderBuiltin.h"
#include "Display/Rtt_ShaderTypes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ShaderName::ShaderName()
:	fCategory( ShaderTypes::kCategoryDefault ),
	fName()
{
}

ShaderName::ShaderName( ShaderTypes::Category category, const char *name )
:	fCategory( category ),
	fName()
{
	if ( ShaderTypes::kCategoryDefault != category && name )
	{
		fName = name;
	}
}

ShaderName::ShaderName( const char *fullyQuallifiedName)
:fCategory( ShaderTypes::kCategoryDefault ),
fName()
{
	std::string fullPath = std::string(fullyQuallifiedName);
	
	std::string::size_type pos = fullPath.find(".");
	
	if(pos != std::string::npos)
    {
		fName = fullPath.substr(pos+1);
		std::string category = fullPath.substr(0,pos);
		
		fCategory = ShaderTypes::CategoryForString( category.c_str() );
	}

}

bool
ShaderName::operator==( const ShaderName& rhs ) const
{
	return
		fCategory == rhs.fCategory
		&& fName == rhs.fName;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

