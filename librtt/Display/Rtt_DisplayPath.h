//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_DisplayPath_H__
#define _Rtt_DisplayPath_H__

#include "Rtt_DisplayTypes.h"
#include "Rtt_LuaUserdataProxy.h"

struct lua_State;

namespace Rtt
{

class DisplayObject;
class Geometry;
class LuaUserdataProxy;
class MLuaUserdataAdapter;
class Matrix;
struct Rect;
struct RenderData;
class Renderer;
class VertexCache;
class Paint;

// ----------------------------------------------------------------------------

// NOTE: DisplayPath instances have no notion of transform
// Only DisplayObjects have that concept!
//
// Therefore the semantics of Translate in a Path are different than that of an
// Object.  In a path, translate means to translate all cached vertices, or
// if that's not possible, to invalidate any cached vertices.
// In an object, translate means to update its transform and also translate
// any paths it might own.

class DisplayPath
{
    public:
        typedef DisplayPath Self;

    public:
        enum {
            kVerticesMask = 0x1,
            kTexVerticesMask = 0x2,
            kIndicesMask = 0x4,
        };

		static void UpdateGeometry(
			Geometry& dst,
			const VertexCache& src,
			const Matrix& srcToDstSpace,
			U32 flags,
		    ArrayIndex *indices );
    
        static const void * ZKey();
        static const void * IndicesKey();

	public:
		DisplayPath();
		virtual ~DisplayPath();

        class ExtensionAdapter : public MLuaUserdataAdapter
        {
        public:
            ExtensionAdapter( bool isFill )
            : fIsFill( isFill )
            {
            }
            
        public:
            static bool IsLineObject( const DisplayObject* object );
            static bool IsFillPaint( const DisplayObject* object, const Paint* paint );
            static Geometry *GetGeometry( DisplayObject* object, bool isFill );
            static const Geometry *GetGeometry( const DisplayObject* object, bool isFill );
        
        public:
            virtual int ValueForKey(
                const LuaUserdataProxy& sender,
                lua_State *L,
                const char *key ) const;

            virtual bool SetValueForKey(
                LuaUserdataProxy& sender,
                lua_State *L,
                const char *key,
                int valueIndex ) const;

            virtual void WillFinalize( LuaUserdataProxy& sender ) const;

            virtual StringHash *GetHash( lua_State *L ) const;
        
        private:
            static int getAttributeDetails( lua_State *L );
            static int setAttributeValue( lua_State *L );
        
        private:
            bool fIsFill;
        };

    public:
        virtual void Update( RenderData& data, const Matrix& srcToDstSpace ) = 0;
        virtual void UpdateResources( Renderer& renderer ) const = 0;
        virtual void Translate( Real deltaX, Real deltaY ) = 0;
        virtual void GetSelfBounds( Rect& rect ) const = 0;

    public:
        DisplayObject *GetObserver() const { return fObserver; }
        void SetObserver( DisplayObject *newValue ) { fObserver = newValue; }

    public:
        const MLuaUserdataAdapter *GetAdapter() const { return fAdapter; }
        void SetAdapter( const MLuaUserdataAdapter *newValue ) { fAdapter = newValue; }
        void PushProxy( lua_State *L ) const;
        void DetachProxy() { fAdapter = NULL; fProxy = NULL; }
        void ReleaseProxy( LuaUserdataProxy *proxy );

    private:
        DisplayObject *fObserver; // weak ptr
        const MLuaUserdataAdapter *fAdapter; // weak ptr
        mutable LuaUserdataProxy *fProxy;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_DisplayPath_H__
