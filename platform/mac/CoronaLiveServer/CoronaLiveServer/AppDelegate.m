//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "AppDelegate.h"
#import "CoronaLiveServerApple.h"

NSString* const CoronaLiveErrorDomain = @"Corona Live";

@interface ServerListEntry : NSObject

@property (strong) NSString *title;
@property (strong) NSString *fullPath;

@end

@implementation ServerListEntry
@end

@interface AppDelegate () <NSTableViewDataSource, NSTableViewDelegate, NSDraggingDestination>

@property (strong) NSMutableArray <ServerListEntry*> * projectsArray;
@property (strong) LiveServer* serverController;
@property (strong) LiveHTTPServer* httpServerController;

@property (weak) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSButton *removeButton;
@property (weak) IBOutlet NSTableView *projectsTable;

@end

static NSString __strong * LIVE_BUILD_CONFIG_NAME = @".CoronaLiveBuild";

@implementation AppDelegate

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app
{
	return NO;
}

-(BOOL)addHTML5Project:(NSString*)project reportErrorToUser:(BOOL)guiError
{
	if(self.httpServerController == nil) {
		self.httpServerController = [[LiveHTTPServer alloc] init];
	}
	[self.httpServerController setPath:project];
	NSURL *url = [self.httpServerController getServerURL];
	if(url) {
		[[NSWorkspace sharedWorkspace] openURL:url];
	} else {
		self.httpServerController = nil;
		if (guiError) {
			[[NSAlert alertWithError:[NSError errorWithDomain:CoronaLiveErrorDomain
														 code:1
													 userInfo:@{NSLocalizedDescriptionKey: @"Error Starting HTML5 Server",
																NSLocalizedFailureReasonErrorKey: @"There was a problem while starting http sever",
																NSLocalizedRecoverySuggestionErrorKey: @"While starting http server there was an error. Make sure that there is no other copy of Live Server running."}]] runModal];

		}
	}
	return YES;
}

