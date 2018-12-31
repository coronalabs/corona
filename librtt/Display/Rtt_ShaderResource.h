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

#ifndef _Rtt_ShaderResource_H__
#define _Rtt_ShaderResource_H__

#include <map>
#include <string>
#include "Display/Rtt_ShaderTypes.h"
#include "Renderer/Rtt_Uniform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Program;
class ShaderData;

// ----------------------------------------------------------------------------

class ShaderResource
{
	public:

		typedef enum ProgramMod
		{
			kDefault	= 0,
			k25D		= 1,
			kNumProgramMods,
		}
		ProgramMod;
	
		typedef std::map< std::string, int > VertexDataMap;

		struct UniformData
		{
			int index;
			Uniform::DataType dataType;
		};
		typedef std::map< std::string, UniformData > UniformDataMap;

	public:
		// Shader takes ownership of the program
		ShaderResource( Program *program, ShaderTypes::Category category );
		ShaderResource( Program *program, ShaderTypes::Category category, const char *name );
		
	public:
		~ShaderResource();

	public:
		ShaderTypes::Category GetCategory() const { return fCategory; }
		const std::string& GetName() const { return fName; }
		const char *GetTag( int index ) const { return NULL; }
		int GetNumTags() const { return 0; }

	public:
		bool UsesUniforms() const { return fUsesUniforms; }
		void SetUsesUniforms( bool newValue ) { fUsesUniforms = newValue; }

		bool UsesTime() const { return fUsesTime; }
		void SetUsesTime( bool newValue ) { fUsesTime = newValue; }

	public:
		// Shader either stores params on per-vertex basis or in uniforms.
		// Batching most likely breaks as soon as you use uniforms,
		// so params are either per-vertex OR uniforms --- never both.
		// The mapping between the (Lua API) property name and the internal
		// location in per-vertex/uniform data is stored by the maps.
		int GetDataIndex( const char *key ) const;
		UniformData GetUniformData( const char *key ) const;
		
		/*
		const VertexDataMap& GetVertexDataMap() const { return fVertexDataMap; }
		const UniformDataMap& GetUniformDataMap() const { return fUniformDataMap; }
		*/

	//protected:
		VertexDataMap& GetVertexDataMap() { return fVertexDataMap; }
		UniformDataMap& GetUniformDataMap() { return fUniformDataMap; }

	public:
		// A filter's default effect param values are stored here.
		ShaderData *GetDefaultData() const { return fDefaultData; }
		void SetDefaultData( ShaderData *defaultData );
		
	public:
		void SetProgramMod(ProgramMod mod, Program *program);
		Program *GetProgramMod(ProgramMod mod) const;
		
	private:
		void Init(Program *defaultProgram);

	private:
		Program *fPrograms[kNumProgramMods];
		
		ShaderTypes::Category fCategory;
		std::string fName;
		VertexDataMap fVertexDataMap;
		UniformDataMap fUniformDataMap;
		ShaderData *fDefaultData;
		bool fUsesUniforms;
		bool fUsesTime;

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderResource_H__
