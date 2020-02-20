//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "Rtt_AppleInAppStoreApple.h"

#include "Rtt_PlatformInAppStore.h"
#include "Rtt_Runtime.h"
#include "Rtt_AppleInAppStore.h"
#include "Rtt_AppleInAppStoreAppleTransactionObjectWrapper.h"

// ----------------------------------------------------------------------------

@implementation AppleStoreManager

@synthesize fOwner;

- (id)initWithOwner:(Rtt::PlatformStore*)owner
{
	self = [super init];
	if ( self )
	{
		fOwner = owner;
		fLoadedProducts = [[NSMutableDictionary alloc] init];
	}
	return self;
}

- (void)dealloc
{
	[self stopObservingTransactions];
	[fLoadedProducts release];
	[super dealloc];
}

- (void)startObservingTransactions
{
	// for some reason Mac does add same transaction observer several times
	// this will avoid duplicate messages about same transaction
	[[SKPaymentQueue defaultQueue] removeTransactionObserver:self];
	
	// restarts any purchases if they were interrupted last time the app was open
	[[SKPaymentQueue defaultQueue] addTransactionObserver:self];	
}

- (void)stopObservingTransactions
{
	// restarts any purchases if they were interrupted last time the app was open
	[[SKPaymentQueue defaultQueue] removeTransactionObserver:self];	
}

- (BOOL)canMakePurchases
{
    return (bool)[SKPaymentQueue canMakePayments];
}

- (void)loadProducts:(NSSet*)productIdentifiers
{
    SKProductsRequest* request = [[SKProductsRequest alloc] initWithProductIdentifiers:productIdentifiers];
    request.delegate = self;
    [request start];
}

// TODO: Consider supporting quantity. API might be clumsy. Current backdoor might be to list the item multiple times.
- (void)purchase:(NSArray*)productIdentifiers
{
	// Use NSCountedSet to support the backdoor hack of listing an item multiple times to set the quantity field
	NSCountedSet* counted_set = [[[NSCountedSet alloc] initWithArray:productIdentifiers] autorelease]; 
	for( NSString* productId in counted_set )
	{
		SKMutablePayment* payment = nil;
		SKProduct *product = [fLoadedProducts objectForKey:productId];
		if(product)
		{
			payment = [SKMutablePayment paymentWithProduct:product];
		}
		else
		{
#if defined( Rtt_IPHONE_ENV )
			if([SKMutablePayment respondsToSelector:@selector(paymentWithProductIdentifier:)])
			{
				payment = [SKMutablePayment performSelector:@selector(paymentWithProductIdentifier:) withObject:productId];
			}
#else
			payment = nil;
#endif
		}
		if(payment)
		{
			payment.quantity = [counted_set countForObject:productId];
			[[SKPaymentQueue defaultQueue] addPayment:payment];
		}
	}
}

- (void)finishTransaction:(Rtt::PlatformStoreTransaction*)transaction
{
	if ( transaction )
	{
		Rtt::AppleStoreTransaction *t = (Rtt::AppleStoreTransaction*)transaction;
		// A restore failure uses a nil transaction object.
		// Our original sample code always called finishedTransaction.
		// Finishing nil causes Apple to throw an exception. I think it is harmless to call finish on nil.
		SKPaymentTransaction* sktransaction = t->GetObject();
		if ( sktransaction )
		{
			[[SKPaymentQueue defaultQueue] finishTransaction:sktransaction];
		}
	}
}

