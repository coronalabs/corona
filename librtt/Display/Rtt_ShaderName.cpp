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

