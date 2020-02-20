//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>

@interface LiveServer : NSObject

- (instancetype)init;

-(BOOL)addProject:(NSString*)path;
-(void)stopProject:(NSString*)path;


@end


@interface LiveHTTPServer : NSObject

- (instancetype)init;
-(void)setPath:(NSString*)path;
-(NSURL*)getServerURL;

@end
