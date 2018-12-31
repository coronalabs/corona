// ----------------------------------------------------------------------------
// 
// ViewController.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------

#import "CoronaCards/CoronaCards.h"

 #define CK_TEST_CORONAVIEWDELEGATE 1

@interface ViewController : CoronaViewController
								#ifdef CK_TEST_CORONAVIEWDELEGATE
									<CoronaViewDelegate>
								#endif
@end