- (void) restoreCompletedTransactions
{
	[[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}

#pragma mark -
#pragma mark SKProductsRequestDelegate methods

- (void)request:(SKRequest *)request didFailWithError:(NSError *)error
{
	using namespace Rtt;
	AppleProductListEvent *event = Rtt_NEW( a, AppleProductListEvent( NULL, nil ) );
	fOwner->GetProductListNotifier().ScheduleDispatch( event );
	
	[request autorelease];
}

-(void) requestDidFinish:(SKRequest *)request
{
	[request autorelease];
}

- (void)productsRequest:(SKProductsRequest*)request didReceiveResponse:(SKProductsResponse*)response
{
	using namespace Rtt;

#if !defined( Rtt_ALLOCATOR_SYSTEM )
	#error This code assumes Rtt_NEW does not use the allocator param.
#endif

	for (SKProduct* product in response.products)
	{
		[fLoadedProducts setObject:product forKey:product.productIdentifier];
	}
	
	AppleProductList *productList = Rtt_NEW( a, AppleProductList( response ) );
	NSArray *invalidList = response.invalidProductIdentifiers;
	AppleProductListEvent *event = Rtt_NEW( a, AppleProductListEvent( productList, invalidList ) );

	fOwner->GetProductListNotifier().ScheduleDispatch( event );
}

#pragma mark -
#pragma mark Helper for SKPaymentTransactionObserver methods

- (void)invokeTransactionCallback:(SKPaymentTransaction*)transaction
{
	using namespace Rtt;

#if !defined( Rtt_ALLOCATOR_SYSTEM )
	#error This code assumes Rtt_NEW does not use the allocator param.
#endif

	AppleStoreTransaction *t = Rtt_NEW( a, AppleStoreTransaction( transaction ) );
	StoreTransactionEvent *event = Rtt_NEW( a, StoreTransactionEvent( t ) );
	
	fOwner->GetTransactionNotifier().ScheduleDispatch( event );
}

//
// called when the transaction was successful
//
- (void)completeTransaction:(SKPaymentTransaction*)transaction
{
	[self invokeTransactionCallback:transaction];
}

//
// called when a transaction has been restored and and successfully completed
//
- (void)restoreTransaction:(SKPaymentTransaction*)transaction
{
	[self invokeTransactionCallback:transaction];
}

//
// called when a transaction has failed
//
- (void)failedTransaction:(SKPaymentTransaction *)transaction
{
/*
    if (transaction.error.code != SKErrorPaymentCancelled)
    {
        // error!
		NSLog(@"failed, %@", [transaction.error localizedDescription]);

    }
    else
    {
        // this is fine, the user just cancelled, so don’t notify
		NSLog(@"user cancelled, %@", [transaction.error localizedDescription]);

    }
*/
	[self invokeTransactionCallback:transaction];

	// Remove the transaction regardless.
	// I think I should let the scripter do this to make usage more consistent.
//	[[SKPaymentQueue defaultQueue] finishTransaction:transaction];
}


#pragma mark -
#pragma mark SKPaymentTransactionObserver methods

//
// called when the transaction status is updated
//
- (void)paymentQueue:(SKPaymentQueue*)queue updatedTransactions:(NSArray*)transactions
{
	for (SKPaymentTransaction* transaction in transactions)
	{
		switch (transaction.transactionState)
		{
			case SKPaymentTransactionStatePurchased:
				[self completeTransaction:transaction];
				break;
			case SKPaymentTransactionStateFailed:
				[self failedTransaction:transaction];
				break;
			case SKPaymentTransactionStateRestored:
				[self restoreTransaction:transaction];
				break;
			case SKPaymentTransactionStatePurchasing:
				break;
			case SKPaymentTransactionStateDeferred:
				break;
		}
	}
}

/*
// This is called after restoreCompletedTransactions successfully finishes. 
// Not implemented currently because we are not sure how to resolve the API differences with Google/Android.
// On Android, there is no differentiation between purchase and restore and no corresponding API to this.
- (void) paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue*)queue
{
}
*/

// This is called if restoreCompletedTransactions has a failure
- (void) paymentQueue:(SKPaymentQueue*)queue restoreCompletedTransactionsFailedWithError:(NSError*)error
{
	// Bug 13526: Hitting cancel at the initial iTunes password login calls this.
	// Since this API doesn't really disinguish between cancelled and failure, we are just passing the failure.
	// Because this API doesn't have a transaction object, I created a special object (AppleStoreRestoreTransactionError)
	// to stand in for the normal one.

	using namespace Rtt;
	
#if !defined( Rtt_ALLOCATOR_SYSTEM )
	#error This code assumes Rtt_NEW does not use the allocator param.
#endif

	AppleStoreRestoreTransactionError *t = Rtt_NEW( a, AppleStoreRestoreTransactionError( error ) );
	StoreTransactionEvent *event = Rtt_NEW( a, StoreTransactionEvent( t ) );
	
	fOwner->GetTransactionNotifier().ScheduleDispatch( event );	
}

-(BOOL)paymentQueue:(SKPaymentQueue *)queue shouldAddStorePayment:(SKPayment *)payment forProduct:(SKProduct *)product
{
	return true;
}

@end

// ----------------------------------------------------------------------------
