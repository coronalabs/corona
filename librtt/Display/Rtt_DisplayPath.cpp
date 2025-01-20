//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_DisplayPath.h"

#include "Rtt_Matrix.h"
#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_VertexCache.h"
#include "Renderer/Rtt_FormatExtensionList.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Rtt_LuaUserdataProxy.h"

#include "CoronaLua.h"
#include "CoronaGraphics.h"
#include "Rtt_LuaContext.h"

#include <vector>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

void
DisplayPath::UpdateGeometry( Geometry& dst, const VertexCache& src, const Matrix& srcToDstSpace, U32 flags, Array<U16> *indices )
{
    if ( 0 == flags ) { return; }

    const ArrayVertex2& vertices = src.Vertices();
    const ArrayVertex2& texVertices = src.TexVertices();
    U32 numVertices = vertices.Length();

    U32 numIndices = indices==NULL?0:indices->Length();
    if ( dst.GetVerticesAllocated() < numVertices || dst.GetIndicesAllocated() < numIndices)
    {
        dst.Resize( numVertices, numIndices, false );
    }
    Geometry::Vertex *dstVertices = dst.GetVertexData();

    bool updateVertices = ( flags & kVerticesMask );
    bool updateTexture = ( flags & kTexVerticesMask );

    Rtt_ASSERT( ! updateTexture || ( vertices.Length() == texVertices.Length() ) );

    const ArrayFloat * floatArray = src.ExtraFloatArray( ZKey() );
    const ArrayIndex * indexArray = src.ExtraIndexArray( IndicesKey() );

    std::vector< float > temp;
    const float zero = 0.f, * zsource;
    size_t step = 0U;

    if (floatArray)
    {
        if (indexArray)
        {
            for (S32 i = 0; i < indexArray->Length(); ++i)
            {
                U16 index = (*indexArray)[i];

                temp.push_back( (*floatArray)[index] );
            }

            zsource = temp.data();
        }

        else
        {
            zsource = floatArray->ReadAccess();
        }

        ++step;
    }

    else
    {
        zsource = &zero;
        step = 0U;
    }

    for ( U32 i = 0, iMax = vertices.Length(); i < iMax; i++, zsource += step )
	{
		Rtt_ASSERT( i < dst.GetVerticesAllocated() );

        Geometry::Vertex& dst = dstVertices[i];

        if ( updateVertices )
        {
            Vertex2 v = vertices[i];
            srcToDstSpace.Apply( v );

			dst.x = v.x;
			dst.y = v.y;
            dst.z = *zsource;
		}

        if ( updateTexture )
        {
            dst.u = texVertices[i].x;
            dst.v = texVertices[i].y;
            dst.q = 1.f;
        }
    }

    dst.SetVerticesUsed( numVertices );
    
    if(flags & kIndicesMask)
    {
        if(indices)
        {
            const U16* indicesData = indices->ReadAccess();
            U16* dstData = dst.GetIndexData();
            numIndices = indices->Length();
            for (U32 i=0; i<numIndices; i++)
            {
                dstData[i] = indicesData[i];
            }
            
            dst.Invalidate();
        }
        dst.SetIndicesUsed(numIndices);
    }
}

const void *
DisplayPath::ZKey()
{
    static int key;

    return &key;
}

const void *
DisplayPath::IndicesKey()
{
    static int key;

    return &key;
}

// ----------------------------------------------------------------------------

DisplayPath::DisplayPath()
:    fObserver( NULL ),
    fAdapter( NULL ),
    fProxy( NULL )
{
}

DisplayPath::~DisplayPath()
{
    if ( fProxy )
    {
        GetObserver()->QueueRelease( fProxy ); // Release native ref to Lua-side proxy
        fProxy->DetachUserdata(); // Notify proxy that object is invalid
    }
}

void
DisplayPath::PushProxy( lua_State *L ) const
{
    if ( ! fProxy )
    {
        fProxy = LuaUserdataProxy::New( L, const_cast< Self * >( this ) );
        fProxy->SetAdapter( GetAdapter() );
    }

    fProxy->Push( L );
}

void
DisplayPath::ReleaseProxy( LuaUserdataProxy *proxy )
{
    if (proxy)
    {
        fObserver->QueueRelease( proxy );
        proxy->DetachUserdata();
    }
}

int
DisplayPath::ExtensionAdapter::ValueForKey(
            const LuaUserdataProxy& sender,
            lua_State *L,
            const char *key ) const
{
    Rtt_ASSERT( key ); // Caller should check at the top-most level

    ExtensionAdapter* adapter = (ExtensionAdapter*)sender.GetAdapter();
    if (!adapter) { return 0; }
    DisplayObject* object = (DisplayObject*)sender.GetUserdata();
    if (!object) { return 0; }
    Geometry* geometry = GetGeometry( object, adapter->fIsFill );
 
    int result = 1, index = GetHash( L )->Lookup( key );
    
    switch ( index )
    {
        case 0:
            Lua::PushCachedFunction( L, getAttributeDetails );
            break;
        case 1:
            {
                const Geometry::ExtensionBlock* block = geometry->GetExtensionBlock();
                
                if ( Geometry::UsesInstancing( block, geometry->GetExtensionList() ) )
                {
                    lua_pushnumber( L, block->fCount );
                }
                
                else
                {
                    lua_pushnumber( L, 0 );
                }
            }
            break;
        case 2:
            Lua::PushCachedFunction( L, setAttributeValue );
            break;
        default:
            result = 0;
            break;
    }

    return result;
}

