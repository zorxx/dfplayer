/*! \copyright 2016-2017 Zorxx Software. All rights reserved.
 *  \file dfplayer.h
 */

#ifndef _DFPLAYER_H
#define _DFPLAYER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DFPLAYER_DEVICE_UDISK    0x0001
#define DFPLAYER_DEVICE_TFCARD   0x0002
#define DFPLAYER_DEVICE_PC       0x0004
#define DFPLAYER_DEVICE_FLASH    0x0008

typedef enum
{
	DFPLAYER_ERROR_BUSY                    = 0,
	DFPLAYER_ERROR_FRAME_DATA_NOT_RECEIVED = 1,
	DFPLAYER_ERROR_VERIFICATION_ERROR      = 2
} dfplayerError_e;

typedef enum
{
	DFPLAYER_EQ_NORMAL    = 0,
	DFPLAYER_EQ_POP       = 1,
	DFPLAYER_EQ_ROCK      = 2,
	DFPLAYER_EQ_JAZZ      = 3,
	DFPLAYER_EQ_CLASSICAL = 4,
	DFPLAYER_EQ_BASS      = 5
} dfplayerEqualizer_e;

typedef enum
{
	DFPLAYER_PLAY_MODE_REPEAT        = 0,
	DFPLAYER_PLAY_MODE_FOLDER_REPEAT = 1,
	DFPLAYER_PLAY_MODE_SINGLE_REPEAT = 2,
	DFPLAYER_PLAY_MODE_RANDOM        = 3
} dfplayerPlaybackMode_e;

/* Asynchronous events */
typedef void (*pfn_dfplayer_HandleInitialize)(void *conext, void *token, uint16_t devices_online);
typedef void (*pfn_dfplayer_HandleTrackFinished)(void *context, void *token, uint16_t track_number, uint16_t device);
typedef void (*pfn_dfplayer_HandleDeviceState)(void *context, void *token, uint16_t device, bool inserted);
typedef void (*pfn_dfplayer_HandleError)(void *context, void *token, dfplayerError_e error);
typedef void (*pfn_dfplayer_HandleReply)(void *context, void *token);

typedef void (*pfn_dfplayer_HandleStatusResponse)(void *context, void *token, bool playing);
typedef void (*pfn_dfplayer_HandleVolumeResponse)(void *context, void *token, uint8_t volume);
typedef void (*pfn_dfplayer_HandleEqualizerResponse)(void *context, void *token, dfplayerEqualizer_e mode);
typedef void (*pfn_dfplayer_HandlePlaybackModeResponse)(void *context, void *token, dfplayerPlaybackMode_e mode);
typedef void (*pfn_dfplayer_HandleFileCountResponse)(void *context, void *token, uint16_t device, uint16_t file_count);
typedef void (*pfn_dfplayer_HandleCurrentTrackResponse)(void *context, void *token, uint16_t device, uint16_t track);

typedef int (*pfn_dfplayer_SendSerial)(void *context, void *token, uint8_t *data, uint32_t bytes);

typedef struct dfplayer_init_info_s
{
	pfn_dfplayer_HandleInitialize pfnHandleInitialize;
	pfn_dfplayer_HandleTrackFinished pfnHandleTrackFinished;
	pfn_dfplayer_HandleDeviceState pfnHandleDeviceState;
	pfn_dfplayer_HandleError pfnHandleError;
	pfn_dfplayer_HandleReply pfnHandleReply;
	pfn_dfplayer_SendSerial pfnSendSerial;
	pfn_dfplayer_HandleStatusResponse pfnHandleStatusResponse;
	pfn_dfplayer_HandleVolumeResponse pfnHandleVolumeResponse;
	pfn_dfplayer_HandleEqualizerResponse pfnHandleEqualizerResponse;
	pfn_dfplayer_HandlePlaybackModeResponse pfnHandlePlaybackModeResponse;
	pfn_dfplayer_HandleFileCountResponse pfnHandleFileCountResponse;
	pfn_dfplayer_HandleCurrentTrackResponse pfnHandleCurrentTrackResponse;
} dfplayer_init_info_t;

void *dfplayer_Initialize(void *token, dfplayer_init_info_t *init_info);

void dfplayer_HandleSerialChar(void *context, uint8_t c);

int dfplayer_Play(void *context);
int dfplayer_Pause(void *context);
int dfplayer_NextTrack(void *context);
int dfplayer_PreviousTrack(void *context);
int dfplayer_SetTrack(void *context, uint16_t track_number);
int dfplayer_SetPlaybackMode(void *context, dfplayerPlaybackMode_e mode);
int dfplayer_SetPlaybackSource(void *context, uint16_t device);
int dfplayer_SetFolder(void *context, uint8_t folder_number);
int dfplayer_EnableRepeatPlayback(void *context, bool enable);

int dfplayer_SetEqualizer(void *context, dfplayerEqualizer_e mode);
int dfplayer_VolumeUp(void *context);
int dfplayer_VolumeDown(void *context);
int dfplayer_VolumeSet(void *context, uint8_t volume);

int dfplayer_SetStandbyMode(void *context, bool enable);
int dfplayer_Reset(void *context);

/* The following functions cause response handler functions to be called */
int dfplayer_QueryStatus(void *context);
int dfplayer_QueryVolume(void *context);
int dfplayer_QueryEqualizer(void *context);
int dfplayer_QueryPlaybackMode(void *context);
int dfplayer_QueryFileCount(void *context, uint8_t device); 
int dfplayer_QueryCurrentTrack(void *context, uint8_t device); 

#ifdef __cplusplus
}
#endif

#endif /* _DFPLAYER_H */
