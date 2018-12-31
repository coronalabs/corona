/* Author: Landon Fuller <landonf@plausiblelabs.com>
 * Copyright (c) 2008-2011 Plausible Labs Cooperative, Inc.
 * All rights reserved.
 *
 * See the LICENSE file for the license on the source code in this file.
 */

#import "iPhoneSimulator.h"
#import "NSString+expandPath.h"
#import "nsprintf.h"
#import <sys/types.h>
#import <sys/stat.h>
@class DTiPhoneSimulatorSystemRoot;

NSString *simulatorPrefrencesName = @"com.apple.iphonesimulator";
NSString *deviceProperty = @"SimulateDevice";
NSString *deviceIphoneRetina3_5Inch = @"iPhone Retina (3.5-inch)";
NSString *deviceIphoneRetina4_0Inch = @"iPhone Retina (4-inch)";
NSString *deviceIphoneRetina4_0Inch_64bit = @"iPhone Retina (4-inch 64-bit)";
NSString *deviceIphone = @"iPhone";
NSString *deviceIpad = @"iPad";
NSString *deviceIpadRetina = @"iPad Retina";
NSString *deviceIpadRetina_64bit = @"iPad Retina (64-bit)";

NSString* deviceTypeIdIphone4s = @"com.apple.CoreSimulator.SimDeviceType.iPhone-4s";
NSString* deviceTypeIdIphone5 = @"com.apple.CoreSimulator.SimDeviceType.iPhone-5";
NSString* deviceTypeIdIphone5s = @"com.apple.CoreSimulator.SimDeviceType.iPhone-5s";
NSString* deviceTypeIdIphone6 = @"com.apple.CoreSimulator.SimDeviceType.iPhone-6";
NSString* deviceTypeIdIphone6Plus = @"com.apple.CoreSimulator.SimDeviceType.iPhone-6-Plus";
NSString* deviceTypeIdIpad2 = @"com.apple.CoreSimulator.SimDeviceType.iPad-2";
NSString* deviceTypeIdIpadRetina = @"com.apple.CoreSimulator.SimDeviceType.iPad-Retina";
NSString* deviceTypeIdIpadAir = @"com.apple.CoreSimulator.SimDeviceType.iPad-Air";
NSString* deviceTypeIdResizableIphone = @"com.apple.CoreSimulator.SimDeviceType.Resizable-iPhone";
NSString* deviceTypeIdResizeableIpad = @"com.apple.CoreSimulator.SimDeviceType.Resizable-iPad";

// The path within the developer dir of the private Simulator frameworks.
NSString* const kSimulatorFrameworkRelativePathLegacy = @"Platforms/iPhoneSimulator.platform/Developer/Library/PrivateFrameworks/DVTiPhoneSimulatorRemoteClient.framework";
NSString* const kSimulatorFrameworkRelativePath = @"../SharedFrameworks/DVTiPhoneSimulatorRemoteClient.framework";
NSString* const kDVTFoundationRelativePath = @"../SharedFrameworks/DVTFoundation.framework";
NSString* const kDevToolsFoundationRelativePath = @"../OtherFrameworks/DevToolsFoundation.framework";
NSString* const kSimulatorRelativePath = @"Platforms/iPhoneSimulator.platform/Developer/Applications/iPhone Simulator.app";
NSString* const kCoreSimulatorRelativePath = @"Library/PrivateFrameworks/CoreSimulator.framework";

static pid_t gDebuggerProcessId;

@interface DVTPlatform : NSObject
+ (BOOL)loadAllPlatformsReturningError:(id*)arg1;
@end

/**
 * A simple iPhoneSimulatorRemoteClient framework.
 */
@implementation iPhoneSimulator

// Helper to find a class by name and die if it isn't found.
-(Class) FindClassByName:(NSString*) nameOfClass {
    Class theClass = NSClassFromString(nameOfClass);
    if (!theClass) {
        nsfprintf(stderr,@"Failed to find class %@ at runtime.", nameOfClass);
        exit(EXIT_FAILURE);
    }
    return theClass;
}

