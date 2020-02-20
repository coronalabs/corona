//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShaderTypes_H__
#define _Rtt_ShaderTypes_H__

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class ShaderTypes
{
	public:
		typedef enum _Category
		{
			kCategoryDefault,
			kCategoryFilter,		// 1 texture effects
			kCategoryComposite,		// 2 texture effects
			kCategoryGenerator,		// Procedural effects

			kNumCategories,
		}
		Category;

		static const char *StringForCategory( Category category );
		static Category CategoryForString( const char *category );

		typedef enum _PredefinedTag
		{
			kTagBuiltIn,
			kTagDistortion,
			kTagGeometry,
			kTagHalftone,
			kTagColorAdjustment,
			kTagColorEffect,
			kTagTransition,
			kTagTileEffect,
			kTagGradient,
			kTagStylize,
			kTagSharpen,
			kTagBlur,

			kNumTags,
		}
		PredefinedTag;

		static const char *StringForPredefinedTag( PredefinedTag t );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderTypes_H__
