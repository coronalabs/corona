//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_BitmapMask__
#define __Rtt_BitmapMask__

#include "Core/Rtt_String.h"
#include "Rtt_MPlatform.h"
#include "Rtt_Transform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class FilePath;
class BitmapPaint;
class UserdataWrapper;

// ----------------------------------------------------------------------------

class BitmapMask
{
	public:
		static BitmapMask* Create( Runtime& runtime, const FilePath& maskData, bool onlyForHitTests );

	public:
		BitmapMask( BitmapPaint *paint, bool onlyForHitTests, bool isTemporary = false );
		BitmapMask( BitmapPaint *paint, Real contentW, Real contentH );

	public:
		~BitmapMask();

	public:
		static const char kHitTestOnlyTable[];

	public:
		const BitmapPaint* GetPaint() const { return fPaint; }
		BitmapPaint* GetPaint() { return fPaint; }
	
	public:
	   bool GetOnlyForHitTests() const { return fOnlyForHitTests; }

	public:
		const Transform& GetTransform() const { return fTransform; }
		Transform& GetTransform() { return fTransform; }

	public:
		void GetSelfBounds( Rect& rect ) const;
		void GetSelfBounds( Real& width, Real& height ) const;
		void SetSelfBounds( Real width, Real height );

	public:
		bool HitTest( Rtt_Allocator *allocator, int i, int j ) const;

	private:
		BitmapPaint* fPaint;
		Transform fTransform;
		Real fContentWidth;
		Real fContentHeight;
		bool fOnlyForHitTests;
		bool fIsTemporary;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_BitmapMask__