// Loads the Simulator framework from the given developer dir.
-(void) LoadSimulatorFramework:(NSString*) developerDir {
    // The Simulator framework depends on some of the other Xcode private
    // frameworks; manually load them first so everything can be linked up.
    NSString* dvtFoundationPath = [developerDir stringByAppendingPathComponent:kDVTFoundationRelativePath];

    NSBundle* dvtFoundationBundle =
    [NSBundle bundleWithPath:dvtFoundationPath];
    if (![dvtFoundationBundle load]){
        nsprintf(@"Unable to dvtFoundationBundle. Error: ");
        exit(EXIT_FAILURE);
        return ;
    }
    NSString* devToolsFoundationPath = [developerDir stringByAppendingPathComponent:kDevToolsFoundationRelativePath];
    NSBundle* devToolsFoundationBundle =
    [NSBundle bundleWithPath:devToolsFoundationPath];
    if (![devToolsFoundationBundle load]){
        nsprintf(@"Unable to devToolsFoundationPath. Error: ");
        return ;
    }
    NSString* coreSimulatorPath = [developerDir stringByAppendingPathComponent:kCoreSimulatorRelativePath];
    if ([[NSFileManager defaultManager] fileExistsAtPath:coreSimulatorPath]) {
        NSBundle* coreSimulatorBundle = [NSBundle bundleWithPath:coreSimulatorPath];
        if (![coreSimulatorBundle load]){
            nsprintf(@"Unable to coreSimulatorPath. Error: ");
            return ;
        }
    }
    // Prime DVTPlatform.
    NSError* error;
    Class DVTPlatformClass = [self FindClassByName:@"DVTPlatform"];
    if (![DVTPlatformClass loadAllPlatformsReturningError:&error]) {
        nsprintf(@"Unable to loadAllPlatformsReturningError. Error: %@",[error localizedDescription]);
        return ;
    }
    NSString* simBundlePath = [developerDir stringByAppendingPathComponent:kSimulatorFrameworkRelativePathLegacy];
    if (![[NSFileManager defaultManager] fileExistsAtPath:simBundlePath]){
        simBundlePath = [developerDir stringByAppendingPathComponent:kSimulatorFrameworkRelativePath];
    }
    NSBundle* simBundle = [NSBundle bundleWithPath:simBundlePath];
    if (![simBundle load]){
        nsprintf(@"Unable to load simulator framework. Error: %@",[error localizedDescription]);
        return ;
    }
    return ;
}

NSString* GetXcodeVersion() {
    // Go look for it via xcodebuild.
    NSTask* xcodeBuildTask = [[[NSTask alloc] init] autorelease];
    [xcodeBuildTask setLaunchPath:@"/usr/bin/xcodebuild"];
    [xcodeBuildTask setArguments:[NSArray arrayWithObject:@"-version"]];

    NSPipe* outputPipe = [NSPipe pipe];
    [xcodeBuildTask setStandardOutput:outputPipe];
    NSFileHandle* outputFile = [outputPipe fileHandleForReading];

    [xcodeBuildTask launch];
    NSData* outputData = [outputFile readDataToEndOfFile];
    [xcodeBuildTask terminate];

    NSString* output =
    [[[NSString alloc] initWithData:outputData
                           encoding:NSUTF8StringEncoding] autorelease];
    output = [output stringByTrimmingCharactersInSet:
              [NSCharacterSet whitespaceAndNewlineCharacterSet]];
    if ([output length] == 0) {
        output = nil;
    } else {
        NSArray* parts = [output componentsSeparatedByCharactersInSet: [NSCharacterSet whitespaceAndNewlineCharacterSet]];
        if ([parts count] >= 2) {
            return parts[1];
        }
    }
    return output;
}



// Finds the developer dir via xcode-select or the DEVELOPER_DIR environment
// variable.
NSString* FindDeveloperDir() {
    // Check the env first.
    NSDictionary* env = [[NSProcessInfo processInfo] environment];
    NSString* developerDir = [env objectForKey:@"DEVELOPER_DIR"];
    if ([developerDir length] > 0) {
        return developerDir;
    }

    // Go look for it via xcode-select.
    NSTask* xcodeSelectTask = [[[NSTask alloc] init] autorelease];
    [xcodeSelectTask setLaunchPath:@"/usr/bin/xcode-select"];
    [xcodeSelectTask setArguments:[NSArray arrayWithObject:@"-print-path"]];

    NSPipe* outputPipe = [NSPipe pipe];
    [xcodeSelectTask setStandardOutput:outputPipe];
    NSFileHandle* outputFile = [outputPipe fileHandleForReading];

    [xcodeSelectTask launch];
    NSData* outputData = [outputFile readDataToEndOfFile];
    [xcodeSelectTask terminate];

    NSString* output =
    [[[NSString alloc] initWithData:outputData
                           encoding:NSUTF8StringEncoding] autorelease];
    output = [output stringByTrimmingCharactersInSet:
              [NSCharacterSet whitespaceAndNewlineCharacterSet]];
    if ([output length] == 0) {
        output = nil;
    }
    return output;
}

