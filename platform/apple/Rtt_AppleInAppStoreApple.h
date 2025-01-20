//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleInAppStoreApple_H__
#define _Rtt_AppleInAppStoreApple_H__

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

// ----------------------------------------------------------------------------

namespace Rtt
{
	class PlatformStore;
	class PlatformStoreTransaction;
}

// ----------------------------------------------------------------------------

@interface AppleStoreManager : NSObject <SKPaymentTransactionObserver, SKProductsRequestDelegate>
{
	Rtt::PlatformStore* fOwner;
	NSMutableDictionary<NSString*, SKProduct*> *fLoadedProducts;
}

@property(nonatomic, assign, readonly, getter=owner) Rtt::PlatformStore* fOwner;

- (id)initWithOwner:(Rtt::PlatformStore*)owner;
- (void)startObservingTransactions;
- (void)stopObservingTransactions;
- (void)loadProducts:(NSSet*)productIdentifiers;
- (BOOL)canMakePurchases;
- (void)purchase:(NSArray*)productIdentifiers;
- (void)finishTransaction:(Rtt::PlatformStoreTransaction*)transaction;
- (void)restoreCompletedTransactions;

@end

// ----------------------------------------------------------------------------

#endif // _Rtt_AppleInAppStoreApple_H__
