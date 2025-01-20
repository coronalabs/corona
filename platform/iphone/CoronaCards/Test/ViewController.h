//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "CoronaCards/CoronaCards.h"

 #define CK_TEST_CORONAVIEWDELEGATE 1

@interface ViewController : CoronaViewController
								#ifdef CK_TEST_CORONAVIEWDELEGATE
									<CoronaViewDelegate>
								#endif
@end
