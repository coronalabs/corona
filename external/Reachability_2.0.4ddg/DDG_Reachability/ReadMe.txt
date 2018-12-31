Reachability

========================================================================
DESCRIPTION:

The Reachability sample application demonstrates how to use the System
Configuration framework to monitor the network state of an iPhone or
iPod touch. In particular, it demonstrates how to know when IP can be
routed and when traffic will be routed through a Wireless Wide Area
Network (WWAN) interface such as EDGE or 3G.

Note: Reachability cannot tell your application if you can connect to a
particular host, only that an interface is available that might allow a
connection, and whether that interface is the WWAN.

========================================================================
USING THE SAMPLE

Build and run the sample using Xcode. When running the iPhone Simulator,
you can exercise the application by disconnecting the Ethernet cable,
turning off AirPort, or by joining an ad-hoc local Wi-Fi network.

By default, the application uses www.apple.com for its remote host. You
can change the host it uses in ReachabilityAppDelegate.m by modifying
the call to [Reachability reachabilityWithHostName] in
-applicationDidFinishLaunching.

IMPORTANT: Reachability must use DNS to resolve the host name before it
can determine the Reachability of that host, and this may take time on
certain network connections.  Because of this, the API will return
NotReachable until name resolution has completed.  This delay may be
visible in the interface on some networks.

The Reachability sample demonstrates the asynchronous use of the
SCNetworkReachability API. You can use the API synchronously, but do not
issue a synchronous check by hostName on the main thread. If the device
cannot reach a DNS server or is on a slow network, a synchronous call to
the SCNetworkReachabilityGetFlags function can block for up to 30
seconds trying to resolve the hostName. If this happens on the main
thread, the application watchdog will kill the application after 20
seconds of inactivity.

SCNetworkReachability API's do not currently provide a means to detect
support for GameKit Peer To Peer networking over BlueTooth.

========================================================================
BUILD REQUIREMENTS

iPhone OS 3.0

========================================================================
RUNTIME REQUIREMENTS

iPhone OS 3.0

========================================================================
PACKAGING LIST

Reachability.h Reachability.m -Basic demonstration of how to use the
SystemConfiguration Reachablity APIs.

ReachabilityAppDelegate.h ReachabilityAppDelegate.m -The application's
controller.

========================================================================
CHANGES FROM PREVIOUS VERSIONS

Version 2.0 
-Greatly simplified UI code.
-Rewrote Reachability object to be fully asychronous and simplify
monitoring of multiple SCNetworkReachabilityRefs.
-Added code showing how to monitor wether a connection will be required.

Version 1.5 
- Updated for and tested with iPhone OS 2.0. First public release.

Version 1.4 
- Updated for Beta 7.

Version 1.3 
- Updated for Beta 6. - Added LSRequiresIPhoneOS key to Info.plist.

Version 1.2
- Updated for Beta 4. Added code signing.

Version 1.1 
- Updated for Beta 3 to use a nib file.

Copyright (C)2009 Apple Inc. All rights reserved.