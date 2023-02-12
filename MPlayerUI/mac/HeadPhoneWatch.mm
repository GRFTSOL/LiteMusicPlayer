//
//  HeadPhoneWatch.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/2/11.
//

#import <CoreAudio/CoreAudio.h>
#import <AVFoundation/AVFoundation.h>
#import "Player.h"
#import "HeadPhoneWatch.hpp"
#import "MPlayer/Player.h"
#import "Skin/Skin.h"

uint32_t getDefaultOutputDeviceID() {
    AudioDeviceID defaultDevice = 0;
    UInt32 defaultSize = sizeof(AudioDeviceID);

    const AudioObjectPropertyAddress defaultAddr = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    AudioObjectGetPropertyData(kAudioObjectSystemObject, &defaultAddr, 0, NULL, &defaultSize, &defaultDevice);

    return defaultDevice;
}

void currentHeadPhoneStatus() {
    UInt32 defaultDevice = getDefaultOutputDeviceID();

    AudioObjectPropertyAddress sourceAddr;
    sourceAddr.mSelector = kAudioDevicePropertyDataSource;
    sourceAddr.mScope = kAudioDevicePropertyScopeOutput;
    sourceAddr.mElement = kAudioObjectPropertyElementMaster;

    UInt32 dataSourceId = 0;
    UInt32 dataSourceIdSize = sizeof(UInt32);
    AudioObjectGetPropertyData(defaultDevice, &sourceAddr, 0, NULL, &dataSourceIdSize, &dataSourceId);

    if (dataSourceId == 'ispk') {
        DBG_LOG0("Current output is Speaker");
        if (g_profile.getBool("PauseOnHeadPhoneOff", true)) {
            g_player.pause();
        }
    } else if (dataSourceId == 'hdpn') {
        // Recognized as headphones
        DBG_LOG0("Current output is HeadPhone");
    }
}

int headPhonePlugCallback(AudioObjectID                       inObjectID,
                           UInt32                              inNumberAddresses,
                           const AudioObjectPropertyAddress*   inAddresses,
                           void* __nullable                    inClientData) {

    currentHeadPhoneStatus();
    return 0;
}


void setupHeadPhonePlugWatch() {
    currentHeadPhoneStatus();

    AudioObjectPropertyAddress sourceAddr;
    sourceAddr.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
    sourceAddr.mScope = kAudioObjectPropertyScopeGlobal;
    sourceAddr.mElement = kAudioObjectPropertyElementMaster;
    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &sourceAddr, headPhonePlugCallback, nullptr);
}