bool
DisplayPath::ExtensionAdapter::SetValueForKey(
            LuaUserdataProxy& sender,
            lua_State *L,
            const char *key,
            int valueIndex ) const
{
    Rtt_ASSERT( key ); // Caller should check at the top-most level

    ExtensionAdapter* adapter = (ExtensionAdapter*)sender.GetAdapter();
    if (!adapter) { return false; }
    DisplayObject* object = (DisplayObject*)sender.GetUserdata();
    if (!object) { return false; }
    Geometry* geometry = GetGeometry( object, adapter->fIsFill );

    bool result = false;
    int index = GetHash( L )->Lookup( key );
    
    switch ( index )
    {
        case 1:
            {
                Geometry::ExtensionBlock* block = geometry->GetExtensionBlock();
                
                if (block && block->fInstanceData)
                {
                    const FormatExtensionList* extensionList = geometry->GetExtensionList();
                    
                    Rtt_ASSERT( extensionList );
                    
                    int instanceCount = luaL_checkint( L, valueIndex );
                    
                    if (instanceCount < 0 )
                    {
                        CoronaLuaWarning( L, "Negative instance count" );
                    }
                    
                    else
                    {
                        block->fCount = (U32)instanceCount;
                        
                        U32 instanceGroupIndex = 0;
                        
                        for (auto iter = FormatExtensionList::InstancedGroups( extensionList ); !iter.IsDone(); iter.Advance())
                        {
                            const FormatExtensionList::Group* group = iter.GetGroup();
                            U32 vertexCount = group->GetVertexCount( block->fCount, iter.GetAttribute() );
                            
                            block->fInstanceData[instanceGroupIndex]->PadToSize( vertexCount * sizeof(Geometry::Vertex), 0 );
                            // TODO? more fine-grained, e.g. instances flag?
                            
                            ++instanceGroupIndex;
                        }
                        
                        object->Invalidate( DisplayObject::kGeometryFlag );
                        
                        result = true;
                    }
                }
                
                else if (!block)
                {
                    CoronaLuaWarning( L, "Unable to assign instances without extension." );
                }
                
                else if (!geometry->GetExtensionList()->IsInstancedByID())
                {
                    CoronaLuaWarning( L, "Extension has no instance-rate members." );
                }
				
				else
				{
					int instanceCount = luaL_checkint( L, valueIndex );
					
					if (instanceCount < 0 )
					{
						CoronaLuaWarning( L, "Negative instance count" );
					}
					
					else
					{
						block->fCount = (U32)instanceCount;
						
						// object->Invalidate( ???? );
						
						result = true;
					}
				}
            }
            break;
            
        default:
            // no-op
            break;
    }

    return result;
}

void
DisplayPath::ExtensionAdapter::WillFinalize( LuaUserdataProxy& sender ) const
{
    DisplayObject* object = (DisplayObject*)sender.GetUserdata();
    if (!object) { return; }
    
    Geometry* geometry = GetGeometry( object, fIsFill );
    
    Rtt_ASSERT( geometry );
    
    Geometry::ExtensionBlock* block = geometry->GetExtensionBlock();
    
    if (block)
    {
        block->fProxy = NULL;
    }
}

StringHash *
DisplayPath::ExtensionAdapter::GetHash( lua_State *L ) const
{
    static const char *keys[] =
    {
        "getAttributeDetails", // 0
        "instances",           // 1
        "setAttributeValue",   // 2
    };
    static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 3, 6, 1, __FILE__, __LINE__ );
    return &sHash;
}

int
DisplayPath::ExtensionAdapter::getAttributeDetails( lua_State *L )
{
    int result = 0;
    int nextArg = 1;
    const LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, nextArg++ );
    if(!sender) { return result; }
    ExtensionAdapter* adapter = (ExtensionAdapter*)sender->GetAdapter();
    if (!adapter) { return 0; }
    DisplayObject* object = (DisplayObject*)sender->GetUserdata();
    if (!object) { return 0; }
    Geometry* geometry = GetGeometry( object, adapter->fIsFill );
    
    Rtt_ASSERT( geometry );
    
    const FormatExtensionList* extensionList = geometry->GetExtensionList();
    
    result = 0;
    
    if (extensionList)
    {
        const char* name = luaL_checkstring( L, nextArg );
        S32 nameIndex = extensionList->FindName( name );

        if (-1 != nameIndex)
        {
            const FormatExtensionList::Attribute& attribute = extensionList->GetAttributes()[nameIndex];
            
            lua_createtable( L, 0, 3 + attribute.normalized );

            switch (attribute.type)
            {
                case kAttributeType_Byte:
                    lua_pushliteral( L, "byte" );
                    break;
                case kAttributeType_Int:
                    Rtt_ASSERT_NOT_IMPLEMENTED();
                    break;
                case kAttributeType_Float:
                    lua_pushliteral( L, "float" );
                    break;
                default:
                    Rtt_ASSERT_NOT_REACHED();
            }
            
            lua_setfield( L, -2, "type" );
            lua_pushinteger( L, attribute.components );
            lua_setfield( L, -2, "components" );
            lua_pushinteger( L, attribute.offset );
            lua_setfield( L, -2, "offset" );
            
            if (attribute.normalized)
            {
                lua_pushboolean( L, 1 );
                lua_setfield( L, -2, "normalized" );
            }

            result = 1;
        }
        
        else
        {
            CoronaLuaWarning( L, "No attribute named %s.", name );
        }
    }
    
    else
    {
        CoronaLuaWarning( L, "No extension bound." );
    }
    
    if (!result)
    {
        lua_pushnil( L );
    
        result = 1;
    }
    
    return result;
}

