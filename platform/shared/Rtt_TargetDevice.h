//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TargetDevice_H__
#define _Rtt_TargetDevice_H__

// ----------------------------------------------------------------------------

#include "Core/Rtt_Build.h"
#include "Rtt_String.h"

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"

namespace Rtt
{

// ----------------------------------------------------------------------------

// NOTE: The values for these constants correspond to values used in `corona_service.module`
class TargetDevice
{
	public:
		typedef enum _Platform
		{
			kUnknownPlatform = -1,
			kIPhonePlatform = 0, // iOS Store
			kAndroidPlatform = 1, // Android Marketplace
			kOSXPlatform = 2, // Mac App Store
			kKindlePlatform = 3, // Amazon app store (Android forks are considered separate platforms)
			kNookPlatform = 4, // Nook store (Android forks are considered separate platforms)
			kWebPlatform = 5, // Web (HTML5/WebGL)
			kWin32Platform = 6, // Win32 Windows desktop app
			kWinPhoneSilverlightPlatform = 7, // Windows Phone 8 Silverlight App
			kTVOSPlatform = 8, // tvOS
			kLinuxPlatform = 9, // Linux desktop

			kNumPlatforms
		}
		Platform;

		static const char* StringForPlatform( TargetDevice::Platform platform );
		static TargetDevice::Platform PlatformForString( const char *str );

	static const char *TagForPlatform( TargetDevice::Platform platform );
	static TargetDevice::Platform PlatformForTag( const char *str );

	public:
		typedef enum _Version
		{
			kUnknownVersion =      -1,
			kIPhoneOS2_2_1 =	20200,
			kIPhoneOS3_0 =		30000,
			kIPhoneOS3_2 =      30200,
			kIPhoneOS4_0 =      40000,
			kIPhoneOS4_2 =      40200,
			kIPhoneOS4_3 =      40300,
			kIPhoneOS5_0 =      50000,
			kIPhoneOS5_1 =      51000,
			kIPhoneOS6_0 =      60000,
			kIPhoneOS6_1 =      61000,
			kIPhoneOS7_0 =      70000,
			kIPhoneOS7_1 =      71000,
			kIPhoneOS7_2 =      72000,
			kIPhoneOS8_0 =      80000,
			kIPhoneOS8_1 =      81000,
			kIPhoneOS8_2 =      82000,
			kIPhoneOS8_3 =      83000,
			kIPhoneOS8_4 =      84000,
			kIPhoneOS9_0 =      90000,
			kAndroidOS1_5   =  115000,
			kAndroidOS1_6   =  116000,
			kAndroidOS2_0   =  120000,
			kAndroidOS2_0_1 =  120100,
			kAndroidOS2_1	=  121000,
			kAndroidOS2_2	=  122000,
			kAndroidOS2_3_3	=  123300,
			kAndroidOS4_0_3 =  140300,
			kWeb1_0         =   10000,
			kLinux         =   160000,
		} Version;

		static Version VersionForPlatform( Platform platform ); // Return current stable version

	public:
		enum _IPhoneDevice
		{
			kIPhone = 0x0,
			kIPad = 0x1,
			kAppleTV = 0x2,
			kIOSUniversal = 0x7F,

			// This is the delta between the XCodeSimulator target and the actual device target
			kXCodeSimulator = 0x80,

			// iOS Simulator
			kIPhoneXCodeSimulator = kIPhone + kXCodeSimulator,
			kIPadXCodeSimulator = kIPad + kXCodeSimulator,
			kIOSUniversalXCodeSimulator = kIOSUniversal + kXCodeSimulator,

			// tvOS Simulator
			kTVOSXCodeSimulator = kAppleTV + kXCodeSimulator,
		};

	public:
		enum _AndroidDevice
		{
			kAndroidGenericDevice = 0x0
		};

	public:
		enum _WebDevice
		{
			kWebGenericBrowser = 0x0
		};

	public:
        class SkinSpec
        {
            Rtt::String fLabel;
            Rtt::String fName;
            Rtt::String fPath;
            Rtt::String fDeviceType;
            int fWidth;
            int fHeight;
            
        public:
#ifdef Rtt_AUTHORING_SIMULATOR
	// This works around some ugly interdependencies with app_sign
            SkinSpec(const char *name, const char *path, const char *type, int width, int height) {
                fName.Set(name);
                fPath.Set(path);
                fDeviceType.Set(type);
                fWidth = width;
                fHeight = height;

                fLabel.Set(GenerateLabel(fPath.GetString()));
            }
#else
            SkinSpec() : fLabel(NULL, NULL), fName(NULL, NULL), fPath(NULL, NULL), fDeviceType(NULL, NULL), fWidth(0), fHeight(0) {}
#endif // Rtt_AUTHORING_SIMULATOR

            const char *GenerateLabel(const char *path);
            
            void SetLabel(const char *path) { fLabel.Set(path); }
            const char *GetLabel() { return fLabel.GetString(); }
            void SetName(const char *name) { fName.Set(name); }
            const char *GetName() { return fName.GetString(); }
            void SetPath(const char *path) { fName.Set(path); }
            const char *GetPath() { return fPath.GetString(); }
            void SetDeviceType(const char *type) { fDeviceType.Set(type); }
            const char *GetDeviceType() { return fDeviceType.GetString(); }
            void SetWidth(int width) { fWidth = width; }
            int GetWidth() { return fWidth; }
            void SetHeight(int height) { fHeight = height; }
            int GetHeight() { return fHeight; }
        };
    
        typedef enum
        {
            kUnknownSkin = -1,
            kDefaultSkin = -2,
            kCustomSkin = -3,
            kOutOfRangeSkin = 100
        } Skin;
        
        static SkinSpec **fSkins;
        static int fSkinCount;
        static Skin fDefaultSkinID;

        static bool Initialize( char **skinFiles, const int skinFileCount );
		static void ReleaseAllSkins();
        static const char* LuaObjectFileFromSkin( int skinID );
        static const char* NameForSkin( int skinID );
        static const int WidthForSkin( int skinID );
        static const int HeightForSkin( int skinID );
        static Skin SkinForLabel( const char* skinname );
        static const char *LabelForSkin( int skinID );
        static const char *DeviceTypeForSkin( int skinID );
        static Platform PlatformForDeviceType( const char *typeName );
        static bool IsSkinForPlatform( int skinID, Platform platform );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TargetDevice_H__