-(BOOL)addProject:(NSString*)project reportErrorToUser:(BOOL)guiError atPosition:(NSInteger)index
{
	if(self.serverController == nil) {
		self.serverController = [[LiveServer alloc] init];
	}

	BOOL res = YES;
	NSString *path = [project stringByAppendingPathComponent:LIVE_BUILD_CONFIG_NAME];
	BOOL isDir = NO;
	res = [[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir];
	res = res && !isDir;

	if(!res)
	{
		if(guiError) {
			[[NSAlert alertWithError:[NSError errorWithDomain:CoronaLiveErrorDomain
														 code:1
													 userInfo:@{NSLocalizedDescriptionKey: @"Project is not set up",
																NSLocalizedFailureReasonErrorKey: @"Project is not set up for Live Build",
																NSLocalizedRecoverySuggestionErrorKey: @"Selected project is not set up for the Live Build. Build it with Corona Live Build option enabled and try again."}]] runModal];
		}
		return res;
	}


	res = res && [self.serverController addProject:project];
	if(!res)
	{
		if(guiError) {
			[[NSAlert alertWithError:[NSError errorWithDomain:CoronaLiveErrorDomain
														 code:1
													 userInfo:@{NSLocalizedDescriptionKey: @"Unable to serve selected project",
																NSLocalizedFailureReasonErrorKey: @"Unable to serve selected project",
																NSLocalizedRecoverySuggestionErrorKey: @"There was an error while trying to serve selected project. Please check your '.CoronaLiveBuild' configuration file."}]] runModal];
		}
		return res;
	}


	ServerListEntry *e = [[ServerListEntry alloc] init];
	e.title = [project lastPathComponent];
	e.fullPath = project;

	if(index<0)
	{
		[self.projectsArray addObject:e];
		index = self.projectsArray.count-1;
	}
	else
	{
		[self.projectsArray insertObject:e atIndex:index];
	}

	[self.projectsTable beginUpdates];
	[self.projectsTable insertRowsAtIndexes:[NSIndexSet indexSetWithIndex:index]  withAnimation:NSTableViewAnimationEffectGap];


	NSIndexSet *toRemove = [self.projectsArray indexesOfObjectsPassingTest:^BOOL(ServerListEntry * obj, NSUInteger idx, BOOL * stop) {
		return (obj != e && [obj.fullPath isEqualToString:project]);
	}];

	[self.projectsTable removeRowsAtIndexes:toRemove withAnimation:NSTableViewAnimationSlideRight];
	[self.projectsArray removeObjectsAtIndexes:toRemove];

	[self.projectsTable endUpdates];

	[[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:[NSURL fileURLWithPath:project]];

	if(!self.window.visible) {
		[self.window orderBack:nil];
	}

	return res;
}


-(BOOL)application:(NSApplication *)sender openFile:(NSString *)filename
{
	return [self addProject:filename reportErrorToUser:YES atPosition:-1];
}

-(BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag {
	if(!flag) {
		[self.window orderFront:nil];
	}
	return YES;
}

-(void)applicationDidFinishLaunching:(NSNotification *)notification
{
	[self.projectsTable registerForDraggedTypes:@[NSFilenamesPboardType]];
}

-(void)html5ProjectBuilt:(NSNotification*)notification
{
	if(notification && notification.userInfo) {
		NSString* root = [notification.userInfo objectForKey:@"root"];
		if(root) {
			[self addHTML5Project:root reportErrorToUser:YES];
		}
	}
}

-(void)applicationWillFinishLaunching:(NSNotification *)notification
{
	self.projectsArray = [[NSMutableArray alloc] init];
	[[NSDistributedNotificationCenter defaultCenter] addObserver:self
													    selector:@selector(html5ProjectBuilt:)
													 	    name:@"html5ProjectBuilt"
														  object:nil];

}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
	self.projectsArray = nil;
}



- (IBAction)showServerList:(id)sender {
	[self.window orderFrontRegardless];
}


- (IBAction)browseForProject:(id)sender {
	NSOpenPanel *openPanel = [NSOpenPanel openPanel];
	openPanel.canChooseDirectories = YES;
	openPanel.canChooseFiles = NO;
	openPanel.allowsMultipleSelection = NO;
	if([openPanel runModal] == NSOKButton) {
		if(openPanel.URLs.firstObject.isFileURL) {
			[self addProject:openPanel.URLs.firstObject.path reportErrorToUser:YES atPosition:-1];
		}
	}
}

-(NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
	return [self.projectsArray count];
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	NSTableCellView * cellView = [tableView makeViewWithIdentifier:tableColumn.identifier owner:self];

	if([tableColumn.identifier isEqualToString:@"Servers"]) {
		ServerListEntry *s = [self.projectsArray objectAtIndex:row];
		cellView.textField.stringValue = s.title;
		NSString *icon = [s.fullPath stringByAppendingPathComponent:@"Icon.png"];
		if([[NSFileManager defaultManager] fileExistsAtPath:icon]) {
			cellView.imageView.image = [[NSImage alloc] initWithContentsOfFile:icon]; //autorelease
		} else {
			cellView.imageView.image = [NSApp applicationIconImage];
		}
		NSString *toolTip = s.fullPath;
		NSString *homeDir = NSHomeDirectory();
		if([toolTip hasPrefix:homeDir]) {
			toolTip = [toolTip stringByReplacingCharactersInRange:NSMakeRange(0, homeDir.length) withString:@"~"];
		}
		cellView.toolTip = toolTip;
	}

	return cellView;
}

-(void)tableViewSelectionDidChange:(NSNotification *)notification {
	self.removeButton.enabled = (self.projectsTable.numberOfSelectedRows>0);
}

- (IBAction)removeSelected:(id)sender {
	NSIndexSet *indices = self.projectsTable.selectedRowIndexes;
	[indices enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL*stop) {
		ServerListEntry *se = [self.projectsArray objectAtIndex:idx];
		[self.serverController stopProject:se.fullPath];
	}];
	[self.projectsArray removeObjectsAtIndexes:indices];
	[self.projectsTable beginUpdates];
	[self.projectsTable removeRowsAtIndexes:indices withAnimation:NSTableViewAnimationSlideRight];
	[self.projectsTable endUpdates];
}


-(BOOL)tableView:(NSTableView *)tableView acceptDrop:(id<NSDraggingInfo>)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)dropOperation
{
	NSPasteboard *pb = [info draggingPasteboard];
	BOOL res = [self addFromPasteboard:pb guiError:NO atPosition:row];
	if(!res) {
		NSBeep();
	}
	return res;
}