- (void) printUsage {
  fprintf(stderr, "Usage: ios-sim <command> <options> [--args ...]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Commands:\n");
  fprintf(stderr, "  showsdks                        List the available iOS SDK versions\n");
  fprintf(stderr, "  showdevicetypes                 List the available device types (Xcode6+)\n");
  fprintf(stderr, "  launch <application path>       Launch the application at the specified path on the iOS Simulator\n");
  fprintf(stderr, "  start                           Launch iOS Simulator without an app\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  --version                       Print the version of ios-sim\n");
  fprintf(stderr, "  --help                          Show this help text\n");
  fprintf(stderr, "  --verbose                       Set the output level to verbose\n");
  fprintf(stderr, "  --exit                          Exit after startup\n");
  fprintf(stderr, "  --debug                         Attach LLDB to the application on startup\n");
  fprintf(stderr, "  --use-gdb                       Use GDB instead of LLDB. (Requires --debug)\n");
  fprintf(stderr, "  --uuid <uuid>                   A UUID identifying the session (is that correct?)\n");
  fprintf(stderr, "  --env <environment file path>   A plist file containing environment key-value pairs that should be set\n");
  fprintf(stderr, "  --setenv NAME=VALUE             Set an environment variable\n");
  fprintf(stderr, "  --stdout <stdout file path>     The path where stdout of the simulator will be redirected to (defaults to stdout of ios-sim)\n");
  fprintf(stderr, "  --stderr <stderr file path>     The path where stderr of the simulator will be redirected to (defaults to stderr of ios-sim)\n");
  fprintf(stderr, "  --timeout <seconds>             The timeout time to wait for a response from the Simulator. Default value: 30 seconds\n");
  fprintf(stderr, "  --args <...>                    All following arguments will be passed on to the application\n");
  fprintf(stderr, "  --devicetypeid <device type>    The id of the device type that should be simulated (Xcode6+). Use 'showdevicetypes' to list devices.\n");
  fprintf(stderr, "                                  e.g \"com.apple.CoreSimulator.SimDeviceType.Resizable-iPhone6, 8.0\"\n");
  fprintf(stderr, "DEPRECATED in 3.x, use devicetypeid instead:\n");
  fprintf(stderr, "  --sdk <sdkversion>              The iOS SDK version to run the application on (defaults to the latest)\n");
  fprintf(stderr, "  --family <device family>        The device type that should be simulated (defaults to `iphone')\n");
  fprintf(stderr, "  --retina                        Start a retina device\n");
  fprintf(stderr, "  --tall                          In combination with --retina flag, start the tall version of the retina device (e.g. iPhone 5 (4-inch))\n");
  fprintf(stderr, "  --64bit                         In combination with --retina flag and the --tall flag, start the 64bit version of the tall retina device (e.g. iPhone 5S (4-inch 64bit))\n");
}

- (void) printDeprecation:(char*)option {
    // fprintf(stderr, "Usage of '%s' is deprecated in 3.x. Use --devicetypeid instead.\n", option);
}


- (int) showSDKs {
  Class systemRootClass = [self FindClassByName:@"DTiPhoneSimulatorSystemRoot"];

  NSArray *roots = [systemRootClass knownRoots];

  nsprintf(@"Simulator SDK Roots:");
  for (DTiPhoneSimulatorSystemRoot *root in roots) {
    nsfprintf(stderr, @"'%@' (%@)\n\t%@", [root sdkDisplayName], [root sdkVersion], [root sdkRootPath]);
  }

  return EXIT_SUCCESS;
}

