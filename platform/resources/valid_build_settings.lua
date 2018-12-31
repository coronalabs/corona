------------------------------------------------------------------------------
--
-- Copyright (C) 2018 Corona Labs Inc.
-- Contact: support@coronalabs.com
--
-- This file is part of the Corona game engine.
--
-- Commercial License Usage
-- Licensees holding valid commercial Corona licenses may use this file in
-- accordance with the commercial license agreement between you and 
-- Corona Labs Inc. For licensing terms and conditions please contact
-- support@coronalabs.com or visit https://coronalabs.com/com-license
--
-- GNU General Public License Usage
-- Alternatively, this file may be used under the terms of the GNU General
-- Public license version 3. The license is as published by the Free Software
-- Foundation and appearing in the file LICENSE.GPL3 included in the packaging
-- of this file. Please review the following information to ensure the GNU 
-- General Public License requirements will
-- be met: https://www.gnu.org/licenses/gpl-3.0.html
--
-- For overview and more information on licensing please refer to README.md
--
------------------------------------------------------------------------------

-- These are items we recognize but cannot validate the contents of
-- (generally because they're arbitrary).  Be sure to add items to
-- the settings table as well.
stoplist = {
	"plugins",
	"usesFeatures",
	"CFBundleURLTypes",
	"titleText",
	"intentFilters",
	"NSExceptionDomains",
	"UIRequiredDeviceCapabilities",
	"iCloud",
	"onDemandResources",
	"strings",
	"coronaActivityFlags",
}

settings = 
{
	android =
	{
		usesFeatures = { },
		intentFilters = { },
		usesPermissions = { "" },
		versionName = "",  -- Can be a string or number. String is preferred.
		installLocation = "",

		supportsTV = true,
		isGame = true,
		versionCode = "",
		usesExpansionFile = true,
		supportsTV = true,
		isGame = true,
		allowAppsReadOnlyAccessToFiles = false,
		minSdkVersion = "",
		largeHeap = true,
		googlePlayGamesAppId = "",
		mainIntentFilter =
		{
			categories =
			{
				"",
			},
		},
		permissions =
		{
			{
				name = "",
				description = "",
				icon = "",
				label = "",
				permissionGroup="",
				protectionLevel=""
			},
		},
		supportsScreens =
		{
			smallScreens  = true,
			normalScreens = true,
			largeScreens  = true,
			xlargeScreens = false,
			resizeable = true,   -- This is not a typo. Google expects an 'e' in "resizeable".
			anyDensity = true,
			requiresSmallestWidthDp = 1,
			compatibleWidthLimitDp = 1,
			largestWidthLimitDp = 1,
		},
		CoronaWindowMovesWhenKeyboardAppears = false,  -- Both camel case and pascal case are accepted.
		coronaWindowMovesWhenKeyboardAppears = false,
		facebookAppId = "",  -- Both camel case and pascal case are accepted.
		FacebookAppID = "",

		applicationChildElements = { "" },
		manifestChildElements = { "" },
		apkFiles = { "" },
		strings = { },
		useGoogleServicesJson = true,
		coronaActivityFlags = { },
	},

	androidPermissions = { "DEPRECATED" },

	build = {
		custom = "",
		neverStripDebugInfo = true,
	},

	corona_sdk_simulator_path = "", -- used by Corona Editor

	excludeFiles =
	{
		iphone = { "", },
		ios = { "", },
		android = { "", },
		osx = { "", },
		macos = { "", },
		win32 = { "", },
		tvos = { "", },
		web = { "", },
		all = { "" },
	},

	iphone =
	{
        iCloud = true,
		skipPNGCrush = true,
		xcassets = "",

		plist =  -- most of these keys are probably valid for tvos too.
		{
			CFBundleId = "",
			CFBundleIconFile = "DEPRECATED",
			CFBundleIconFiles =
			{
				"", 
			},
			CFBundleURLTypes =
			{
				CFBundleURLSchemes = {
					"", 
				}
			},
			UISupportedInterfaceOrientations = { "CORONA" },
			UIApplicationExitsOnSuspend = false,
			UIStatusBarHidden = true,
			UIPrerenderedIcon = true,
			LSApplicationQueriesSchemes = { "" },
			FacebookAppID = "",
			CoronaUseIOS6PortraitOnlyWorkaround = true,
			CFBundleIdentifier = "",
			CFBundleShortVersionString = "",
			CFBundleDisplayName = "",
            CFBundleName = "",
			CFBundleVersion = "",
            UILaunchImages = {
                {
                    ["UILaunchImageMinimumOSVersion"] = "",
                    ["UILaunchImageName"] = "",
                    ["UILaunchImageOrientation"] = "",
                    ["UILaunchImageSize"] = "",
                },
			},
            UILaunchStoryboardName = "", -- optional replacement for UILaunchImages
            UIPrerenderedIcon = true,
            MinimumOSVersion = "",
			UIRequiredDeviceCapabilities = { },
            UIAppFonts = { "DEPRECATED", },
            UIInterfaceOrientation = "",
            UISupportedInterfaceOrientation = 
            {
            	"UIInterfaceOrientationLandscapeLeft", -- etc.
            },
			UIViewControllerBasedStatusBarAppearance = false,
            CoronaUseIOS6LandscapeOnlyWorkaround = true,
            CoronaUseIOS7LandscapeOnlyWorkaround = true,
            CoronaUseIOS8LandscapeOnlyWorkaround = true,
            CoronaUseIOS7IPadPhotoPickerLandscapeOnlyWorkaround = true,
            CoronaUseIOS6IPadPhotoPickerLandscapeOnlyWorkaround = true,
			UIBackgroundModes = {'remote-notification'},
			CoronaWindowMovesWhenKeyboardAppears = false,
			CFBundleLocalizations = {
				"en",
			},
			NSAppTransportSecurity = {
				NSAllowsArbitraryLoads = false,
			    NSAllowsArbitraryLoadsInMedia = false,
			    NSAllowsArbitraryLoadsInWebContent = false,
			    NSAllowsLocalNetworking = false,
			    NSExceptionDomains = {},
			},
			NSAppleMusicUsageDescription = "",
			NSBluetoothPeripheralUsageDescription = "",
			NSCalendarsUsageDescription = "",
			NSCameraUsageDescription = "",
			NSContactsUsageDescription = "",
			NSHealthShareUsageDescription = "",
			NSHealthUpdateUsageDescription = "",
			NSHomeKitUsageDescription = "",
			NSLocationAlwaysUsageDescription = "",
			NSLocationUsageDescription = "DEPRECATED",
			NSLocationAlwaysAndWhenInUseUsageDescription = "",
			NSLocationWhenInUseUsageDescription = "",
			NSMicrophoneUsageDescription = "",
			NSMotionUsageDescription = "",
			NSPhotoLibraryUsageDescription = "",
			NSPhotoLibraryAddUsageDescription = "",
			NSRemindersUsageDescription = "",
			NSSiriUsageDescription = "",
			NSSpeechRecognitionUsageDescription = "",
			ITSAppUsesNonExemptEncryption = false,
			GooglePlayGamesOAuth2ClientId = "",
		},
	},

	orientation =
	{
		default = "",
		supported = { "landscapeLeft", "landscapeRight" },
	},

	osx = 
	{
		iCloud = {},
		bundleResourcesDirectory = "",
		entitlements = 
		{
			[""] = true,
		},
		plist = 
		{
			CFBundleURLTypes = {
				{
					CFBundleURLName = "",
					CFBundleURLSchemes = 
					{
						"",
					}
				}
			},
			CFBundleDocumentTypes = 
			{
				{
                    CFBundleTypeExtensions =
                    {
                        "png",
                    },
					CFBundleTypeIconFile = "",
					CFBundleTypeName = "",
                    CFBundleTypeRole = "",
					LSHandlerRank = "",
					LSItemContentTypes = 
					{
						"",
					}
				}
			},
			NSHumanReadableCopyright = "",
			NSContactsUsageDescription = "",
			NSLocationUsageDescription = "",
			NSLocationWhenInUseUsageDescription = "",
			NSAppTransportSecurity = {
				NSAllowsArbitraryLoads = true,
			    NSAllowsArbitraryLoadsInMedia = true,
			    NSAllowsArbitraryLoadsInWebContent = true,
			    NSAllowsLocalNetworking = true,
			    NSExceptionDomains = {},
			},
			ITSAppUsesNonExemptEncryption = false,
			LSMinimumSystemVersion = "",
		}
	},

	buildQueue = "",

	splashScreen =
	{
		enable = true,
		image = "",

		ios = {
			enable = true,
			image = "",
		},
		android = {
			enable = true,
			image = "",
		},
	},

	plugins = 
	{
	},

	tvos =
	{
		iCloud = true,
		xcassets = "",
		onDemandResources = {
			{ },
		},
		-- tvOS app icons require multiple layers, and must provide both a small and a large size.
		icon =
		{
			-- A collection of 400x240 pngs, in order of top to bottom.
			small =
			{
				"",
			},
			-- A collection of 1280x768 pngs, in order of top to bottom.
			large =
			{
				"",
			}
		},

		topShelfImage = "",
		topShelfImageWide = "",
		launchImage = "",

		plist = {
			UIApplicationExitsOnSuspend = true,
			NSVideoSubscriberAccountUsageDescription = "",
			ITSAppUsesNonExemptEncryption = false,
		},
	},

	win32 = {
		preferenceStorage = "",
		singleInstance = true,
	},

	window =
	{
		defaultMode = "",
		enableMinimizeButton = true,
		enableCloseButton = true,
		minViewWidth = 1,
		minViewHeight = 1,
		resizable = true,
		enableMaximizeButton = true,
		suspendWhenMinimized = true,
		defaultViewWidth = 1,
		defaultViewHeight = 1,
		showWindowTitle = false,
		titleText =
		{
			default = "",
		},
	},
}

-- forwards compatibility
settings.ios = settings.iphone
settings.macos = settings.osx