-(NSDragOperation)tableView:(NSTableView *)tableView validateDrop:(id<NSDraggingInfo>)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)dropOperation
{

	NSPasteboard *pb = [info draggingPasteboard];

	__block NSDragOperation res = NSDragOperationNone;

	NSArray<NSURL*> *urls = [pb readObjectsForClasses:@[[NSURL class]] options:[NSDictionary dictionary]];
	[urls enumerateObjectsUsingBlock:^(NSURL* url, NSUInteger idx, BOOL * stop) {
		NSURL *config = [url URLByAppendingPathComponent:LIVE_BUILD_CONFIG_NAME];
		if([[NSFileManager defaultManager] fileExistsAtPath:config.path]) {
			res = NSDragOperationCopy;
			*stop = YES;
		}
	}];

	return res;

}

- (IBAction)doubleClickProject:(id)sender {
	NSIndexSet *indices = self.projectsTable.selectedRowIndexes;
	[indices enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL*stop) {
		ServerListEntry *se = [self.projectsArray objectAtIndex:idx];
		[[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:se.fullPath] ];
	}];
}


- (IBAction)cut:(id)sender {
	[self copy:nil];
	[self removeSelected:nil];
}

- (IBAction)copy:(id)sender {
	NSPasteboard *pb = [NSPasteboard generalPasteboard];
	[pb clearContents];
	
	NSIndexSet *indices = self.projectsTable.selectedRowIndexes;
	NSMutableArray<NSString*> *paths = [NSMutableArray arrayWithCapacity:self.projectsArray.count];
	[indices enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL*stop) {
		ServerListEntry *se = [self.projectsArray objectAtIndex:idx];
		[paths addObject:se.fullPath];
	}];

	[pb setString:[paths componentsJoinedByString:@"\n"] forType:NSStringPboardType];
}

- (IBAction)paste:(id)sender {
	NSPasteboard *pb = [NSPasteboard generalPasteboard];
	if(![self addFromPasteboard:pb guiError:NO atPosition:-1]) {
		NSBeep();
	}
}

- (IBAction)pasteAsPlainText:(id)sender {
	[self paste:nil];
}

- (IBAction)delete:(id)sender {
	[self removeSelected:nil];
}

-(BOOL)addFromPasteboard:(NSPasteboard*)pb guiError:(BOOL)gui atPosition:(NSInteger)row {
	__block BOOL res = NO;

	NSArray<NSURL*> *urls = [pb readObjectsForClasses:@[[NSURL class]] options:[NSDictionary dictionary]];
	[urls enumerateObjectsUsingBlock:^(NSURL* url, NSUInteger idx, BOOL * stop) {
		NSURL *config = [url URLByAppendingPathComponent:LIVE_BUILD_CONFIG_NAME];
		if([[NSFileManager defaultManager] fileExistsAtPath:config.path]) {
			res = [self addProject:url.path reportErrorToUser:NO atPosition:row];
		}
	}];

	if(res) return YES;

	[[[pb stringForType:NSStringPboardType] componentsSeparatedByString:@"\n"] enumerateObjectsUsingBlock:^(NSString * path, NSUInteger idx, BOOL * stop) {
		if([[NSFileManager defaultManager] fileExistsAtPath:[path stringByAppendingPathComponent:LIVE_BUILD_CONFIG_NAME]]) {
			res = [self addProject:path reportErrorToUser:NO atPosition:row];
		}
	}];

	return res;
}

- (IBAction)onlineDocs:(id)sender {
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://docs.coronalabs.com/guide/distribution/liveBuild/"]];
}

- (IBAction)openHTTPUrl:(id)sender {
	NSURL *url = [self.httpServerController getServerURL];
	if(url) {
		[[NSWorkspace sharedWorkspace] openURL:url];
	} else {
		self.httpServerController = nil;
	}
}

- (IBAction)setHTTPURL:(id)sender {
	NSOpenPanel *openPanel = [NSOpenPanel openPanel];
	openPanel.canChooseDirectories = YES;
	openPanel.canChooseFiles = NO;
	openPanel.allowsMultipleSelection = NO;
	if([openPanel runModal] == NSOKButton) {
		if(openPanel.URLs.firstObject.isFileURL) {
			[self addHTML5Project:openPanel.URLs.firstObject.path reportErrorToUser:YES];
		}
	}
}

@end


