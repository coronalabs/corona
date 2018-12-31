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
