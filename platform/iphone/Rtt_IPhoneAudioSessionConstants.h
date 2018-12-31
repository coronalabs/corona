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

#ifndef _Rtt_IPhoneAudioSessionConstants_H__
#define _Rtt_IPhoneAudioSessionConstants_H__


// Redefined from <objc/runtime.h> so I don't need to expose Obj-C to this header.

#define RTT_OBJCTYPE_UINT     'I'
#define RTT_OBJCTYPE_UINT_ULNG     'L'
#define RTT_OBJCTYPE_UINT_FLT      'f'
#define RTT_OBJCTYPE_UINT_DBL      'd'
#define RTT_OBJCTYPE_UINT_BOOL     'B'

// This is a duplicate which is also defined in Rtt_IPhoneAudioSessionManagerImplementation.
// It sucks to duplicate it, but the implementation is kind of circular if I include this file there. I suppose I could make an entirely new common file, but that sucks too for just one constant.
// I'm inventing a fake enum for AudioSessionSetActive so it can be used like the rest of the audio session properties through the same interfaces.
enum { // typedef UInt32 AudioSessionPropertyID
	kRtt_Fake_AudioSessionProperty_SetActiveMode = 'wxyz'
};


#endif // _Rtt_IPhoneAudioSessionConstants_H__