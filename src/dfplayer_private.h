/*! \copyright 2016-2017 Zorxx Software. All rights reserved.
 *  \file dfplayer_private.h
 */
#ifndef _DFPLAYER_PRIVATE_H
#define _DFPLAYER_PRIVATE_H

#include <stdint.h>
#include "dfplayer.h"

#define DFPLAYER_CMD_NEXT_TRACK          0x01
#define DFPLAYER_CMD_PREVIOUS_TRACK      0x02
#define DFPLAYER_CMD_SET_TRACK           0x03 /* 0-2999 */
#define DFPLAYER_CMD_VOLUME_UP           0x04
#define DFPLAYER_CMD_VOLUME_DOWN         0x05
#define DFPLAYER_CMD_VOLUME_SET          0x06 /* 0-30 */
#define DFPLAYER_CMD_SET_EQUALIZER       0x07
#define DFPLAYER_CMD_SET_PLAYBACK_MODE   0x08
#define DFPLAYER_CMD_SET_PLAYBACK_SOURCE 0x09
#define DFPLAYER_CMD_POWER_MODE_STANDBY  0x0a
#define DFPLAYER_CMD_POWER_MODE_NORMAL   0x0b
#define DFPLAYER_CMD_RESET               0x0c
#define DFPLAYER_CMD_PLAY                0x0d
#define DFPLAYER_CMD_PAUSE               0x0e
#define DFPLAYER_CMD_SET_FOLDER          0x0f /* 1-10 */
#define DFPLAYER_CMD_VOLUME_ADJUST       0x10 /* 0-31 */
#define DFPLAYER_CMD_REPEAT              0x11 /* 1=enable, 0=disable */
#define DFPLAYER_CMD_DEVICE_PUSH_IN      0x3a
#define DFPLAYER_CMD_DEVICE_PULL_OUT     0x3b
#define DFPLAYER_CMD_UDISK_FINISH        0x3c
#define DFPLAYER_CMD_TFCARD_FINISH       0x3d
#define DFPLAYER_CMD_FLASH_FINISH        0x3e
#define DFPLAYER_CMD_INITIALIZE          0x3f
#define DFPLAYER_CMD_ERROR_REPORT        0x40
#define DFPLAYER_CMD_REPLY               0x41
#define DFPLAYER_CMD_QUERY_STATUS        0x42
#define DFPLAYER_CMD_QUERY_VOLUME        0x43
#define DFPLAYER_CMD_QUERY_EQUALIZER     0x44
#define DFPLAYER_CMD_QUERY_PLAYBACK_MODE 0x45
#define DFPLAYER_CMD_QUERY_VERSION       0x46
#define DFPLAYER_CMD_QUERY_TFCARD_FILES  0x47
#define DFPLAYER_CMD_QUERY_UDISK_FILES   0x48
#define DFPLAYER_CMD_QUERY_FLASH_FILES   0x49
#define DFPLAYER_CMD_QUERY_TFCARD_TRACK  0x4B
#define DFPLAYER_CMD_QUERY_UDISK_TRACK   0x4C
#define DFPLAYER_CMD_QUERY_FLASH_TRACK   0x4D

#define DFPLAYER_MSG_START               0x7e
#define DFPLAYER_MSG_END                 0xef
#define DFPLAYER_MSG_VERSION             0xff
#define DFPLAYER_MSG_LENGTH              10   /* bytes */
#define DFPLAYER_MSG_PARAMETER_LENGTH    2    /* bytes */
#define DFPLAYER_MSG_DATA_LENGTH         6    /* bytes */

#define DFPLAYER_VOL_MIN                 0
#define DFPLAYER_VOL_MAX                 30

#define DFPLAYER_FOLDER_MIN              0
#define DFPLAYER_FOLDER_MAX              10

#define DFPLAYER_TRACK_MIN               0
#define DFPLAYER_TRACK_MAX               2999

typedef struct dfplayer_context_s
{
	/* Receive message state information */
	uint8_t message_offset;
	uint16_t expected_checksum;
	uint16_t calculated_checksum;
	uint8_t message_command;
	uint8_t message_feedback;
	uint8_t message_parameter[DFPLAYER_MSG_PARAMETER_LENGTH];

	/* User's message handler functions */
	void *token;
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
} dfplayer_context_t;

#endif /* _DFPLAYER_PRIVATE_H */
