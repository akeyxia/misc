/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "AudioPolicyManagerALSA"
//#define LOG_NDEBUG 0
#include <utils/Log.h>
#include "AudioPolicyManagerALSA.h"
#include <media/mediarecorder.h>

#define TESTED_VOLUME_LEVELS

namespace android {

status_t AudioPolicyManagerALSA::stopInput(audio_io_handle_t input)
{
    LOGV("stopInput() input %d", input);
    ssize_t index = mInputs.indexOfKey(input);
    if (index < 0) {
        LOGW("stopInput() unknow input %d", input);
        return BAD_VALUE;
    }
    AudioInputDescriptor *inputDesc = mInputs.valueAt(index);

    if (inputDesc->mRefCount == 0) {
        LOGW("stopInput() input %d already stopped", input);
        return INVALID_OPERATION;
    } else {
        AudioParameter param = AudioParameter();
      //  param.addInt(String8(AudioParameter::keyRouting), 0);
         param.addInt(String8(AudioParameter::keyRouting), inputDesc->mDevice); //set the device name
        mpClientInterface->setParameters(input, param.toString());
        inputDesc->mRefCount = 0;
        return NO_ERROR;
    }
}

// ----------------------------------------------------------------------------
// Pateo Specifics
// ----------------------------------------------------------------------------

void AudioPolicyManagerALSA::setForceUse(AudioSystem::force_use usage, AudioSystem::forced_config config) {
    // ----------------------------------------------------------------------------
    // zq: same as Super Class procedure
    // ----------------------------------------------------------------------------
    LOGI("setForceUse() usage %d, config %d, mPhoneState %d", usage, config, mPhoneState);

    bool forceVolumeReeval = false;
    switch(usage) {
        case AudioSystem::FOR_COMMUNICATION:
            if (config != AudioSystem::FORCE_SPEAKER && config != AudioSystem::FORCE_BT_SCO &&
                    config != AudioSystem::FORCE_NONE) {
                LOGW("setForceUse() invalid config %d for FOR_COMMUNICATION", config);
                return;
            }
            mForceUse[usage] = config;
            break;
        case AudioSystem::FOR_MEDIA:
            if (config != AudioSystem::FORCE_HEADPHONES && config != AudioSystem::FORCE_BT_A2DP &&
                    config != AudioSystem::FORCE_WIRED_ACCESSORY && config != AudioSystem::FORCE_NONE) {
                LOGW("setForceUse() invalid config %d for FOR_MEDIA", config);
                return;
            }
            mForceUse[usage] = config;
            break;
        case AudioSystem::FOR_RECORD:
            if (config != AudioSystem::FORCE_BT_SCO && config != AudioSystem::FORCE_WIRED_ACCESSORY &&
                    config != AudioSystem::FORCE_NONE) {
                LOGW("setForceUse() invalid config %d for FOR_RECORD", config);
                return;
            }
            mForceUse[usage] = config;
            break;
        case AudioSystem::FOR_DOCK:
            if (config != AudioSystem::FORCE_NONE && config != AudioSystem::FORCE_BT_CAR_DOCK &&
                    config != AudioSystem::FORCE_BT_DESK_DOCK && config != AudioSystem::FORCE_WIRED_ACCESSORY) {
                LOGW("setForceUse() invalid config %d for FOR_DOCK", config);
            }
            forceVolumeReeval = true;
            mForceUse[usage] = config;
            break;
        default:
            LOGW("setForceUse() invalid usage %d", usage);
            break;
    }

    // ----------------------------------------------------------------------------
    // zq: Specific process
    // ----------------------------------------------------------------------------

    // AudioHardware change route
    AudioParameter param = AudioParameter();
    param.addInt(String8("forceuse_usage"), (int)usage);
    param.addInt(String8("forceuse_config"), (int)config);
    mpClientInterface->setParameters(mHardwareOutput, param.toString());
}

// Override AudioPolicyManagerBase::computeVolume
// Convert 0~32 index to 0~100 volume with our tested volumes
float AudioPolicyManagerALSA::computeVolume(int stream, int index, audio_io_handle_t output, uint32_t device)
{
    // ----------------------------------------------------------------------------
    // zq: same as Super Class procedure
    // ----------------------------------------------------------------------------
    float volume = 1.0;
    AudioOutputDescriptor *outputDesc = mOutputs.valueFor(output);
    StreamDescriptor &streamDesc = mStreams[stream];

    if (device == 0) {
        device = outputDesc->device();
    }

    // ----------------------------------------------------------------------------
    // zq: Specific process
    // ----------------------------------------------------------------------------
    // TODO: For AUX test
    if (stream == AudioSystem::AUX) {
        volume = AudioSystem::linearToLog(index);
        LOGI("AudioPolicyManagerALSA::computeVolume() : stream %d, index %d (%d ~ %d), Linear volume %d, Log volume %f", 
                stream, index, streamDesc.mIndexMin, streamDesc.mIndexMax, index, volume);
        return volume;
    }

#ifdef TESTED_VOLUME_LEVELS
    // Convert 0~32 index to the tested volume in 0~100
    int volInt = mStreamVolumeLevels[stream][index];
    if (volInt == -1) {
        LOGE("AudioPolicyManagerALSA::computeVolume() : wrong stream type %d", stream);
        volInt = (AudioSystem::VOLUME_LEVEL_MAX * (index - streamDesc.mIndexMin)) / (streamDesc.mIndexMax - streamDesc.mIndexMin);
    }
    if (stream == AudioSystem::FM ||
        stream == AudioSystem::AM ||
        stream == AudioSystem::AUX ||
        stream == AudioSystem::VOICE_CALL) {
        volInt = (int)((float)((AudioSystem::VOLUME_LEVEL_MAX * volInt) )/ 72.0 + 0.5);
    }
#else
    int volInt = (AudioSystem::VOLUME_LEVEL_MAX * (index - streamDesc.mIndexMin)) / (streamDesc.mIndexMax - streamDesc.mIndexMin);
#endif
    volume = AudioSystem::linearToLog(volInt);
    LOGI("AudioPolicyManagerALSA::computeVolume() : stream %d, index %d (%d ~ %d), Linear volume %d, Log volume %f", 
            stream, index, streamDesc.mIndexMin, streamDesc.mIndexMax, volInt, volume);

    return volume;
}


// ----------------------------------------------------------------------------
// AudioPolicyManagerALSA
// ----------------------------------------------------------------------------

// ---  class factory

extern "C" AudioPolicyInterface* createAudioPolicyManager(AudioPolicyClientInterface *clientInterface)
{
    return new AudioPolicyManagerALSA(clientInterface);
}

extern "C" void destroyAudioPolicyManager(AudioPolicyInterface *interface)
{
    delete interface;
}

// Nothing currently different between the Base implementation.

AudioPolicyManagerALSA::AudioPolicyManagerALSA(AudioPolicyClientInterface *clientInterface)
    : AudioPolicyManagerBase(clientInterface)
{
    // ----------------------------------------------------------------------------
    // zq: Specific process
    // ----------------------------------------------------------------------------
    for (int i = 0; i < AudioSystem::NUM_STREAM_TYPES; i++) {
        if (i == AudioSystem::MUSIC ||
                i == AudioSystem::BLUETOOTH_MUSIC
           ) {
            mStreamVolumeLevels[i] = {0, 10,    66,  76,   90,  99, 106, 113,   118, 123, 128, 133,  138, 143,  148, 153,  158, 161,  163,    166,    168, 171,    173,   175, 
                                     178, 181,  183,   186,  188, 191,   193, 196, 200};
            //mStreamVolumeLevels[i] = new int[]{0, 73, 79, 85, 91, 97, 100, 100, 100};
        } else if (i == AudioSystem::NOTIFICATION) {
            mStreamVolumeLevels[i] = {0, 10,    66,  76,   90,  99, 106, 113,   118, 123, 128, 133,  138, 143,  148, 153,  158, 161,  163,    166,    168, 171,    173,   175, 
                                     178, 181,  183,   186,  188, 191,   193, 196, 200};
            //mStreamVolumeLevels[i] = new int[]{0, 59, 65, 71, 77, 83, 89, 89, 89};
        } else if (i == AudioSystem::TTS) {
            mStreamVolumeLevels[i] = {0, 10,    66,  76,   90,  99, 106, 113,   118, 123, 128, 133,  138, 143,  148, 153,  158, 161,  163,    166,    168, 171,    173,   175, 
                                     178, 181,  183,   186,  188, 191,   193, 196, 200};
            //mStreamVolumeLevels[i] = new int[]{0, 59, 65, 71, 77, 83, 89, 89, 89};
        } else if (i == AudioSystem::AUX) {
 //           mStreamVolumeLevels[i] = {0, 1,    7,  16,   24,  28, 33,   36,   39,  41,    44,   46,   49,  51,    54,   56,   59,   60,     61,    63,    64,   65,     66,   68,
 //                                            69, 70,  71,      72,   74,   75,   76,   77, 80};

 mStreamVolumeLevels[i] = {0,3,9,18,26,30,35,38,41,43,46,48,51,53,56,58,61,62,63,65,66,67,68,70,71,72,73,74,76,77,78,79,82};

        } else if (i == AudioSystem::FM) {
            mStreamVolumeLevels[i] = {0, 1,    4,  11,   18,  22, 27,   30,   33,  35,    38,   40,   43,  45,    48,   50,   53,   54,     55,    57,    58,   59,     60,   62,
                                             63, 64,  65,      66,   68,   69,   70,   71, 74};
        } else if (i == AudioSystem::AM) {
            //mStreamVolumeLevels[i] = {0, 1,    4,  11,   18,  22, 27,   30,   33,  35,    38,   40,   43,  45,    48,   50,   53,   54,     55,    57,    58,   59,     60,   62,
                                             //63, 64,  65,      66,   68,   69,   70,   71, 74};
mStreamVolumeLevels[i] = {0, 2,    5,  12,   19,  23, 28,   31,   34,  36,    39,   41,   				44,  46,    49,   51,   54,   55,     56,    58,    59,   60,     				61,   63,64, 65,  66,      67,   69,   70,   71,   72, 75};
        } else if (i == AudioSystem::NAV ||
                   i == AudioSystem::NAV_NOVICE) {
           /* int arraySize = sizeof(mStreamVolumeLevels[i]) / sizeof(mStreamVolumeLevels[i][0]);
            for (int j = 0; j < arraySize; j++) {
                mStreamVolumeLevels[i][j] = -1;
            }*/
            mStreamVolumeLevels[i] = {0, 10,    66,  76,   90,  99, 106, 113,   118, 123, 128, 133,  138, 143,  148, 153,  158, 161,  163,    166,    168, 171,    173,   175,
                                     178, 181,  183,   186,  188, 191,   193, 196, 200};
            //mStreamVolumeLevels[i] = new int[]{0, 59, 65, 71, 77, 83, 89, 89, 89};
        } else if (i == AudioSystem::IVOKA) {
            mStreamVolumeLevels[i] = {0, 10,    66,  76,   90,  99, 106, 113,   118, 123, 128, 133,  138, 143,  148, 153,  158, 161,  163,    166,    168, 171,    173,   175, 
                                     178, 181,  183,   186,  188, 191,   193, 196, 200};
            //mStreamVolumeLevels[i] = new int[]{0, 59, 65, 71, 77, 83, 89, 89, 89};
        } else if (i == AudioSystem::WALKIETALKIE) {
            mStreamVolumeLevels[i] = {0, 10,    66,  76,   90,  99, 106, 113,   118, 123, 128, 133,  138, 143,  148, 153,  158, 161,  163,    166,    168, 171,    173,   175, 
                                     178, 181,  183,   186,  188, 191,   193, 196, 200};
            //mStreamVolumeLevels[i] = new int[]{0, 73, 79, 85, 91, 97, 100, 100, 100};
        } else if (i == AudioSystem::VOICE_CALL) {
            mStreamVolumeLevels[i] = {0, 10,    13,  22,   29,  32, 36,   40,   42,  45,    47,   50,   52,  55,    57,   60,   62,   63,     65,    66,    67,   68,     69,   71,
                                             72, 73,  74,      76,   77,   78,   79,   80, 81};
            //mStreamVolumeLevels[i] = new int[]{0, 79, 81, 83, 85, 87, 91, 91, 91};
        } else if (i == AudioSystem::RING) {
            mStreamVolumeLevels[i] = {0, 10,    66,  76,   90,  99, 106, 113,   118, 123, 128, 133,  138, 143,  148, 153,  158, 161,  163,    166,    168, 171,    173,   175, 
                                     178, 181,  183,   186,  188, 191,   193, 196, 200};
            //mStreamVolumeLevels[i] = new int[]{0, 59, 65, 71, 77, 83, 89, 89, 89};
        } else {
            mStreamVolumeLevels[i] = {0, 10,    66,  76,   90,  99, 106, 113,   118, 123, 128, 133,  138, 143,  148, 153,  158, 161,  163,    166,    168, 171,    173,   175, 
                                     178, 181,  183,   186,  188, 191,   193, 196, 200};
        }
    }
}

AudioPolicyManagerALSA::~AudioPolicyManagerALSA()
{
}

}; // namespace android