- (int) showDeviceTypes {
    Class simDeviceSet = NSClassFromString(@"SimDeviceSet");
    if (simDeviceSet) {
        SimDeviceSet* deviceSet = [simDeviceSet defaultSet];
        NSArray* devices = [deviceSet availableDevices];
        for (SimDevice* device in devices) {
            nsfprintf(stderr, @"%@, %@", device.deviceType.identifier, device.runtime.versionString);
        }
    }

    return EXIT_SUCCESS;
}

- (void)session:(DTiPhoneSimulatorSession *)session didEndWithError:(NSError *)error {
  if (verbose) {
    nsprintf(@"Session did end with error %@", error);
  }

  if (stderrFileHandle != nil) {
    NSString *stderrPath = [[session sessionConfig] simulatedApplicationStdErrPath];
    [self removeStdioFIFO:stderrFileHandle atPath:stderrPath];
  }

  if (stdoutFileHandle != nil) {
    NSString *stdoutPath = [[session sessionConfig] simulatedApplicationStdOutPath];
    [self removeStdioFIFO:stdoutFileHandle atPath:stdoutPath];
  }

  if (error != nil) {
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}

static void IgnoreSignal(int arg) {
}

static void ChildSignal(int arg) {
  int status;
  waitpid(gDebuggerProcessId, &status, 0);
  exit(EXIT_SUCCESS);
}

- (void)session:(DTiPhoneSimulatorSession *)session didStart:(BOOL)started withError:(NSError *)error {
  if (startOnly && session) {
    nsprintf(@"Simulator started (no session)");
    exit(EXIT_SUCCESS);
  }
  if (started) {
      if (shouldStartDebugger) {
        int pid  = [session simulatedApplicationPID];
        char*args[4] = { NULL, NULL, (char*)[[@(pid) description] UTF8String], NULL };
        if (useGDB) {
          args[0] = "gdb";
          args[1] = "program";
        } else {
          args[0] = "lldb";
          args[1] = "--attach-pid";
        }
        // The parent process must live on to process the stdout/stderr fifos,
        // so start the debugger as a child process.
        pid_t child_pid = fork();
        if (child_pid == 0) {
            execvp(args[0], args);
        } else if (child_pid < 0) {
            nsprintf(@"Could not start debugger process: %@", errno);
            exit(EXIT_FAILURE);
        }
        gDebuggerProcessId = child_pid;
        signal(SIGINT, IgnoreSignal);
        signal(SIGCHLD, ChildSignal);
      }
    if (verbose) {
      nsprintf(@"Session started");
    }
    if (exitOnStartup) {
      exit(EXIT_SUCCESS);
    }
  } else {
    nsprintf(@"Session could not be started: %@", error);
    exit(EXIT_FAILURE);
  }
}


- (void)stdioDataIsAvailable:(NSNotification *)notification {
  [[notification object] readInBackgroundAndNotify];
  NSData *data = [[notification userInfo] valueForKey:NSFileHandleNotificationDataItem];
  NSString *str = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
  if (!alreadyPrintedData) {
    if ([str length] == 0) {
      return;
    } else {
      alreadyPrintedData = YES;
    }
  }
  if ([notification object] == stdoutFileHandle) {
    printf("%s", [str UTF8String]);
  } else {
    nsprintf(@"%@", str);
  }
}


- (void)createStdioFIFO:(NSFileHandle **)fileHandle ofType:(NSString *)type atPath:(NSString **)path {
  *path = [NSString stringWithFormat:@"%@/ios-sim-%@-pipe-%d", NSTemporaryDirectory(), type, (int)time(NULL)];
  if (mkfifo([*path UTF8String], S_IRUSR | S_IWUSR) == -1) {
    nsprintf(@"Unable to create %@ named pipe `%@'", type, *path);
    exit(EXIT_FAILURE);
  } else {
    if (verbose) {
      nsprintf(@"Creating named pipe at `%@'", *path);
    }
    int fd = open([*path UTF8String], O_RDONLY | O_NDELAY);
    *fileHandle = [[[NSFileHandle alloc] initWithFileDescriptor:fd] retain];
    [*fileHandle readInBackgroundAndNotify];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(stdioDataIsAvailable:)
                                                 name:NSFileHandleReadCompletionNotification
                                               object:*fileHandle];
  }
}


