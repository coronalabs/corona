#import <Foundation/Foundation.h>
#import "XMLRPCEncoder.h"

@interface XMLRPCDefaultEncoder : NSObject <XMLRPCEncoder> {
    NSString *myMethod;
    NSArray *myParameters;
}

@end
