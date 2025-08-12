//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Format_Extension_List_H__
#define _Rtt_Format_Extension_List_H__

#include "Core/Rtt_Array.h"
#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

struct CoronaVertexExtension;

namespace Rtt
{

class CommandBuffer;
class String;

// ----------------------------------------------------------------------------

class FormatExtensionList {
    public:
        FormatExtensionList();
        ~FormatExtensionList();
   
    public:
        struct Attribute {
            U32 nameHash;
            U16 type;
            U16 offset : 13;
            U16 components : 2;
            U16 normalized : 1;
            
            U32 GetSize() const;
            bool IsFloat() const;
        };
    
        struct Group {
            U32 divisor;
            U16 count;
            U16 size;
            
            bool IsInstanceRate() const { return 0 != divisor; }
            bool IsWindowed() const { return 0 == size; }
            bool NeedsDivisor() const { return divisor > 1; }
            U32 GetWindowAttributeCount( U32 valueCount ) const;
            U32 GetValueCount( U32 instanceCount ) const;
            size_t GetDataSize( U32 instanceCount, const Attribute * firstAttribute ) const;
            U32 GetVertexCount( U32 instanceCount, const Attribute * firstAttribute ) const;
        };

        class Iterator {
        public:
            enum GroupFilter { kAllGroups, kVertexRateGroups, kInstancedGroups };
            enum IterationPolicy { kIterateGroups, kIterateAttributes };
        
            Iterator( const FormatExtensionList* list, GroupFilter filter, IterationPolicy policy );
        
        public:
            void Advance();
            bool IsDone() const;
            U32 GetAttributeIndex() const;
            U32 GetGroupIndex() const;
            const Attribute* GetAttribute() const;
            const Group* GetGroup() const;
        
        private:
            void AdvanceGroup();
            void UpdateGroup();

        private:
            GroupFilter fFilter;
            IterationPolicy fPolicy;
            const FormatExtensionList * fList;
            U32 fFirstInGroup;
            U32 fOffsetInGroup;
            U32 fGroupIndex;
        };
    
    public:
        static Iterator AllGroups( const FormatExtensionList* list );
        static Iterator AllAttributes( const FormatExtensionList* list );
        static Iterator InstancedGroups( const FormatExtensionList* list );
    
        static FormatExtensionList FromArrays( const Array<Group>& groups, const Array<Attribute>& attributes );

    public:
        U32 ExtraVertexCount() const;
        U32 InstanceGroupCount() const;
	    bool IsInstanced() const { return fInstancedByID || HasInstanceRateData(); }
        bool IsInstancedByID() const { return fInstancedByID; }
        bool HasInstanceRateData() const;
        bool HasVertexRateData() const;
        void SortNames() const;
        const char* FindNameByAttribute( U32 attributeIndex, S32* index = NULL ) const;
        S32 FindHash( size_t hash ) const;
        S32 FindName( const char* name ) const;
        U32 FindGroup( U32 attributeIndex ) const;
        S32 FindCorrespondingInstanceGroup( const Group* group, const Attribute* attribute, U32 * attributeIndex ) const;
    
    public:
        const Attribute* GetAttributes() const { return fAttributes; }
        const Group* GetGroups() const { return fGroups; }
        U16 GetAttributeCount() const { return fAttributeCount; }
        U16 GetGroupCount() const { return fGroupCount; }

    public:
        static size_t GetExtraVertexSize( const FormatExtensionList * list );
        static size_t GetVertexSize( const FormatExtensionList * list );
        static bool Compatible( const FormatExtensionList * shaderList, const FormatExtensionList * geometryList );
        static bool Match( const FormatExtensionList * list1, const FormatExtensionList * list2 );
        static void ReconcileFormats( Rtt_Allocator* allocator, CommandBuffer * buffer, const FormatExtensionList * shaderList, const FormatExtensionList * geometryList, U32 offset );
    
    public:
        void Build( Rtt_Allocator* allocator, const CoronaVertexExtension * extension );
    
    private:
        struct NamePair {
            String* str;
            S32 index;
        
            bool operator<( const NamePair & other ) const { return index < other.index; }
        };
    
        Attribute * fAttributes;
        Group * fGroups;
        mutable NamePair* fNames; // allow reordering
        U16 fAttributeCount;
        U16 fGroupCount;
	    bool fInstancedByID;
        bool fOwnsData;
        mutable bool fSorted;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Format_Extension_List_H__