int
DisplayPath::ExtensionAdapter::setAttributeValue( lua_State *L )
{
    int nextArg = 1;
    LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, nextArg++ );
    if(!sender) { return 0; }
    ExtensionAdapter* adapter = (ExtensionAdapter*)sender->GetAdapter();
    if (!adapter) { return 0; }
    DisplayObject* object = (DisplayObject*)sender->GetUserdata();
    if (!object) { return 0; }
    Geometry* geometry = GetGeometry( object, adapter->fIsFill );

    const FormatExtensionList* extensionList = geometry->GetExtensionList();
    
    if (extensionList)
    {
        int index = luaL_checkint( L, nextArg++ );
        const char* name = luaL_checkstring( L, nextArg++ );
        
        if (index <= 0 )
        {
            CoronaLuaWarning( L, "Index is non-positive" );
            
            return 0;
        }
        
        S32 nameIndex = extensionList->FindName( name );

        if (-1 != nameIndex)
        {
            U32 groupIndex = extensionList->FindGroup( (U32)nameIndex );
            const FormatExtensionList::Group& group = extensionList->GetGroups()[groupIndex];
            const FormatExtensionList::Attribute& attribute = extensionList->GetAttributes()[nameIndex];
            U8 data[4 * 8] = {}; // 4 components, up to double-type
            
            for (U32 i = 0; i < attribute.components; ++i)
            {
                if (i > 0 && lua_isnoneornil( L, nextArg ))
                {
                    break;
                }
                
                lua_Number n = luaL_checknumber( L, nextArg++ );
                
                switch (attribute.type)
                {
                case kAttributeType_Byte:
                    data[i] = (U8)n;
                    break;
                case kAttributeType_Int:
                    Rtt_ASSERT_NOT_IMPLEMENTED();
                    break;
                case kAttributeType_Float:
                    {
                        float f = (float)n;
                        
                        memcpy( data + i * sizeof(float), &f, sizeof(float) );
                    }
                    break;
                default:
                    Rtt_ASSERT_NOT_REACHED();
                }
            }
            
            if (group.IsInstanceRate())
            {
                if (extensionList->HasVertexRateData())
                {
                    --groupIndex;
                }
                
                Geometry::ExtensionBlock* block = geometry->GetExtensionBlock();
                Array<U8>* instanceData = block->fInstanceData[groupIndex];
                U32 offset = attribute.offset;
                
                if (group.IsWindowed())
                {
                    offset += (index - 1) * attribute.GetSize();
                }
                
                else
                {
                    offset += (index - 1) * group.size;
                }

                if (offset + attribute.GetSize() > group.GetDataSize( block->fCount, &attribute ))
                {
                    CoronaLuaWarning( L, "Index is out of bounds" );
                    
                    return 0;
                }
                
                memcpy( instanceData->WriteAccess() + offset, data, attribute.GetSize() );
            }
            
            else
            {
                S32 extendedDataLength = -1;
                Geometry::Vertex* extendedData = geometry->GetWritableExtendedVertexData( &extendedDataLength );
                
                Rtt_ASSERT( extendedData );
                                
                U32 offset = attribute.offset;
                size_t vertexSize = FormatExtensionList::GetExtraVertexSize( extensionList );
                
                if (geometry->GetStoredOnGPU())
                {
                    offset += sizeof(Geometry::Vertex);
                    vertexSize += sizeof(Geometry::Vertex);
                }
                
                offset += (index - 1) * vertexSize;
                
                if (offset + attribute.GetSize() > extendedDataLength * sizeof(Geometry::Vertex))
                {
                    CoronaLuaWarning( L, "Index %i is out of bounds.", index );
                    
                    return 0;
                }
                
                memcpy( reinterpret_cast<U8*>( extendedData ) + offset, data, attribute.GetSize() );
            }
            
            object->Invalidate( DisplayObject::kGeometryFlag );
            // TODO: if (group->IsInstanceRate()) flag |= instancesFlag
        }
        
        else
        {
            CoronaLuaWarning( L, "No attribute named %s.", name );
        }
    }
    
    else
    {
        CoronaLuaWarning( L, "No extension bound." );
    }
    
    return 0;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