- (void)removeStdioFIFO:(NSFileHandle *)fileHandle atPath:(NSString *)path {
  if (verbose) {
    nsprintf(@"Removing named pipe at `%@'", path);
  }
  [fileHandle closeFile];
  [fileHandle release];
  if (![[NSFileManager defaultManager] removeItemAtPath:path error:NULL]) {
    nsprintf(@"Unable to remove named pipe `%@'", path);
  }
}


- (int)launchApp:(NSString *)path withFamily:(NSString *)family
                                        uuid:(NSString *)uuid
                                 environment:(NSDictionary *)environment
                                  stdoutPath:(NSString *)stdoutPath
                                  stderrPath:(NSString *)stderrPath
                                     timeout:(NSTimeInterval)timeout
                                        args:(NSArray *)args {
  DTiPhoneSimulatorApplicationSpecifier *appSpec;
  DTiPhoneSimulatorSessionConfig *config;
  DTiPhoneSimulatorSession *session;
  NSError *error;

  NSFileManager *fileManager = [[[NSFileManager alloc] init] autorelease];
  if (!startOnly && ![fileManager fileExistsAtPath:path]) {
    nsprintf(@"Application path %@ doesn't exist!", path);
    exit(EXIT_FAILURE);
  }

  /* Create the app specifier */
    appSpec = startOnly ? nil : [[self FindClassByName:@"DTiPhoneSimulatorApplicationSpecifier"] specifierWithApplicationPath:path];

  if (verbose) {
    nsprintf(@"App Spec: %@", appSpec);
    nsprintf(@"SDK Root: %@", sdkRoot);

    for (id key in environment) {
      nsprintf(@"Env: %@ = %@", key, [environment objectForKey:key]);
    }
  }

  /* Set up the session configuration */
  config = [[[[self FindClassByName:@"DTiPhoneSimulatorSessionConfig"] alloc] init] autorelease];
  [config setApplicationToSimulateOnStart:appSpec];
  [config setSimulatedSystemRoot:sdkRoot];
  [config setSimulatedApplicationShouldWaitForDebugger:shouldStartDebugger];

  [config setSimulatedApplicationLaunchArgs:args];
  [config setSimulatedApplicationLaunchEnvironment:environment];

  if (stderrPath) {
    stderrFileHandle = nil;
  } else if (!exitOnStartup) {
    [self createStdioFIFO:&stderrFileHandle ofType:@"stderr" atPath:&stderrPath];
  }
  [config setSimulatedApplicationStdErrPath:stderrPath];

  if (stdoutPath) {
    stdoutFileHandle = nil;
  } else if (!exitOnStartup) {
    [self createStdioFIFO:&stdoutFileHandle ofType:@"stdout" atPath:&stdoutPath];
  }
  [config setSimulatedApplicationStdOutPath:stdoutPath];

  [config setLocalizedClientName: @"ios-sim"];

  // this was introduced in 3.2 of SDK
  if ([config respondsToSelector:@selector(setSimulatedDeviceFamily:)]) {
    if (family == nil) {
      family = @"iphone";
    }

    if (verbose) {
      nsprintf(@"using device family %@",family);
    }

    if ([family isEqualToString:@"ipad"]) {
[config setSimulatedDeviceFamily:[NSNumber numberWithInt:2]];
    } else{
      [config setSimulatedDeviceFamily:[NSNumber numberWithInt:1]];
    }
  }

  if ([config respondsToSelector:@selector(setDevice:)]) {
    // Xcode6+
    config.device = [self findDeviceWithFamily:family retina:retinaDevice isTallDevice:tallDevice is64Bit:is64BitDevice];

    // The iOS 8 simulator treats stdout/stderr paths relative to the simulator's data directory.
    // Create symbolic links in the data directory that points at the real stdout/stderr paths.
    if ([config.simulatedSystemRoot.sdkVersion isEqual:@"8.0"]) {
      NSString* dataPath = config.device.dataPath;
      [[NSFileManager defaultManager] createSymbolicLinkAtPath:[dataPath stringByAppendingPathComponent:stdoutPath] withDestinationPath:stdoutPath error:NULL];
      [[NSFileManager defaultManager] createSymbolicLinkAtPath:[dataPath stringByAppendingPathComponent:stderrPath] withDestinationPath:stderrPath error:NULL];
    }
  } else {
    // Xcode5 or older
    NSString* devicePropertyValue = [self changeDeviceType:family retina:retinaDevice isTallDevice:tallDevice is64Bit:is64BitDevice];
    [config setSimulatedDeviceInfoName:devicePropertyValue];
  }

  /* Start the session */
  session = [[[[self FindClassByName:@"DTiPhoneSimulatorSession"] alloc] init] autorelease];
  [session setDelegate:self];
  if (uuid != nil){
    [session setUuid:uuid];
  }

  if (![session requestStartWithConfig:config timeout:timeout error:&error]) {
    nsprintf(@"Could not start simulator session: %@", error);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

- (SimDevice*) findDeviceWithFamily:(NSString *)family retina:(BOOL)retina isTallDevice:(BOOL)isTallDevice is64Bit:(BOOL)is64Bit {
    NSString* devTypeId = self->deviceTypeId;

    if (!devTypeId) {
        devTypeId = deviceTypeIdIphone5;
        if (retina) {
            if ([family isEqualToString:@"ipad"]) {
                if (is64Bit) {
                    devTypeId = deviceTypeIdIpadAir;
                } else {
                    devTypeId = deviceTypeIdIpadRetina;
                }
            } else {
                if (isTallDevice) {
                    if (is64Bit) {
                        devTypeId = deviceTypeIdIphone5s;
                    } else {
                        devTypeId = deviceTypeIdIphone5;
                    }
                } else {
                    devTypeId = deviceTypeIdIphone4s;
                }
            }
        } else {
            if ([family isEqualToString:@"ipad"]) {
                devTypeId = deviceTypeIdIpad2;
            } else {
                devTypeId = deviceTypeIdIphone4s;
            }
        }
    }

    SimDeviceSet* deviceSet = [[self FindClassByName:@"SimDeviceSet"] defaultSet];
    NSArray* devices = [deviceSet availableDevices];
	NSArray* deviceTypeAndVersion = [devTypeId componentsSeparatedByString:@","];
	if(deviceTypeAndVersion.count == 2) {
		NSString* typeIdentifier = [deviceTypeAndVersion.firstObject stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
		NSString* versionString = [deviceTypeAndVersion.lastObject stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];;
		for (SimDevice* device in devices) {
			if ([device.deviceType.identifier isEqualToString:typeIdentifier] && [device.runtime.versionString isEqualToString:versionString]) {
				return device;
			}
		}
	}
	//maintain old behavior (if the device identifier doesn't have a version as part of the identifier, loop through to find the first matching)
	else
	{
		for (SimDevice* device in devices) {
			if ([device.deviceType.identifier isEqualToString:devTypeId]) {
				return device;
			}
		}
	}
    // Default to whatever is the first device
    return [devices count] > 0 ? [devices objectAtIndex:0] : nil;
}

- (NSString*) changeDeviceType:(NSString *)family retina:(BOOL)retina isTallDevice:(BOOL)isTallDevice is64Bit:(BOOL)is64Bit {
  NSString *devicePropertyValue;
  if (retina) {
    if (verbose) {
      nsprintf(@"using retina");
    }
    if ([family isEqualToString:@"ipad"]) {
      if (is64Bit) {
        devicePropertyValue = deviceIpadRetina_64bit;
      } else {
        devicePropertyValue = deviceIpadRetina;
      }
    }
    else {
        if (isTallDevice) {
          if (is64Bit) {
            devicePropertyValue = deviceIphoneRetina4_0Inch_64bit;
          } else {
            devicePropertyValue = deviceIphoneRetina4_0Inch;
          }
        } else {
            devicePropertyValue = deviceIphoneRetina3_5Inch;
        }
    }
  } else {
    if ([family isEqualToString:@"ipad"]) {
      devicePropertyValue = deviceIpad;
    } else {
      devicePropertyValue = deviceIphone;
    }
  }
  CFPreferencesSetAppValue((CFStringRef)deviceProperty, (CFPropertyListRef)devicePropertyValue, (CFStringRef)simulatorPrefrencesName);
  CFPreferencesAppSynchronize((CFStringRef)simulatorPrefrencesName);

  return devicePropertyValue;
}


/**
 * Execute 'main'
 */
- (void)runWithArgc:(int)argc argv:(char **)argv {
  if (argc < 2) {
    [self printUsage];
    exit(EXIT_FAILURE);
  }
  
  NSString* xcodeVersion = GetXcodeVersion();
  if (!([xcodeVersion compare:@"6.0" options:NSNumericSearch] != NSOrderedAscending)) {
      nsprintf(@"You need to have at least Xcode 6.0 installed -- you have version %@.", xcodeVersion);
      nsprintf(@"Run 'xcode-select --print-path' to check which version of Xcode is enabled.");
      exit(EXIT_FAILURE);
  }

  retinaDevice = NO;
  tallDevice = NO;
  is64BitDevice = NO;
  exitOnStartup = NO;
  alreadyPrintedData = NO;
  startOnly = strcmp(argv[1], "start") == 0;
  deviceTypeId = nil;

  NSString* developerDir = FindDeveloperDir();
  if (!developerDir) {
    nsprintf(@"Unable to find developer directory.");
    exit(EXIT_FAILURE);
  }

  if (strcmp(argv[1], "showsdks") == 0) {
	[self LoadSimulatorFramework:developerDir];
    exit([self showSDKs]);
  } else if (strcmp(argv[1], "showdevicetypes") == 0) {
    [self LoadSimulatorFramework:developerDir];
     exit([self showDeviceTypes]);
  } else if (strcmp(argv[1], "launch") == 0 || startOnly) {
    if (strcmp(argv[1], "launch") == 0 && argc < 3) {
      fprintf(stderr, "Missing application path argument\n");
      [self printUsage];
      exit(EXIT_FAILURE);
    }

    NSString *appPath = nil;
    int argOffset;
    if (startOnly) {
      argOffset = 2;
    }
    else {
      argOffset = 3;
      appPath = [[NSString stringWithUTF8String:argv[2]] expandPath];
    }

    NSString *family = nil;
    NSString *uuid = nil;
    NSString *stdoutPath = nil;
    NSString *stderrPath = nil;
    NSString *xctest = nil;
    NSTimeInterval timeout = 30;
    NSMutableDictionary *environment = [NSMutableDictionary dictionary];

    int i = argOffset;
    for (; i < argc; i++) {
      if (strcmp(argv[i], "--version") == 0) {
        printf("%s\n", IOS_SIM_VERSION);
        exit(EXIT_SUCCESS);
      } else if (strcmp(argv[i], "--help") == 0) {
        [self printUsage];
        exit(EXIT_SUCCESS);
      } else if (strcmp(argv[i], "--verbose") == 0) {
        verbose = YES;
      } else if (strcmp(argv[i], "--exit") == 0) {
        exitOnStartup = YES;
      } else if (strcmp(argv[i], "--debug") == 0) {
        shouldStartDebugger = YES;
      } else if (strcmp(argv[i], "--use-gdb") == 0) {
        useGDB = YES;
      } else if (strcmp(argv[i], "--timeout") == 0) {
        if (i + 1 < argc) {
          timeout = [[NSString stringWithUTF8String:argv[++i]] doubleValue];
          NSLog(@"Timeout: %f second(s)", timeout);
        }
      }
      else if (strcmp(argv[i], "--sdk") == 0) {
        [self printDeprecation:argv[i]];
        i++;
	   [self LoadSimulatorFramework:developerDir];
        NSString* ver = [NSString stringWithCString:argv[i] encoding:NSUTF8StringEncoding];
        Class systemRootClass = [self FindClassByName:@"DTiPhoneSimulatorSystemRoot"];
        NSArray *roots = [systemRootClass knownRoots];
        for (DTiPhoneSimulatorSystemRoot *root in roots) {
          NSString *v = [root sdkVersion];
          if ([v isEqualToString:ver]) {
            sdkRoot = root;
            break;
          }
        }
        if (sdkRoot == nil) {
          fprintf(stderr,"Unknown or unsupported SDK version: %s\n",argv[i]);
          [self showSDKs];
          exit(EXIT_FAILURE);
        }
      } else if (strcmp(argv[i], "--family") == 0) {
        [self printDeprecation:argv[i]];
        i++;
        family = [NSString stringWithUTF8String:argv[i]];
      } else if (strcmp(argv[i], "--uuid") == 0) {
        i++;
        uuid = [NSString stringWithUTF8String:argv[i]];
      } else if (strcmp(argv[i], "--devicetypeid") == 0) {
          i++;
          deviceTypeId = [NSString stringWithUTF8String:argv[i]];
      } else if (strcmp(argv[i], "--setenv") == 0) {
        i++;
        NSArray *parts = [[NSString stringWithUTF8String:argv[i]] componentsSeparatedByString:@"="];
        [environment setObject:[parts objectAtIndex:1] forKey:[parts objectAtIndex:0]];
      } else if (strcmp(argv[i], "--env") == 0) {
        i++;
        NSString *envFilePath = [[NSString stringWithUTF8String:argv[i]] expandPath];
        [environment setValuesForKeysWithDictionary:[NSDictionary dictionaryWithContentsOfFile:envFilePath]];
        if (!environment) {
          fprintf(stderr, "Could not read environment from file: %s\n", argv[i]);
          [self printUsage];
          exit(EXIT_FAILURE);
        }
      } else if (strcmp(argv[i], "--stdout") == 0) {
        i++;
        stdoutPath = [[NSString stringWithUTF8String:argv[i]] expandPath];
        NSLog(@"stdoutPath: %@", stdoutPath);
      } else if (strcmp(argv[i], "--stderr") == 0) {
          i++;
          stderrPath = [[NSString stringWithUTF8String:argv[i]] expandPath];
          NSLog(@"stderrPath: %@", stderrPath);
      } else if (strcmp(argv[i], "--xctest") == 0) {
          i++;
          xctest = [[NSString stringWithUTF8String:argv[i]] expandPath];
          NSLog(@"xctest: %@", xctest);
      } else if (strcmp(argv[i], "--retina") == 0) {
          [self printDeprecation:argv[i]];
          retinaDevice = YES;
      } else if (strcmp(argv[i], "--tall") == 0) {
          [self printDeprecation:argv[i]];
          tallDevice = YES;
      } else if (strcmp(argv[i], "--64bit") == 0) {
          [self printDeprecation:argv[i]];
          is64BitDevice = YES;
      } else if (strcmp(argv[i], "--args") == 0) {
        i++;
        break;
      } else {
        fprintf(stderr, "unrecognized argument:%s\n", argv[i]);
        [self printUsage];
        exit(EXIT_FAILURE);
      }
    }
    NSMutableArray *args = [NSMutableArray arrayWithCapacity:MAX(argc - i,0)];
    for (; i < argc; i++) {
      [args addObject:[NSString stringWithUTF8String:argv[i]]];
    }

    if (sdkRoot == nil) {
	   [self LoadSimulatorFramework:developerDir];
        Class systemRootClass = [self FindClassByName:@"DTiPhoneSimulatorSystemRoot"];
        sdkRoot = [systemRootClass defaultRoot];
    }
    if (xctest) {
        NSString *appName = [appPath lastPathComponent];
        NSString *executableName = [appName stringByDeletingPathExtension];
        NSString *injectionPath = @"/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/Library/PrivateFrameworks/IDEBundleInjection.framework/IDEBundleInjection";
        [environment setValuesForKeysWithDictionary:@{
                                                      @"DYLD_INSERT_LIBRARIES" : injectionPath,
                                                      @"XCInjectBundle" : xctest,
                                                      @"XCInjectBundleInto" : [appPath stringByAppendingFormat:@"/%@", executableName],
                                                      }];
    }

    /* Don't exit, adds to runloop */
    [self launchApp:appPath
         withFamily:family
               uuid:uuid
        environment:environment
         stdoutPath:stdoutPath
         stderrPath:stderrPath
            timeout:timeout
               args:args];
  } else {
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
      [self printUsage];
      exit(EXIT_SUCCESS);
    } else if (argc == 2 && strcmp(argv[1], "--version") == 0) {
      printf("%s\n", IOS_SIM_VERSION);
      exit(EXIT_SUCCESS);
    } else {
      fprintf(stderr, "Unknown command\n");
      [self printUsage];
      exit(EXIT_FAILURE);
    }
  }
}

@end
