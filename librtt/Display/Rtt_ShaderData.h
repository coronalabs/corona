//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShaderData_H__
#define _Rtt_ShaderData_H__

#include "Core/Rtt_WeakPtr.h"

#include "Renderer/Rtt_Uniform.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class DisplayObject;
class LuaUserdataProxy;
class Paint;
class Shader;
class ShaderFactory;
class ShaderResource;
class Uniform;

// ----------------------------------------------------------------------------

class ShaderData
{
	public:
		typedef ShaderData Self;

	public:
		typedef enum _DataIndex
		{
			kDataUnknown = -1,
			kData0 = 0,
			kData1,
			kData2,
			kData3,

			kNumData,
			kDataMin = kData0,
			kDataMax = kNumData - 1,
		}
		DataIndex;

	public:
		ShaderData( const WeakPtr< ShaderResource >& resource );
		~ShaderData();

	public:
		void SetOwner( const Shader *shader );

	public:
		void QueueRelease( DisplayObject *observer );
		ShaderData *Clone( Rtt_Allocator *allocator );

	public:
		Uniform *InitializeUniform(
			Rtt_Allocator *allocator,
			DataIndex index,
			Uniform::DataType dataType );

	protected:
		Paint *GetPaint() const;

	public:
		void PushProxy( lua_State *L );
		void DetachProxy();

	public:
		Real GetVertexData( DataIndex index ) const;
		void CopyVertexData( Real& a, Real& b, Real& c, Real& d ) const;
		void SetVertexData( DataIndex index, Real newValue );

	public:
		int PushUniform( lua_State *L, DataIndex dataIndex ) const;
		void SetUniform( lua_State *L, int valueIndex, DataIndex dataIndex );

	public:
		Uniform *GetUniform( DataIndex index ) const;
		void DidUpdateUniform( DataIndex index );
        void Invalidate();
        void * GetExtraSpace() const { return fExtraSpace; }
        U32 GetExtraCount() const { return fExtraCount; }

	public:
		WeakPtr< ShaderResource >& GetShaderResource() { return fShaderResource; }
		const WeakPtr< ShaderResource >& GetShaderResource() const { return fShaderResource; }
//		void Invalidate() { fShader = NULL; }
		
//		void Invalidate( DirtyFlags flags ) { fDirtyFlags |= flags; }
//		bool IsValid( DirtyFlags flags ) const { return 0 == (fDirtyFlags & flags); }
//		void SetValid( DirtyFlags flags ) { fDirtyFlags &= (~flags); }

	private:
		LuaUserdataProxy *fProxy;
		Real fVertexData[kNumData];
		Uniform *fUniformData[kNumData];
		WeakPtr< ShaderResource > fShaderResource;
        void * fExtraSpace;
        U32 fExtraCount;
		const Shader *fOwner;
		U8 fDirtyFlags;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderData_H__
